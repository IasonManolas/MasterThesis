#include "scene.h"

void Scene::Draw(Shader *modelShader, Shader *axisShader) {
  setSceneUniforms(modelShader, axisShader);
  M.handle_drawing(modelShader);
  if (m_showPointSpheresOnVertices) {
    modelShader->Use();
    for (PointSphere ps : m_pointSpheresOnVertices) {
      ps.handle_drawing(modelShader, M.getModelMatrix());
    }
  }
  if (showAxes) {
    axisShader->Use();
    sceneAxes.Draw();
  }
}

void Scene::updateProjectionMatrix(int w, int h) {
  projectionMatrix =
      glm::perspective(camera.fov, float(w) / float(h), nearPlane, farPlane);
}

void Scene::handle_cameraZoomChange(float delta) {
  camera.processWheelMovement(delta);
}

void Scene::handle_cameraReset() {
  camera.resetCamera();
  light.changeLightDirection(camera.getPosition());
}

void Scene::initializeScene() {
  // loadMesh("../Models/Small/test.obj");
  loadMesh("../Models/tyra.obj");
  loadPointSphere();
  // loadMesh("../Models/cylinder.obj");
  // loadMesh("../Models/Small/coctel.obj");
  // loadMesh("../Models/Small/Wrong by assimp/stretched cube.obj");
  // loadMesh("../Models/teapot.obj");
  // loadMesh("../Models/icosahedron.obj");

  PS.setPosition(0, 0, 0);
}

void Scene::loadPointSphere() { PS.load("../Models/Small/icosahedron.obj"); }

void Scene::handle_segmentSelection(float mousePosX, float mousePosY,
                                    int windowWidth, int windowHeight) {

  Ray_intersection intersection;
  bool intersectionFound = rayIntersectsPolyhedron(
      mousePosX, mousePosY, windowWidth, windowHeight, intersection);
  if (intersectionFound) {
    M.handle_segmentSelection(intersection);
  }
}

void Scene::handle_showSegments() { M.handle_showSegments(); }

void Scene::handle_segmentContraction() { M.handle_segmentContraction(); }

void Scene::handle_meshContraction() { contractMesh(); }

void Scene::handle_meshInflation() { M.handle_inflation(); }

void Scene::handle_meshDeflation() { M.handle_deflation(); }

void Scene::loadMesh(std::__cxx11::string filename) {
  camera.resetCamera();
  light.changeLightDirection(camera.getPosition());

  M = Mesh{};
  M.load(filename);

  PS.updateRadius(M.M);
}

void Scene::handle_axesStateChange(bool value) { showAxes = value; }

bool Scene::rayIntersectsPolyhedron(const int &mouseX, const int &mouseY,
                                    const int width, const int height,
                                    Ray_intersection &intersection) {
  glm::vec3 camPos = camera.getPosition();
  float x = 2.0 * mouseX / width - 1;
  float y = 1 - (2.0 * mouseY) / height;
  float z = -1; // we want the ray to point into the screen
  glm::vec3 ndcs(x, y, z);
  // NDCSpace -> clipSpace
  glm::vec4 ray4_clipSpace(ndcs.x, ndcs.y, ndcs.z, 1.0);
  // clipSpace -> eyeSpace
  glm::vec4 ray4_eyeSpace = glm::inverse(projectionMatrix) * ray4_clipSpace;
  ray4_eyeSpace = glm::vec4(ray4_eyeSpace.x, ray4_eyeSpace.y, -1.0, 0.0);
  // eyeSpace -> worldSpace
  glm::vec4 ray4_worldSpace =
      glm::inverse(camera.getViewMatrix()) * ray4_eyeSpace;

  // worldSpace -> modelSpace
  glm::vec4 ray4_modelSpace =
      glm::inverse(M.getModelMatrix()) * ray4_worldSpace;

  glm::vec3 ray3_modelSpace(ray4_modelSpace.x, ray4_modelSpace.y,
                            ray4_modelSpace.z);
  ray3_modelSpace = glm::normalize(ray3_modelSpace);

  glm::vec4 camPos4_worldSpace = glm::vec4(camPos, 1.0);
  glm::vec4 camPos4_modelSpace =
      glm::inverse(M.getModelMatrix()) * camPos4_worldSpace;
  glm::vec3 camPos3_modelSpace(camPos4_modelSpace.x, camPos4_modelSpace.y,
                               camPos4_modelSpace.z);

  // glm ray -> CGAL ray
  CGAL::Ray_3<Kernel> ray_modelSpace(
      Kernel::Point_3(camPos3_modelSpace.x, camPos3_modelSpace.y,
                      camPos3_modelSpace.z),
      Kernel::Direction_3(ray3_modelSpace.x, ray3_modelSpace.y,
                          ray3_modelSpace.z));

  intersection = M.intersects(ray_modelSpace);

  if (intersection) {
    return true;
  }
  return false;
}

