#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define PAUSE_BETWEEN_PIXELS 16

uint32_t palette[] = {0x000000, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF, 0xFFFFFF};

struct screen {
	int offset, spacing, scale, pix_size;
	int current_x;
	bool done;
	int current_scale, workers;
	int time;
};

struct scale_points {
	int scale, pix_size;
};

struct screen screens[] = {
				{.offset = 0, .spacing = 1, .scale = 40, .pix_size = 15, .current_x = 0, .done = false, .current_scale = 0, .workers = 1, .time = 0}, 
				{.offset = 0, .spacing = 10, .scale = 40, .pix_size = 15, .current_x = 0, .done = false, .current_scale = 0, .workers = 4, .time = 0}, 
				{.offset = 0, .spacing = 4, .scale = 40, .pix_size = 15, .current_x = 0, .done = false, .current_scale = 0, .workers = 10, .time = 0}
			  };
			  
const struct scale_points points[] = {{40, 15}, {60, 10}, {100, 6}, {120, 5}, {200, 3}, {300, 2}, {600, 1}};

SDL_Surface *surface;

void plot(int x, int y, int size, Uint32 color) {
	SDL_FillRect(surface, &((SDL_Rect) {x, y, size, size}), color);
}

float lerp(float old_value, float old_max, float new_min, float new_max) {
	return ((old_value) / (old_max)) * (new_max - new_min) + new_min;
}

uint32_t get_pixel(int px, int py, int scale) {
	float x0 = lerp(px, scale, -2.0f, 0.47f);
	float y0 = lerp(py, scale, -1.12f, 1.12f);
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

void tick() {
	for (unsigned int i = 0; i < sizeof(screens) / sizeof(screens[i]); i++) {
		struct screen *s = &screens[i];
		
		if (s->done) continue;
		
		if (s->spacing == 1) {
			plot((i * 600) + s->current_x * s->pix_size, s->offset * s->pix_size, s->pix_size, get_pixel(s->current_x, s->offset, s->scale));
		} else {
			for (int y = s->offset; y < s->scale; y += s->spacing) {
				plot((i * 600) + (s->current_x * s->pix_size), y * s->pix_size, s->pix_size, get_pixel(s->current_x, y, s->scale));
			}
		}
		
		s->current_x++;
		if (s->current_x == s->scale) {
			s->current_x = 0;
			s->offset++;
			if (s->spacing == 1) {
				if (s->offset == s->scale) {
					s->offset = 0;
					s->done = true;
				}
			} else {
				if (s->offset == s->spacing) {
					//puts("got here");
					s->offset = 0;
					s->done = true;
				}
			}
		}
		
		if (s->done) {
			s->current_scale++;
			
			if (s->current_scale < 7) {
				s->scale = points[s->current_scale].scale;
				s->pix_size = points[s->current_scale].pix_size;
				s->spacing = (s->scale / s->workers);
				
				SDL_Rect clear_rect = {.x = (i * 600), .y = 0, .w = 600, .h = 600};
				SDL_FillRect(surface, &clear_rect, 0x000000);
				s->done = false;
			}
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
	return interval;
}

uint32_t timer_callback(uint32_t interval, void *param) {
	SDL_Event event;
	SDL_UserEvent userevent;

	userevent.type = SDL_USEREVENT;
	userevent.code = 1;
	userevent.data1 = NULL;
	userevent.data2 = NULL;

	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);
	return interval;
}

int main() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		printf("error: %s\n", SDL_GetError());
		SDL_Quit();
	}
	
	if (TTF_Init() != 0) {
		printf("error: %s\n", TTF_GetError());
		SDL_Quit();
	}
	
	SDL_Window *window = SDL_CreateWindow("Mandelbrot Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1800, 800, 0);
	surface = SDL_GetWindowSurface(window);
	
	SDL_TimerID draw_timer = SDL_AddTimer(PAUSE_BETWEEN_PIXELS, draw_callback, &draw_timer);
	SDL_TimerID clock = SDL_AddTimer(1000, timer_callback, &clock);
	
	const SDL_Rect text_area = {.x = 0, .y = 600, .w = 1800, .h = 200};
	TTF_Font *font = TTF_OpenFont("courbd.ttf", 36);
	
	const SDL_Color WHITE = {255, 255, 255, 255}, RED = {255, 0, 0, 255};
	
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
					//screens_tick();
					if (current_event.user.code == 0) {
						tick();
						
						SDL_FillRect(surface, &text_area, 0x00000000);
						for (int i = 0; i < 3; i++) {
							char iter[100], timer[100], n[100];
							
							sprintf(iter, "Iteration: %d/7 (%dx%d)", screens[i].current_scale + 1, screens[i].scale, screens[i].scale);
							
							int time = screens[i].time;
							sprintf(timer, "Elapsed: %d:%02d", time / 60, time % 60);
							
							sprintf(n, "%d %s", screens[i].workers, (screens[i].workers == 1) ? "worker" : "parallel workers");
							
							SDL_Surface *iter_s = TTF_RenderText_Solid(font, iter, WHITE);
							SDL_Surface *timer_s = TTF_RenderText_Solid(font, timer, WHITE);
							SDL_Surface *n_s = TTF_RenderText_Solid(font, n, RED);
							
							SDL_Rect iter_dest = {.x = (i * 600) + 50, .y = 650}, timer_dest = {.x = (i * 600) + 50, .y = 686}, n_dest = {.x = (i * 600) + 50, .y = 722};
							SDL_BlitSurface(iter_s, NULL, surface, &iter_dest);
							SDL_BlitSurface(timer_s, NULL, surface, &timer_dest);
							SDL_BlitSurface(n_s, NULL, surface, &n_dest);
							
							SDL_FreeSurface(n_s);
							SDL_FreeSurface(timer_s);
							SDL_FreeSurface(iter_s);
						}
					} else if (current_event.user.code == 1) {
						for (int i = 0; i < 3; i++) {
							if (screens[i].done) continue;
							screens[i].time++;
						}
					}
					
					
					break;
				}
			}
		}
		
		SDL_UpdateWindowSurface(window);
	}
	
	SDL_RemoveTimer(clock);
	SDL_RemoveTimer(draw_timer);
	
	SDL_DestroyWindow(window);
	
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	
	return 0;
}
