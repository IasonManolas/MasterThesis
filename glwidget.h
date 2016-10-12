#ifndef GLWIDGET_H
#define GLWIDGET_H


// Include GLM
#include <glm/glm.hpp>

#define GLEW_STATIC
#include <GL/glew.h>
using namespace glm;

#include <QOpenGLWidget>
#include <QTimer>
#include <shader.h>
class GLWidget:public QOpenGLWidget
{
public:
    explicit GLWidget(QWidget *parent = 0);
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    Shader *shaderObject;
    GLuint shaderProgram;
    GLuint VAO;
    QTimer timer;
    GLuint texture1;
    GLuint texture2;

};

#endif // GLWIDGET_H
