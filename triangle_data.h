#ifndef TRIANGLE_DATA_H
#define TRIANGLE_DATA_H


#include <SDL2/SDL.h>

#define N_POSITIONS 10


typedef struct Coordinate {
    int x;
    int y;
} coordinate;

//p1,p2,p3 represent indexes in a positions array
typedef struct Triangle
{
	int p1;
	int p2;
	int p3;
	SDL_Color c;
} triangle;



coordinate positions[N_POSITIONS];
triangle triangles[N_POSITIONS*N_POSITIONS];
int n_triangles;


//generates a set of positions which it stores in the positions array
//this set of positions is valid for the given SCREEN_WIDTH and SCREEN_HEIGHT
//it is also guaranteed to include the 4 corners to make sure that the triangulation of these positions results in a proper rectangle
//returns true if generation as successfull and returns false otherwise.
bool generate_positions();

//after the positions array has been filled, this functions will store the positions in a file in a suitable format for the triangle library
//the triangle library can then read this file and triangulate the points
bool write_positions_to_file(char* name);

//This function uses the triangle application which is contained in the build folder to make a triangulation of the positions writen to a file by write_positions_to_file
bool generate_triangles_file(char *name);

//This function reads the triangles from the file generated by the triange library. The parameter name is assumed to be the same string as used by the write_positions_to_file function to setup the positions
//It stores the triangles in the triangles array
bool read_triangles_from_file(char* name);

//generates initial colours for the triangles array
//these are fully random
void generate_triangle_colours();


//blits all the triangles to the renderer.
//it blits the rectangle formed by all the triangles on the right half of the window
//the window should be 2*SCREEN_WIDTH by SCREEN_HEIGHT in size
void blit_triangles(SDL_Renderer* renderer, coordinate *_positions);





#endif