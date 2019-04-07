#ifndef djs_chip8_CChip8_hpp
#define djs_chip8_CChip8_hpp

#include <string>
#include <functional>

namespace djs{
	namespace chip8{
		class CChip8{
			public:
				CChip8(std::function<void (unsigned char*)> draw_callback, std::function<void (bool start)> sound_callback);
				~CChip8();
				void update(float delta_time);
				void reset();
				bool load_game(std::string filename);
				void set_key(char index, bool pressed);

			protected:

			private:
				void emulate_cycle();
				void illegal_opcode();
				void start_sound();
				void stop_sound();

				static const int DISPLAY_WIDTH = 64;
				static const int DISPLAY_HEIGHT = 32;
				static const int MEMORY_SIZE = 4096;
				static const int NUM_REGISTERS = 16;
				static const int NUM_STACK = 16;
				static const int NUM_KEYS = 16;

				std::function<void (unsigned char*)> m_draw_callback;
				std::function<void (bool start)> m_sound_callback;

				float m_cpu_timer;
				float m_sound_and_delay_timer;
				std::string m_game_filename;
				unsigned short m_opcode;
				unsigned char m_memory[MEMORY_SIZE];
				unsigned char m_registers[NUM_REGISTERS];
				unsigned short m_index_register;
				unsigned short m_program_counter;
				unsigned char m_pixels[DISPLAY_WIDTH * DISPLAY_HEIGHT];
				unsigned char m_delay_timer;
				unsigned char m_sound_timer;
				unsigned short m_stack[NUM_STACK];
				unsigned char m_stack_pointer;
				bool m_keys[NUM_KEYS];
				bool m_is_sound_playing;
		};
	}
}
#endif