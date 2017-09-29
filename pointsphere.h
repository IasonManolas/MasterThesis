#ifndef POINTSPHERE_H
#define POINTSPHERE_H

#include <string>

#include "material.h"
#include "mesh.h"
class PointSphere {

public:
  PointSphere() {}

  void updateRadius(const CGALSurfaceMesh &onWhichMesh) {
    m_desiredRadius = m_radius_averageEdge_proportion *
                      meshMeasuring::findAverageEdgeLength(onWhichMesh);
  }

  // loads a model and sets the sphere's size
  void load(std::string filename) {
    resetMeshAttributes();
    std::tie(indices, vertices) =
        meshLoader::load(filename); // vertices contains coords & normals
    setupDrawingBuffers();

    // Find the model matrix that normalizes the mesh
    centerOfMass = meshMeasuring::findCenterOfMass(vertices);
    maxDim = meshMeasuring::findMaxDimension(vertices);
    updateModelMatrix();

    // buildPolygonMesh();
    // printMeshInformation();
  }
  void setPosition(double x, double y, double z) {
    CGALSurfaceMesh::Point p(x, y, z);
    m_position = p;
    updateModelMatrix();
  }

  void setPosition(CGALSurfaceMesh::Point newPosition) {
    m_position = newPosition;
    updateModelMatrix();
  }
  Kernel::Point_3 getPosition() { return m_position; }
  void handle_drawing(Shader *shader,
                      glm::mat4 meshModelMatrix = glm::mat4(1.0)) {
    shader->Use();
    setUniforms(shader, meshModelMatrix);
    Draw(shader);
  }

  void setColor(glm::vec3 color) {
    m_material = Material(color, color, color, 128 * 0.25);
  }

  void doubleSize() {
    m_desiredRadius *= 2;
    updateModelMatrix();
  }

private:
  GLuint VAO, VBO, EBO;
  double alphaValue{1.0};
  glm::mat4 modelMatrix;
  glm::vec3 centerOfMass;
  double maxDim;
  std::vector<MyVertex> vertices;
  std::vector<GLuint> indices;
  double m_desiredRadius;
  double m_radius_averageEdge_proportion{0.15};
  CGALSurfaceMesh::Point m_position;
  Material m_material{Material(glm::vec3(1, 0, 0), glm::vec3(1, 0, 0),
                               glm::vec3(1, 0, 0), 128 * 0.25)};

  void updateModelMatrix() // Model space -> World space
  {
    modelMatrix = glm::mat4{1.0};
    glm::vec3 translationVector(-centerOfMass + glm::vec3(m_position.x(),
                                                          m_position.y(),
                                                          m_position.z()));
    modelMatrix = glm::translate(modelMatrix, translationVector);

    double scaleFactor =
        m_desiredRadius /
        std::sqrt(pow(centerOfMass.x - vertices[0].Position.x, 2) +
                  pow(centerOfMass.y - vertices[0].Position.y, 2) +
                  pow(centerOfMass.z - vertices[0].Position.z, 2));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scaleFactor));
  }
  void setUniforms(Shader *shader,
                   glm::mat4 meshModelMatrix =
                       glm::mat4(1.0)) { // the second argument is optional and
                                         // it is used when pointsphere is used
                                         // to represent a meshes vertex
    m_material.setUniforms(shader);
    glUniformMatrix4fv(
        glGetUniformLocation(shader->programID, "model"), 1, GL_FALSE,
        glm::value_ptr(meshModelMatrix * modelMatrix)); // TODO this
                                                        // multiplication
                                                        // affects the radius
                                                        // since meshModelMatrix
                                                        // scales it even
                                                        // further
    glUniform1f(glGetUniformLocation(shader->programID, "alpha"), alphaValue);
  } // fix it by breaking the meshModelMatrix into two parts. One translates and
  // one scales and pass here only the translation part.

  void Draw(Shader *shader) {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
  void setupDrawingBuffers() {

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MyVertex),
                 &vertices[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                 &indices[0], GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MyVertex),
                          (GLvoid *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MyVertex),
                          (GLvoid *)offsetof(MyVertex, Normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MyVertex),
                          (GLvoid *)offsetof(MyVertex, Color));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    // std::cout << "printDebugInformation was called in "<<__func__<<std::endl;
    //     // printDebugInformation();
    //
  }
  void resetMeshAttributes() {
    indices.clear();
    vertices.clear();
    centerOfMass = glm::vec3(1.0);
    modelMatrix = glm::mat4(1.0);
  }
};
#endif
