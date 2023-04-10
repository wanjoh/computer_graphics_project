#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadCubemap(vector<std::string> faces);

void renderQuad();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0.5f, 0.25f, 0.25f);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;

    // ship consts
    float shipScale = 0.3f;
    glm::vec3 ship1Position = glm::vec3(-1.5, 0.0, -1.0);
    glm::vec3 ship2Position = glm::vec3(1.5, 0.0, -1.0);

    // explosion consts
    glm::vec3 explosionPosition = glm::vec3(0.0, 0.0, 5.0);
    glm::vec3 explosionColor = glm::vec3(1.0, 0.34, 0.08);

    // bullet consts
    float bulletScale = 0.05f;
    int bulletCount = -1;

    bool pause = false;

    // animation parameters
    float counter;
    float animation_speed;
    bool loop;

    // hdr options
    bool hdr = false;
    float exposure = 1.0f;

    // light options
    glm::vec3 dirLightIntensity = glm::vec3(0.2, 0.1, 0.1);

    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 2.0f, 10.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << ImGuiEnabled << '\n'
        << pause << '\n'
        << loop << '\n'
        << hdr;
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in  >> ImGuiEnabled
            >> pause
            >> loop
            >> hdr;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
};

float cubeVertices[] = {
        // back face
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        // front face
        -0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

        // left face
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        // right face
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,

        // bottom face
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,

        // top face
        -0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
};

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "spaceship shootout", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
//    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        programState->CameraMouseMovementUpdateEnabled = false;
    }
