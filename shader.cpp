#include "shader.hpp"
#include <fstream>

using namespace gl;

void compile_info(const GLuint shader) {
  GLint status(0);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if (1 != status) {
    GLint maxLength(0);
    GLint logLength(0);

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    GLchar *log = new GLchar[maxLength];
    glGetShaderInfoLog(shader, maxLength, &logLength, log);

    LOG(FATAL) << "glsl compile error: " << log;
  }
}

void link_info(const GLuint program) {
  GLint status(0);
  glGetProgramiv(program, GL_LINK_STATUS, &status);

  if (1 != status) {
    GLint maxLength(0);
    GLint logLength(0);

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

    GLchar *log = new GLchar[maxLength];
    glGetProgramInfoLog(program, maxLength, &logLength, log);

    LOG(FATAL) << "glsl link error: " << log;
  }
}

shader::shader() : _program(0) {}

shader::shader(const GLchar *vs_src, const GLchar *fs_src) {
  // Compile shaders
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vs_src, nullptr);
  glCompileShader(vertexShader);
  compile_info(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fs_src, nullptr);
  glCompileShader(fragmentShader);
  compile_info(fragmentShader);

  // Create program and specify transform feedback variables
  _program = glCreateProgram();
  glAttachShader(_program, vertexShader);
  glAttachShader(_program, fragmentShader);

  // daddy the dox sed to put it before glLinkProgram()
  const GLchar *feedbackVaryings[] = {"outPosition", "outVelocity"};
  glTransformFeedbackVaryings(_program, 2, feedbackVaryings,
                              GL_INTERLEAVED_ATTRIBS);

  glLinkProgram(_program);
  link_info(_program);

  GLint n_active_attr = 0;
  GLint n_active_ufrm = 0;
  GLint max_attr_namelength = 0;
  GLint max_ufrm_namelength = 0;
  glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTES, &n_active_attr);
  glGetProgramiv(_program, GL_ACTIVE_UNIFORMS, &n_active_ufrm);
  glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,
                 &max_attr_namelength);
  glGetProgramiv(_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_ufrm_namelength);

  LOG(INFO) << n_active_ufrm << " ufrms";

  std::vector<GLchar> ufrm_nameData(max_ufrm_namelength);
  for (int ufrm = 0; ufrm < n_active_ufrm; ++ufrm) {
    GLint arraySize = 0;
    GLenum type = GL_FLOAT;
    GLsizei actualLength = 0;
    glGetActiveUniform(_program, ufrm, ufrm_nameData.size(), &actualLength,
                       &arraySize, &type, &ufrm_nameData[0]);

    LOG(INFO) << std::string((char *)&ufrm_nameData[0], actualLength);
  }

  LOG(INFO) << n_active_attr << " attrs";

  std::vector<GLchar> nameData(max_attr_namelength);
  for (int attr = 0; attr < n_active_attr; ++attr) {
    GLint arraySize = 0;
    GLenum type = GL_FLOAT;
    GLsizei actualLength = 0;
    glGetActiveAttrib(_program, attr, nameData.size(), &actualLength,
                      &arraySize, &type, &nameData[0]);

    LOG(INFO) << std::string((char *)&nameData[0], actualLength);
  }
}

shader::~shader() {
  LOG(INFO) << "shader deleted";
  glDeleteProgram(_program);
}

void shader::use() { glUseProgram(_program); }