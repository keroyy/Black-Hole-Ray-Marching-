#include <iostream>
#include <string>
#include <cmath>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL/SOIL.h>
#include <hdrloader.h>

// imGui
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>

// Other includes
#include "Shader.h"
#include "Camera.h"

// Properties
GLuint screenWidth = 1920, screenHeight = 1080;
const float PI = 3.1415926;

GLuint loadCubemap(vector<const GLchar*> faces);
GLuint loadTexture(const GLchar* path);

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void Do_Movement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 15.0f));
bool keys[1024];
bool buttons[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// parameters
float adiskParticle = 1.0;
float adiskHeight = 0.25;
float adiskLit = 2.0;
float adiskDensityV = 1.5;
float adiskDensityH = 3.4;
float adiskNoiseScale = 0.6;
float adiskNoiseLOD = 8.7;
float adiskSpeed = 2.0;

bool adiskEnabled = true;
bool bloom = true;
bool tonemapping = true;
bool gammaCorrection = true;
float gamma = 2.2;
float tone = 1.0;
float bloomStrength = 0.5;

// The MAIN function, from here we start our application and run our Game loop
int main()
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "black hole", nullptr, nullptr); // Windowed
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Options
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLEW to setup the OpenGL Function pointers
    glewExperimental = GL_TRUE;
    glewInit();

    // Define the viewport dimensions
    glViewport(0, 0, screenWidth, screenHeight); 

    // Setup some OpenGL options
    glEnable(GL_DEPTH_TEST);

    // Setup and compile our shaders
    Shader blackHoleShader("blackhole.vs", "blackhole.frag");
    Shader blurShader("blackhole.vs", "blur.frag");
    Shader bloomShader("blackhole.vs", "bloom.frag");

#pragma region "object_initialization"
    // Set the object data (buffers, vertex attributes)

    // skybox
    float skyboxVertices[] = {
        // positions          
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

    // rectangle
    float rectangleVertices[] = {
             1.0f,  1.0f, 0.0f,  // top right
             1.0f, -1.0f, 0.0f,  // bottom right
            -1.0f, -1.0f, 0.0f,  // bottom left
            -1.0f,  1.0f, 0.0f   // top left 
    };
    unsigned int rectangleIndices[] = {  // note that we start from 0!
        0, 3, 1,   // first triangle
        1, 3, 2    // second triangle
    };

    // Setup skybox VAO
    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);

    // Setup rectangle VAO
    GLuint rayVAO, rayEBO;
    glGenVertexArrays(1, &rayVAO);
    glGenBuffers(1, &rayEBO);
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray(rayVAO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices) * sizeof(float), &rectangleVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rayEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectangleIndices) * sizeof(int), &rectangleIndices, GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0); // Unbind VAO

    // Cubemap (Skybox)
    vector<const GLchar*> faces_test;
    faces_test.push_back("resources/skybox_test/right.png");
    faces_test.push_back("resources/skybox_test/left.png");
    faces_test.push_back("resources/skybox_test/top.png");
    faces_test.push_back("resources/skybox_test/bottom.png");
    faces_test.push_back("resources/skybox_test/front.png");
    faces_test.push_back("resources/skybox_test/back.png");
    vector<const GLchar*> faces;
    faces.push_back("resources/skybox/right.png");
    faces.push_back("resources/skybox/left.png");
    faces.push_back("resources/skybox/top.png");
    faces.push_back("resources/skybox/bottom.png");
    faces.push_back("resources/skybox/front.png");
    faces.push_back("resources/skybox/back.png");
    GLuint cubemapTexture = loadCubemap(faces);

    // colorMap
    GLuint colorMapTexture = loadTexture("resources/Noise/color_map.png");

    // configure (floating point) framebuffers
    // ---------------------------------------
    GLuint hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    GLuint colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (GLuint i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0
        );
    }
    GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    // shader configuration
    // --------------------
    blurShader.Use();
    glUniform1i(glGetUniformLocation(blurShader.Program, "image"), 1);
    bloomShader.Use();
    glUniform1i(glGetUniformLocation(bloomShader.Program, "scene"), 0);
    glUniform1i(glGetUniformLocation(bloomShader.Program, "bloomBlur"), 1);

    // imgui
    ImGui::CreateContext();
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsDark();

