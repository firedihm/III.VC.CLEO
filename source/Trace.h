#pragma once

class Script;

namespace trace
{
		void open();
		void close();

		void line(const char* format, ...);
		void opcode(Script* script, ushort op);
}
