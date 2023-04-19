#pragma once

// Std. Includes
#include <vector>

// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 3.0f;
const GLfloat SENSITIVTY = 0.25f;
const GLfloat ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Eular Angles
    GLfloat Yaw;
    GLfloat Pitch;
    // Camera options
    GLfloat MovementSpeed;
    GLfloat MouseSensitivity;
    GLfloat Zoom;

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
    {
        this->Position = position;
        this->WorldUp = up;
        this->Yaw = yaw;
        this->Pitch = pitch;
        this->updateCameraVectors();
    }
    // Constructor with scalar values
    Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
    {
        this->Position = glm::vec3(posX, posY, posZ);
        this->WorldUp = glm::vec3(upX, upY, upZ);
        this->Yaw = yaw;
        this->Pitch = pitch;
        this->updateCameraVectors();
    }

    // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
    {
        GLfloat velocity = this->MovementSpeed * deltaTime;
        if (direction == FORWARD)
            this->Position += this->Front * velocity;
        if (direction == BACKWARD)
            this->Position -= this->Front * velocity;
        if (direction == LEFT)
            this->Position -= this->Right * velocity;
        if (direction == RIGHT)
            this->Position += this->Right * velocity;
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= this->MouseSensitivity;
        yoffset *= this->MouseSensitivity;

        this->Yaw += xoffset * 0.3;
        this->Pitch += yoffset * 0.3;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (this->Pitch > 89.0f)
                this->Pitch = 89.0f;
            if (this->Pitch < -89.0f)
                this->Pitch = -89.0f;
        }

        // Update Front, Right and Up Vectors using the updated Eular angles
        this->updateCameraVectors();
    }

    // Rotate AxisY
    void RotateAxisY(GLfloat theta)
    {
        //theta *= this->MouseSensitivity;
        theta *= 0.3;

        glm::mat4 rotate = this->RotateAxis(glm::vec3(0.0, 1.0, 0.0), theta);
        glm::mat4 def = this->RotateAxis(glm::vec3(0.0, 1.0, 0.0), theta);

        this->Position = glm::vec3(rotate * glm::vec4(this->Position, 1.0));
        this->Front = glm::vec3(rotate * glm::vec4(this->Front, 1.0));

        // Update Front, Right and Up Vectors using the updated Eular angles
        //this->updateCameraVectors();
    }
    
    // Rotate AxisX
    void RotateAxisX(GLfloat alpha)
    {
        //alpha *= this->MouseSensitivity;
        alpha *= -0.2;

        glm::mat4 rotate = this->RotateAxis(glm::vec3(1.0, 0.0, 0.0), alpha);
        glm::mat4 def = this->RotateAxis(glm::vec3(1.0, 0.0, 0.0), alpha);

        this->Position = glm::vec3(rotate * glm::vec4(this->Position, 1.0));
        this->Front = glm::vec3(rotate * glm::vec4(this->Front, 1.0));

        // Update Front, Right and Up Vectors using the updated Eular angles
        //this->updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(GLfloat yoffset)
    {
        if (this->Zoom >= 1.0f && this->Zoom <= 45.0f)
            this->Zoom -= yoffset * 0.1;
        if (this->Zoom <= 1.0f)
            this->Zoom = 1.0f;
        if (this->Zoom >= 45.0f)
            this->Zoom = 45.0f;
    }

private:
    // Calculates the front vector from the Camera's (updated) Eular Angles
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        front.y = sin(glm::radians(this->Pitch));
        front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        this->Front = glm::normalize(front);
        // Also re-calculate the Right and Up vector
        this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        this->Up = glm::normalize(glm::cross(this->Right, this->Front));
    }

    // Rotate any Axis
    glm::mat4 RotateAxis(glm::vec3 axis, GLfloat theta)
    {
        glm::vec3 a = axis;
        GLfloat c = cos(theta), s = sin(theta), p = 1 - cos(theta);

        // Move camera
        glm::mat4 rotate;
        rotate[0][0] = c + pow(a.x, 2) * p; rotate[0][1] = a.x * a.y * p - a.z * s; rotate[0][2] = a.x * a.z * p + a.y * s; rotate[0][3] = 0.0f;
        rotate[1][0] = a.y * a.x * p + a.z * s; rotate[1][1] = c + pow(a.y, 2) * p; rotate[1][2] = a.y * a.z * p - a.x * s; rotate[1][3] = 0.0f;
        rotate[2][0] = a.z * a.x * p - a.y * s; rotate[2][1] = a.z * a.y * p + a.x * s; rotate[2][2] = c + pow(a.z, 2) * p; rotate[2][3] = 0.0f;
        rotate[3][0] = 0.0f; rotate[3][1] = 0.0f; rotate[3][2] = 0.0f; rotate[3][3] = 1.0f;

        return rotate;
    }
};