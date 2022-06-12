//  main.cpp
//  NewProject
//


// Std. Includes
#include <string>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>

// GL includes
#include <shader.h>
#include <text.h>
#include <arcball.h>
#include <Model.h>
#include <plane.h>
#include "mycube.h"

// for animation
#include <keyframe.h>

// for text
#include <text.h>

// for sound
#include "fmod.hpp"


FMOD::System *g_System;  // need to create a system
FMOD::Sound *sound1;    // mp3 file will be loaded here
FMOD::Channel *channel;  // only one channel used in this program
FMOD_RESULT result;   // to check for errors
unsigned int version;
int Music_Init();

bool paused = false;  // flag for paused

// function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow *window, double x, double y);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
unsigned int loadTexture(const char *texFileName);

// animation related functions
void initKeyframes();
void updateAnimData();

// void loadTexture();
GLFWwindow *glAllInit();

// functions for rendering
void render();
void render_moldings();
void render_room();
void render_ground();
void render_ceilinglight();
void render_frame();
void render_vase();
void render_window();
void render_door();
void render_table();
void render_armchair();
void render_chair();
void render_egg();
void render_lamp();

// window size
unsigned int SCR_WIDTH = 600;
unsigned int SCR_HEIGHT = 600;

// global variables
GLFWwindow *mainWindow = NULL;
Shader *roomShader, *modelShader, *groundShader, *lightShader, *textShader = NULL;
glm::mat4 projection, view, model;

// for camera
glm::vec3 camPosition_player(0.0f, 10.0f, 28.0f);
glm::vec3 camTarget(0.0f, 10.0f, -10.0f);
glm::vec3 camUp(0.0f, 1.0f, 0.0f);
bool entered = false;

// for lighting
glm::vec3 lightPos(0.0f, 2.0f, 2.1f);

// for arcball
float arcballSpeed = 0.005f;
static Arcball camArcBall(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);

// for room texture
static unsigned int wall, carpet;

// for models
Cube *room;
Model *molding, *door, *armchair, *table, *frame, *vase, *window, *ceilinglight, *chair, *egg, *lamp;
Plane *ground;


// Texts
Text *text = NULL;
Text *status_text = NULL;

// for animation
enum RenderMode { INIT, ANIM, STOP };
RenderMode renderMode;   // current rendering mode
float beginT = 0.0f;   // animation beginning time
float timeT = 0.0f;      // current time (in sec)
float endT = 10.0f;    // animation ending time

float xTrans, yTrans, zTrans;  // current translation factors
float xTrans2, yTrans2, zTrans2;  // current translation factors

KeyFraming xTKF(5), yTKF(5), zTKF(5);  // translation keyframes
KeyFraming xTKF2(3), yTKF2(3), zTKF2(3);  // translation keyframes







