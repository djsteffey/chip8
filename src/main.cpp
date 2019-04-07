#include <SFML/Graphics.hpp>
#include "CChip8.hpp"
#include <sstream>

int main(int argc, char* argv[]){
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
	text.setFillColor(sf::Color::Red);

	// timing
	sf::Clock clock;
	int frames = 0;
	int fps = 0;
	float frame_timer = 0.0f;

	// texture for rendering pixels to
	sf::Texture texture;
	texture.create(64, 32);
	sf::Sprite sprite(texture);
	sprite.setScale(10, 10);
	sf::Uint8 texture_pixels[64 * 32 * 4];

	// draw
	auto draw_callback = [&texture_pixels, &texture](unsigned char* pixels)-> void {
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
	};

	// sound
	auto sound_callback = [](bool start)-> void{
		if (start){
			std::printf("Start Sound\n");
		}
		else{
			std::printf("Stop Sound\n");
		}
		
	};

	// the chip8
	djs::chip8::CChip8 chip8(draw_callback, sound_callback);
	chip8.load_game(std::string(argv[1]));
	chip8.reset();

	// run the loop
	bool running = true;
	while (running){
		// events
		sf::Event event;
		while (render_window.pollEvent(event)){
			if (event.type == sf::Event::Closed){
				running = false;
			}
			else if (event.type == sf::Event::KeyPressed){
				switch (event.key.code){
					case sf::Keyboard::Key::Num1: { chip8.set_key(0x1, true); } break;
					case sf::Keyboard::Key::Num2: { chip8.set_key(0x2, true); } break;
					case sf::Keyboard::Key::Num3: { chip8.set_key(0x3, true); } break;
					case sf::Keyboard::Key::Num4: { chip8.set_key(0xC, true); } break;
					case sf::Keyboard::Key::Q: { chip8.set_key(0x4, true); } break;
					case sf::Keyboard::Key::W: { chip8.set_key(0x5, true); } break;
					case sf::Keyboard::Key::E: { chip8.set_key(0x6, true); } break;
					case sf::Keyboard::Key::R: { chip8.set_key(0xD, true); } break;
					case sf::Keyboard::Key::A: { chip8.set_key(0x7, true); } break;
					case sf::Keyboard::Key::S: { chip8.set_key(0x8, true); } break;
					case sf::Keyboard::Key::D: { chip8.set_key(0x9, true); } break;
					case sf::Keyboard::Key::F: { chip8.set_key(0xE, true); } break;
					case sf::Keyboard::Key::Z: { chip8.set_key(0xA, true); } break;
					case sf::Keyboard::Key::X: { chip8.set_key(0x0, true); } break;
					case sf::Keyboard::Key::C: { chip8.set_key(0xB, true); } break;
					case sf::Keyboard::Key::V: { chip8.set_key(0xF, true); } break;
				}
			}
			else if (event.type == sf::Event::KeyReleased){
				switch (event.key.code){
					case sf::Keyboard::Key::Num1: { chip8.set_key(0x1, false); } break;
					case sf::Keyboard::Key::Num2: { chip8.set_key(0x2, false); } break;
					case sf::Keyboard::Key::Num3: { chip8.set_key(0x3, false); } break;
					case sf::Keyboard::Key::Num4: { chip8.set_key(0xC, false); } break;
					case sf::Keyboard::Key::Q: { chip8.set_key(0x4, false); } break;
					case sf::Keyboard::Key::W: { chip8.set_key(0x5, false); } break;
					case sf::Keyboard::Key::E: { chip8.set_key(0x6, false); } break;
					case sf::Keyboard::Key::R: { chip8.set_key(0xD, false); } break;
					case sf::Keyboard::Key::A: { chip8.set_key(0x7, false); } break;
					case sf::Keyboard::Key::S: { chip8.set_key(0x8, false); } break;
					case sf::Keyboard::Key::D: { chip8.set_key(0x9, false); } break;
					case sf::Keyboard::Key::F: { chip8.set_key(0xE, false); } break;
					case sf::Keyboard::Key::Z: { chip8.set_key(0xA, false); } break;
					case sf::Keyboard::Key::X: { chip8.set_key(0x0, false); } break;
					case sf::Keyboard::Key::C: { chip8.set_key(0xB, false); } break;
					case sf::Keyboard::Key::V: { chip8.set_key(0xF, false); } break;
				}
			}
		}

		// update
		float delta_time = clock.restart().asSeconds();
		frame_timer += delta_time;
		if (frame_timer >= 1.0f){
			frame_timer -= 1.0f;
			fps = frames;
			frames = 0;
		}
		std::stringstream ss;
		ss << "FPS: " << fps;
		text.setString(ss.str());
		chip8.update(delta_time);

		// clear
		render_window.clear();

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