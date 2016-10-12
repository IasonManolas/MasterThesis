#include "glwidget.h"
// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <QDebug>
#include <QOpenGLContext>
#include <QCoreApplication>
#include <QTime>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//SOIL Image Loader
#include <SOIL.h>
// Window dimensions
#define  WIDTH GLuint(800)
#define  HEIGHT GLuint(600)

float thetaDegrees=0;
GLWidget::GLWidget(QWidget *parent):shaderObject() //I have to constructors for class Shader in order
  //to use the "real" one after the opengl context is active
{

    QOpenGLWidget *widget=this;
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(4, 1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    widget->setFormat(format);
}
void GLWidget::initializeGL()
{

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    GLenum err = glewInit();
    if( GLEW_OK != err ){
    qDebug() << "[Error] GLEW failed to initialize. " << (const char*)glewGetErrorString(err);
    }
    glViewport(0,0,WIDTH,HEIGHT);

    glEnable(GL_DEPTH_TEST);

    //I have to constructors for class Shader in order
    //to use the "real" one after the opengl context is active
    //IS THIS SYNTAX OS dependant? build dir must be in the same folder(aka Projects) as the sources(aka OpenGL_WithoutWrappers)
    shaderObject=new Shader("../OpenGL_WithoutWrappers/shaders/vertex.glsl","../OpenGL_WithoutWrappers/shaders/fragment.glsl");

        GLfloat vertices[] = {
            0.5f, -0.5f,  0.5f, 	1.0f, 0.0f, //FBR 0
           -0.5f, -0.5f,  0.5f, 	0.0f, 0.0f, //FBL 1
            0.5f,  0.5f,  0.5f, 	1.0f, 1.0f, //FTR 2
           -0.5f,  0.5f,  0.5f, 	0.0f, 1.0f, //FTL 3

            0.5f, -0.5f, -0.5f, 	1.0f, 0.0f, //BBR 4
           -0.5f, -0.5f, -0.5f, 	0.0f, 0.0f, //BBL 5
            0.5f,  0.5f, -0.5f, 	1.0f, 1.0f, //BTR 6
           -0.5f,  0.5f, -0.5f, 	0.0f, 1.0f //BTL 7
            };
        GLuint indices[] = {  // Note that we start from 0!
                2, 3, 1,  //Triangle 1
                1, 0, 2,  // Triangle 2

                4, 7, 6,  // Triangle 3
                7, 4, 5,  // Triangle 4

                2, 6, 7,  // Triangle 5
                7, 3, 2,  // Triangle 6

                0, 1, 5,  // Triangle 7
                5, 4, 0,  // Triangle 8

                1, 3, 7,  // Triangle 9
                1, 7, 5,  // Triangle 10

                2, 0, 4,  // Triangle 11
                4, 6, 2  // Triangle 12
            };
    GLuint VBO,EBO;

    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glGenBuffers(1,&EBO);

    //Bind the Vertex Array Object first,then bind and set vertex buffers and attribute pointers (WHY?)
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(GLfloat),(GLvoid*)0);
    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(GLfloat),(GLvoid*)(3*sizeof(GLfloat)));
//    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,5*sizeof(GLfloat),(GLvoid*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(2);


    glBindBuffer(GL_ARRAY_BUFFER,0);

    glBindVertexArray(0); // Unbind VAO

    // Load and create a texture
     // ====================
    // Texture 1
    // ====================
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load, create texture and generate mipmaps
    int width, height;
    unsigned char* image = SOIL_load_image("../OpenGL_WithoutWrappers/container.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
    // ===================
    // Texture 2
    // ===================
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load, create texture and generate mipmaps
    image = SOIL_load_image("../OpenGL_WithoutWrappers/awesomeface.png", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    connect(&timer,SIGNAL(timeout()),this,SLOT(update()));
            timer.start(30);

}
void GLWidget::resizeGL(int w, int h)
{

}

void GLWidget::paintGL()
{
    //Render
    //Clear the colorbuffer
    glClearColor(0.2f,0.3f,0.3f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind Textures using texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glUniform1i(glGetUniformLocation(shaderObject->programID, "ourTexture1"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glUniform1i(glGetUniformLocation(shaderObject->programID, "ourTexture2"), 1);

    shaderObject->Use();

    thetaDegrees+=1.0f;
    // Create transformations
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    model = glm::rotate(model, glm::radians(thetaDegrees), glm::vec3(1.0f, 0.1f, 0.4f));
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    projection = glm::perspective(45.0f, (GLfloat)(WIDTH / HEIGHT), 0.1f, 100.0f);
    // Get their uniform location
    GLint modelLoc = glGetUniformLocation(shaderObject->programID, "model");
    GLint viewLoc = glGetUniformLocation(shaderObject->programID, "view");
    GLint projLoc = glGetUniformLocation(shaderObject->programID, "projection");
    // Pass them to the shaders
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    // Note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
    glBindVertexArray(0);

    //remember to create a destroy widget function for delete Shader, DeleteVertexArrays and DeleteBuffers

}
