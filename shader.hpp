#include <glbinding/gl/gl.h>

class shader {
private:
  
public:
  shader(const GLchar *vs_src, const GLchar *fs_src);
  shader(const GLchar *vs_src, const GLchar *gs_src, const GLchar *fs_src);
};