#pragma once

#include "engine/core/EngineCore.h"
#include "engine/core/exception/EngineException.h"

#ifdef _DEBUG
#define GLCHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"OpenGL error " + wStr(err)); } }
#else
#define GLCHECKERROR
#endif // DEBUG
