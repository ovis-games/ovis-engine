#pragma once

#if defined(__IPHONEOS__)
#include <SDL2/SDL_opengles2.h>
#elif defined(__EMSCRIPTEN__)
#include <GLES2/gl2.h>
#elif __APPLE__
#include <OpenGL/gl3.h>
#elif _WIN32
#include <GL/glew.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#endif
