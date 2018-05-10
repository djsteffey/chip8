#include "CChip8.hpp"
#include <cstring>
#include <fstream>
#include <iostream>

namespace djs{
	namespace chip8{
		CChip8::CChip8(){
			this->m_loaded_rom = "";
			this->reset();
		}
	
		CChip8::~CChip8(){
			// nothing created with new
		}

		bool CChip8::reset(){
			// setup instruction counter
			this->m_instructions_count.clear();
			this->m_instructions_count[0x00E0] = 0;
			this->m_instructions_count[0x00EE] = 0;
			this->m_instructions_count[0x1000] = 0;
			this->m_instructions_count[0x2000] = 0;
			this->m_instructions_count[0x3000] = 0;
			this->m_instructions_count[0x4000] = 0;
			this->m_instructions_count[0x5000] = 0;
			this->m_instructions_count[0x6000] = 0;
			this->m_instructions_count[0x7000] = 0;
			this->m_instructions_count[0x8000] = 0;
			this->m_instructions_count[0x8001] = 0;
			this->m_instructions_count[0x8002] = 0;
			this->m_instructions_count[0x8003] = 0;
			this->m_instructions_count[0x8004] = 0;
			this->m_instructions_count[0x8005] = 0;
			this->m_instructions_count[0x8006] = 0;
			this->m_instructions_count[0x8007] = 0;
			this->m_instructions_count[0x800E] = 0;
			this->m_instructions_count[0x9000] = 0;
			this->m_instructions_count[0xA000] = 0;
			this->m_instructions_count[0xB000] = 0;
			this->m_instructions_count[0xC000] = 0;
			this->m_instructions_count[0xD000] = 0;
			this->m_instructions_count[0xE09E] = 0;
			this->m_instructions_count[0xE0A1] = 0;
			this->m_instructions_count[0xF007] = 0;
			this->m_instructions_count[0xF00A] = 0;
			this->m_instructions_count[0xF015] = 0;
			this->m_instructions_count[0xF018] = 0;
			this->m_instructions_count[0xF01E] = 0;
			this->m_instructions_count[0xF029] = 0;
			this->m_instructions_count[0xF033] = 0;
			this->m_instructions_count[0xF055] = 0;
			this->m_instructions_count[0xF065] = 0;
			this->m_instructions_count[0xFFFF] = 0;

			// reset state of machine to blank
			this->m_opcode = 0;
			std::memset(this->m_memory, 0, 4096 * sizeof(unsigned char));
			std::memset(this->m_registers, 0, 16 * sizeof(unsigned char));
			this->m_index_register = 0;
			this->m_program_counter = 0x200;
			std::memset(this->m_pixels, 0, 64 * 32 * sizeof(unsigned char));
			this->m_delay_timer = 0;
			this->m_sound_timer = 0;
			std::memset(this->m_stack, 0, 16 * sizeof(unsigned short));
			this->m_stack_pointer = 0;
			std::memset(this->m_keys, 0, 16 * sizeof(unsigned char));

			// todo load fontset into memory
			static unsigned char chip8_fontset[80] =
			{ 
				0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
				0x20, 0x60, 0x20, 0x20, 0x70, // 1
				0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
				0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
				0x90, 0x90, 0xF0, 0x10, 0x10, // 4
				0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
				0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
				0xF0, 0x10, 0x20, 0x40, 0x40, // 7
				0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
				0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
				0xF0, 0x90, 0xF0, 0x90, 0x90, // A
				0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
				0xF0, 0x80, 0x80, 0x80, 0xF0, // C
				0xE0, 0x90, 0x90, 0x90, 0xE0, // D
				0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
				0xF0, 0x80, 0xF0, 0x80, 0x80  // F
			};
			std::memcpy(&(this->m_memory[0x50]), chip8_fontset, sizeof(unsigned char) * 16 * 5);

			// load rom into memory if set
			if (this->m_loaded_rom != ""){
				return this->load(this->m_loaded_rom);
			}

			// done
			return true;
		}

		bool CChip8::load(std::string filename){
			// save rom filename
			this->m_loaded_rom = filename;

			// open the file in binary mode
			std::ifstream in(filename, std::ios::binary);

			// make sure open
			if (in.is_open()){
				// seek to end of file
				in.seekg(0, std::ios::end);

				// compute filesize
				std::streampos size = in.tellg();

				// seek to beginning of file
				in.seekg(0, std::ios::beg);

				// read entire file into memory starting at position 0x200
				in.read((char*)&(this->m_memory[0x0200]), size);

				// close the file
				in.close();

				// return success
				return true;
			}

			// file not found or inaccessable
			return false;
		}

