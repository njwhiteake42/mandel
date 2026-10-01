#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#define PIXEL_SIZE 15
#define SCREEN_SIZE 600 / PIXEL_SIZE
#define PAUSE_BETWEEN_PIXELS 16

uint32_t palette[] = {0x0000FF, 0x00FF00, 0x00FFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF, 0x000000};
uint32_t screen_one[SCREEN_SIZE][SCREEN_SIZE], screen_two[SCREEN_SIZE][SCREEN_SIZE], screen_three[SCREEN_SIZE][SCREEN_SIZE];

int screen_x = 0;
int screen_one_y = 0;

int screen_two_offset = 0;
int screen_two_spacing = SCREEN_SIZE / 4;

int screen_three_offset = 0;
int screen_three_spacing = SCREEN_SIZE / 10;

void plot(SDL_Surface *surface, int x, int y, Uint32 color) {
	/*
	if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
		return;
	}
	
	Uint32 *base = surface->pixels;
	
	*(base + (y * (surface->pitch / sizeof(Uint32)) + x)) = color;
	*/
	SDL_FillRect(surface, &((SDL_Rect) {x, y, PIXEL_SIZE, PIXEL_SIZE}), color);
}

float scale(float old_value, float old_max, float new_min, float new_max) {
	return ((old_value) / (old_max)) * (new_max - new_min) + new_min;
}

uint32_t get_pixel(int px, int py) {
	float x0 = scale(px, SCREEN_SIZE, -2.0f, 0.47f);
	float y0 = scale(py, SCREEN_SIZE, -1.12f, 1.12f);
	float x = 0.0f, y = 0.0f;
	
	int iteration = 0, max_iteration = 256;
	
	do {
		float xtemp = x * x - y * y + x0;
		y = 2 * x * y + y0;
		x = xtemp;
		
		iteration = iteration + 1;
	} while ((x * x + y * y <= (2 * 2)) && iteration < max_iteration);
	
	uint32_t color = palette[iteration % 8];
	return color;
}

void screens_tick() {
	screen_one[screen_x][screen_one_y] = get_pixel(screen_x, screen_one_y);
	
	if (screen_two_offset != screen_two_spacing) {
		for (int s2_y = screen_two_offset; s2_y < SCREEN_SIZE; s2_y += screen_two_spacing) {
			screen_two[screen_x][s2_y] = get_pixel(screen_x, s2_y);
		}
	}
	
	if (screen_three_offset != screen_three_spacing) {
		for (int s3_y = screen_three_offset; s3_y < SCREEN_SIZE; s3_y += screen_three_spacing) {
			screen_three[screen_x][s3_y] = get_pixel(screen_x, s3_y);
		}
	}
	
	screen_x++;
	
	if (screen_x == SCREEN_SIZE) {
		screen_one_y++;
		if (screen_one_y == SCREEN_SIZE) {
			screen_one_y = 0;
		}
		screen_x = 0;
		if (screen_two_offset != screen_two_spacing) {
			screen_two_offset++;
		}
		if (screen_three_offset != screen_three_spacing) {
			screen_three_offset++;
		}
	}
}

uint32_t draw_callback(uint32_t interval, void *param) {
	SDL_Event event;
	SDL_UserEvent userevent;

	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = NULL;
	userevent.data2 = NULL;

	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);
	
	//puts("callback!");
	return interval;
}

int main() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		printf("error: %s\n", SDL_GetError());
		SDL_Quit();
	}
	
	SDL_Window *window = SDL_CreateWindow("Mandalbrot Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1800, 600, 0);
	SDL_Surface *surface = SDL_GetWindowSurface(window);
	
	SDL_TimerID draw_timer = SDL_AddTimer(PAUSE_BETWEEN_PIXELS, draw_callback, &draw_timer);
	
	for (int y = 0; y < SCREEN_SIZE; y++) {
		for (int x = 0; x < SCREEN_SIZE; x++) {
			screen_one[x][y] = 0x00000000;
			screen_two[x][y] = 0x00000000;
			screen_three[x][y] = 0x00000000;
		}
	}
	
	/*
	screen_one[0][0] = 0x00FF0000;
	screen_one[1][0] = 0x0000FF00;
	screen_one[0][1] = 0x000000FF;
	
	screen_two[0][0] = 0x00FF0000;
	screen_two[1][0] = 0x0000FF00;
	screen_two[0][1] = 0x000000FF;
	
	screen_three[0][0] = 0x00FF0000;
	screen_three[1][0] = 0x0000FF00;
	screen_three[0][1] = 0x000000FF;
	*/
	
	bool should_close = false;
	while (!should_close) {
		SDL_Event current_event;
		
		if (SDL_WaitEvent(&current_event) != 0) {
			switch (current_event.type) {
				case SDL_QUIT:
					should_close = true;
					break;
				case SDL_KEYDOWN: {
					SDL_Keycode key = current_event.key.keysym.sym;
					if (key == SDLK_r) {
						puts("reset");
					}
					if (key == SDLK_ESCAPE || key == SDLK_q) {
						should_close = true;
					}
					break;
				}
				case SDL_USEREVENT: {
					screens_tick();
					break;
				}
			}
		}
		
		SDL_FillRect(surface, NULL, 0x000000);
		
		for (int y = 0; y < SCREEN_SIZE; y++) {
			for (int x = 0; x < SCREEN_SIZE; x++) {
				plot(surface, x * PIXEL_SIZE, y * PIXEL_SIZE, screen_one[x][y]);
				plot(surface, 600 + x * PIXEL_SIZE, y * PIXEL_SIZE, screen_two[x][y]);
				plot(surface, 1200 + x * PIXEL_SIZE, y * PIXEL_SIZE, screen_three[x][y]);
			}
		}
		SDL_UpdateWindowSurface(window);
	}
	
	SDL_RemoveTimer(draw_timer);
	
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	return 0;
}
