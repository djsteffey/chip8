#include <SFML/Graphics.hpp>
#include "CChip8.hpp"
#include <iostream>
#include <sstream>
#include <thread>

void test_00E0(){
	std::cout << "test_00E0: ";

	static unsigned char test_instructions[] {
		0x00, 0xE0,			// 0200;	clear display
		0x12, 0x02,			// 0202;	jump to 0202
	};

	djs::chip8::CChip8 chip8;
	chip8.reset();
	chip8.load((unsigned char*)test_instructions, sizeof(test_instructions));

	// manually set some pixels
	for (int i = 0; i < 64; ++i){
		chip8.get_pixels()[i] = 1;		
	}

	// run one cycle that is clear screen
	chip8.emulate_cycle();

	// ensure all pixels are blank
	for (int i = 0; i < 64 * 32; ++i){
		if (chip8.get_pixels()[i] != 0){
			std::cout << "FAILED\n";
			return;
		}
	}

	std::cout << "PASSED\n";
}

void test_00EE(){
	std::cout << "test_00EE: ";

	static unsigned char test_instructions[] {
		0x00, 0x00,			// 0200;	nop
		0x22, 0x06,			// 0202;	call to 0206
		0x12, 0x0A,			// 0204;	jump to 020A
		0x00, 0x00,			// 0206;	nop
		0x00, 0xEE,			// 0208;	ret
		0x00, 0x00,			// 020A;	nop
		0x12, 0x0C,			// 020C;	jump 020C
	};

	djs::chip8::CChip8 chip8;
	chip8.reset();
	chip8.load((unsigned char*)test_instructions, sizeof(test_instructions));

	// verify stack pointer is -1
	if (chip8.get_stack_pointer() != -1){
		std::cout << "FAILED STACK POINTER 0\n";
		return;		
	}

	// nop
	chip8.emulate_cycle();

	// call to 0206
	chip8.emulate_cycle();
	if (chip8.get_stack_pointer() != 0){
		std::cout << "FAILED STACK POINTER 1\n";
		return;	
	}
	if (chip8.get_program_counter() != 0x0206){
		std::cout << "FAILED PROGRAM COUNTER\n";
		return;	
	}
	if (chip8.get_stack()[0] != 0x0204){
		std::cout << "FAILED STACK\n";
		return;	
	}

	// nop
	chip8.emulate_cycle();

	// ret
	chip8.emulate_cycle();
	if (chip8.get_stack_pointer() != -1){
		std::cout << "FAILED STACK POINTER 2\n";
		return;	
	}
	if (chip8.get_program_counter() != 0x0204){
		std::cout << "FAILED PROGRAM COUNTER\n";
		return;	
	}

	// jump 020A
	chip8.emulate_cycle();
	if (chip8.get_program_counter() != 0x020A){
		std::cout << "FAILED PROGRAM COUNTER\n";
		return;	
	}

	// nop
	chip8.emulate_cycle();

	// jump 020C
	chip8.emulate_cycle();
	if (chip8.get_program_counter() != 0x020C){
		std::cout << "FAILED PROGRAM COUNTER\n";
		return;	
	}


	// ensure all pixels are blank
	for (int i = 0; i < 64 * 32; ++i){
		if (chip8.get_pixels()[i] != 0){
			std::cout << "FAILED\n";
			return;
		}
	}

	std::cout << "PASSED\n";
}

