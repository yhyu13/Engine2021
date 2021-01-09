#include "engine-precompiled-header.h"
#include "RenderCommand.h"
#include "platform/OpenGL/OpenGLRendererAPI.h"

namespace AAAAgames {
	RendererAPI* RenderCommand::s_RendererAPI = OpenGLRendererAPI::GetInstance();
}