		bool CChip8::load(unsigned char* instructions, int size){
			std::memcpy(&(this->m_memory[0x0200]), instructions, size);
		}

		bool CChip8::emulate_cycle(){
			// fetch the next opcode
			this->m_opcode = this->m_memory[this->m_program_counter] << 8 | this->m_memory[this->m_program_counter + 1];

			// break it apart
			unsigned char x = (this->m_opcode >> 8) & 0x000F;
			unsigned char y = (this->m_opcode >> 4) & 0x000F;
			unsigned char n = this->m_opcode & 0x000F;
			unsigned char kk = this->m_opcode & 0x00FF;
			unsigned short nnn = this->m_opcode & 0x0FFF;

			// execute opcode
			switch (this->m_opcode & 0xF000){
				case 0x0000:{
					switch(kk){
						case 0xE0:{		// clear the screen
							this->m_instructions_count[0x00E0] += 1;

							std::memset(this->m_pixels, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(unsigned char));
							this->m_program_counter += 2;
						} break;
						case 0xEE:{		// ret
							this->m_instructions_count[0x00EE] += 1;

							this->m_program_counter = this->m_stack[--(this->m_stack_pointer)];
						} break;
						default:{
							this->m_instructions_count[0xFFFF] += 1;

							this->unhandled_opcode(this->m_opcode);
							this->m_program_counter += 2;
							return false;
						} break;
					}
				}
				break;

				case 0x1000:{	// 1nnn: jump to address nnn
					this->m_instructions_count[0x1000] += 1;

					this->m_program_counter = this->m_opcode & nnn;
				} break;

				case 0x2000:{	// 2nnn: call address nnn
					this->m_instructions_count[0x2000] += 1;

					this->m_stack[(this->m_stack_pointer)++] = this->m_program_counter + 2;
					this->m_program_counter = nnn;
				} break;

				case 0x3000:{	// 3xkk: skip next instruction if V[x] = kk
					this->m_instructions_count[0x3000] += 1;

					this->m_program_counter += (this->m_registers[x] == kk) ? 4 : 2;
				} break;

				case 0x4000:{	// 4xkk: skip next instruction if V[x] != kk
					this->m_instructions_count[0x4000] += 1;

					this->m_program_counter += (this->m_registers[x] != kk) ? 4 : 2;
				} break;

				case 0x5000:{	// 5xy0: skip next instruction if V[x] == V[y]
					this->m_instructions_count[0x5000] += 1;

					this->m_program_counter += (this->m_registers[x] == this->m_registers[y]) ? 4 : 2;
				} break;

				case 0x6000:{	// 6xkk: set V[x] = kk
					this->m_instructions_count[0x6000] += 1;

					// set
					this->m_registers[x] = kk;
					this->m_program_counter += 2;
				} break;

				case 0x7000:{	// 7xkk: set V[x] = V[x] + kk
					this->m_instructions_count[0x7000] += 1;

					this->m_registers[x] += kk;
					this->m_program_counter += 2;
				} break;

				case 0x8000:{	// 8xyn:  mathex_y = (this->m_opcode & 0x00F0) >> 4;
					switch (n){
						case 0x0:{	// V[x] = V[y]
							this->m_instructions_count[0x8000] += 1;

							this->m_registers[x] = this->m_registers[y];
							this->m_program_counter += 2;
						} break;
						case 0x1:{	// V[x] = V[x] | V[y]
							this->m_instructions_count[0x8001] += 1;

							this->m_registers[x] = this->m_registers[x] | this->m_registers[y];
							this->m_program_counter += 2;
						} break;
						case 0x2:{	// V[x] = V[x] & V[y]
							this->m_instructions_count[0x8002] += 1;

							this->m_registers[x] = this->m_registers[x] & this->m_registers[y];
							this->m_program_counter += 2;
						} break;
						case 0x3:{	// V[x] = V[x] ^ V[y]
							this->m_instructions_count[0x8003] += 1;

							this->m_registers[x] = this->m_registers[x] ^ this->m_registers[y];
							this->m_program_counter += 2;
						} break;
						case 0x4:{	// V[x] = V[x] + V[y]; set carry
							this->m_instructions_count[0x8004] += 1;

							this->m_registers[0xF] = ((int) this->m_registers[x] + (int)this->m_registers[y]) > 255 ? 1 : 0;
							this->m_registers[x] = this->m_registers[x] + this->m_registers[y];
							this->m_program_counter += 2;
						} break;
						case 0x5:{	// V[x]  = V[x] - V[y]; set borrow
							this->m_instructions_count[0x8005] += 1;

							this->m_registers[0xF] = (this->m_registers[x] > this->m_registers[y]) ? 1 : 0;
							this->m_registers[x] = this->m_registers[x] - this->m_registers[y];
							this->m_program_counter += 2;
						} break;
						case 0x6:{	// V[x] = V[x] >> 1
							this->m_instructions_count[0x8006] += 1;

							this->m_registers[0xF] = this->m_registers[x] & 0x1;
							this->m_registers[x] = (this->m_registers[x] >> 1);
							this->m_program_counter += 2;
						} break;
						case 0x7:{	// V[x] = V[y] - V[x]; set borrow
							this->m_instructions_count[0x8007] += 1;

							this->m_registers[0xF] = (this->m_registers[y] > this->m_registers[x]) ? 1 : 0;
							this->m_registers[x] = this->m_registers[y] - this->m_registers[x];
							this->m_program_counter += 2;
						} break;
						case 0xE:{	// V[x] = V[x] << 1
							this->m_instructions_count[0x800E] += 1;

							this->m_registers[0xF] = (this->m_registers[x] >> 7) & 0x1;
							this->m_registers[x] = (this->m_registers[x] << 1);
							this->m_program_counter += 2;
						} break;
						default:{
							this->m_instructions_count[0xFFFF] += 1;

							this->unhandled_opcode(this->m_opcode);
							this->m_program_counter += 2;
							return false;
						}
					}
				} break;

				case 0x9000:{	
					switch (n){
						case 0x0:{	// 9xy0: skip instruction if V[x] != V[y]
							this->m_instructions_count[0x9000] += 1;

							this->m_program_counter += (this->m_registers[x] != this->m_registers[y]) ? 4 : 2;
						} break;
						default:{
							this->m_instructions_count[0xFFFF] += 1;

							this->unhandled_opcode(this->m_opcode);
							this->m_program_counter += 2;
							return false;
						} break;
					}
				} break;

				case 0xA000:{	// Annn: set I to address nnn
					this->m_instructions_count[0xA000] += 1;

					this->m_index_register = nnn;
					this->m_program_counter += 2;
				} break;

				case 0xB000:{	// Bnnn: jump to location nnn + V[0]
					this->m_instructions_count[0xB000] += 1;

					this->m_program_counter = nnn + this->m_registers[0];
				} break;

				case 0xC000:{	// Cxkk: V[x] = random byte AND kk
					this->m_instructions_count[0xC000] += 1;

					this->m_registers[x] = this->random_byte() & kk;
					this->m_program_counter += 2;
				} break;

				case 0xD000:{	// Dxyn: draw an n-byte sprite from location I at V[x], V[y] on screen; V[F] = collision 
					this->m_instructions_count[0xD000] += 1;

					this->draw_sprite(this->m_registers[x], this->m_registers[y], n);
					this->m_program_counter += 2;
				} break;

				case 0xE000:{	// key pressed events
					switch (kk){
						case 0x9E:{	// skip next instruction if key[V[x]] is pressed
							this->m_instructions_count[0xE09E] += 1;

							this->m_program_counter += (this->m_keys[this->m_registers[x]]) ? 4 : 2;
						} break;
						case 0xA1:{	// skip next instruction if key[V[x]] is pressed
							this->m_instructions_count[0xE0A1] += 1;

							this->m_program_counter += (!this->m_keys[this->m_registers[x]]) ? 4 : 2;
						} break;
						default:{
							this->m_instructions_count[0xFFFF] += 1;

							this->unhandled_opcode(this->m_opcode);
							this->m_program_counter += 2;
							return false;
						}
					}
				} break;

				case 0xF000:{	// misc
					switch (kk){
						case 0x07:{	// V[x] = delay_timer
							this->m_instructions_count[0xF007] += 1;

							this->m_registers[x] = this->m_delay_timer;
							this->m_program_counter += 2;
						} break;
						case 0x0A:{	// wait for keypress
							this->m_instructions_count[0xF00A] += 1;

							bool found = false;
							for (int i = 0; i < 16; ++i){
								if (this->m_keys[i] == 1){
									// got a key press
									this->m_registers[x] = i;
									found = true;
									break;
								}
							}

							if (found){
								this->m_program_counter += 2;
							}

						} break;
						case 0x15:{	// delay_timer = V[x]
							this->m_instructions_count[0xF015] += 1;

							this->m_delay_timer = this->m_registers[x];
							this->m_program_counter += 2;
						} break;
						case 0x18:{	// sound_timer = V[x]
							this->m_instructions_count[0xF018] += 1;

							this->m_sound_timer = this->m_registers[x];
							this->m_program_counter += 2;
						} break;
						case 0x1E:{	// I = I + V[x]
							this->m_instructions_count[0xF01E] += 1;

							this->m_registers[0xF] = (this->m_index_register + this->m_registers[x] > 0xFFF) ? 1 : 0;
							this->m_index_register = this->m_index_register + this->m_registers[x];
							this->m_program_counter += 2;
						} break;
						case 0x29:{	// I = memory location of font character V[x]
							this->m_instructions_count[0xF029] += 1;

							this->m_index_register = FONT_BASE_ADDRESS + (this->m_registers[x] * FONT_BYTES_PER_CHARACTER);
							this->m_program_counter += 2;
						} break;
						case 0x33:{	// store hundreds / tens/ ones in consecutive memory addresses starting at I
							this->m_instructions_count[0xF033] += 1;

							this->m_memory[this->m_index_register + 0] = (this->m_registers[x] % 1000) / 100;
							this->m_memory[this->m_index_register + 1] = (this->m_registers[x] % 100) / 10;
							this->m_memory[this->m_index_register + 2] = (this->m_registers[x] % 10) / 1;
							this->m_program_counter += 2;
						} break;
						case 0x55:{	// dump registers V[0] -> V[x] into memory starting at I
							this->m_instructions_count[0xF055] += 1;


							for (int i = 0; i <= x; ++i){
								this->m_memory[this->m_index_register + i] = this->m_registers[i];
							}
							this->m_index_register += (x + 1);
							this->m_program_counter += 2;
						} break;
						case 0x65:{	// copy memory starting at I into registers V[0] -> V[x]
							this->m_instructions_count[0xF065] += 1;

							for (int i = 0; i <= x; ++i){
								this->m_registers[i] = this->m_memory[this->m_index_register + i];
							}
							this->m_index_register += (x + 1);
							this->m_program_counter += 2;
						} break;
						default:{
							this->m_instructions_count[0xFFFF] += 1;

							this->unhandled_opcode(this->m_opcode);
							this->m_program_counter += 2;
							return false;
						}
					}
				} break;

				default:{
					this->m_instructions_count[0xFFFF] += 1;

					this->unhandled_opcode(this->m_opcode);
							this->m_program_counter += 2;
							return false;
				}
			}
		}

