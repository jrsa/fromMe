#ifndef _shader_hpp
#define _shader_hpp

#include <glbinding/gl/gl.h>
#include <glog/logging.h>

class shader {
private:
  gl::GLuint _program;

public:
  shader();
  shader(const gl::GLchar *vs_src, const gl::GLchar *fs_src);
  //  shader(const gl:: GLchar *vs_src, const gl::GLchar *gs_src, const
  //  gl::GLchar *fs_src);

  ~shader();

  // gross hack
  gl::GLuint program() { return _program; }
  void use();
};

#endif