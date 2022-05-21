#pragma once

#include "engine/core/exception/EngineException.h"
#include "engine/core/EngineCore.h"

#ifndef _SHIPPING
#pragma comment(linker, "/SUBSYSTEM:console")
#else
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

extern longmarch::Engine* longmarch::CreateEngineApplication();

int main(int argc, char** argv)
{
	Engine* engine = longmarch::CreateEngineApplication();
	try
	{
		engine->Init();
		engine->Run();
	}
	catch (const EngineException& e)
	{
		const std::wstring eMsg = e.GetFullMessage() +
			L"\n\nException caught!";
		ERROR_PRINT(wstr2str(e.GetExceptionType()));
		ERROR_PRINT(wstr2str(eMsg));
		longmarch::Engine::ShowMessageBox(e.GetExceptionType(), eMsg);
		longmarch::Engine::SetQuit(true);
		ASSERT(false, "Engine Exception Caught, please check log");
	}
	catch (const std::exception& e)
	{
		const std::string whatStr(e.what());
		const std::wstring eMsg = std::wstring(whatStr.begin(), whatStr.end()) +
			L"\n\nException caught!";
		ERROR_PRINT("Unhandled STL Exception");
		ERROR_PRINT(wstr2str(eMsg));
		longmarch::Engine::ShowMessageBox(str2wstr("Unhandled STL Exception"), eMsg);
		longmarch::Engine::SetQuit(true); 
		ASSERT(false, "Engine Exception Caught, please check log");
	}
	catch (...)
	{
		ERROR_PRINT("Unhandled Non-STL Exception");
		ERROR_PRINT("...");
		longmarch::Engine::ShowMessageBox(str2wstr("Unhandled STL Exception"), str2wstr("..."));
		longmarch::Engine::SetQuit(true);
		ASSERT(false, "Engine Exception Caught, please check log");
	}
	delete engine;
	return 0;
}
