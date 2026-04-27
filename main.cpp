#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader.hpp"
#include "Vector.h"
#include "Matrix.h"

using namespace std;

const char *getError()
{
    const char *errorDescription;
    glfwGetError(&errorDescription);
    return errorDescription;
}

inline void startUpGLFW()
{
    glewExperimental = true;
    if (!glfwInit())
    {
        throw getError();
    }
}

inline void startUpGLEW()
{
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        glfwTerminate();
        throw getError();
    }
}

inline GLFWwindow *setUp()
{
    startUpGLFW();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window;
    window = glfwCreateWindow(1000, 1000, "Practical 4", NULL, NULL);

    if (window == NULL)
    {
        cout << getError() << endl;
        glfwTerminate();
        throw "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n";
    }

    glfwMakeContextCurrent(window);
    startUpGLEW();
    return window;
}

int main()
{
    GLFWwindow *window = setUp();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint CubeShader = LoadShaders("cube_vertex.glsl", "cube_fragment.glsl");

    vector<float> CubePoints =
    {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f
    };

    vector<unsigned int> CubeIndexes = {
        0, 1, 2,
        2, 3, 0,

        4, 5, 6,
        6, 7, 4,

        0, 3, 7,
        7, 4, 0,

        1, 5, 6,
        6, 2, 1,

        0, 1, 5,
        5, 4, 0,

        3, 2, 6,
        6, 7, 3
    };

    GLuint CubeVAO, CubeVBO, CubeEBO;

    glGenVertexArrays(1, &CubeVAO);
    glGenBuffers(1, &CubeVBO);
    glGenBuffers(1, &CubeEBO);

    glBindVertexArray(CubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, CubeVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 CubePoints.size() * sizeof(float),
                 CubePoints.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 CubeIndexes.size() * sizeof(unsigned int),
                 CubeIndexes.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);


    float angleY = 0.0f;
float angleX = 0.0f;
float angleZ = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            angleX = angleX + 0.03f;  
        }
         

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        angleX = angleX - 0.03f;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
         angleY = angleY + 0.03f; 
    }  

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        angleY = angleY - 0.03f;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        angleZ = angleZ + 0.03f;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        angleZ = angleZ - 0.03f;
    }



        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        

        Matrix<4,4> RotationX;
    Matrix<4,4> RotationY;
    Matrix<4,4> RotationZ;

    RotationX[0][0] = 1.0f;
    RotationX[1][1] = cosf(angleX);
    RotationX[1][2] = -sinf(angleX);
    RotationX[2][1] = sinf(angleX);
    RotationX[2][2] = cosf(angleX);
    RotationX[3][3] = 1.0f;

    RotationY[0][0] = cosf(angleY);
    RotationY[0][2] = sinf(angleY);
    RotationY[1][1] = 1.0f;
    RotationY[2][0] = -sinf(angleY);
    RotationY[2][2] = cosf(angleY);
    RotationY[3][3] = 1.0f;

    RotationZ[0][0] = cosf(angleZ);
    RotationZ[0][1] = -sinf(angleZ);
    RotationZ[1][0] = sinf(angleZ);
    RotationZ[1][1] = cosf(angleZ);
    RotationZ[2][2] = 1.0f;
    RotationZ[3][3] = 1.0f;

    Matrix<4,4> Scale;
    Scale[0][0] = 2.0f;
    Scale[1][1] = 1.5f;
    Scale[2][2] = 0.5f;
    Scale[3][3] = 1.0f;

    Matrix<4,4> gettingtheModel = RotationY * RotationX * RotationZ * Scale;

        

        Matrix<4,4> ToShowView;
        ToShowView[0][0] = 1.0f;
        ToShowView[1][1] = 1.0f;
        ToShowView[2][2] = 1.0f;
        ToShowView[2][3] = -3.0f;
        ToShowView[3][3] = 1.0f;

        Matrix<4,4> TheProjection;
        float fovY  = 3.14159f / 4.0f;
        float gettingZ = 0.1f;
        float gettingFromFar = 100.0f;
        float theFreq     = 1.0f / tanf(fovY * 0.5f);

        TheProjection[0][0] = theFreq;
        TheProjection[1][1] = theFreq;
        TheProjection[2][2] = (gettingFromFar + gettingZ) / (gettingZ - gettingFromFar);
        TheProjection[2][3] = (2.0f * gettingFromFar * gettingZ) / (gettingZ - gettingFromFar);
        TheProjection[3][2] = -1.0f;

        Matrix<4,4> MVP = TheProjection * (ToShowView * gettingtheModel);

        float MVPArray[16];
        for (int c = 0; c < 4; c++)
        {
            for (int r = 0; r < 4; r++)
            {
                 MVPArray[c * 4 + r] = MVP[r][c];
            }
               
        }
            

        glUseProgram(CubeShader);
        glUniformMatrix4fv(glGetUniformLocation(CubeShader, "MVP"), 1, GL_FALSE, MVPArray);

        glBindVertexArray(CubeVAO);
        glDrawElements(GL_TRIANGLES, CubeIndexes.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &CubeVAO);
    glDeleteBuffers(1, &CubeVBO);
    glDeleteBuffers(1, &CubeEBO);

    glDeleteProgram(CubeShader);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