int main()
{
    mainWindow = glAllInit();
    
    // shader initialization
    roomShader = new Shader("res/shaders/global.vs", "res/shaders/global.fs");
    modelShader = new Shader("res/shaders/modelLoading.vs", "res/shaders/modelLoading.frag");
    groundShader = new Shader("res/shaders/ground.vs", "res/shaders/ground.fs");
    lightShader = new Shader("res/shaders/lighting.vs", "res/shaders/lighting.fs");
    textShader = new Shader("res/shaders/text.vs", "res/shaders/text.frag");
    
    
    // Load Models
    molding = new Model((GLchar *)"res/models/molding/molding.obj" );
    door = new Model((GLchar *)"res/models/door/door.obj");
    table = new Model((GLchar *)"res/models/table/table.obj");
    armchair = new Model((GLchar *)"res/models/armchair/armchair.obj");
    frame = new Model((GLchar *)"res/models/frame/frame.obj");
    vase = new Model((GLchar *)"res/models/flowervase/flowervase.obj");
    window = new Model((GLchar *)"res/models/window/window.obj");
    ceilinglight = new Model((GLchar *)"res/models/ceilinglight/ceilinglight.obj");
    chair = new Model((GLchar *)"res/models/chair/chair.obj");
    egg = new Model((GLchar *)"res/models/egg/egg.obj");
    lamp = new Model((GLchar *)"res/models/lamp/lamp.obj");
    
    // text
    text = new Text("fonts/PlayfairDisplay-Bold.ttf", textShader, SCR_WIDTH, SCR_HEIGHT);
    
    status_text = new Text("fonts/BebasNeue-Regular.ttf", textShader, SCR_WIDTH, SCR_HEIGHT);
    
    // projection initialization
    projection = glm::perspective(glm::radians(90.0f),
                                  (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                  0.1f, 100.0f);
    
    roomShader->use();
    roomShader->setMat4("projection", projection);
    groundShader->use();
    groundShader->setMat4("projection", projection);
    lightShader->use();
    lightShader->setMat4("projection", projection);
    modelShader->use();
    modelShader->setMat4("projection", projection);
    
    // be sure to activate shader when setting uniforms/drawing objects
    modelShader->setVec3("light.position", lightPos);
    modelShader->setVec3("viewPos", camPosition_player);
    
    // light properties
    modelShader->setVec3("light.ambient", 0.5f, 0.5f, 0.5f);
    modelShader->setVec3("light.diffuse", .6f, .6f, .6f);
    modelShader->setVec3("light.specular", .1f, .1f, .1f);
    
    // material properties
    modelShader->setFloat("shininess", 1.0f);
    
    // ground
    ground = new Plane();
    carpet = loadTexture("carpet.jpeg");
    
    // room
    room = new Cube();
    wall = loadTexture("beige.jpeg");
    
    
    // initialize animation data
    initKeyframes();
    timeT = 0.0f;
    updateAnimData();
    renderMode = INIT;
    
    // start music
    Music_Init();
    
    while (!glfwWindowShouldClose(mainWindow))
    {
        glfwPollEvents();
        
        // render
        render();
    }
    
    result = g_System->release();
    glfwTerminate();
    return 0;
    
}




// window creation
//-------------------------------------------------------------------------------------------
GLFWwindow *glAllInit() {
    GLFWwindow *window;
    
    // glfw: initialize and configure
    if (!glfwInit()) {
        printf("GLFW initialization failed!");
        glfwTerminate();
        exit(-1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );

    //glfw window creation
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "You are Home", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    
    glfwMakeContextCurrent(window);
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    
    // set some OpenGL states
    glEnable(GL_DEPTH_TEST);
    
    // Allow modern extension features
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW initialization failed!" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(-1);
    }
    
    // set OpenGL options
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_FRONT);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return window;
}




unsigned int loadTexture(const char *texFileName) {
    unsigned int texture;
    
    // Loading Texture1
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);   // vertical flip the texture
    unsigned char *image = stbi_load(texFileName, &width, &height, &nrChannels, 0);
    if (!image) {
        printf("texture %s loading error ... \n", texFileName);
    }
    else printf("texture %s loaded\n", texFileName);
    
    GLenum format;
    if (nrChannels == 1) format = GL_RED;
    else if (nrChannels == 3) format = GL_RGB;
    else if (nrChannels == 4) format = GL_RGBA;
    
    glBindTexture( GL_TEXTURE_2D, texture);
    glTexImage2D( GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    return texture;
}




