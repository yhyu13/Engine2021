#pragma once

#include "engine/core/EngineCore.h"
#include "engine/core/exception/EngineException.h"

namespace longmarch
{
    extern bool g_bGLCheckError;
    
#ifndef _SHIPPING
#define GLCHECKERROR() do { if (g_bGLCheckError) { GLenum err = glGetError(); if ( err != GL_NO_ERROR) { throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"OpenGL error " + wStr(err));}} } while (0)
#else
#define GLCHECKERROR()
#endif // _SHIPPING

    // inspired from https://gist.github.com/liam-middlebrook/c52b069e4be2d87a6d2f
    __LongMarch_TRIVIAL_TEMPLATE__
    void GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
                                         GLenum severity, GLsizei length,
                                         const GLchar* msg, const void* data)
    {
        std::string _source;
        std::string _type;
        std::string _severity;

        switch (source)
        {
        case GL_DEBUG_SOURCE_API:
            _source = "API";
            break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            _source = "WINDOW SYSTEM";
            break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            _source = "SHADER COMPILER";
            break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
            _source = "THIRD PARTY";
            break;

        case GL_DEBUG_SOURCE_APPLICATION:
            _source = "APPLICATION";
            break;

        case GL_DEBUG_SOURCE_OTHER:
            _source = "OTHER";
            break;

        default:
            return;
            _source = "UNKNOWN";
            break;
        }

        switch (type)
        {
        case GL_DEBUG_TYPE_ERROR:
            _type = "ERROR";
            break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            _type = "DEPRECATED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            _type = "UDEFINED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_PORTABILITY:
            _type = "PORTABILITY";
            break;

        case GL_DEBUG_TYPE_PERFORMANCE:
            _type = "PERFORMANCE";
            break;

        case GL_DEBUG_TYPE_OTHER:
            _type = "OTHER";
            break;

        case GL_DEBUG_TYPE_MARKER:
            _type = "MARKER";
            break;

        default:
            return;
            _type = "UNKNOWN";
            break;
        }

        switch (severity)
        {
        case GL_DEBUG_SEVERITY_HIGH:
            _severity = "HIGH";
            break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            _severity = "MEDIUM";
            break;

        case GL_DEBUG_SEVERITY_LOW:
            _severity = "LOW";
            break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
            _severity = "NOTIFICATION";
            break;

        default:
            return;
            _severity = "UNKNOWN";
            break;
        }

        auto output = Str("[OpenGL] %d: %s of %s severity, raised from %s: %s\n",
   id, _type, _severity, _source, msg);

        switch (severity)
        {
        case GL_DEBUG_SEVERITY_HIGH:
            CRITICAL_PRINT(output);
            break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            ERROR_PRINT(output);
            break;

        case GL_DEBUG_SEVERITY_LOW:
            WARN_PRINT(output);
            break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
            DEBUG_PRINT(output);
            break;

        default:
            return;
            DEBUG_PRINT(output);
            break;
        }
    }
}
