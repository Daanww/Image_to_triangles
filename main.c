#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2_gfxPrimitives.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "image_data.h"
#include "triangle_data.h"

#define N_CHILDREN 500
#define TIME_BETWEEN_ITERATIONS 0

//Screen dimension constants
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

//filepath to image
const char file_path[] = "../Pink_elephant.jpeg";

//The window we'll be rendering to
SDL_Window *gWindow = NULL;

//The surface contained by the window
SDL_Surface *gScreenSurface = NULL;

//The image we will load and show on the screen
SDL_Surface *gImage = NULL;

//Renderer
SDL_Renderer *gRenderer = NULL;

//texture of the image which will be blitted to the renderer
SDL_Texture *gImageText = NULL;

//Starts up SDL
bool init();

//creates window
bool createWindow(int x_modifier, int y_modifier);

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Loads individual image
SDL_Surface *loadSurface();

//gets width and height of SDL_surface
void get_surface_width_height(SDL_Surface *surface, int *w, int *h);


bool init()
{
	//Initialization flag
	bool success = true;

	// load support for the JPG and PNG image formats
	int flags = IMG_INIT_JPG | IMG_INIT_PNG;
	int initted = IMG_Init(flags);

	if ((initted & flags) != flags)
	{
		printf("IMG_Init: Failed to init required jpg and png support!\n");
		printf("IMG_Init: %s\n", IMG_GetError());
		success = false;
		// handle error

	} //Initialize SDL
	else if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		success = false;
	}


	return success;
}

