#include "engine-precompiled-header.h"
#include "TerminalLogger.h"
#include "TerminalCommands.cpp"

ImTerm::terminal<longmarch::TerminalCommands>* longmarch::TerminalLogger::GetInstance()
{
	static std::once_flag flag;
	static CustomCommand cmd_struct;
	static ImTerm::terminal<TerminalCommands> terminal_log(cmd_struct, "Terminal");
	std::call_once(flag, [&]()
	{
		terminal_log.set_max_log_len(256);
		terminal_log.set_min_log_level(ImTerm::message::severity::trace);
	});

	return &terminal_log;
}