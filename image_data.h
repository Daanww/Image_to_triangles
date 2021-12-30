#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <SDL2/SDL.h>

SDL_Color *raw_image_data = NULL;
int image_pitch = 0;
int image_bpp = 0;

//allocate the memory to store the pixel data of the image
//returns true if allocations was succesfull, returns false otherwise
bool allocate_image_data();

//deallocate the memory to store the pixel data of the image
void deallocate_image_data();

//gets width and height of SDL_surface
void get_surface_width_height(SDL_Surface *surface, int *w, int *h);

//gets the bytesperpixel of SDL_surface
int get_bytesperpixel(SDL_Surface *surface);

//get the pitch of SDL_surface
int get_pitch(SDL_Surface *surface);

//sets the image_pitch and image_bpp
void get_image_data(SDL_Surface *surface);

//retrieving pixel data from image
SDL_Color find_image_data(SDL_Surface *surface, int pixel_index);

//copies the pixel data of the surface to the memory allocated at raw_image_data
void copy_image_data(SDL_Surface* surface);

#endif