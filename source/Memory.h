#pragma once

// wrapper over memory operations to keep <Windows.h> isolated
namespace memory
{
		int read(void* dest, size_t count, bool vp = false);

		void write(void* dest, void* src, size_t count, bool vp);

		template <typename Dest, typename Value>
		void write(Dest dest, Value value)
		{
				write((void*)dest, &value, sizeof(value), true);
		}

		void intercept(uchar op, void* dest, void* addr);
		inline void call(void* dest, void* addr) { intercept(0xE8, dest, addr); }
		inline void jump(void* dest, void* addr) { intercept(0xE9, dest, addr); }

		void* load_library(const char* name);
		void free_library(const void* handle);

		void* get_module_handle(const char* name);
		void* get_proc_address(const void* handle, const char* proc_name);

		short get_key_state(int virtual_key);
}
