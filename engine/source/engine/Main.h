#pragma once

#include "engine/core/exception/EngineException.h"
#include "engine/core/EngineCore.h"

#ifdef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:console")
#else
#pragma comment(linker, "/SUBSYSTEM:console") //comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

extern AAAAgames::Engine* AAAAgames::CreateEngineApplication();

int main(int argc, char** argv)
{
	Engine* engine{ nullptr };
	try
	{
		engine = AAAAgames::CreateEngineApplication();
		engine->Run();
	}
	catch (const EngineException& e)
	{
		const std::wstring eMsg = e.GetFullMessage() +
			L"\n\nException caught!";
		ERROR_PRINT(wstr2str(e.GetExceptionType()));
		ERROR_PRINT(wstr2str(eMsg));
		AAAAgames::Engine::ShowMessageBox(e.GetExceptionType(), eMsg);
		AAAAgames::Engine::SetQuit(true);
	}
	catch (const std::exception& e)
	{
		const std::string whatStr(e.what());
		const std::wstring eMsg = std::wstring(whatStr.begin(), whatStr.end()) +
			L"\n\nException caught!";
		ERROR_PRINT("Unhandled STL Exception");
		ERROR_PRINT(wstr2str(eMsg));
		AAAAgames::Engine::ShowMessageBox(str2wstr("Unhandled STL Exception"), eMsg);
		AAAAgames::Engine::SetQuit(true);
	}
	catch (...)
	{
		ERROR_PRINT("Unhandled Non-STL Exception");
		ERROR_PRINT("...");
		AAAAgames::Engine::ShowMessageBox(str2wstr("Unhandled STL Exception"), str2wstr("..."));
		AAAAgames::Engine::SetQuit(true);
	}
	delete engine;
}