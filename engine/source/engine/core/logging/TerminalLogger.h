#pragma once
#include "TerminalCommands.h"

namespace longmarch {

	/**
	 * @brief Display all logging info
	 * It should act like a dockable menu
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class TerminalLogger {

	public:

		TerminalLogger() = delete; ~TerminalLogger() = delete; TerminalLogger(const TerminalLogger&) = delete; TerminalLogger(const TerminalLogger&&) = delete; \
			TerminalLogger& operator=(const TerminalLogger&) = delete; TerminalLogger& operator=(const TerminalLogger&&) = delete;

		static ImTerm::terminal<TerminalCommands>* GetInstance();
	};
}