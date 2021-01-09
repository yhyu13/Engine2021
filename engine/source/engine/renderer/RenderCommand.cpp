#include "engine-precompiled-header.h"
#include "RenderCommand.h"
#include "platform/OpenGL/OpenGLRendererAPI.h"

namespace longmarch {
	RendererAPI* RenderCommand::s_RendererAPI = OpenGLRendererAPI::GetInstance();
}