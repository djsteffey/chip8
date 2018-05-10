#ifndef djs_chip8_CChip8_hpp
#define djs_chip8_CChip8_hpp

#include <string>
#include <map>

namespace djs{
	namespace chip8{
		class CChip8{
			public:
				CChip8();
				~CChip8();

				bool reset();

				bool load(std::string filename);

				bool load(unsigned char* instructions, int size);

				bool emulate_cycle();

				void update_timers();

				unsigned char* get_pixels();

				void set_keys(unsigned char keys[16]);

				unsigned short get_current_opcode();

				const unsigned char* get_memory();

				const unsigned char* get_registers();

				unsigned short get_index_register();

				unsigned short get_program_counter();

				const unsigned short* get_stack();

				char get_stack_pointer();

				const std::map<unsigned int, long>& get_instructions_count();

			protected:

			private:
				void unhandled_opcode(unsigned short opcode);
				unsigned char random_byte();
				void draw_sprite(unsigned char x, unsigned char y, unsigned char n);

				const static unsigned char SCREEN_WIDTH = 64;
				const static unsigned char SCREEN_HEIGHT = 32;
				const static unsigned short FONT_BASE_ADDRESS = 0x50;
				const static unsigned char FONT_BYTES_PER_CHARACTER = 5;

				std::string m_loaded_rom;
				unsigned short m_opcode;
				unsigned char m_memory[4096];
				unsigned char m_registers[16];
				unsigned short m_index_register;
				unsigned short m_program_counter;
				unsigned char m_pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
				unsigned char m_delay_timer;
				unsigned char m_sound_timer;
				unsigned short m_stack[16];
				char m_stack_pointer;
				unsigned char m_keys[16];

				std::map<unsigned int, long> m_instructions_count;
		};
	}
}
#endif