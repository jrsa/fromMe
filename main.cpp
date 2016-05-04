#include <glbinding/Binding.h>
#include <glbinding/gl/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <chrono>
#include <glog/logging.h>

#include "kinect.h"
#include "shader.hpp"
#include "simple_file.hpp"
#include "vertex_format.h"

#define VS_FN "/Users/jrsa/code/gl/xformFb/1.vs.glsl"
#define FS_FN "/Users/jrsa/code/gl/xformFb/1.fs.glsl"

using namespace gl;
using namespace std::chrono;
using namespace simple_file;


std::string f_fn, v_fn;
GLFWwindow *g_window; shader *s; kinect k; bool seed = false;

void keycb(GLFWwindow* window, int key, int , int , int ) {
  switch(key) {
    case 'R': {
      LOG(INFO) << "reloading shader";
      s = new shader(read(v_fn).c_str(), read(f_fn).c_str());
      break;
    }
    case 'P': {
      seed = true;
    }
    default: {
      break;
    }
  }
}

int main(int argc, char **argv) {

  f_fn = std::string(FS_FN);
  v_fn = std::string(VS_FN);

  bool kinect_isgood = false;
  uint16_t *depths = nullptr;
  try {
    k.setup();

    int depthsize = k.get_buffer_size();
    depths = new uint16_t[depthsize];
    memset(depths, 0, depthsize);

    kinect_isgood = true;
  }
  catch (...)  {
    LOG(ERROR) << "running with no kinect";
  }

  glbinding::Binding::initialize(false);

  if (!glfwInit()) {
    LOG(FATAL) << "failed to initialize glfw";
  }

  glfwDefaultWindowHints();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  g_window = glfwCreateWindow(640, 480, argv[0], nullptr, nullptr);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  s = new shader(read(v_fn).c_str(), read(f_fn).c_str());

  GLint u_time = glGetUniformLocation(s->program(), "time");
  GLint u_mouse_pos = glGetUniformLocation(s->program(), "mousePos");
  GLint u_depth = glGetUniformLocation(s->program(), "depth");

  GLuint depth_texture = 0;
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &depth_texture);
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)GL_NEAREST);

  GLuint fb_vao;
  glGenVertexArrays(1, &fb_vao);
  glBindVertexArray(fb_vao);

  int field_width(800);
  int field_area = field_width * field_width;

  vert *orig_points = new vert[field_area];
  vert *point_data = new vert[field_area];

  for (int y = 0; y < field_width; y++) {
    for (int x = 0; x < field_width; x++) {
      glm::vec2 position = { (1.5 / field_width) * x - 0.7f, (1.5 / field_width) * y - 0.7f };
      point_data[field_width * y + x] = { position, { 0.0, 0.0 }, position };
      orig_points[field_width * y + x] = { position, { 0.0, 0.0 }, position };
    }
  }

  GLuint xform_in_buf;
  glGenBuffers(1, &xform_in_buf);
  glBindBuffer(GL_ARRAY_BUFFER, xform_in_buf);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vert) * field_area, point_data, GL_STREAM_DRAW);

  GLint a_position = glGetAttribLocation(s->program(), "position");
  glEnableVertexAttribArray(a_position);
  glVertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);

  GLint a_velocity = glGetAttribLocation(s->program(), "velocity");
  glEnableVertexAttribArray(a_velocity);
  glVertexAttribPointer(a_velocity, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *) sizeof(glm::vec2));

  GLint a_original_pos = glGetAttribLocation(s->program(), "originalPos");
  glEnableVertexAttribArray(a_original_pos);
  glVertexAttribPointer(a_original_pos, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void *) (sizeof(glm::vec2) * 2));

  GLuint xform_out_buf;
  glGenBuffers(1, &xform_out_buf);
  glBindBuffer(GL_ARRAY_BUFFER, xform_out_buf);
  glBufferData(GL_ARRAY_BUFFER, field_area * sizeof(fb_vert), nullptr, GL_STATIC_READ);

  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xform_out_buf);
  fb_vert *feedback_buffer = new fb_vert[field_area];
  glBindBuffer(GL_ARRAY_BUFFER, xform_in_buf);

  glPointSize(3.0);

  glfwSetKeyCallback(g_window, keycb);

  auto t_prev = high_resolution_clock::now();
  glfwMakeContextCurrent(g_window);

  while (!glfwWindowShouldClose(g_window)) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    s->use();

    auto t_now = high_resolution_clock::now();
    float time = duration_cast<duration<float>>(t_now - t_prev).count();
    t_prev = t_now;
    glUniform1f(u_time, time);

    double mouseX, mouseY; int h, w = 0;
    glfwGetCursorPos(g_window, &mouseX, &mouseY);
    glfwGetWindowSize(g_window, &w, &h);

    float mousePosX = mouseX / w;
    float mousePosY = mouseY / h;

    glUniform2f(u_mouse_pos, mousePosX, mousePosY);

    if (kinect_isgood) {
      glTexImage2D(GL_TEXTURE_2D, 0, (GLint) GL_RED, 640, 480, 0, GL_RED, GL_UNSIGNED_SHORT, k.get_depthmap_pointer());
    }

    glUniform1i(u_depth, 0);

    if(seed) {
      glBufferData(GL_ARRAY_BUFFER, sizeof(vert) * field_area, orig_points, GL_STREAM_DRAW);
      seed = false;
    }

    glBeginTransformFeedback(GL_POINTS);
      glDrawArrays(GL_POINTS, 0, field_area);
    glEndTransformFeedback();

    glfwSwapBuffers(g_window);

    // feed vertex output back into input
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(fb_vert) * field_area, feedback_buffer);
    for (int i = 0; i < field_area; i++) {
      point_data[i].position = feedback_buffer[i].outPosition;
      point_data[i].velocity = feedback_buffer[i].outVelocity;
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vert) * field_area, point_data);

    glfwPollEvents();
  }

  glDeleteTextures(1, &depth_texture);

  glDeleteBuffers(1, &xform_out_buf);
  glDeleteBuffers(1, &xform_in_buf);

  glDeleteVertexArrays(1, &fb_vao);

  glfwTerminate();
  return 0;
}