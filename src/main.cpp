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
    glm::vec3 carPosition = glm::vec3(0.0f);
    float carScale = 0.5f;
    // sun consts
    float sunScale = 0.5f;
    glm::vec3 sunPosition = glm::vec3(0.0f, 0.0f, -10.0f);
    glm::vec3 sunColor = glm::vec3(0.977f, 0.367f, 0.325f);
    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.5f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
//    out << clearColor.r << '\n'
//        << clearColor.g << '\n'
//        << clearColor.b << '\n'
//        << camera.Position.x << '\n'
//        << camera.Position.y << '\n'
//        << camera.Position.z << '\n'
    out << ImGuiEnabled << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
//        in >> clearColor.r
//           >> clearColor.g
//           >> clearColor.b
//        >> camera.Position.x
//        >> camera.Position.y
//        >> camera.Position.z
        in >> ImGuiEnabled
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

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
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "projekat", NULL, NULL);
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
    glEnable(GL_DEPTH_TEST);
//    glDepthFunc(GL_LESS);

    // build and compile shaders
    // -------------------------
    Shader carShader("resources/shaders/model_lighting.vs", "resources/shaders/model_lighting.fs");
    Shader sunShader("resources/shaders/sun_shader.vs", "resources/shaders/sun_shader.fs");


    // load models
    // -----------
    Model car("resources/objects/dodge-charger-1969-obj/source/K1AGJVQ50VCSD2UOPBS13FD0S_obj/K1AGJVQ50VCSD2UOPBS13FD0S.obj");
    car.SetShaderTextureNamePrefix("material.");
    Model sun("resources/objects/moon-obj/source/Moon.obj");
    sun.SetShaderTextureNamePrefix("material.");

    // set light
    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(programState->sunPosition);
    pointLight.ambient = glm::vec3(1.0, 0.5, 0.5);
    pointLight.diffuse = programState->sunColor;// glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 0.0, 0.0);// programState->sunColor; //glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.0f;
    pointLight.quadratic = 0.0f;



    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // set constant uniform variables
    // sun
    sunShader.use();
    sunShader.setVec3("sunColor_u", programState->sunColor);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, programState->sunPosition);
    // model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0,1,0));
    // make it smaller instead of placing it far away
    model = glm::scale(model, glm::vec3(programState->sunScale));
    sunShader.setMat4("model", model);


    // car
    carShader.use();
    carShader.setVec3("sunPointLight.position", pointLight.position);
    carShader.setVec3("sunPointLight.ambient", pointLight.ambient);
    carShader.setVec3("sunPointLight.diffuse", pointLight.diffuse);
    carShader.setVec3("sunPointLight.specular", pointLight.specular);
    carShader.setFloat("sunPointLight.constant", pointLight.constant);
    carShader.setFloat("sunPointLight.linear", pointLight.linear);
    carShader.setFloat("sunPointLight.quadratic", pointLight.quadratic);
    carShader.setFloat("material.shininess", 32.0f);

    model = glm::mat4(1.0f);
    model = glm::translate(model,programState->carPosition);
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
    model = glm::scale(model, glm::vec3(programState->carScale));


//    model = glm::scale(model, glm::vec3(programState->carScale));
    carShader.setMat4("model", model);

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

//        sunShader.use();
//        sunShader.setVec3("sunColor_u", programState->sunColor);
//        glm::mat4 model = glm::mat4(1.0f);
//        model = glm::translate(model, programState->sunPosition);
//        // model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0,1,0));
//        // make it smaller instead of placing it far away
//        model = glm::scale(model, glm::vec3(programState->sunScale));
//        sunShader.setMat4("model", model);

        // don't forget to enable shader before setting uniforms
        carShader.use();
        carShader.setVec3("viewPosition", programState->camera.Position);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        carShader.setMat4("projection", projection);
        carShader.setMat4("view", view);

        // render car

//        model = glm::mat4(1.0f);
//        model = glm::translate(model,programState->carPosition);
//        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
//        model = glm::scale(model, glm::vec3(programState->carScale));
//        carShader.setMat4("model", model);
        car.Draw(carShader);

        /* template for a new object
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(programState->tempPosition));
        model = glm::rotate(model, glm::radians(programState->tempRotation), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(programState->tempScale));
        objShader.setMat4("model", model);
        x.Draw(objShader);
        */

        // draw sun
        sunShader.use();
        sunShader.setMat4("projection", projection);
        sunShader.setMat4("view", view);
        sun.Draw(sunShader);

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
        ImGui::Begin("Model settings");
//        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);
//        ImGui::Button("Reset background color");
        ImGui::DragFloat3("Model position", (float*)&programState->carPosition);
        ImGui::DragFloat("Model scale", &programState->carScale, 0.05, 0.1, 4.0);
//        if (ImGui::Button("Reset camera position"))
//        {
//            &programState->camera.reset();
//        }
        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);
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
}