int main(int argc, char* argv[]){
	test_00E0();
	test_00EE();

	// create the window
	sf::RenderWindow render_window(
		sf::VideoMode(64 * 10, 32 * 10),
		"SFML Window",
		sf::Style::Titlebar | sf::Style::Close
	);
	render_window.setVerticalSyncEnabled(true);

	// text for rendering information
	sf::Font font;
	font.loadFromFile("resources/DroidSans.ttf");
	sf::Text text;
	text.setFont(font);
	text.setString("Test");
	text.setCharacterSize(16);
	text.setPosition(0, 0);
	text.setColor(sf::Color::Red);

	// timing
	sf::Clock clock;
	int frames = 0;
	int fps = 0;
	float frame_timer = 0.0f;

	// the chip8
	djs::chip8::CChip8 chip8;


	if (chip8.load(std::string(argv[1]))){
//	if (chip8.load((unsigned char*)test_instructions, sizeof(test_instructions))){
		std::cout << "Load successful" << std::endl;
	}
	else{
		std::cout << "Load failed" << std::endl;	
		return -1;
	}

	int index = 0;
	while (index < 4096){
		if (index % 8 == 0){
			std::cout << std::hex << "\n" << index << "\t";			
		}
		std::cout << (int)(chip8.get_memory()[index]) << "\t";
		index += 1;
	}


	// texture for rendering pixels to
	sf::Texture texture;
	texture.create(64, 32);

	sf::Sprite sprite(texture);
	sprite.setScale(10, 10);

	sf::Uint8 texture_pixels[64 * 32 * 4];

	// thread to emulate chip8
	std::thread t(
		[](djs::chip8::CChip8* c8){
			while (true){
				c8->emulate_cycle();
			}
		},
		&chip8
	);

	// run the loop
	bool running = true;
	while (running){
		sf::Event event;
		while (render_window.pollEvent(event)){
			if (event.type == sf::Event::Closed){
				running = false;
			}
			if (event.type == sf::Event::KeyPressed){
//				chip8.emulate_cycle();

				std::cout << std::hex;
				// dump the registers
				for (int i = 0; i < 16; ++i){
					std::cout << "Register: " << i << " = " << (int)(chip8.get_registers()[i]) << "\n";
				}			

				// stack
				for (int i = 0; i < 16; ++i){
					std::cout << "Stack: " << i << " = " << (int)(chip8.get_stack()[i]) << "\n";
				}

				// stack pointer
				std::cout << "Stack Pointer: " << (int)(chip8.get_stack_pointer()) << "\n";

				// index register
				std::cout << "Index Register: " << (int)(chip8.get_index_register()) << "\n";

				// program counter
				std::cout << "Program Counter: " << (int)(chip8.get_program_counter()) << "\n";

				// opcode
				std::cout << "OPCODE: " << (int)(chip8.get_current_opcode()) << "\n";	

				// instructions count
				/*
				for (auto const& pair : chip8.get_instructions_count()){
					std::cout << std::hex << pair.first << " " << std::dec << pair.second << "\n";
				}*/
				
			}
		}

		// delta time
		float delta_time = clock.restart().asSeconds();
		frame_timer += delta_time;
		if (frame_timer >= 1.0f){
			frame_timer -= 1.0f;
			fps = frames;
			frames = 0;
		}
		std::stringstream ss;
		ss << "FPS: " << fps << "\n" << "OPCODE: " << std::hex << chip8.get_current_opcode();
		text.setString(ss.str());


		// update
//		chip8.emulate_cycle();
		chip8.update_timers();

		// todo set keys
		unsigned char keys[16];
		keys[1] = (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) ? 1 :0;
		keys[2] = (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) ? 1 :0;
		keys[3] = (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) ? 1 :0;
		keys[12] = (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) ? 1 :0;
		keys[4] = (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) ? 1 :0;
		keys[5] = (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) ? 1 :0;
		keys[6] = (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) ? 1 :0;
		keys[13] = (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) ? 1 :0;
		keys[7] = (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) ? 1 :0;
		keys[8] = (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) ? 1 :0;
		keys[9] = (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) ? 1 :0;
		keys[14] = (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) ? 1 :0;
		keys[10] = (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) ? 1 :0;
		keys[0] = (sf::Keyboard::isKeyPressed(sf::Keyboard::X)) ? 1 :0;
		keys[11] = (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) ? 1 :0;
		keys[15] = (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) ? 1 :0;
		chip8.set_keys(keys);

		// clear
		render_window.clear();

		// draw chip8
		const unsigned char* pixels = chip8.get_pixels();
		for (int i = 0; i < 64 * 32; ++i){
			if (pixels[i] == 1){
				texture_pixels[i * 4 + 0] = 0xFF;
				texture_pixels[i * 4 + 1] = 0xFF;
				texture_pixels[i * 4 + 2] = 0xFF;
				texture_pixels[i * 4 + 3] = 0xFF;
			}
			else{
				texture_pixels[i * 4 + 0] = 0x00;
				texture_pixels[i * 4 + 1] = 0x00;
				texture_pixels[i * 4 + 2] = 0x00;
				texture_pixels[i * 4 + 3] = 0xFF;	
			}
		}
		texture.update(texture_pixels);


		// todo draw into frame buffer or something

		render_window.draw(sprite);

		// draw sfml stuff
		render_window.draw(text);

		// show the results
		render_window.display();
		frames += 1;
	}

	return 0;
}