		void CChip8::update_timers(){
			// update timers
			if (this->m_delay_timer > 0){
				--(this->m_delay_timer);
			}
			if (this->m_sound_timer > 0){
				if (this->m_sound_timer == 1){
					// todo make beep
				}
				--(this->m_sound_timer);
			}
		}

		unsigned char* CChip8::get_pixels(){
			return this->m_pixels;
		}

		void CChip8::set_keys(unsigned char keys[16]){
			std::memcpy(this->m_keys, keys, sizeof(unsigned char) * 16);
		}

		unsigned short CChip8::get_current_opcode(){
			return this->m_opcode;
		}

		const unsigned char* CChip8::get_memory(){
			return this->m_memory;
		}

		const unsigned char* CChip8::get_registers(){
			return this->m_registers;
		}

		unsigned short CChip8::get_index_register(){
			return this->m_index_register;
		}

		unsigned short CChip8::get_program_counter(){
			return this->m_program_counter;
		}

		const unsigned short* CChip8::get_stack(){
			return this->m_stack;
		}

		char CChip8::get_stack_pointer(){
			return this->m_stack_pointer;
		}

		const std::map<unsigned int, long>& CChip8::get_instructions_count(){
			return this->m_instructions_count;
		}

		void CChip8::unhandled_opcode(unsigned short opcode){
			// todo
		}

