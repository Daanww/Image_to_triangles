#include "stdbool.h"
#include <SDL.h>

extern SDL_Color *raw_image_data;
extern int image_pitch;
extern int image_bpp;
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

bool allocate_image_data()
{
	raw_image_data = (SDL_Color *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(SDL_Color));
	if (raw_image_data == NULL)
	{
		return false;
	}
	return true;
}

//deallocate the memory to store the pixel data of the image
void deallocate_image_data()
{
	free(raw_image_data);
}

void get_surface_width_height(SDL_Surface *surface, int *w, int *h)
{

	SDL_LockSurface(surface);

	*w = surface->w;
	*h = surface->h;

	SDL_UnlockSurface(surface);
	return;
}

int get_bytesperpixel(SDL_Surface *surface)
{

	int bpp = 0;

	SDL_LockSurface(surface);

	bpp = surface->format->BytesPerPixel;

	SDL_UnlockSurface(surface);

	return bpp;
}

int get_pitch(SDL_Surface *surface)
{

	int pitch = 0;

	SDL_LockSurface(surface);

	pitch = surface->pitch;

	SDL_UnlockSurface(surface);

	return pitch;
}

//set the image data
void get_image_data(SDL_Surface *surface)
{
	image_bpp = get_bytesperpixel(surface);
	image_pitch = get_pitch(surface);

	return;
}

SDL_Color find_image_data(SDL_Surface *surface, int pixel_index)
{
	/* Extracting color components from a 32-bit color value */
	SDL_PixelFormat *fmt;

	Uint32 temp, pixel;
	Uint8 red, green, blue, alpha;

	fmt = surface->format;
	SDL_LockSurface(surface);
	pixel = ((Uint32 *)surface->pixels)[pixel_index];
	SDL_UnlockSurface(surface);

	/* Get Red component */
	temp = pixel & fmt->Rmask;	/* Isolate red component */
	temp = temp >> fmt->Rshift; /* Shift it down to 8-bit */
	temp = temp << fmt->Rloss;	/* Expand to a full 8-bit number */
	red = (Uint8)temp;

	/* Get Green component */
	temp = pixel & fmt->Gmask;	/* Isolate green component */
	temp = temp >> fmt->Gshift; /* Shift it down to 8-bit */
	temp = temp << fmt->Gloss;	/* Expand to a full 8-bit number */
	green = (Uint8)temp;

	/* Get Blue component */
	temp = pixel & fmt->Bmask;	/* Isolate blue component */
	temp = temp >> fmt->Bshift; /* Shift it down to 8-bit */
	temp = temp << fmt->Bloss;	/* Expand to a full 8-bit number */
	blue = (Uint8)temp;

	/* Get Alpha component */
	temp = pixel & fmt->Amask;	/* Isolate alpha component */
	temp = temp >> fmt->Ashift; /* Shift it down to 8-bit */
	temp = temp << fmt->Aloss;	/* Expand to a full 8-bit number */
	alpha = (Uint8)temp;

	//printf("Pixel Color -> R: %d,  G: %d,  B: %d,  A: %d\n", red, green, blue, alpha);
	SDL_Color colour = {red, green, blue, alpha};
	return colour;
}


//copy the image_data to the memory allocate at raw_image_data
void copy_image_data(SDL_Surface* surface) {

	for(int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++) {
		raw_image_data[i] = find_image_data(surface, i);
	}

	return;

}