#pragma endregion

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Set frame time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check and call events
        //glfwPollEvents();
        //Do_Movement();

        // Clear the colorbuffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // imgui
        ImGui_ImplGlfwGL3_NewFrame(); 

        // 1.render scene into floating point framebuffer
        // ---------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Ray marching
        blackHoleShader.Use();

        // Initialize matrix
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        GLfloat aspect = (GLfloat)screenWidth / (GLfloat)screenHeight;
        GLfloat near = 1.0f;
        GLfloat far = 100.0f;

        // glDepthMask(GL_FALSE);// Remember to turn depth writing off
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
        projection = glm::perspective(camera.Zoom, aspect, near, far);
        glUniformMatrix4fv(glGetUniformLocation(blackHoleShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(blackHoleShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glUniform1f(glGetUniformLocation(blackHoleShader.Program, "time"), (GLfloat)glfwGetTime() * 0.1f);
        glUniform3f(glGetUniformLocation(blackHoleShader.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);

        // paramemters
        glUniform1i(glGetUniformLocation(blackHoleShader.Program, "adiskEnabled"), adiskEnabled);
        glUniform1f(glGetUniformLocation(blackHoleShader.Program, "adiskParticle"), adiskParticle);
        glUniform1f(glGetUniformLocation(blackHoleShader.Program, "adiskHeight"), adiskHeight);
        glUniform1f(glGetUniformLocation(blackHoleShader.Program, "adiskLit"), adiskLit);
        glUniform1f(glGetUniformLocation(blackHoleShader.Program, "adiskDensityV"), adiskDensityV);
        glUniform1f(glGetUniformLocation(blackHoleShader.Program, "adiskDensityH"), adiskDensityH);
        glUniform1f(glGetUniformLocation(blackHoleShader.Program, "adiskNoiseScale"), adiskNoiseScale);
        glUniform1f(glGetUniformLocation(blackHoleShader.Program, "adiskNoiseLOD"), adiskNoiseLOD);
        glUniform1f(glGetUniformLocation(blackHoleShader.Program, "adiskSpeed"), adiskSpeed);
        
        // skybox cubeTexture
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(blackHoleShader.Program, "skybox"), 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        // noise texture
        glActiveTexture(GL_TEXTURE1);
        glUniform1i(glGetUniformLocation(blackHoleShader.Program, "colorMap"), 1);
        glBindTexture(GL_TEXTURE_2D, colorMapTexture);

        glBindVertexArray(rayVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. blur bright fragments with two-pass Gaussian Blur 
        // --------------------------------------------------
        GLboolean horizontal = true, first_iteration = true;
        GLuint amount = 10;
        blurShader.Use();
        for (GLuint i = 0; i < amount; i++)
        {  
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            glUniform1i(glGetUniformLocation(blurShader.Program, "horizontal"), horizontal);
            glBindTexture(
                GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]
            );

            glBindVertexArray(rayVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
        // --------------------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        bloomShader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        glUniform1i(glGetUniformLocation(bloomShader.Program, "bloom"), bloom);
        glUniform1i(glGetUniformLocation(bloomShader.Program, "tonemapping"), tonemapping);
        glUniform1i(glGetUniformLocation(bloomShader.Program, "gammaCorrection"), gammaCorrection);
        glUniform1f(glGetUniformLocation(bloomShader.Program, "gamma"), gamma);
        glUniform1f(glGetUniformLocation(bloomShader.Program, "tone"), tone);
        glUniform1f(glGetUniformLocation(bloomShader.Program, "bloomStrength"), bloomStrength);

        glBindVertexArray(rayVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // imgui
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        {
            //ImGui::Text("AccrectionDisk");                           // Display some text (you can use a format string too)
            ImGui::Checkbox("enabledAdisk", &adiskEnabled);
            ImGui::SliderFloat("adiskParticle", &adiskParticle, 0.0f, 5.0f);            // Edit 1 float using a slider from 0.0f to 1.0f   
            ImGui::SliderFloat("adiskHeight", &adiskHeight, 0.0f, 1.0f);
            ImGui::SliderFloat("adiskLit", &adiskLit, 0.0f, 20.0f);
            ImGui::SliderFloat("adiskDensityV", &adiskDensityV, 0.0f, 10.0f);
            ImGui::SliderFloat("adiskDensityH", &adiskDensityH, 0.0f, 10.0f);
            ImGui::SliderFloat("adiskNoiseScale", &adiskNoiseScale, 0.0f, 10.0f);
            ImGui::SliderFloat("adiskNoiseLOD", &adiskNoiseLOD, 0.0f, 10.0f);
            ImGui::SliderFloat("adiskSpeed", &adiskSpeed, 0.0f, 10.0f);
            //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            ImGui::Checkbox("Bloom", &bloom);
            ImGui::SliderFloat("tone", &tone, 0.0f, 10.0f);
            ImGui::SliderFloat("bloomStrength", &bloomStrength, 0.0f, 5.0f);
            ImGui::Checkbox("tone mapping", &tonemapping);
            ImGui::Checkbox("gamma correction", &gammaCorrection);
            
            ImGui::SliderFloat("gamma", &gamma, 0.0f, 5.0f);
            
            //ImGui::SameLine();

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &rayVAO);
    glDeleteBuffers(1, &rayEBO);
    // imgui
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

#pragma region "Load texture"
// Loads a cubemap texture from 6 individual texture faces
// Order should be:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
GLuint loadCubemap(vector<const GLchar*> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glActiveTexture(GL_TEXTURE0);

    int width, height;
    unsigned char* image;

    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    for (GLuint i = 0; i < faces.size(); i++)
    {
        image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
            GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}

GLuint loadTexture(const GLchar* path)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height;
    unsigned char* data = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
    if (data)
    {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
    }

    return textureID;
}
#pragma endregion

#pragma region "User input"

// Moves/alters the camera positions based on user input
void Do_Movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);

}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (action == GLFW_PRESS)
        keys[key] = true;
    else if (action == GLFW_RELEASE)
        keys[key] = false;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        buttons[button] = true;
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        buttons[button] = false;

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    //camera.ProcessMouseMovement(xoffset, yoffset);
    
    float theta = 2 * PI * ( (GLfloat)xoffset / screenWidth );
    float alpha = 2 * PI * ( (GLfloat)yoffset / screenHeight );

    if (buttons[GLFW_MOUSE_BUTTON_RIGHT]) {
        camera.RotateAxisY(theta);
        camera.RotateAxisX(alpha);
    }
   
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    //camera.ProcessMouseScroll(yoffset);
}

#pragma endregion