void render() {
    if (renderMode == ANIM) {
        float currentTime = glfwGetTime();
        timeT = currentTime - beginT;
        updateAnimData();
    }
    
    // clear the colorbuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
    view = glm::lookAt(camPosition_player, camTarget, camUp);
    view = view * camArcBall.createRotationMatrix();

    // render room
    render_room();
    
    // draw ground
    render_ground();
    
    // models
    modelShader->use();
    modelShader->setMat4("view", view);
    
    // moldings
    render_moldings();

    // frame
    render_frame();
    
    // window
    render_window();
    
    // vase
    render_vase();
    
    // door
    render_door();
    
    // armchair
    render_armchair();
    
    // table
    render_table();
    
    // egg
    render_egg();
    
    // lamp
    render_lamp();
    
    // chair
    render_chair();
    
    // ceiling light
    render_ceilinglight();

    
    // render text
    glm::vec3 text_color(1.0, 1.0f, 1.0f);
    if (entered) text_color = glm::vec3(0.0f, 0.0f, 0.0f);
    
    if (paused) {
        status_text->RenderText("Paused", 270.0f, 500.0f, 0.5f, text_color);
    } else {
        status_text->RenderText("Now Playing", 250.0f, 500.0f, 0.5f, text_color);

    }
    text->RenderText("Music for a Sushi Restaurant", 100.0f, 90.0f, 0.5f, text_color);
    text->RenderText("Harry Styles", 100.0f, 60.0f, 0.3f, text_color);

    g_System->update();
    
    glfwSwapBuffers(mainWindow);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0,0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

// glfw: keyboard callback
// ---------------------------------------------------------------------------------------------
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        // reset arcball
        // model = glm::mat4(1.0f);
        // view = glm::mat4(1.0f);
        camArcBall.init(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);
        if (entered) {
            camPosition_player = glm::vec3(0.0f, 10.0f, 8.0f);
        }
        else {
            camPosition_player = glm::vec3(0.0f, 10.0f, 28.0f);
        }
    }
    // pause and resume
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        channel->getPaused(&paused);
        if (!paused) {
            channel->setPaused(true);
            paused = true;
        }
        else {
            channel->setPaused(false);
            printf("something wrong\n");
            paused = false;
        }
        
    }
    // enter, exit the house
    else if (key == GLFW_KEY_ENTER &&  action == GLFW_PRESS) {
        if (!entered) {
            entered = true;
            camPosition_player = glm::vec3(0.0f, 10.0f, 8.0f);
            // camera pos change
            // arrow keys to adjust your view
        }
        else {
            entered = false;
            camPosition_player = glm::vec3(0.0f, 10.0f, 28.0f);
        }
    }
    else if(key == GLFW_KEY_A &&  action == GLFW_PRESS) {
        if (renderMode == INIT) {
            renderMode = ANIM;
            beginT = glfwGetTime();
        }
        else if (renderMode == STOP) {
            if (timeT == endT) renderMode = INIT;
            else {
                renderMode = ANIM;
                beginT = glfwGetTime() - timeT;
            }
        } else if (renderMode == ANIM) renderMode = STOP;
        
        if (renderMode == INIT) {
            timeT = 0.0f;
            updateAnimData();
        }
    }
    else if (key == GLFW_KEY_U &&  action == GLFW_PRESS) {
        camUp = -camUp;
    }
    
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        float volume;
        channel->getVolume(&volume);
        if ( volume < 1.0) {
            channel->setVolume(volume + 0.05);
        }
    }
    
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        float volume;
        channel->getVolume(&volume);
        if ( volume > 0.0) {
            channel->setVolume(volume - 0.05);
        }
    }
}



void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (entered) camArcBall.mouseButtonCallback( window, button, action, mods );
}

void cursor_position_callback(GLFWwindow *window, double x, double y) {
    if (entered) camArcBall.cursorCallback( window, x, y );
}
 
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (entered) {
        camPosition_player[2] -= (float)yoffset * 0.01;
        if (camPosition_player[2]  < 1.0f) {camPosition_player[2]  = 1.0f; }
        if (camPosition_player[2]  > 10.0f) {camPosition_player[2]  = 10.0f; }
    }
}



int Music_Init() {
    // create system object
    result = FMOD::System_Create(&g_System);
    if (result != FMOD_OK) return -1;
    
    // check system version
    result = g_System->getVersion(&version);
    if(result != FMOD_OK) return -1;
    else printf("FMOD version%08x\n", version);
    
    // initialize
    result = g_System->init(32, FMOD_INIT_NORMAL, NULL); //use 32 channels
    if (result != FMOD_OK) return -1;
    
    // load sound
    result = g_System->createSound("music2.mp3",FMOD_LOOP_NORMAL, 0, &sound1);
    if (result != FMOD_OK) return -1;
    
    /*
    result = g_System->createSound("music1.mp3", FMOD_LOOP_NORMAL, 0, &sound2);
    if (result != FMOD_OK) return -1;
    */
    
    // play sound
    result = g_System->playSound(sound1, 0, false, &channel);
    if (result != FMOD_OK) return -1;
    
    return 0;
}





void render_room() {
    roomShader->use();
    roomShader->setMat4("view", view);
    roomShader->setVec3("light.position", lightPos);
    roomShader->setVec3("viewPos", camPosition_player);
    roomShader->setVec3("light.ambient", 0.65f, 0.65f, 0.65f);
    roomShader->setVec3("light.diffuse", .35f, .35f, .35f);
    roomShader->setVec3("light.specular", .4f, .4f, .4f);
    roomShader->setVec3("material.specular", glm::vec3(0.1f, 0.1f, 0.1f));
    roomShader->setFloat("material.shininess", 2.0f);
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));
    model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));
    roomShader->setMat4("model", model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, wall);
    room->draw(roomShader);
}

