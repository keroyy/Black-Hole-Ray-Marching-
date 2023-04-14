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

// Other includes
#include "Shader.h"
#include "Camera.h"

// Properties
GLuint screenWidth = 1600, screenHeight = 900;
const float PI = 3.1415926;

GLuint loadCubemap(vector<const GLchar*> faces);

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

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
    glfwSetScrollCallback(window, scroll_callback);

    // Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLEW to setup the OpenGL Function pointers
    glewExperimental = GL_TRUE;
    glewInit();

    // Define the viewport dimensions
    glViewport(0, 0, screenWidth, screenHeight);

    // Setup some OpenGL options
    glEnable(GL_DEPTH_TEST);

    // Setup and compile our shaders
    //Shader shader("myShader.vs", "myShader.frag");
    //Shader skyboxShader("skybox.vs", "skybox.frag");
    Shader rayTrackingShader("blackhole.vs", "blackhole.frag");

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

    // sphere
    const float r = 0.8f;
    const int stacks = 50, sectors = 2 * stacks;
    vector<float>vertices;
    vector<int>indices;

    for (int j = 0; j <= stacks; j++) {
        float v = (float)j / stacks;
        float phi = PI * v;

        for (int i = 0; i <= sectors; i++) {
            float u = (float)i / sectors;
            float theta = 2 * PI * u;

            vertices.push_back(r * sin(phi) * cos(theta));
            vertices.push_back(r * sin(phi) * sin(theta));
            vertices.push_back(r * cos(phi));
        }
    }

    for (int j = 0; j < stacks; j++) {
        int k1 = j * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int i = 0; i < sectors; i++, k1++, k2++) {
            if (j != 0) {
                indices.push_back(k1);
                indices.push_back(k1 + 1);
                indices.push_back(k2);
            }
            if (j != stacks - 1) {
                indices.push_back(k2);
                indices.push_back(k1 + 1);
                indices.push_back(k2 + 1);
            }
        }
    }

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

    // Setup sphere VAO
    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0); // Unbind VAO

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

#pragma endregion

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Set frame time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check and call events
        glfwPollEvents();
        Do_Movement();

        // Clear the colorbuffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw skybox first
        //glDepthMask(GL_FALSE);// Remember to turn depth writing off
        //skyboxShader.Use();
        //view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
        //projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
        //ratote = glm::rotate(ratote, (GLfloat)glfwGetTime() * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));    // rotate skybox
        //glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        //glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        //glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "ratote"), 1, GL_FALSE, glm::value_ptr(ratote));
        //// skybox cube
        //glBindVertexArray(skyboxVAO);
        //glActiveTexture(GL_TEXTURE0);
        //glUniform1i(glGetUniformLocation(rayTrackingShader.Program, "skybox"), 0);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        //glDrawArrays(GL_TRIANGLES, 0, 36);
        //glBindVertexArray(0);
        //glDepthMask(GL_TRUE);

        //// Then draw scene as normal
        //shader.Use();
        //view = camera.GetViewMatrix();
        //projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
        //glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        //glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        //glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        //// sphere
        //glBindVertexArray(VAO);
        //glDrawElements(GL_TRIANGLES, stacks * sectors * 6, GL_UNSIGNED_INT, 0);
        //glBindVertexArray(0);

        // ray tracking
        rayTrackingShader.Use();

        // Initialize matrix
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        // TODO:test
        // Camera property
        glm::vec3 lower_left_corner = glm::vec3(0.0f);// 左下角
        glm::vec3 horizontal = glm::vec3(0.0f);// 水平
        glm::vec3 vertical = glm::vec3(0.0f);// 垂直

        GLfloat aspect = (GLfloat)screenWidth / (GLfloat)screenHeight;
        GLfloat near = 1.0f;
        GLfloat far = 100.0f;
        /*horizontal = glm::vec3(2 * ((far - near) / 2 + near) * tan(camera.Zoom / 2), 0.0, 0.0);
        vertical = glm::vec3(0.0, aspect * horizontal.x, 0.0);
        lower_left_corner = glm::vec3(-horizontal.x / 2, -vertical.y / 2, -((far - near) / 2 + near));*/
        horizontal = glm::vec3(2 * near * tan(camera.Zoom / 2), 0.0, 0.0);
        vertical = glm::vec3(0.0, (1 / aspect) * horizontal.x, 0.0);
        lower_left_corner = glm::vec3(-horizontal.x / 2, -vertical.y / 2, -near);

        // glDepthMask(GL_FALSE);// Remember to turn depth writing off
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
        projection = glm::perspective(camera.Zoom, aspect, near, far);
        //ratote = glm::rotate(ratote, (GLfloat)glfwGetTime() * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));    // rotate skybox
        glUniformMatrix4fv(glGetUniformLocation(rayTrackingShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(rayTrackingShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        //glUniformMatrix4fv(glGetUniformLocation(rayTrackingShader.Program, "ratote"), 1, GL_FALSE, glm::value_ptr(ratote));
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(rayTrackingShader.Program, "skybox"), 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        //glDepthMask(GL_TRUE);

        glUniform1f(glGetUniformLocation(rayTrackingShader.Program, "time"), (GLfloat)glfwGetTime() * 0.07f);
        glUniform3f(glGetUniformLocation(rayTrackingShader.Program, "camera.lower_left_corner"), lower_left_corner.x, lower_left_corner.y, lower_left_corner.z);
        glUniform3f(glGetUniformLocation(rayTrackingShader.Program, "camera.horizontal"), horizontal.x, horizontal.y, horizontal.z);
        glUniform3f(glGetUniformLocation(rayTrackingShader.Program, "camera.vertical"), vertical.x, vertical.y, vertical.z);
        glUniform3f(glGetUniformLocation(rayTrackingShader.Program, "camera.origin"), 0.0, 0.0, 0.0);
        

        glBindVertexArray(rayVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Swap the buffers
        glfwSwapBuffers(window);
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &rayVAO);
    glDeleteBuffers(1, &rayEBO);

    glfwTerminate();
    return 0;
}

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
            GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}

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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    //camera.ProcessMouseScroll(yoffset);
}

#pragma endregion