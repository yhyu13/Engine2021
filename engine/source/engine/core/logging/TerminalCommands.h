#pragma once

#include <array>
#include <string>
#include <vector>
#include <mutex>

#include <imgui/addons/imterm/terminal.hpp>
#include <imgui/addons/imterm/terminal_helpers.hpp>
#include <imgui/addons/imterm/misc.hpp>

namespace longmarch
{
	struct CustomCommand {
		bool should_close = false;
	};

	/**
	 * @brief Inspired by <imgui/addons/imterm/terminal_commands.hpp>
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
	 */
	class TerminalCommands : public ImTerm::basic_spdlog_terminal_helper<TerminalCommands, CustomCommand, misc::no_mutex> {
	public:

		TerminalCommands();

		static std::vector<std::string> no_completion(argument_type&) { return {}; }

		static void clear(argument_type&);
		static void configure_term(argument_type&);
		static std::vector<std::string> configure_term_autocomplete(argument_type&);
		static void echo(argument_type&);
		static void exit(argument_type&);
		static void help(argument_type&);
		static void quit(argument_type&);
	};
}