void render_moldings() {
    
    modelShader->use();
    
    // bottom left
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-10.f, 0.5f, -2.1f ));
    model = glm::scale( model, glm::vec3( 0.06f, 0.06f, 0.31f ));
    model = glm::rotate( model,glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
    model = glm::rotate( model,glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f));
    modelShader->setMat4("model", model);
    molding->Draw(modelShader);
    
    // bottom center
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, 0.0f, -9.45f ));
    model = glm::scale( model, glm::vec3(0.31f, 0.06f , 0.06f));
    model = glm::rotate(model,glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f));
    modelShader->setMat4("model", model);
    molding->Draw(modelShader);
    
    // bottom right
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(9.5f, 0.0f, -2.1f ));
    model = glm::scale( model, glm::vec3( 0.06f, 0.06f, 0.31f ));
    model = glm::rotate( model,glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f));
    model = glm::rotate( model,glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f));
    modelShader->setMat4("model", model);
    molding->Draw(modelShader);
    
    // top left
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-9.5f, 20.0f, -2.1f ));
    model = glm::scale( model, glm::vec3( 0.06f, 0.06f, 0.31f ));
    model = glm::rotate( model,glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f));
    modelShader->setMat4("model", model);
    molding->Draw(modelShader);
    
    // top center
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.50f, 20.0f, -9.50f ));
    model = glm::scale( model, glm::vec3(  0.31f, 0.06f , 0.06f));
    model = glm::rotate( model,glm::radians(-180.f), glm::vec3(0.f, 1.f, 0.f));
    modelShader->setMat4("model", model);
    molding->Draw(modelShader);
    
    // top right
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(9.5f, 20.0f, 1.65f ));
    model = glm::scale( model, glm::vec3( 0.06f, 0.06f, 0.31f ));
    model = glm::rotate( model,glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
    modelShader->setMat4("model", model);
    molding->Draw(modelShader);
}

void render_ground() {
    groundShader->use();
    
    groundShader->setMat4("view", view);
    groundShader->setVec3("light.position", lightPos);
    groundShader->setVec3("viewPos", camPosition_player);
    groundShader->setVec3("light.ambient", 0.7f, 0.7f, 0.7f);
    groundShader->setVec3("light.diffuse", 0.4f, 0.4f, 0.4f);
    groundShader->setVec3("light.specular", .3f, .3f, .3f);
    groundShader->setVec3("material.specular", glm::vec3( 0.1f, 0.1f, 0.1f));
    groundShader->setFloat("material.shininess", 1.0f);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 19.98f, 0.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));
    groundShader->setMat4("model", model);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, carpet);
    ground->draw(groundShader);
}

void render_ceilinglight() {
    
    lightShader->use();
    
    lightShader->setMat4("view", view);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.f, 2.0f, 0.f ));
    model = glm::rotate( model,glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    model = glm::scale( model, glm::vec3( .2f, 0.2f, 0.2f ));
    lightShader->setMat4("model", model);
    
    ceilinglight->Draw(lightShader);
}

void render_frame() {
    
    modelShader->use();
    
    modelShader->setMat4("view", view);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-9.5f, 10.0f,-3.7f ));
    model = glm::rotate( model,glm::radians(180.f), glm::vec3(1.0f, 0.0f, 0.0f ));
    model = glm::scale( model, glm::vec3( .5f, 0.5f, 0.5f ));
    modelShader->setMat4("model", model);
    
    frame->Draw(modelShader);
}

void render_vase() {
    
    modelShader->use();
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(xTrans, yTrans, zTrans));
    model = glm::translate(model, glm::vec3(5.f, 19.0f, -2.5f ));
    model = glm::scale( model, glm::vec3( .35f, 0.35f, 0.35f ));
    model = glm::rotate( model,glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
    model = glm::rotate( model,glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    modelShader->setMat4("model", model);
    
    vase->Draw(modelShader);
}

void render_window() {
    
    modelShader->use();
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(9.5f, 8.0f, 2.f ));
    model = glm::rotate( model,glm::radians(180.f), glm::vec3(0.f, 0.0f, 1.f));
    model = glm::rotate( model,glm::radians(180.f), glm::vec3(0.f, 1.0f, 0.f));
    model = glm::rotate( model,glm::radians(270.f), glm::vec3(1.f, 0.f, 0.f));
    model = glm::scale( model, glm::vec3( .2f, 0.2f, 0.2f ));
    modelShader->setMat4("model", model);
    
    window->Draw(modelShader);
}


