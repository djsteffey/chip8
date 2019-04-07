#include "CChip8.hpp"
#include <cstring>
#include <fstream>
#include <cstdio>

namespace djs{
	namespace chip8{
		CChip8::CChip8(std::function<void (unsigned char*)> draw_callback, std::function<void (bool start)> sound_callback){
			this->m_game_filename = "";
			this->m_draw_callback = draw_callback;
			this->m_sound_callback = sound_callback;
			this->reset();
		}

		CChip8::~CChip8(){

		}

		void CChip8::update(float delta_time){
			// run the cpu at 1000 Hz; which is a delay of 1/1000
			float delay = 1.0f /1000.0f;
			this->m_cpu_timer += delta_time;
			while (this->m_cpu_timer >= delay){
				this->emulate_cycle();
				this->m_cpu_timer -= delay;
			}

			// run the delay and sound timers at 60Hz
			delay = 1.0f / 60.0f;
			this->m_sound_and_delay_timer += delta_time;
			while (this->m_sound_and_delay_timer >= delay){
				if (this->m_sound_timer > 0){
					this->m_sound_timer -= 1;
					if (this->m_sound_timer == 0){
						this->stop_sound();
					}
				}
				if (this->m_delay_timer > 0){
					this->m_delay_timer -= 1;
				}
				this->m_sound_and_delay_timer -= delay;
			}

		}
		void CChip8::reset(){
			// setup program counter, opcode, index reg, and sp
			this->m_program_counter = 0x0200;
			this->m_opcode = 0x0000;
			this->m_index_register = 0x0000;
			this->m_stack_pointer = 0x00;

			// clear display
			for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; ++i){
				this->m_pixels[i] = 0x00;
			}
			this->m_draw_callback(this->m_pixels);

			// clear stack
			for (int i = 0; i < NUM_STACK; ++i){
				this->m_stack[i] = 0x0000;
			}

			// clear registers
			for (int i = 0; i < NUM_REGISTERS; ++i){
				this->m_registers[i] = 0x00;
			}

			// clear memory
			for (int i = 0; i < MEMORY_SIZE; ++i){
				this->m_memory[i] = 0x00;
			}

			// load fontset
			unsigned char chip8_fontset[80] = {
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
			std::memcpy(&(this->m_memory[0x50]), chip8_fontset, 0x50);

			// clear keys
			for (int i = 0; i < NUM_KEYS; ++i){
				this->m_keys[i] = false;
			}

			// reset timers
			this->m_delay_timer = 0x00;
			this->m_sound_timer = 0x00;
			this->m_is_sound_playing = false;
			this->m_sound_callback(false);

			// timing
			this->m_cpu_timer = 0.0f;
			this->m_sound_and_delay_timer = 0.0f;

			// load the game file if set
			if (this->m_game_filename.empty() == false){
				this->load_game(this->m_game_filename);
			}
		}

		bool CChip8::load_game(std::string filename){
			// save filename
			this->m_game_filename = filename;

			// open the file
			std::ifstream in;
			in.open(this->m_game_filename, std::ios::binary);
			if (!in){
				// failed to open the file
				std::printf("Unable to open game file: %s\n", this->m_game_filename.c_str());
				return false;
			}

			// get size of the file
			in.seekg(0, std::ios::end);
			int size = in.tellg();
			in.seekg(0, std::ios::beg);

			// now read this amount into memory at 0x200
			in.read((char*)(&(this->m_memory[0x0200])), size);

			// close file
			in.close();

			// success
			return true;
		}

