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

GLFWwindow *g_window;

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

  

  // Compile shaders
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
  glCompileShader(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
  glCompileShader(fragmentShader);

  // Create program and specify transform feedback variables
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);

  const GLchar *feedbackVaryings[] = {"outPosition", "outVelocity"};
  glTransformFeedbackVaryings(program, 2, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);

  glLinkProgram(program);
  glUseProgram(program);

  GLint uniTime = glGetUniformLocation(program, "time");
  GLint uniMousePos = glGetUniformLocation(program, "mousePos");

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLfloat data[600] = {};

  for (int y = 0; y < 10; y++) {
    for (int x = 0; x < 10; x++) {
      data[60 * y + 6 * x] = 0.2f * x - 0.9f;
      data[60 * y + 6 * x + 1] = 0.2f * y - 0.9f;
      data[60 * y + 6 * x + 4] = 0.2f * x - 0.9f;
      data[60 * y + 6 * x + 5] = 0.2f * y - 0.9f;
    }
  }

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STREAM_DRAW);

  GLint posAttrib = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);

  GLint velAttrib = glGetAttribLocation(program, "velocity");
  glEnableVertexAttribArray(velAttrib);
  glVertexAttribPointer(velAttrib, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));

  GLint origPosAttrib = glGetAttribLocation(program, "originalPos");
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

  auto t_prev = std::chrono::high_resolution_clock::now();

  glfwMakeContextCurrent(g_window);

  while (!glfwWindowShouldClose(g_window)) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Calculate delta time
    auto t_now = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_prev).count();
    t_prev = t_now;
    glUniform1f(uniTime, time);

    // Update mouse position
    double mouseX, mouseY; int h, w = 0;
    glfwGetCursorPos(g_window, &mouseX, &mouseY);
    glfwGetWindowSize(g_window, &w, &h);
    
    glUniform2f(uniMousePos, mouseX / w, mouseY/ h);

    // Perform feedback transform and draw vertices
    glBeginTransformFeedback(GL_POINTS);
      glDrawArrays(GL_POINTS, 0, 100);
    glEndTransformFeedback();

    glfwSwapBuffers(g_window);

    
    
    // Update vertices' position and velocity using transform feedback
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback),
                       feedback);

    for (int i = 0; i < 100; i++) {
      data[6 * i] = feedback[4 * i];
      data[6 * i + 1] = feedback[4 * i + 1];
      data[6 * i + 2] = feedback[4 * i + 2];
      data[6 * i + 3] = feedback[4 * i + 3];
    }

    // glBufferData() would reallocate the whole vertex data buffer, which is
    // unnecessary here.
    // glBufferSubData() is used instead - it updates an existing buffer.
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);

    glfwPollEvents();
  }

  glDeleteProgram(program);
  glDeleteShader(fragmentShader);
  glDeleteShader(vertexShader);

  glDeleteBuffers(1, &tbo);
  glDeleteBuffers(1, &vbo);

  glDeleteVertexArrays(1, &vao);

  glfwTerminate();
  return 0;
}