void render_door() {
    
    modelShader->use();
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-10.6f, 13.2f, 3.f ));
    model = glm::scale( model, glm::vec3(.8f, 0.95f, .7f ));
    model = glm::rotate( model,glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
    model = glm::rotate( model,glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
    modelShader->setMat4("model", model);
    door->Draw(modelShader);
}

void render_table() {
    
    modelShader->use();
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(xTrans, yTrans, zTrans));
    model = glm::translate(model, glm::vec3(-7.0f, 18.5f, -6.2f ));
    model = glm::scale( model, glm::vec3(4.f, 4.0f, 4.0f ));
    model = glm::rotate( model,glm::radians(35.f), glm::vec3(0.f, 1.f, 0.f));
    model = glm::rotate( model,glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    modelShader->setMat4("model", model);
    table->Draw(modelShader);
}


void render_egg() {
    modelShader->use();
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(xTrans, yTrans, zTrans));
    model = glm::translate(model, glm::vec3(-5.8f, 16.22f, -5.0f ));
    model = glm::rotate( model,glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f));
    model = glm::scale( model, glm::vec3( .4f, 0.4f, 0.4f ));
    modelShader->setMat4("model", model);
    egg->Draw(modelShader);
    
}

void render_lamp() {
    modelShader->use();
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(xTrans, yTrans, zTrans));
    model = glm::translate(model, glm::vec3(-6.9f, 16.0f, -5.5f ));
    model = glm::rotate( model,glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f));
    model = glm::scale( model, glm::vec3( 3.5f, 3.5f, 3.5f ));
    modelShader->setMat4("model", model);
    lamp->Draw(modelShader);
}

void render_armchair() {
    
    modelShader->use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(xTrans2, yTrans2, zTrans2));
    model = glm::translate(model, glm::vec3(0.0f, 16.6f, -5.5f ));
    model = glm::rotate( model,glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f));
    model = glm::rotate( model,glm::radians(270.f), glm::vec3(1.f, 0.f, 0.f));
    model = glm::scale( model, glm::vec3( .58f, 0.58f, 0.58f ));
    model = glm::scale( model, glm::vec3( .009f, 0.009f, 0.009f ));
    modelShader->setMat4("model", model);
    armchair->Draw(modelShader);
}

void render_chair() {
    modelShader->use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(xTrans2, yTrans2, zTrans2));
    model = glm::translate(model, glm::vec3(6.8f, 19.0f, -4.5f ));
    model = glm::rotate( model,glm::radians(-165.f), glm::vec3(0.f, 1.f, 0.f));
    model = glm::rotate( model,glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    model = glm::scale( model, glm::vec3( .3f, 0.3f, 0.3f ));
    modelShader->setMat4("model", model);
    chair->Draw(modelShader);
}





void initKeyframes() {
    
    // x-translation keyframes
    xTKF.setKey(0, 0, 0.0);
    xTKF.setKey(1, 2.5, -.5);
    xTKF.setKey(2, 5.0, -1.0);
    xTKF.setKey(3 , 7.5, 0.5);
    xTKF.setKey(4, endT, 0.0);

    // y-translation keyframes
    yTKF.setKey(0, 0, 0.0);
    yTKF.setKey(1, 2.50, -2.0);
    yTKF.setKey(2, 5.0, -4.0);
    yTKF.setKey(3, 7.5, -6.0);
    yTKF.setKey(4, endT, -7.0);
    
    // z-translation keyframes
    zTKF.setKey(0, 0, 0.0);
    zTKF.setKey(1, 2.5, 2.0);
    zTKF.setKey(2, 5.0, 3.0);
    zTKF.setKey(3, 7.5, 4.0);
    zTKF.setKey(4, endT, 3.0);
    
    
    
    // chairs
    
    // x-translation keyframes
    xTKF2.setKey(0, 0, 0.0);
    xTKF2.setKey(1, 5.0, -0.2);
    xTKF2.setKey(2, endT, 0.0);

    // y-translation keyframes
    yTKF2.setKey(0, 0, 0.0);
    yTKF2.setKey(1, 5.0, -3.0);
    yTKF2.setKey(2, endT, -5.0);
    
    // z-translation keyframes
    zTKF2.setKey(0, 0, 0.0);
    zTKF2.setKey(1, 5.0, 1.0);
    zTKF2.setKey(2, endT, 2.0);
    
}



void updateAnimData() {
    if (timeT > endT) {
        renderMode = STOP;
        timeT = endT;
    }
    xTrans = xTKF.getValLinear(timeT);
    yTrans = yTKF.getValLinear(timeT);
    zTrans = zTKF.getValLinear(timeT);
    
    xTrans2 = xTKF2.getValLinear(timeT);
    yTrans2 = yTKF2.getValLinear(timeT);
    zTrans2 = zTKF2.getValLinear(timeT);

}