bool createWindow(int x_modifier, int y_modifier)
{
	//creating succes flag
	bool success = true;

	//Create window, using x_modifier=2 for having the window be twice the size of the image
	gWindow = SDL_CreateWindow( "Images to Triangles", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, x_modifier*SCREEN_WIDTH, y_modifier*SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
	//gWindow = SDL_CreateWindow("Images to Triangles", 0, 9, width, height, SDL_WINDOW_SHOWN);
	if (gWindow == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Get window surface
		gScreenSurface = SDL_GetWindowSurface(gWindow);
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load the image
	gImage = loadSurface();
	if (gImage == NULL)
	{
		printf("Unable to load image %s! SDL Error: %s\n", file_path, SDL_GetError());
		success = false;
	}

	return success;
}

//loads the image and creates the window to hold it
SDL_Surface *loadSurface()
{
	//The final optimized image
	SDL_Surface *optimizedSurface = NULL;

	//Load image at specified path
	SDL_Surface *loadedSurface = IMG_Load(file_path);
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL Error: %s\n", file_path, SDL_GetError());
	}
	else
	{
		
		get_surface_width_height(loadedSurface, &SCREEN_WIDTH, &SCREEN_HEIGHT);
		printf("Image data: width: %i, height %i\n", SCREEN_WIDTH, SCREEN_HEIGHT);

		//create the window
		if(!createWindow(2,1)) {
			return NULL;
		}


		//Convert surface to screen format
		optimizedSurface = SDL_ConvertSurface(loadedSurface, gScreenSurface->format, 0);
		if (optimizedSurface == NULL)
		{
			printf("Unable to optimize image %s! SDL Error: %s\n", file_path, SDL_GetError());
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return optimizedSurface;
}

void close_sdl()
{
	//Deallocate surface
	SDL_FreeSurface(gImage);
	gImage = NULL;

	//Destroy window
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	// unload the dynamically loaded image libraries
	IMG_Quit();

	//Quit SDL subsystems
	SDL_Quit();
}


/* ---- Filled Polygon */

/*!
\brief Internal helper qsort callback functions used in filled polygon drawing.

\param a The surface to draw on.
\param b Vertex array containing X coordinates of the points of the polygon.

\returns Returns 0 if a==b, a negative number if a<b or a positive number if a>b.
*/
int _gfxPrimitivesCompareInt(const void *a, const void *b)
{
	return (*(const int *) a) - (*(const int *) b);
}

/*!
\brief Global vertex array to use if optional parameters are not given in filledPolygonMT calls.

Note: Used for non-multithreaded (default) operation of filledPolygonMT.
*/
static int *gfxPrimitivesPolyIntsGlobal = NULL;

/*!
\brief Flag indicating if global vertex array was already allocated.

Note: Used for non-multithreaded (default) operation of filledPolygonMT.
*/
static int gfxPrimitivesPolyAllocatedGlobal = 0;

//I adapted this function from the SDL2_gfx library for use with this application
//It can now also accept a pixel array where it wil 'draw' the triangle from a positions array

/*!
\brief Draw filled polygon with alpha blending (multi-threaded capable).

Note: The last two parameters are optional; but are required for multithreaded operation.  

\param renderer The renderer to draw on.
\param vx Vertex array containing X coordinates of the points of the filled polygon.
\param vy Vertex array containing Y coordinates of the points of the filled polygon.
\param n Number of points in the vertex array. Minimum number is 3.
\param r The red value of the filled polygon to draw. 
\param g The green value of the filled polygon to draw. 
\param b The blue value of the filled polygon to draw. 
\param a The alpha value of the filled polygon to draw.
\param polyInts Preallocated, temporary vertex array used for sorting vertices. Required for multithreaded operation; set to NULL otherwise.
\param polyAllocated Flag indicating if temporary vertex array was allocated. Required for multithreaded operation; set to NULL otherwise.

\returns Returns 0 on success, -1 on failure.
*/
int draw_array_triangle(SDL_Renderer * renderer, const Sint16 * vx, const Sint16 * vy, int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a, int **polyInts, int *polyAllocated, SDL_Color *pixel_array)
{
	int result;
	int i;
	int y, xa, xb;
	int miny, maxy;
	int x1, y1;
	int x2, y2;
	int ind1, ind2;
	int ints;
	int *gfxPrimitivesPolyInts = NULL;
	int *gfxPrimitivesPolyIntsNew = NULL;
	int gfxPrimitivesPolyAllocated = 0;

	/*
	* Vertex array NULL check 
	*/
	if (vx == NULL) {
		return (-1);
	}
	if (vy == NULL) {
		return (-1);
	}

	/*
	* Sanity check number of edges
	*/
	if (n < 3) {
		return -1;
	}

	/*
	* Map polygon cache  
	*/
	if ((polyInts==NULL) || (polyAllocated==NULL)) {
		/* Use global cache */
		gfxPrimitivesPolyInts = gfxPrimitivesPolyIntsGlobal;
		gfxPrimitivesPolyAllocated = gfxPrimitivesPolyAllocatedGlobal;
	} else {
		/* Use local cache */
		gfxPrimitivesPolyInts = *polyInts;
		gfxPrimitivesPolyAllocated = *polyAllocated;
	}

	/*
	* Allocate temp array, only grow array 
	*/
	if (!gfxPrimitivesPolyAllocated) {
		gfxPrimitivesPolyInts = (int *) malloc(sizeof(int) * n);
		gfxPrimitivesPolyAllocated = n;
	} else {
		if (gfxPrimitivesPolyAllocated < n) {
			gfxPrimitivesPolyIntsNew = (int *) realloc(gfxPrimitivesPolyInts, sizeof(int) * n);
			if (!gfxPrimitivesPolyIntsNew) {
				if (!gfxPrimitivesPolyInts) {
					free(gfxPrimitivesPolyInts);
					gfxPrimitivesPolyInts = NULL;
				}
				gfxPrimitivesPolyAllocated = 0;
			} else {
				gfxPrimitivesPolyInts = gfxPrimitivesPolyIntsNew;
				gfxPrimitivesPolyAllocated = n;
			}
		}
	}

	/*
	* Check temp array
	*/
	if (gfxPrimitivesPolyInts==NULL) {        
		gfxPrimitivesPolyAllocated = 0;
	}

	/*
	* Update cache variables
	*/
	if ((polyInts==NULL) || (polyAllocated==NULL)) { 
		gfxPrimitivesPolyIntsGlobal =  gfxPrimitivesPolyInts;
		gfxPrimitivesPolyAllocatedGlobal = gfxPrimitivesPolyAllocated;
	} else {
		*polyInts = gfxPrimitivesPolyInts;
		*polyAllocated = gfxPrimitivesPolyAllocated;
	}

	/*
	* Check temp array again
	*/
	if (gfxPrimitivesPolyInts==NULL) {        
		return(-1);
	}

	/*
	* Determine Y maxima 
	*/
	miny = vy[0];
	maxy = vy[0];
	for (i = 1; (i < n); i++) {
		if (vy[i] < miny) {
			miny = vy[i];
		} else if (vy[i] > maxy) {
			maxy = vy[i];
		}
	}

	/*
	* Draw, scanning y 
	*/
	result = 0;
	for (y = miny; (y <= maxy); y++) {
		ints = 0;
		for (i = 0; (i < n); i++) {
			if (!i) {
				ind1 = n - 1;
				ind2 = 0;
			} else {
				ind1 = i - 1;
				ind2 = i;
			}
			y1 = vy[ind1];
			y2 = vy[ind2];
			if (y1 < y2) {
				x1 = vx[ind1];
				x2 = vx[ind2];
			} else if (y1 > y2) {
				y2 = vy[ind1];
				y1 = vy[ind2];
				x2 = vx[ind1];
				x1 = vx[ind2];
			} else {
				continue;
			}
			if ( ((y >= y1) && (y < y2)) || ((y == maxy) && (y > y1) && (y <= y2)) ) {
				gfxPrimitivesPolyInts[ints++] = ((65536 * (y - y1)) / (y2 - y1)) * (x2 - x1) + (65536 * x1);
			} 	    
		}

		qsort(gfxPrimitivesPolyInts, ints, sizeof(int), _gfxPrimitivesCompareInt);

		/*
		* Set color 
		*/
		result = 0;
	    result |= SDL_SetRenderDrawBlendMode(renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
		result |= SDL_SetRenderDrawColor(renderer, r, g, b, a);	

		for (i = 0; (i < ints); i += 2) {
			xa = gfxPrimitivesPolyInts[i] + 1;
			xa = (xa >> 16) + ((xa & 32768) >> 15);
			xb = gfxPrimitivesPolyInts[i+1] - 1;
			xb = (xb >> 16) + ((xb & 32768) >> 15);
			//result |= hline(renderer, xa, xb, y);

			for(int a = xa; a <= xb; a++) {
				pixel_array[a+SCREEN_WIDTH*y].r = r;
				pixel_array[a+SCREEN_WIDTH*y].g = g;
				pixel_array[a+SCREEN_WIDTH*y].b = b;
				pixel_array[a+SCREEN_WIDTH*y].a = a;
			}
		}
	}

	return (result);
}


//'draws' all triangles in the _triangles array and _positions array to the specified pixel array
//this requires the triangles array and the _positions array to be filled with valid data.
void draw_array_triangles(SDL_Color *pixel_array, coordinate* _positions, triangle *_triangles) {

	int vx[3] = {0};
	int vy[3] = {0};

	//setting the pixel array to all black before drawing
	for(int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++) {
		pixel_array[i].r = 0;
		pixel_array[i].g = 0;
		pixel_array[i].b = 0;
	}


	for(int i = 0 ; i < n_triangles; i++) {

		//printf("_triangles[%i].p1 = %i\n", i, _triangles[i].p1);
		//printf("_triangles[%i].p2 = %i\n", i, _triangles[i].p2);
		//printf("_triangles[%i].p3 = %i\n", i, _triangles[i].p3);

		


		vx[0] = positions[_triangles[i].p1].x;
		vx[1] = positions[_triangles[i].p2].x;
		vx[2] = positions[_triangles[i].p3].x;

		vy[0] = positions[_triangles[i].p1].y;
		vy[1] = positions[_triangles[i].p2].y;
		vy[2] = positions[_triangles[i].p3].y;


		draw_array_triangle(gRenderer, vx, vy, 3, _triangles[i].c.r, _triangles[i].c.g, _triangles[i].c.b, _triangles[i].c.a, NULL, NULL, pixel_array);
	}

	return;
}


//returns the difference between the colour values of the reference image and the colour values of the pixel array
//This assumes that the pixel_array is already drawn with the colours from a positions and triangles combination
//uses sum squared error
unsigned long get_difference_image_triangles(SDL_Color *pixel_array) {

	unsigned long sum = 0;

	for(int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++) {

		/*
		//using standard distance comparison
		sum += (raw_image_data[i].r-pixel_array[i].r)*(raw_image_data[i].r-pixel_array[i].r);
		sum += (raw_image_data[i].g-pixel_array[i].g)*(raw_image_data[i].g-pixel_array[i].g);
		sum += (raw_image_data[i].b-pixel_array[i].b)*(raw_image_data[i].b-pixel_array[i].b);
		*/

		//using fancy formula found on wikipedia
		//https://en.wikipedia.org/wiki/Color_difference
		int delta_r_sq = (raw_image_data[i].r-pixel_array[i].r)*(raw_image_data[i].r-pixel_array[i].r);
		int delta_g_sq = (raw_image_data[i].g-pixel_array[i].g)*(raw_image_data[i].g-pixel_array[i].g);
		int delta_b_sq = (raw_image_data[i].b-pixel_array[i].b)*(raw_image_data[i].b-pixel_array[i].b);
		int average_r = (raw_image_data[i].r + pixel_array[i].r)/2;

		sum += (2.0 + ((float)average_r)/256.0)*delta_r_sq + 4*delta_g_sq + (2.0 + (255.0 - (float)average_r)/256.0)*delta_b_sq;


	}

	return sum;	
}

//generates a random number between -max and max (inclusive)
//example: if max = 5 this will generate {-5,5} inclusive
int generate_random(int max) {

	int num = rand() % (max+1);
	if(rand() % 2)
		num = -num;

	return num;
}

//generates positions and colours for a child of the parent based on the parents coordinates and the parent_score and the triangles array
//it stores these positions in the child_positions array and child_triangles
//both the parent_positions and child_positions arrays are assumed to be N_POSITIONS in size and the child_triangles is assumed to be n_triangles in size
void generate_child(coordinate *parent_positions, coordinate *child_positions, triangle *child_triangles, unsigned long parent_score) {

	//the maximal amount of change in a single number for the child
	int delta = 2*ceil(logl(parent_score/SCREEN_HEIGHT/SCREEN_WIDTH));

	//generating new positions
	for(int i = 0; i < N_POSITIONS; i++) {
		//first check to see if positions actually need to be modified
		if(parent_positions[i].x != 0 && parent_positions[i].x != SCREEN_WIDTH-1) {

			//generate a new position
			child_positions[i].x = (parent_positions[i].x+generate_random(delta));
			//printf("child_positions[%i].x = %i\n", i, child_positions[i].x);

			//make sure that it is within bounds
			if(child_positions[i].x < 1)
				child_positions[i].x = 1;
			if(child_positions[i].x > SCREEN_WIDTH-2)
				child_positions[i].x = SCREEN_WIDTH-2;

		}
		else {
			//copy old position
			child_positions[i].x = parent_positions[i].x;
		}

		if(parent_positions[i].y != 0 && parent_positions[i].y != SCREEN_HEIGHT-1) {

			child_positions[i].y = (parent_positions[i].y+generate_random(delta));
			//printf("child_positions[%i].y = %i\n", i, child_positions[i].y);

			//make sure that it is within bounds.
			if(child_positions[i].y < 1)
				child_positions[i].y = 1;
			if(child_positions[i].y > SCREEN_HEIGHT-2)
				child_positions[i].y = SCREEN_HEIGHT-2;

		}
		else {
			child_positions[i].y = parent_positions[i].y;
		}
	}



	//generating new colours
	for(int i = 0; i < n_triangles; i++) {

		int red = triangles[i].c.r+generate_random(delta);
		red = fmin(red, 255);
		red = fmax(red, 0);
		child_triangles[i].c.r = red;

		int green = triangles[i].c.g+generate_random(delta);
		green = fmin(green, 255);
		green = fmax(green, 0);
		child_triangles[i].c.g = green;

		int blue = triangles[i].c.b+generate_random(delta);
		blue = fmin(blue, 255);
		blue = fmax(blue, 0);
		child_triangles[i].c.b = blue;

	}


	return;

}

//stores the specified positions and triangles arrays in the specified positions and specified arrays
//assumes that source_positions has size N_POSITIONS and source_triangles has size n_triangles
//same for dest
void copy_positions_triangles(coordinate *source_positions, triangle *source_triangles, coordinate *dest_positions, triangle *dest_triangles) {

	memcpy(dest_positions, source_positions, N_POSITIONS*sizeof(coordinate));
	memcpy(dest_triangles, source_triangles, n_triangles*sizeof(triangle));

	return;
}


//from the current positions and triangles array this function will try to find the best child.
//it will generate N_CHILDREN and find the one with the lowest error score
//it will then store that child in the positions and triangles array
//it also returns the score of that child
unsigned long find_next_parent(unsigned long parent_score) {

	//set the lowest score to the maximum value, makes for easy comparisons
	unsigned long lowest_score = (unsigned long) -1;

	//allocating stuff for children
	coordinate *child_positions = malloc(N_POSITIONS*sizeof(coordinate));
	triangle *child_triangles = malloc(n_triangles*sizeof(triangle));

	//copying the configuration of the triangles from the parent because it is needed
	//else the child_triangles does not contain the relations between the positions
	memcpy(child_triangles, triangles, n_triangles*sizeof(triangle));


	//allocating stuff for the best of the children
	coordinate *best_child_positions = malloc(N_POSITIONS*sizeof(coordinate));
	triangle *best_child_triangles = malloc(n_triangles*sizeof(triangle));

	//pixel array for storing the pixels of a generated child
	SDL_Color *pixel_array = malloc(SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(SDL_Color));

	//keep track of wether it found a better child than its parent
	bool has_found_better_child = false;

	for(int i = 0; i < N_CHILDREN; i++) {
		//first generate a new child
		generate_child(positions, child_positions, child_triangles, parent_score);
		//then draw its pixels
		draw_array_triangles(pixel_array, child_positions, child_triangles);
		//then retrieve its score
		unsigned long score = get_difference_image_triangles(pixel_array);
		
		//if this child is better than the current best child and better than the parent
		if(score < lowest_score && score < parent_score) {
			//copy the childs attributes to best child
			copy_positions_triangles(child_positions, child_triangles, best_child_positions, best_child_triangles);
			//set the new lowest score
			lowest_score = score;
			//we have found a better child
			has_found_better_child = true;
		}
	}

	//set the best child as the new parent
	if(has_found_better_child) {
		copy_positions_triangles(best_child_positions, best_child_triangles, positions, triangles);
	}




	free(child_positions);
	free(child_triangles);
	free(best_child_positions);
	free(best_child_triangles);
	free(pixel_array);
	
	
	/*
	if(has_found_better_child) {
		return lowest_score;
	}
	else {
		return parent_score;
	}
	*/
	

	return lowest_score;

}

//finds all neighbours for a given point
//the order in which they are added to the data structure is also the right order for connecting the lines between points to form a polygon around the central point
//https://stackoverflow.com/questions/16140831/how-to-find-the-polygon-enclosing-a-point-from-a-set-of-lines

int main(int argc, char *args[])
{

	//initialize random seed
	srand(time(NULL));


	//Start up SDL
	if (!init())
	{
		printf("Failed to initialize!\n");
		return 1;
	}
	else
	{
		//Load media and create a window
		if (!loadMedia(file_path))
		{
			printf("Failed to load media!\n");
			return 1;
		}
		else
		{
			get_image_data(gImage);
			printf("BytesPerPixel = %i\n", image_bpp);
			printf("pitch = %i\n", image_pitch);


			int mouse_x = 0;
			int mouse_y = 0;
			int pixel_index = 0;
			SDL_Color colour = find_image_data(gImage, pixel_index);

			if(!allocate_image_data()) {
				printf("Could not allocate memory for raw_image_data!\n");
				return 1;
			}

			copy_image_data(gImage);

			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			gImageText = SDL_CreateTextureFromSurface(gRenderer, gImage);


			char generated_file_name[128] = "Test_file"; 

			//small section to test the generation of positions for triangles
			if(!generate_positions()) {
				return 1;
			}
			if(!write_positions_to_file(generated_file_name)) {
				return 1;
			}
			if(!generate_triangles_file(generated_file_name)) {
				return 1;
			}
			if(!read_triangles_from_file(generated_file_name)) {
				return 1;
			}

			generate_triangle_colours();

			SDL_Color *data = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(SDL_Color));
			if (data == NULL)
			{
				return 1;
			}
			

			draw_array_triangles(data, positions, triangles);

			unsigned long difference = get_difference_image_triangles(data);

			printf("Difference between image and triangles: %lu\n", difference);

			free(data);


			

			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//While application is running
			while (!quit)
			{
				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
				}

				SDL_GetMouseState(&mouse_x, &mouse_y);
				pixel_index = SCREEN_WIDTH*mouse_y + mouse_x;
				//colour = find_image_data(gImage, pixel_index);
				colour = raw_image_data[pixel_index];



				//Apply the image stretched
				SDL_Rect stretchRect;
				stretchRect.x = 0;
				stretchRect.y = 0;
				stretchRect.w = SCREEN_WIDTH;
				stretchRect.h = SCREEN_HEIGHT;
				SDL_RenderCopy(gRenderer, gImageText, NULL, &stretchRect);



				blit_triangles(gRenderer, positions);

				//Update the surface
				SDL_RenderPresent(gRenderer);


				SDL_Delay(TIME_BETWEEN_ITERATIONS);


				//get the next iteration of the algorithm
				difference = find_next_parent(difference);
				printf("Difference between image and triangles: %lu\n", difference);

			}
		}
	}

	//Free resources and close SDL
	close_sdl();

	deallocate_image_data();

	return 0;
}