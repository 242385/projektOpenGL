#include "imgui.h"
#include "imgui_impl/imgui_impl_glfw.h"
#include "imgui_impl/imgui_impl_opengl3.h"
#include <glm/glm.hpp>

#include "Camera.h"
#include "Model.h"
#include "Shader.h"
#include "Node.h"

#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "LightPosition.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions
#include <spdlog/spdlog.h>

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void process_input(GLFWwindow* window, int key, int scancode, int action, int mods);
GLuint loadCubemapTexture(std::vector<std::string> faces);

static void glfw_error_callback(int err, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", err, description);
}

constexpr int WINDOW_WIDTH = 1600;
constexpr int WINDOW_HEIGHT = 900;

// Camera setup
Camera cam(glm::vec3(0.0f, 30.0f, 25.0f));
float camX = WINDOW_WIDTH * 0.5;
float camY = WINDOW_HEIGHT * 0.5;
float lastX = static_cast<float>(WINDOW_WIDTH) / 2.0;
float lastY = static_cast<float>(WINDOW_HEIGHT) / 2.0;
static bool firstMouse = true;
static bool isMouseButtonPressed = false;
static bool driveMode = false;
static glm::vec3 cachedCamPosition;
static glm::vec2 cachedCamRotation;

// Time setup
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Excavator setup
static float excavatorRotation = 0.0f;
static float cabinRotation = 0.0f;
static glm::vec3 modelPos = glm::vec3(0.0f, 25.0f, 0.0f);

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 4.3 + GLSL 430
    const char* glsl_version = "#version 430";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Programowanie animacji", nullptr, nullptr);
    if (window == nullptr) return 1;
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, process_input);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSwapInterval(1);

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#endif
    if (err)
    {
        spdlog::error("Failed to initialize OpenGL loader!");
        return 1;
    }
    spdlog::info("Successfully initialized OpenGL loader!");

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    const ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup ImGui style
    ImGui::StyleColorsDark();
    constexpr ImVec4 clear_color = ImVec4(0.11f, 0.11f, 0.22f, 1.0f);//(0.11f, 0.11f, 0.11f, 1.00f);
    
        
    // SHADER SETUP //
    //Shader shaderProgram("res/shaders/basic.vert", "res/shaders/basic.frag");
    Shader shaderLit("res/shaders/lit.vert", "res/shaders/lit.frag");
    Shader shaderInstance("res/shaders/instance.vert", "res/shaders/instance.frag");
    Shader shaderSkybox("res/shaders/skybox.vert", "res/shaders/skybox.frag");
    Shader shaderRefraction("../../res/shaders/refraction.vert", "../../res/shaders/refraction.frag");

    // SKYBOX SETUP //
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

	std::vector<std::string> faces =
    {
        "../../res/textures/skybox/right.jpg",
        "../../res/textures/skybox/left.jpg",
        "../../res/textures/skybox/top.jpg",
        "../../res/textures/skybox/bottom.jpg",
        "../../res/textures/skybox/front.jpg",
        "../../res/textures/skybox/back.jpg"
    };

    GLuint cubemap = loadCubemapTexture(faces);
	GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));

    shaderSkybox.use();
    shaderSkybox.setInt("skybox", 0);

    // load models
    Model loadedModel("res/models/sword.obj");

    // Scene root object (SceneGraph Node)
    Node root(&loadedModel);

    // WATCH OUT! If something will be translated unproperly in the future it might be because of
    // a model being a root itself... Maybe another "model" should be a root?

    root.getNewWorld(glm::mat4(1.0f), true);

    // Lights
    DirectionalLight directionalLight(
        glm::vec3(-0.2f, -1.0f, -0.3f),
        glm::vec3(0.1f, 0.1f, 0.1f),
        glm::vec3(0.8f, 0.8f, 0.8f),
        glm::vec3(0.0f, 0.0f, 0.0f));

    LightPosition dirPosition;
    Node dirLightPosition(&dirPosition);
    root.addChild(&dirLightPosition);

    // Excavator
    Node mainModel(&loadedModel);
    root.addChild(&mainModel);

    glm::mat4 mainModelTransform = glm::mat4(1.0f);
    mainModelTransform = scale(mainModelTransform, glm::vec3(0.5f));
    mainModelTransform = translate(mainModelTransform, modelPos);
    mainModel.setTransform(mainModelTransform);

    glm::mat4 cabinTransform = glm::mat4(1.0f); // scaled anyway since cabin is a child of tracks/excavator - will be used later

    static bool enableDirectional = true;

    // enable depth test
    glEnable(GL_DEPTH_TEST);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Delta time
        const float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui window
        {
            ImGui::Begin("Programowanie animacji");

            ImGui::Separator();

            ImGui::Text("Directional light");
            ImGui::Checkbox("DL_Enabled", &enableDirectional);
            ImGui::ColorEdit4("DL_Ambient", reinterpret_cast<float*>(&directionalLight.ambient));
            ImGui::ColorEdit4("DL_Diffuse", reinterpret_cast<float*>(&directionalLight.diffuse));
            ImGui::ColorEdit4("DL_Specular", reinterpret_cast<float*>(&directionalLight.specular));
            ImGui::SliderFloat3("DL_Direction", reinterpret_cast<float*>(&directionalLight.direction), -1.0f, 1.0f);

            ImGui::Separator();

            ImGui::Text("Performance");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::End();
        }

        // Rendering - ImGui
        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h); 

        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Setting lit shader uniforms
        shaderInstance.use();
        {
            shaderInstance.setVec3("viewPos", cam.position);
            shaderInstance.setFloat("material.shininess", 32.0f);

            // We set all the uniforms for the types of lights we have. 
            // We have to set them manually and index the proper struct in the array 
            // to set each uniform variable.

            //Directional light
            shaderInstance.setBool("dirLight.enabled", enableDirectional);
            shaderInstance.setVec3("dirLight.direction", directionalLight.direction);
            shaderInstance.setVec3("dirLight.ambient", directionalLight.ambient);
            shaderInstance.setVec3("dirLight.diffuse", directionalLight.diffuse);
            shaderInstance.setVec3("dirLight.specular", directionalLight.specular);
        }

        // view/projection transform
        glm::mat4 projection = glm::perspective(glm::radians(cam.zoom), static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT), 0.1f, 500.0f);
        glm::mat4 view = cam.getViewMatrix();
        shaderInstance.setMat4("projection", projection);
        shaderInstance.setMat4("view", view);

        // world transform
        glm::mat4 model = glm::mat4(1.0f);
        shaderInstance.setMat4("model", model);

        glBindVertexArray(0);

        // Setting lit shader uniforms
        shaderLit.use();
        {
            shaderLit.setVec3("viewPos", cam.position);
            shaderLit.setFloat("material.shininess", 32.0f);
            
            //Directional light
            shaderLit.setBool("dirLight.enabled", enableDirectional);
            shaderLit.setVec3("dirLight.direction", directionalLight.direction);
            shaderLit.setVec3("dirLight.ambient", directionalLight.ambient);
            shaderLit.setVec3("dirLight.diffuse", directionalLight.diffuse);
            shaderLit.setVec3("dirLight.specular", directionalLight.specular);

            shaderLit.setMat4("projection", projection);
            shaderLit.setMat4("view", view);
            shaderLit.setMat4("model", model);
        }

        // Camera speed
        cam.speed = 20.0f; // When in drive mode, camera follows the excavator and does not move on its own

        // Excavator
        glm::mat4 newModelTransform = scale(newModelTransform, glm::vec3(0.5f));
        newModelTransform = translate(mainModelTransform, modelPos);
        newModelTransform = glm::rotate(newModelTransform, glm::radians(excavatorRotation), glm::vec3(0.0f, 1.0f, 0.0f));
        mainModel.setTransform(newModelTransform);

        mainModel.getNewWorld(model, true);

        glActiveTexture(GL_TEXTURE9);
        shaderLit.setInt("skybox", 9);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

        mainModel.drawThis(glm::mat4(1.0f), shaderLit);

        shaderRefraction.use();
        {
            shaderRefraction.setVec3("cameraPos", cam.position);
            shaderRefraction.setMat4("projection", projection);
            shaderRefraction.setMat4("view", view);
            shaderRefraction.setMat4("model", model);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

        // Show light sources
        glm::mat4 proview = projection * view;
            
        // Skybox
        glDepthFunc(GL_LEQUAL);
        shaderSkybox.use();
        view = glm::mat4(glm::mat3(cam.getViewMatrix())); // remove translation part from view matrix
        shaderSkybox.setMat4("view", view);
        shaderSkybox.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDepthFunc(GL_LESS);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void process_input(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ;   
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    isMouseButtonPressed =
        button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS;
}

void mouse_callback(GLFWwindow* window, double xPosIn, double yPosIn)
{
    if (isMouseButtonPressed)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        const float xPos = static_cast<float>(xPosIn);
        const float yPos = static_cast<float>(yPosIn);

        if (firstMouse)
        {
            lastX = xPos;
            lastY = yPos;
            firstMouse = false;
        }

        const float xOffset = xPos - lastX;
        const float yOffset = lastY - yPos;
        lastX = xPos;
        lastY = yPos;

        cam.processMouse(xOffset, yOffset);
    }
    else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
    cam.processScroll(static_cast<float>(yOffset));
}

GLuint loadCubemapTexture(std::vector<std::string> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, channels;
    for (GLuint i = 0; i < faces.size(); i++)
    {
	    if (unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0))
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
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
