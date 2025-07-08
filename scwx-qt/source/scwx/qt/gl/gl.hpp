#pragma once

#include <GL/glew.h>

#define SCWX_GL_CHECK_ERROR()                                                  \
   {                                                                           \
      GLenum err;                                                              \
      while ((err = glGetError()) != GL_NO_ERROR)                              \
      {                                                                        \
         logger_->error("GL Error: {}, {}: {}", err, __FILE__, __LINE__);      \
      }                                                                        \
   }