		unsigned char CChip8::random_byte(){
			return std::rand() % 256;
		}

		void CChip8::draw_sprite(unsigned char x, unsigned char y, unsigned char n){
			// set collision flag to 0
			this->m_registers[0xF] = 0;

			// go through each byte in the sprite
			for (int byte_index = 0; byte_index < n; ++byte_index){
				// get the sprite byte from memory
				unsigned char byte = this->m_memory[this->m_index_register + byte_index];

				// go through each bit in the byte
				for (int bit_index = 0; bit_index < 8; ++bit_index){
					// the value of the bit
					unsigned char bit = (byte >> (7 - bit_index)) & 0x1;

					// current pixel value
					unsigned char current_bit = this->m_pixels[(y + byte_index) * SCREEN_WIDTH + (x + bit_index)];

					// if we flip a pixel off then set flag
					if (bit == 1){
						// we need to flip if
						if (current_bit == 1){
							// turn off and flag
							this->m_pixels[(y + byte_index) * SCREEN_WIDTH + (x + bit_index)] = 0;
							this->m_registers[0xF] = 1;
						}
						else{
							// turn on and no flag
							this->m_pixels[(y + byte_index) * SCREEN_WIDTH + (x + bit_index)] = 1;							
						}
					}
				}
			}
		}
	}
}
