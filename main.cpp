#include <glbinding/Binding.h>
#include <glbinding/gl/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLM_COMPILER 0
#include <glm/glm.hpp>

#include <chrono>
#include <glog/logging.h>

#include "shader.hpp"
#include "simple_file.hpp"

using namespace gl;
using namespace std::chrono;
using namespace glm;

GLFWwindow *g_window;

struct vert {
    vec2 position;
    vec2 velocity;
    vec2 originalPos;
};

struct fbVert {
    vec2 outPosition;
    vec2 outVelocity;
};

int main(int argc, char **argv) {
  glbinding::Binding::initialize(false);

  if (!glfwInit()) {
    LOG(FATAL) << "failed to initialize glfw";
  }

  glfwDefaultWindowHints();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  g_window = glfwCreateWindow(640, 480, "glfw_app", nullptr, nullptr);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  std::string vs_src(simple_file::read("/Users/jrsa/code/gl/xformFb/physExample.vs.glsl"));
  std::string fs_src(simple_file::read("/Users/jrsa/code/gl/xformFb/physExample.fs.glsl"));
  shader s(vs_src.c_str(), fs_src.c_str());

  GLint uniTime = glGetUniformLocation(s.program(), "time");
  GLint uniMousePos = glGetUniformLocation(s.program(), "mousePos");

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  int field_width(10);
  int field_area = field_width * field_width;

  GLfloat data[600] = {};
  vert *point_data = new vert[field_width * field_width];

  for (int y = 0; y < 10; y++) {
    for (int x = 0; x < 10; x++) {
      data[60 * y + 6 * x] = 0.2f * x - 0.9f;
      data[60 * y + 6 * x + 1] = 0.2f * y - 0.9f;
      data[60 * y + 6 * x + 4] = 0.2f * x - 0.9f;
      data[60 * y + 6 * x + 5] = 0.2f * y - 0.9f;
    }
  }

  for (int y = 0; y < field_width; y++) {
    for (int x = 0; x < field_width; x++) {
      vec2 position = { 0.2f * x - 0.9f, 0.2f * y - 0.9f };
      point_data[10 * y + x] = { position, { 0.0, 0.0 }, position };
    }
  }

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), point_data, GL_STREAM_DRAW);

  GLint posAttrib = glGetAttribLocation(s.program(), "position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);

  GLint velAttrib = glGetAttribLocation(s.program(), "velocity");
  glEnableVertexAttribArray(velAttrib);
  glVertexAttribPointer(velAttrib, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));

  GLint origPosAttrib = glGetAttribLocation(s.program(), "originalPos");
  glEnableVertexAttribArray(origPosAttrib);
  glVertexAttribPointer(origPosAttrib, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(4 * sizeof(GLfloat)));

  GLuint tbo;
  glGenBuffers(1, &tbo);
  glBindBuffer(GL_ARRAY_BUFFER, tbo);
  glBufferData(GL_ARRAY_BUFFER, 400 * sizeof(GLfloat), nullptr, GL_STATIC_READ);

  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);
  GLfloat feedback[400];

  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glPointSize(5.0);

  auto t_prev = high_resolution_clock::now();

  glfwMakeContextCurrent(g_window);
  
  s.use();

  while (!glfwWindowShouldClose(g_window)) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.1f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto t_now = high_resolution_clock::now();
    float time = duration_cast<duration<float>>(t_now - t_prev).count();
    t_prev = t_now;
    glUniform1f(uniTime, time);

    double mouseX, mouseY; int h, w = 0;
    glfwGetCursorPos(g_window, &mouseX, &mouseY);
    glfwGetWindowSize(g_window, &w, &h);

    float mousePosX = mouseX / w;
    float mousePosY = mouseY / h;

    glUniform2f(uniMousePos, mousePosX, mousePosY);

    glBeginTransformFeedback(GL_POINTS);
      glDrawArrays(GL_POINTS, 0, field_width * field_width);
    glEndTransformFeedback();

    glfwSwapBuffers(g_window);

    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback), feedback);

    for (int i = 0; i < 100; i++) {
      data[6 * i] = feedback[4 * i];
      data[6 * i + 1] = feedback[4 * i + 1];
      data[6 * i + 2] = feedback[4 * i + 2];
      data[6 * i + 3] = feedback[4 * i + 3];
    }
    for (int i = 0; i < field_area; i++) {
      data[6 * i] = feedback[4 * i];
      data[6 * i + 1] = feedback[4 * i + 1];
      data[6 * i + 2] = feedback[4 * i + 2];
      data[6 * i + 3] = feedback[4 * i + 3];
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);

    glfwPollEvents();
  }

  glDeleteBuffers(1, &tbo);
  glDeleteBuffers(1, &vbo);

  glDeleteVertexArrays(1, &vao);

  glfwTerminate();
  return 0;
}