//    programState
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;


    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glm::vec3 initialBulletPositions[] = {
            programState->ship1Position,
            programState->ship2Position,
            glm::vec3(-1.0f, 0.0f, 1.0f),
            glm::vec3(-0.5f, 0.0f, 3.0f),
            glm::vec3(0.75f, 0.0f, 2.0f)
    };
    programState->bulletCount = sizeof(initialBulletPositions) / sizeof(glm::vec3);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // bullet VAO
    unsigned int VBO, VAO;
    glGenVertexArrays(2, &VAO);
    glGenBuffers(2, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // load skybox textures
    vector<std::string> faces {
            FileSystem::getPath("resources/textures/skybox/front.png"),
            FileSystem::getPath("resources/textures/skybox/back.png"),
            FileSystem::getPath("resources/textures/skybox/top.png"),
            FileSystem::getPath("resources/textures/skybox/bottom.png"),
            FileSystem::getPath("resources/textures/skybox/right.png"),
            FileSystem::getPath("resources/textures/skybox/left.png")
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    // build and compile shaders
    // -------------------------
    Shader shipShader("resources/shaders/model_lighting.vs", "resources/shaders/model_lighting.fs");
    Shader explodingShipShader("resources/shaders/model_exploding.vs", "resources/shaders/model_exploding.fs", "resources/shaders/model_exploding.gs");
    Shader explosionBallShader("resources/shaders/explosion_ball.vs", "resources/shaders/explosion_ball.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader bulletShader("resources/shaders/bullet.vs", "resources/shaders/bullet.fs");
    Shader hdrShader("resources/shaders/hdr.vs", "resources/shaders/hdr.fs");

    // setting up HDR
    // --------------------------
    // floating point framebuffer
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    // floating point color buffer
    unsigned int colorBuffer;
    glGenTextures(1, &colorBuffer);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // depth buffer (render buffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    // attach buffers
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // load models
    // -----------
    Model ship("resources/objects/99-intergalactic_spaceship-obj/Intergalactic_Spaceship-(Wavefront).obj");
    ship.SetShaderTextureNamePrefix("material.");
    Model explosionBall("resources/objects/moon-obj/source/Moon.obj");
    explosionBall.SetShaderTextureNamePrefix("material.");

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // shader configuration
    // -------------------------
    // skybox
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // set dirLight parameters
    shipShader.use();
    shipShader.setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
    shipShader.setVec3("dirLight.ambient", glm::vec3(1.0));
    shipShader.setVec3("dirLight.diffuse", glm::vec3(1.0));
    shipShader.setVec3("dirLight.specular", glm::vec3(1.0));
    shipShader.setVec3("dirLight.intensity", programState->dirLightIntensity);


    // set explosion light
    shipShader.setVec3("explosionLight.position", programState->explosionPosition);
    shipShader.setVec3("explosionLight.ambient", programState->explosionColor);
    shipShader.setVec3("explosionLight.diffuse", programState->explosionColor);
    shipShader.setVec3("explosionLight.specular", programState->explosionColor);
    shipShader.setFloat("explosionLight.constant", 1.0f);
    shipShader.setFloat("explosionLight.linear", 0.1f);
    shipShader.setFloat("explosionLight.quadratic", 0.032f);
    shipShader.setFloat("material.shininess", 8.0f);

    explodingShipShader.use();
    explodingShipShader.setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
    explodingShipShader.setVec3("dirLight.ambient", glm::vec3(1.0));
    explodingShipShader.setVec3("dirLight.diffuse", glm::vec3(1.0));
    explodingShipShader.setVec3("dirLight.specular", glm::vec3(1.0));
    explodingShipShader.setVec3("dirLight.intensity", programState->dirLightIntensity);

    explodingShipShader.setVec3("explosionLight.position", programState->explosionPosition);
    explodingShipShader.setVec3("explosionLight.ambient", programState->explosionColor);
    explodingShipShader.setVec3("explosionLight.diffuse", programState->explosionColor);
    explodingShipShader.setVec3("explosionLight.specular", programState->explosionColor);
    explodingShipShader.setFloat("explosionLight.constant", 1.0f);
    explodingShipShader.setFloat("explosionLight.linear", 0.09f);
    explodingShipShader.setFloat("explosionLight.quadratic", 0.032f);
    explodingShipShader.setFloat("material.shininess", 8.0f);

    hdrShader.use();
    hdrShader.setInt("hdrBuffer", 0);


    // time counter; 0 - 3000 -> explosion grows; >3000 -> explosion shrinks
    programState->counter = 0.0f;
    // delay before the animation starts
    int delay_counter = 100;

    float explosion_scale;
    float explosion_light_intensity;

    programState->animation_speed = 2;
    programState->loop = true;

    int current_fb;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render into floating point framebuffer
        // ===================================================================================================
        if (programState->hdr)
            glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &current_fb);

        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1);

        // explosion ball
        explosion_scale = (programState->counter <= 3000) ?
                0.0002f * programState->counter : 0.6f - 0.0002f * (programState->counter - 3000.0f);
        explosion_scale = max(explosion_scale, 0.0f);
        explosion_light_intensity = (explosion_scale <= 0 || programState->counter < 400.0f) ? 0 : 4*explosion_scale;

        explosionBallShader.use();
        explosionBallShader.setVec3("explosionColor_u", (explosion_scale + 0.5f) * programState->explosionColor);
//        if (current_fb == (int)hdrFBO){
//            explosion_light_intensity = explosion_light_intensity*10;
//            explosionBallShader.setVec3("explosionColor_u", 10*(explosion_scale + 0.5f) * programState->explosionColor);
//        }
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->explosionPosition);
        model = glm::scale(model, glm::vec3(explosion_scale));
        explosionBallShader.setMat4("model", model);
        explosionBallShader.setMat4("projection", projection);
        explosionBallShader.setMat4("view", view);
        explosionBall.Draw(explosionBallShader);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // attacking battleship #1
        shipShader.use();
        shipShader.setVec3("explosionLight.intensity",  glm::vec3(explosion_light_intensity));
        shipShader.setVec3("dirLight.intensity", programState->dirLightIntensity);

        model = glm::mat4(1.0f);
        model = glm::translate(model,programState->ship1Position);
        model = glm::rotate(model, glm::radians(20.0f), glm::vec3(0.0, 1.0, 0.0));
        model = glm::scale(model, glm::vec3(programState->shipScale));
        shipShader.setMat4("model", model);
        shipShader.setVec3("viewPosition", programState->camera.Position);
        shipShader.setMat4("projection", projection);
        shipShader.setMat4("view", view);

        ship.Draw(shipShader);

        // attacking battleship #2
        model = glm::mat4(1);
        model = glm::translate(model, programState->ship2Position);
        model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0.0, 1.0, 0.0));
        model = glm::scale(model, glm::vec3(programState->shipScale));
        shipShader.setMat4("model", model);
        ship.Draw(shipShader);


        /* template for a new object
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3());
        model = glm::rotate(model, glm::radians(), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3());
        objShader.setMat4("model", model);
        x.Draw(objShader);
        */

        // bullets
        bulletShader.use();
        bulletShader.setMat4("view", view);
        bulletShader.setMat4("projection", projection);
        glBindVertexArray(VAO);

        for (int i = 0; i < programState->bulletCount; i++) {
            glm::vec3 bulletPos = initialBulletPositions[i] +
                    (programState->explosionPosition - initialBulletPositions[i])*programState->counter/500.0f;
            if (bulletPos.z >= programState->explosionPosition.z - 0.5f || bulletPos.z <= programState->ship2Position.z)
                continue;
            model = glm::mat4(1);
            model = glm::translate(model, bulletPos);
            model = glm::scale(model, glm::vec3(programState->bulletScale));
            bulletShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glDisable(GL_CULL_FACE);

        // draw skybox
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        // exploding battleship
        if (programState->counter <= 3000.0f) {
            model = glm::mat4(1);
            model = glm::translate(model, programState->explosionPosition - glm::vec3(0.0f, 0.0f, 0.2f));
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
            model = glm::scale(model, glm::vec3(programState->shipScale));
            view = programState->camera.GetViewMatrix();
            // if explosion has not started yet, use regular ship shader
            if (programState->counter < 400) {
                shipShader.use();
                shipShader.setMat4("model", model);
                ship.Draw(shipShader);
            } else{
                explodingShipShader.use();
                explodingShipShader.setVec3("dirLight.intensity", programState->dirLightIntensity);
                explodingShipShader.setVec3("explosionLight.intensity", glm::vec3(explosion_light_intensity));
                model = glm::mat4(1);
                model = glm::translate(model, programState->explosionPosition - glm::vec3(0.0f, 0.0f, 0.2f));
                model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
                model = glm::scale(model, glm::vec3(programState->shipScale));
                explodingShipShader.setMat4("model", model);
                view = programState->camera.GetViewMatrix();
                explodingShipShader.setMat4("view", view);
                explodingShipShader.setMat4("projection", projection);
                explodingShipShader.setFloat("time", (programState->counter < 400.0f) ? 0.0f : programState->counter / 1000.0f);
                explodingShipShader.setFloat("transparency", 3.0f - 1.0f * programState->counter / 1000.0f);
                ship.Draw(explodingShipShader);
            }
        }

        // render floating point color buffer to 2D quad and tone-map HDR colors to default frame buffer's (clamped) color range

        if (current_fb == (int)hdrFBO) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            hdrShader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, colorBuffer);
            hdrShader.setFloat("exposure", programState->exposure);
            renderQuad();
        }

        if (delay_counter >= 0 && !programState->pause) {
            delay_counter--;
        } else {
            // increment counter if animation is not finished yet
            if (programState->counter < 8000 && !programState->pause)
                programState->counter += programState->counter <= 3000 ? programState->animation_speed : 5.0f*programState->animation_speed;

            // reset counter if loop is true
            else if (programState->loop && !programState->pause)
                programState->counter = 0;

        }


        if (programState->ImGuiEnabled)
            DrawImGui(programState);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        ImGui::Begin("Animation settings");
        ImGui::DragFloat("Time", &programState->counter, 5.0, 0.0, 8000.0);
        ImGui::DragFloat("Animation speed", &programState->animation_speed, 0.3, 0.0, 20.0);
        ImGui::DragFloat("HDR exposure parameter", &programState->exposure, 0.1, 0.0, 10.0);
        ImGui::Checkbox("Loop", &programState->loop);
        ImGui::Checkbox("Pause", &programState->pause);
        ImGui::Checkbox("HDR", &programState->hdr);
        if (ImGui::Button("Reset time")) {
            programState->counter = 0;
        }
        // uncomment for testing
//        ImGui::Text("DirLight settings:");
//        ImGui::DragFloat("Ambient", &programState->dirLightIntensity.x, 0.05, 0.0, 5.0);
//        ImGui::DragFloat("Diffuse", &programState->dirLightIntensity.y, 0.05, 0.0, 5.0);
//        ImGui::DragFloat("Specular", &programState->dirLightIntensity.z, 0.05, 0.0, 5.0);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            programState->CameraMouseMovementUpdateEnabled = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        programState->pause = !programState->pause;
    }
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        programState->hdr = !programState->hdr;
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        programState->loop = !programState->loop;
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        programState->counter = 0;
    }
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