		void CChip8::emulate_cycle(){
			// check program counter
			if (this->m_program_counter < 0x200){
				std::printf("Illegal Program Counter:\n\tPC: 0x%04x", this->m_program_counter);
				return;
			}

			// fetch opcode
			this->m_opcode = this->m_memory[this->m_program_counter] << 8 | this->m_memory[this->m_program_counter + 1];
			this->m_program_counter += 2;

			// decode x, y, n, kk, nnn
			unsigned char x = (this->m_opcode >> 8) & 0x000F;
			unsigned char y = (this->m_opcode >> 4) & 0x000F;
			unsigned char n = this->m_opcode & 0x000F;
			unsigned char kk = this->m_opcode & 0x00FF;
			unsigned short nnn = this->m_opcode & 0x0FFF;

			// decode and execute the opcode
			switch (this->m_opcode & 0xF000){
				case 0x0000:{
					switch (kk){
						case 0xE0:{
							// clear the display
							for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; ++i){
								this->m_pixels[i] = 0x00;
							}
							this->m_draw_callback(this->m_pixels);
						} break;
						case 0xEE:{
							// return from subroutine
							this->m_program_counter = this->m_stack[this->m_stack_pointer - 1];
							this->m_stack_pointer -= 1;
						} break;
						default:{
							this->illegal_opcode();
						} break;
					}
				} break;

				case 0x1000:{
					// jump to location nnn
					this->m_program_counter = nnn;
				} break;

				case 0x2000:{
					// call subroutine at nnn
					this->m_stack[this->m_stack_pointer] = this->m_program_counter;
					this->m_stack_pointer += 1;
					this->m_program_counter = nnn;
				} break;

				case 0x3000:{
					// skip next instruction if reg[x] == kk
					if (this->m_registers[x] == kk){
						this->m_program_counter += 2;
					}
				} break;

				case 0x4000:{
					// skip next instruction if reg[x] != kk
					if (this->m_registers[x] != kk){
						this->m_program_counter += 2;
					}
				} break;

				case 0x5000:{
					// skip next instruction if reg[x] == reg[y]
					if (this->m_registers[x] == this->m_registers[y]){
						this->m_program_counter += 2;
					}
				} break;

				case 0x6000:{
					// set reg[x] = kk
					this->m_registers[x] = kk;
				} break;

				case 0x7000:{
					// set reg[x] = reg[x] + kk
					this->m_registers[x] += kk;
				} break;

				case 0x8000:{
					switch (n){
						case 0x0:{
							// set reg[x] = reg[y]
							this->m_registers[x] = this->m_registers[y];
						} break;

						case 0x0001:{
							// set reg[x] = reg[x] | reg[y]
							this->m_registers[x] = this->m_registers[x] | this->m_registers[y];
						} break;

						case 0x0002:{
							// set reg[x] = reg[x] & reg[y]
							this->m_registers[x] = this->m_registers[x] & this->m_registers[y];
						} break;

						case 0x0003:{
							// set reg[x] = reg[x] ^ reg[y]
							this->m_registers[x] = this->m_registers[x] ^ this->m_registers[y];
						} break;

						case 0x0004:{
							// set reg[x] = reg[x] + reg[y], reg[0x0F] = carry
							if ((int)this->m_registers[x] + (int)this->m_registers[y] > 255){
								this->m_registers[0xF] = 1;
							}
							else{
								this->m_registers[0xF] = 0;
							}
							this->m_registers[x] = this->m_registers[x] + this->m_registers[y];
						} break;

						case 0x0005:{
							// set reg[x] = reg[x] - reg[y], reg[0x0F] = not borrow
							if (this->m_registers[x] > this->m_registers[y]){
								this->m_registers[0xF] = 1;
							}
							else{
								this->m_registers[0xF] = 0;
							}
							this->m_registers[x] = this->m_registers[x] - this->m_registers[y];
						} break;

						case 0x0006:{
							// set reg[x] = reg[x] >> 1, reg[0x0F] = previous LSB
							this->m_registers[0xF] = this->m_registers[x] & 0x1;
							this->m_registers[x] = this->m_registers[x] >> 1;
						} break;

						case 0x0007:{
							// set reg[x] = reg[y] - reg[x], reg[0x0F] = not borrow
							if (this->m_registers[y] > this->m_registers[x]){
								this->m_registers[0xF] = 1;
							}
							else{
								this->m_registers[0xF] = 0;
							}
							this->m_registers[x] = this->m_registers[y] - this->m_registers[x];
						} break;

						case 0x000E:{
							// set reg[x] = reg[x] << 1, reg[0x0F] = previous MSB
							this->m_registers[0xF] = (this->m_registers[x] >> 7) & 0x1;
							this->m_registers[x] = this->m_registers[x] << 1;
						} break;

						default:{
							this->illegal_opcode();
						} break;
					}
				} break;

				case 0x9000:{
					switch (n){
						case 0x0:{
							if (this->m_registers[x] != this->m_registers[x]){
								this->m_program_counter += 2;
							}
						} break;
						default:{
							this->illegal_opcode();
						} break;
					}
				} break;

				case 0xA000:{
					// set I = nnn
					this->m_index_register = nnn;
				} break;

				case 0xB000:{
					// jump to location nnn + reg[0]
					this->m_program_counter = nnn + this->m_registers[0];
				} break;

				case 0xC000:{
					// set reg[x] = rand() & kk
					this->m_registers[x] = (rand() % 256) & kk;
				} break;

				case 0xD000:{
					// display n byte sprite starting at memory location I at (reg[x], reg[y]), set reg[0x0F] = collision
					unsigned char x = (this->m_opcode & 0x0F00) >> 8;
					unsigned char y = (this->m_opcode & 0x00F0) >> 4;
					unsigned char n = this->m_opcode & 0x000F;
					this->m_registers[0xF] = 0;

					// loop through the sprites bytes
					for (int i = 0; i < n; ++i){
						// read the sprite byte from memory
						unsigned char sprite_byte = this->m_memory[this->m_index_register + i];

						// loop through the 8 bits of the sprite byte
						for (int j = 0; j < 8; ++j){
							// calculate the bit
							unsigned char bit = sprite_byte & (0x80 >> j);
							if (bit != 0x00){
								// calculate the display address
								unsigned short display_index = ((this->m_registers[y] + i) * DISPLAY_WIDTH) + (this->m_registers[x] + j);
								if (this->m_pixels[display_index] == 0x01){
									this->m_registers[0xF] = 1;
								}
								this->m_pixels[display_index] ^= 1;
							}
						}
					}

					// need to draw
					this->m_draw_callback(this->m_pixels);
				} break;

				case 0xE000:{
					switch (kk){
						case 0x9E:{
							// skip next instruction if key reg[x] is pressed
							if (this->m_keys[this->m_registers[x]]){
								this->m_program_counter += 2;
							}
						} break;
						case 0x00A1:{
							// skip next instruction if key reg[x] is not pressed
							if (this->m_keys[this->m_registers[x]] == false){
								this->m_program_counter += 2;
							}
						} break;
						default:{
							this->illegal_opcode();
						} break;
					}
				} break;

				case 0xF000:{
					switch (kk){
						case 0x07:{
							// Vx = delay timer value
							this->m_registers[x] = this->m_delay_timer;
						} break;

						case 0x0A:{
							// check all keys for a press
							bool key_press = false;
							for (unsigned char i = 0; i < NUM_KEYS; ++i){
								if (this->m_keys[i]){
									this->m_registers[x] = i;
									key_press = true;
									break;
								}
							}

							// pressed?
							if (key_press == false){
								// nothing pressed so repeat instruction
								this->m_program_counter -= 2;
							}
						} break;

						case 0x15:{
							// set delay timer = Vx
							this->m_delay_timer = this->m_registers[x];
						} break;

						case 0x18:{
							// set sound timer = Vx
							this->m_sound_timer = this->m_registers[x];

							// start or stop sound as neccessary
							if (this->m_sound_timer > 0){
								this->start_sound();
							}
							else{
								this->stop_sound();
							}
						} break;

						case 0x1E:{
							// set I = I + Vx
							if (this->m_index_register + this->m_registers[x] > 0x0FFF){
								this->m_registers[0xF] = 1;
							}
							else{
								this->m_registers[0xF] = 0;
							}
							this->m_index_register = this->m_index_register + this->m_registers[x];
						} break;

						case 0x29:{
							// set I = location of sprite for digit Vx
							this->m_index_register = 0x50 + 5 * this->m_registers[x];
						} break;

						case 0x33:{
							// store BCD representation of Vx in memory locations I, I+1, I+2
							this->m_memory[this->m_index_register + 0] = (this->m_registers[x] % 1000) / 100;
							this->m_memory[this->m_index_register + 1] = (this->m_registers[x] % 100) / 10;
							this->m_memory[this->m_index_register + 2] = this->m_registers[x] % 10;
						} break;

						case 0x55:{
							// store registers V0 through Vx in memory starting at location I
							for (int i = 0; i <= x; ++i){
								this->m_memory[this->m_index_register + i] = this->m_registers[i];
							}
							this->m_index_register = this->m_index_register + x + 1;
						} break;

						case 0x0065:{
							// read registers V0 through Vx from memory starting at location I
							for (int i = 0; i <= x; ++i){
								this->m_registers[i] = this->m_memory[this->m_index_register + i];
							}
							this->m_index_register = this->m_index_register + x + 1;
						} break;

						default:{
							this->illegal_opcode();
						} break;
					}
				} break;

				default:{
					this->illegal_opcode();
				} break;
			}
		}

		void CChip8::set_key(char index, bool pressed){
			this->m_keys[index] = pressed;
		}

		void CChip8::illegal_opcode(){
			std::printf("Illegal Opcode:\n\tPC: 0x%04x\n\tOC: 0x%04x\n\n", this->m_program_counter, this->m_opcode);
			this->m_program_counter -= 2;
		}

		void CChip8::start_sound(){
			if (this->m_is_sound_playing == false){
				this->m_is_sound_playing = true;
				this->m_sound_callback(true);
			}
		}

		void CChip8::stop_sound(){
			if (this->m_is_sound_playing){
				this->m_is_sound_playing = false;
				this->m_sound_callback(false);
			}
		}
	}
}