void Scene::updatePointSpheresOnVerticesPositions() {
  m_pointSpheresOnVertices.clear();
  // std::vector<size_t> highLVertices =
  //    M.getVertexIndicesWithHighLaplacianValue();
  std::vector<size_t> fixedVertices = M.MC.getFixedVertices();
  for (const size_t v : fixedVertices) {
    PointSphere tempPS = PS;
    auto p(M.M.point(CGALSurfaceMesh::vertex_index(v)));
    tempPS.setPosition(p);
    tempPS.setColor(glm::vec3(1, 0, 0));
    tempPS.doubleSize();
    tempPS.doubleSize();
    m_pointSpheresOnVertices.push_back(tempPS);
  }
  // for (const auto v : M.M.vertices()) {
  //   PointSphere tempPS = PS;
  //   auto p(M.M.point(v));
  //   tempPS.setPosition(p);
  //   // if (std::find(highLVertices.begin(), highLVertices.end(), size_t(v))
  //   !=
  //   //     highLVertices.end()) {
  //   //   tempPS.setColor(glm::vec3(0, 1, 0));
  //   //   tempPS.doubleSize();
  //   //   tempPS.doubleSize();
  //   //   tempPS.doubleSize();
  //   // }
  //   m_pointSpheresOnVertices.push_back(tempPS);
  // }
  // for (size_t v : highLVertices) {
  //   PointSphere tempPS = PS;
  //   auto p(M.M.point(CGALSurfaceMesh::vertex_index(v)));
  //   tempPS.setPosition(p);
  //   if (std::find(highLVertices.begin(), highLVertices.end(), v) !=
  //       highLVertices.end()) {
  //     tempPS.setColor(glm::vec3(0, 1, 0));
  //     tempPS.doubleSize();
  //     tempPS.doubleSize();
  //   }
  //   m_pointSpheresOnVertices.push_back(tempPS);
  // }
  // for (size_t v : highLVerticesInPreviousIteration) {
  //   PointSphere tempPS = PS;
  //   auto p(M.M.point(CGALSurfaceMesh::vertex_index(v)));
  //   tempPS.setPosition(p);
  //   if (std::find(highLVertices.begin(), highLVertices.end(), v) !=
  //       highLVertices.end()) {
  //     tempPS.setColor(glm::vec3(1, 0, 0));
  //     tempPS.doubleSize();
  //     tempPS.doubleSize();
  //   }
  //   m_pointSpheresOnVertices.push_back(tempPS);
  // }
  // highLVerticesInPreviousIteration = highLVertices;
}

void Scene::handle_mouseMovement(const QVector2D &mouseDV) {
  camera.processMouseMovement(mouseDV);
  light.changeLightDirection(camera.getPosition());
}

void Scene::handle_meshVerticesStateChange(int state) {
  m_showPointSpheresOnVertices = bool(state);
  updatePointSpheresOnVerticesPositions();
}

void Scene::handle_saveModel(const std::__cxx11::string destinationDirectory) {
  M.handle_saveModel(destinationDirectory);
}

void Scene::contractMesh() {
  M.handle_meshContraction();
  if (m_showPointSpheresOnVertices)
    updatePointSpheresOnVerticesPositions();
}

void Scene::setSceneUniforms(Shader *modelShader, Shader *axisShader) {
  modelShader->Use();
  light.setUniforms(modelShader);
  camera.setUniforms(modelShader);
  setProjectionMatrixUniform(modelShader);

  axisShader->Use();
  setProjectionMatrixUniform(axisShader);
  glUniformMatrix4fv(glGetUniformLocation(axisShader->programID, "model"), 1,
                     GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
  camera.setUniforms(axisShader);
}

void Scene::setProjectionMatrixUniform(Shader *shader) {
  glUniformMatrix4fv(glGetUniformLocation(shader->programID, "projection"), 1,
                     GL_FALSE, glm::value_ptr(projectionMatrix));
}
