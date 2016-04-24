#ifndef _shader_hpp
#define _shader_hpp

#include <glbinding/gl/gl.h>
#include <glog/logging.h>

class shader {
private:
  GLuint _program;
  
public:
  shader(const GLchar *vs_src, const GLchar *fs_src);
  shader(const GLchar *vs_src, const GLchar *gs_src, const GLchar *fs_src);
};

#endif