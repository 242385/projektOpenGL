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

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr int HOUSE_AMOUNT = 200 * 200;

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
static glm::vec3 excavatorPosition = glm::vec3(0.0f, -0.2f, 12.0f);
static float speed = 0.5f;

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
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Zadanie 4", nullptr, nullptr);
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
    Model surface("res/models/ground/Ground.obj");
    Model walls("res/models/wall/Wall.obj");
    Model ceiling("res/models/roof/Roof.obj");
    Model tracksModel("res/models/digger/tracks.obj");
    Model cabinModel("res/models/digger/cabin.obj");
    Model windowsModel("res/models/digger/windows.obj");

    // Scene root object (SceneGraph Node)
    Node root(&surface);
    Node houses(&walls);

    Node* groundArray = new Node[HOUSE_AMOUNT];
    for (int i = 0; i < 200; i++)
    {
        for (int k = 0; k < 200; k++)
        {
            Node ground(&surface);
            glm::mat4 transform(1.0f);
            transform = glm::translate(transform, glm::vec3(0.0f, -1.00f, 0.0f));
            ground.setTransform(transform);
            groundArray[i * 200 + k] = ground;
        }
    }

    Node* houseArray = new Node[HOUSE_AMOUNT];
    float gap = 12.0f;
    for (int i = 0; i < 200; i++)
    {
        for (int k = 0; k < 200; k++)
        {
            Node house(&walls);
            glm::mat4 transform(1.0f);
            transform = translate(transform, glm::vec3(gap * i - 100 * gap, 0.0f, gap * k - 100 * gap));
            transform = glm::scale(transform, glm::vec3(2.0f));
            house.setTransform(transform);
            houseArray[i * 200 + k] = house;
        }
    }

    Node* roofArray = new Node[HOUSE_AMOUNT];
    float roofOffset = 1.0f;
    for (int i = 0; i < 200; i++)
    {
        for (int k = 0; k < 200; k++)
        {
            Node roof(&ceiling);
            glm::mat4 transform(1.0f);
            transform = translate(transform, glm::vec3(0.0f, roofOffset, 0.0f));
            roof.setTransform(transform);
            roofArray[i * 200 + k] = roof;
        }
    }

    // Scene hierarchy
    for (int i = 0; i < HOUSE_AMOUNT; i++)
    {
        houseArray[i].children.push_back(&groundArray[i]);
        houseArray[i].children.push_back(&roofArray[i]);
        houses.addChild(&houseArray[i]);
    }
    root.addChild(&houses);

    // get all transforms
    glm::mat4* groundTransforms = new glm::mat4[40000];
    glm::mat4* houseTransforms = new glm::mat4[40000];
    glm::mat4* roofTransforms = new glm::mat4[40000];

    root.getNewWorld(glm::mat4(1.0f), true);

    for (int i = 0; i < 40000; i++)
    {
        groundTransforms[i] = groundArray[i].getWorld();
        houseTransforms[i] = houseArray[i].getWorld();
        roofTransforms[i] = roofArray[i].getWorld();
    }

    // Instance arrays
    {
        // ground
        GLuint groundInstances;
        glGenBuffers(1, &groundInstances);
        glBindBuffer(GL_ARRAY_BUFFER, groundInstances);
        glBufferData(GL_ARRAY_BUFFER, HOUSE_AMOUNT * sizeof(glm::mat4), groundTransforms, GL_STATIC_DRAW);

        GLuint VAO = surface.meshes[0].VAO;
        glBindVertexArray(VAO);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), static_cast<GLvoid*>(nullptr));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<GLvoid*>(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<GLvoid*>(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<GLvoid*>(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glBindVertexArray(0);

        // houses
        GLuint houseInstances;
        glGenBuffers(1, &houseInstances);
        glBindBuffer(GL_ARRAY_BUFFER, houseInstances);
        glBufferData(GL_ARRAY_BUFFER, HOUSE_AMOUNT * sizeof(glm::mat4), houseTransforms, GL_STATIC_DRAW);

        VAO = walls.meshes[0].VAO;
        glBindVertexArray(VAO);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), static_cast<GLvoid*>(nullptr));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<GLvoid*>(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<GLvoid*>(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<GLvoid*>(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glBindVertexArray(0);

        // roofs
        GLuint roofInstances;
        glGenBuffers(1, &roofInstances);
        glBindBuffer(GL_ARRAY_BUFFER, roofInstances);
        glBufferData(GL_ARRAY_BUFFER, HOUSE_AMOUNT * sizeof(glm::mat4), roofTransforms, GL_STATIC_DRAW);

        VAO = ceiling.meshes[0].VAO;
        glBindVertexArray(VAO);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), static_cast<GLvoid*>(nullptr));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<GLvoid*>(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<GLvoid*>(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<GLvoid*>(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }

    // Lights
    DirectionalLight directionalLight(
        glm::vec3(-0.2f, -1.0f, -0.3f),
        glm::vec3(0.1f, 0.1f, 0.1f),
        glm::vec3(0.8f, 0.8f, 0.8f),
        glm::vec3(0.0f, 0.0f, 0.0f));

    PointLight pointLight(
        glm::vec3(0.7f, 0.2f, 2.0f),
        glm::vec3(0.05f, 0.05f, 0.05f),
        glm::vec3(0.8f, 0.8f, 0.8f),
        glm::vec3(1.0f, 1.0f, 1.0f));

    SpotLight spotLight1(
        glm::vec3(7.0f, 5.0f, 2.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f));

    SpotLight spotLight2(
        glm::vec3(-7.0f, 5.0f, -2.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f));


    LightPosition dirPosition, pointPosition, spotOnePosition, spotTwoPosition;
    Node dirLightPosition(&dirPosition);
    Node pointLightPosition(&pointPosition);
    Node spotLightOnePosition(&spotOnePosition);
    Node spotLightTwoPosition(&spotTwoPosition);
    root.addChild(&dirLightPosition);
    root.addChild(&pointLightPosition);
    root.addChild(&spotLightOnePosition);
    root.addChild(&spotLightTwoPosition);

    float pointAngle = 0.0f;

    // Excavator
    Node excavator(&tracksModel);
    Node cabin(&cabinModel);
    Node windows(&windowsModel);
    cabin.addChild(&windows);
    excavator.addChild(&cabin);
    root.addChild(&excavator);

    glm::mat4 excavatorTransform = glm::mat4(1.0f);
    excavatorTransform = scale(excavatorTransform, glm::vec3(0.5f));
    excavatorTransform = translate(excavatorTransform, excavatorPosition);
    excavator.setTransform(excavatorTransform);

    glm::mat4 cabinTransform = glm::mat4(1.0f); // scaled anyway since cabin is a child of tracks/excavator - will be used later
    cabin.setTransform(cabinTransform);
    windows.setTransform(cabinTransform);

    static bool enableDirectional = true;
    static bool enablePoint = true;
    static bool enableSpot1 = true;
    static bool enableSpot2 = true;

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
            ImGui::Begin("Zadanie 4");
            ImGui::SliderFloat("Excavator speed", &speed, 0.1f, 5.0f);

            ImGui::Separator();

            ImGui::Text("Directional light");
            ImGui::Checkbox("DL_Enabled", &enableDirectional);
            ImGui::ColorEdit4("DL_Ambient", reinterpret_cast<float*>(&directionalLight.ambient));
            ImGui::ColorEdit4("DL_Diffuse", reinterpret_cast<float*>(&directionalLight.diffuse));
            ImGui::ColorEdit4("DL_Specular", reinterpret_cast<float*>(&directionalLight.specular));
            ImGui::SliderFloat3("DL_Direction", reinterpret_cast<float*>(&directionalLight.direction), -1.0f, 1.0f);

            ImGui::Separator();

            ImGui::Text("Point light");
            ImGui::Checkbox("PL_Enabled", &enablePoint);
            ImGui::ColorEdit4("PL_Ambient", reinterpret_cast<float*>(&pointLight.ambient));
            ImGui::ColorEdit4("PL_Diffuse", reinterpret_cast<float*>(&pointLight.diffuse));
            ImGui::ColorEdit4("PL_Specular", reinterpret_cast<float*>(&pointLight.specular));

            ImGui::Separator();

            ImGui::Text("Spot light 1");
            ImGui::Checkbox("S1_Enabled", &enableSpot1);
            ImGui::ColorEdit4("S1_Ambient", reinterpret_cast<float*>(&spotLight1.ambient));
            ImGui::ColorEdit4("S1_Diffuse", reinterpret_cast<float*>(&spotLight1.diffuse));
            ImGui::ColorEdit4("S1_Specular", reinterpret_cast<float*>(&spotLight1.specular));
            ImGui::SliderFloat3("S1_Direction", reinterpret_cast<float*>(&spotLight1.direction), -10.0f, 10.0f);
            ImGui::SliderFloat3("S1_Position", reinterpret_cast<float*>(&spotLight1.position), -120.0f, 120.0f);

            ImGui::Separator();

            ImGui::Text("Spot light 2");
            ImGui::Checkbox("S2_Enabled", &enableSpot2);
            ImGui::ColorEdit4("S2_Ambient", reinterpret_cast<float*>(&spotLight2.ambient));
            ImGui::ColorEdit4("S2_Diffuse", reinterpret_cast<float*>(&spotLight2.diffuse));
            ImGui::ColorEdit4("S2_Specular", reinterpret_cast<float*>(&spotLight2.specular));
            ImGui::SliderFloat3("S2_Direction", reinterpret_cast<float*>(&spotLight2.direction), -10.0f, 10.0f);
            ImGui::SliderFloat3("S2_Position", reinterpret_cast<float*>(&spotLight2.position), -120.0f, 120.0f);

            ImGui::Separator();

            ImGui::Text("Performance");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::End();
        }

        if (driveMode)
        {
            glm::mat4 pos = excavator.getWorld();
            cam.position = glm::vec3(pos[3]) + glm::vec3(20.0f * cosf(glm::radians(-excavatorRotation)), 15.0f, 20.0f * sinf(glm::radians(-excavatorRotation)));
        }

        // Rendering - ImGui
        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h); 

        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Point light position
        pointAngle += deltaTime;
        pointLight.position = glm::vec3(cos(pointAngle) * 15.0f, 6.0f, sin(pointAngle) * 15.0f);

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

            // Point light
            shaderInstance.setBool("pointLight.enabled", enablePoint);
            shaderInstance.setVec3("pointLight.position", pointLight.position);
            shaderInstance.setVec3("pointLight.ambient", pointLight.ambient);
            shaderInstance.setVec3("pointLight.diffuse", pointLight.diffuse);
            shaderInstance.setVec3("pointLight.specular", pointLight.specular);
            shaderInstance.setFloat("pointLight.constant", pointLight.constant);
            shaderInstance.setFloat("pointLight.linear", pointLight.linear);
            shaderInstance.setFloat("pointLight.quadratic", pointLight.quadratic);

            // Spot light 1
            shaderInstance.setBool("spotLights[0].enabled", enableSpot1);
            shaderInstance.setVec3("spotLights[0].position", spotLight1.position);
            shaderInstance.setVec3("spotLights[0].direction", spotLight1.direction);
            shaderInstance.setVec3("spotLights[0].ambient", spotLight1.ambient);
            shaderInstance.setVec3("spotLights[0].diffuse", spotLight1.diffuse);
            shaderInstance.setVec3("spotLights[0].specular", spotLight1.specular);
            shaderInstance.setFloat("spotLights[0].constant", spotLight1.constant);
            shaderInstance.setFloat("spotLights[0].linear", spotLight1.linear);
            shaderInstance.setFloat("spotLights[0].quadratic", spotLight1.quadratic);
            shaderInstance.setFloat("spotLights[0].cutOff", spotLight1.cutOff);
            shaderInstance.setFloat("spotLights[0].outerCutOff", spotLight1.outerCutOff);

            // Spot light 2
            shaderInstance.setBool("spotLights[1].enabled", enableSpot2);
            shaderInstance.setVec3("spotLights[1].position", spotLight2.position);
            shaderInstance.setVec3("spotLights[1].direction", spotLight2.direction);
            shaderInstance.setVec3("spotLights[1].ambient", spotLight2.ambient);
            shaderInstance.setVec3("spotLights[1].diffuse", spotLight2.diffuse);
            shaderInstance.setVec3("spotLights[1].specular", spotLight2.specular);
            shaderInstance.setFloat("spotLights[1].constant", spotLight2.constant);
            shaderInstance.setFloat("spotLights[1].linear", spotLight2.linear);
            shaderInstance.setFloat("spotLights[1].quadratic", spotLight2.quadratic);
            shaderInstance.setFloat("spotLights[1].cutOff", spotLight2.cutOff);
            shaderInstance.setFloat("spotLights[1].outerCutOff", spotLight2.outerCutOff);
        }

        // view/projection transform
        glm::mat4 projection = glm::perspective(glm::radians(cam.zoom), static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT), 0.1f, 500.0f);
        glm::mat4 view = cam.getViewMatrix();
        shaderInstance.setMat4("projection", projection);
        shaderInstance.setMat4("view", view);

        // world transform
        glm::mat4 model = glm::mat4(1.0f);
        shaderInstance.setMat4("model", model);

        // Rendering - instances
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, surface.loadedTextures[0].id);
        glBindVertexArray(surface.meshes[0].VAO);
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(surface.meshes[0].indices.size()), GL_UNSIGNED_INT, nullptr, HOUSE_AMOUNT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, walls.loadedTextures[0].id);
        glBindVertexArray(walls.meshes[0].VAO);
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(walls.meshes[0].indices.size()), GL_UNSIGNED_INT, nullptr, HOUSE_AMOUNT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ceiling.loadedTextures[0].id);
        glBindVertexArray(ceiling.meshes[0].VAO);
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(ceiling.meshes[0].indices.size()), GL_UNSIGNED_INT, nullptr, HOUSE_AMOUNT);

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

            // Point light
            shaderLit.setBool("pointLight.enabled", enablePoint);
            shaderLit.setVec3("pointLight.position", pointLight.position);
            shaderLit.setVec3("pointLight.ambient", pointLight.ambient);
            shaderLit.setVec3("pointLight.diffuse", pointLight.diffuse);
            shaderLit.setVec3("pointLight.specular", pointLight.specular);
            shaderLit.setFloat("pointLight.constant", pointLight.constant);
            shaderLit.setFloat("pointLight.linear", pointLight.linear);
            shaderLit.setFloat("pointLight.quadratic", pointLight.quadratic);

            // Spot light 1
            shaderLit.setBool("spotLights[0].enabled", enableSpot1);
            shaderLit.setVec3("spotLights[0].position", spotLight1.position);
            shaderLit.setVec3("spotLights[0].direction", spotLight1.direction);
            shaderLit.setVec3("spotLights[0].ambient", spotLight1.ambient);
            shaderLit.setVec3("spotLights[0].diffuse", spotLight1.diffuse);
            shaderLit.setVec3("spotLights[0].specular", spotLight1.specular);
            shaderLit.setFloat("spotLights[0].constant", spotLight1.constant);
            shaderLit.setFloat("spotLights[0].linear", spotLight1.linear);
            shaderLit.setFloat("spotLights[0].quadratic", spotLight1.quadratic);
            shaderLit.setFloat("spotLights[0].cutOff", spotLight1.cutOff);
            shaderLit.setFloat("spotLights[0].outerCutOff", spotLight1.outerCutOff);

            // Spot light 2
            shaderLit.setBool("spotLights[1].enabled", enableSpot2);
            shaderLit.setVec3("spotLights[1].position", spotLight2.position);
            shaderLit.setVec3("spotLights[1].direction", spotLight2.direction);
            shaderLit.setVec3("spotLights[1].ambient", spotLight2.ambient);
            shaderLit.setVec3("spotLights[1].diffuse", spotLight2.diffuse);
            shaderLit.setVec3("spotLights[1].specular", spotLight2.specular);
            shaderLit.setFloat("spotLights[1].constant", spotLight2.constant);
            shaderLit.setFloat("spotLights[1].linear", spotLight2.linear);
            shaderLit.setFloat("spotLights[1].quadratic", spotLight2.quadratic);
            shaderLit.setFloat("spotLights[1].cutOff", spotLight2.cutOff);
            shaderLit.setFloat("spotLights[1].outerCutOff", spotLight2.outerCutOff);

            shaderLit.setMat4("projection", projection);
            shaderLit.setMat4("view", view);
            shaderLit.setMat4("model", model);
        }

        // Camera speed
        cam.speed = driveMode ? 0.0f : 20.0f; // When in drive mode, camera follows the excavator and does not move on its own

        // Excavator
        glm::mat4 newExcavatorTransform = scale(newExcavatorTransform, glm::vec3(0.5f));
        newExcavatorTransform = translate(excavatorTransform, excavatorPosition);
        newExcavatorTransform = glm::rotate(newExcavatorTransform, glm::radians(excavatorRotation), glm::vec3(0.0f, 1.0f, 0.0f));
        excavator.setTransform(newExcavatorTransform);

        glm::mat4 newCabinTransform = glm::rotate(cabinTransform, glm::radians(cabinRotation), glm::vec3(0.0f, 1.0f, 0.0f));
        cabin.setTransform(newCabinTransform);

        glm::mat4 newWindowsTransform = glm::mat4(1.0f);
        windows.setTransform(newWindowsTransform);

        excavator.getNewWorld(model, true);

        glActiveTexture(GL_TEXTURE9);
        shaderLit.setInt("skybox", 9);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

        excavator.drawThis(glm::mat4(1.0f), shaderLit);
        cabin.drawThis(glm::mat4(1.0f), shaderLit);

        shaderRefraction.use();
        {
            shaderRefraction.setVec3("cameraPos", cam.position);
            shaderRefraction.setMat4("projection", projection);
            shaderRefraction.setMat4("view", view);
            shaderRefraction.setMat4("model", model);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
        windows.drawThis(glm::mat4(1.0f), shaderRefraction);

        // Show light sources
        glm::mat4 proview = projection * view;
        if (enableDirectional)
        {
            dirPosition.drawSphere(glm::vec3(0.f, 10.f, 0.f), glm::vec4(directionalLight.diffuse, 1.0f), proview);
            dirPosition.drawArrow(glm::vec3(0.f, 10.f, 0.f), glm::vec3(0.f, 10.f, 0.f) + directionalLight.direction * 2.0f, glm::vec4(1.0f), proview);
        }
        if (enablePoint)
        {
            pointPosition.drawSphere(pointLight.position, glm::vec4(pointLight.diffuse, 1.0f), proview);
        }
        if (enableSpot1)
        {
            spotOnePosition.drawSphere(spotLight1.position, glm::vec4(spotLight1.diffuse, 1.0f), proview);
            spotOnePosition.drawArrow(spotLight1.position, (spotLight1.position + spotLight1.direction) * 2.0f, glm::vec4(1.0f), proview);
        }
        if (enableSpot2)
        {
            spotTwoPosition.drawSphere(spotLight2.position, glm::vec4(spotLight2.diffuse, 1.0f), proview);
            spotTwoPosition.drawArrow(spotLight2.position, (spotLight2.position + spotLight2.direction) * 2.0f, glm::vec4(1.0f), proview);
        }
            
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

    // Cleanup
    delete[] groundArray;
    delete[] houseArray;
    delete[] roofArray;
    delete[] groundTransforms;
    delete[] houseTransforms;
    delete[] roofTransforms;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void process_input(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        if (driveMode) 
        {
            excavatorPosition.x -= speed * cosf(glm::radians(excavatorRotation));
            excavatorPosition.z += speed * sinf(glm::radians(excavatorRotation));
        }
    	else cam.processKeyboard(FRONT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        if (driveMode)
        {
            excavatorPosition.x += speed * cosf(glm::radians(excavatorRotation));
            excavatorPosition.z -= speed * sinf(glm::radians(excavatorRotation));
        }
        else cam.processKeyboard(BACK, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        if (driveMode)
        {
            excavatorRotation += 1.0f;
        	excavatorPosition.x -= 0.5f * speed * cosf(glm::radians(excavatorRotation));
            excavatorPosition.z += 0.5f * speed * sinf(glm::radians(excavatorRotation));
        }
        else cam.processKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        if (driveMode)
        {
            excavatorRotation -= 1.0f;
            excavatorPosition.x -= 0.5f * speed * cosf(glm::radians(excavatorRotation));
            excavatorPosition.z += 0.5f * speed * sinf(glm::radians(excavatorRotation));
        }
        else cam.processKeyboard(RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (driveMode)
        {
            cabinRotation += 1.0f;
            if (cabinRotation >= 30.0f) cabinRotation = 30.0f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (driveMode)
        {
            cabinRotation -= 1.0f;
            if (cabinRotation <= -30.0f) cabinRotation = -30.0f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        if (driveMode)
        {
            //cachedCamPosition = cam.position; //doesn't work :|
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            //cam.position = cachedCamPosition;
        }

        driveMode = !driveMode;
    }
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
