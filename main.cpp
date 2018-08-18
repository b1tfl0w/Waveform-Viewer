/* 
Part of this code is taken from Lazy Foo SDL tutorials, check it out:
http://lazyfoo.net/tutorials/SDL/
 */

//Using SDL, SDL OpenGL, standard IO, and, strings
#include <SDL.h>
#include <SDL_opengl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <string>
#include "main.h"

int argcM = 3;
char *argsM[] = {"program", "2", "44100"};

//Screen dimension constants
const int SCREEN_WIDTH = 512;
const int SCREEN_HEIGHT = 512;

//Starts up SDL, creates window, and initializes OpenGL
bool init();

//Initializes matrices and clear color
bool initGL();

//Input handler
void handleKeys(unsigned char key, int x, int y);

//Per frame update
void update();

//Renders quad to the screen
void render();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window *gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

//Render flag
bool gRenderQuad = false;

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Use OpenGL 2.1
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

		//Create window
		gWindow = SDL_CreateWindow("Waveform Viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create context
			gContext = SDL_GL_CreateContext(gWindow);
			if (gContext == NULL)
			{
				printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Use Vsync
				if (SDL_GL_SetSwapInterval(1) < 0)
				{
					printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
				}

				//Initialize OpenGL
				if (!initGL())
				{
					printf("Unable to initialize OpenGL!\n");
					success = false;
				}
			}
		}
	}

	return success;
}

bool initGL()
{
	bool success = true;
	GLenum error = GL_NO_ERROR;

	//Initialize Projection Matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//Check for error
	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Error initializing OpenGL! %s\n", gluErrorString(error));
		success = false;
	}

	//Initialize Modelview Matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//Check for error
	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Error initializing OpenGL! %s\n", gluErrorString(error));
		success = false;
	}

	//Initialize clear color
	glClearColor(0.f, 0.f, 0.f, 1.f);

	//Check for error
	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Error initializing OpenGL! %s\n", gluErrorString(error));
		success = false;
	}

	return success;
}

void handleKeys(unsigned char key, int x, int y)
{
	//Toggle quad
	if (key == 'q')
	{
		gRenderQuad = !gRenderQuad;
	}
	if (key == '1')
	{
		// Stop the stream
		cleanUp();
		//saw mode
		if (!playSaw(argcM, argsM))
		{
			printf("Failed to initialize audio!\n");
		}
	}
	if (key == '2')
	{
		// Stop the stream
		cleanUp();
		//duplex mode
		if (!playDuplex(argcM, argsM))
		{
			printf("Failed to initialize audio!\n");
		}
	}
}

void update()
{
	//No per frame update needed
}

void render()
{
	//Clear color buffer
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(map(*mybuffer, -0.1f, 0.1f, 0.0f, 1.0f), map(*mybuffer, -0.1f, 0.1f, 0.0f, 1.0f), map(*mybuffer, -0.1f, 0.1f, 0.0f, 1.0f), 1.f);
	for (int i = 0; i < 512; i++)
	{
		//draw waveform
		glBegin(GL_LINES);
		glVertex2f(map(i, 0, 512, -1.0f, 1.0f), map(mybuffer[i], -0.1, 0.1, -1.0f, 1.0f));
		glVertex2f(map(i + 1, 0, 512, -1.0f, 1.0f), map(mybuffer[i + 1], -0.1, 0.1, -1.0f, 1.0f));
		glEnd();
	}

	//Render quad
	if (gRenderQuad)
	{
		glBegin(GL_QUADS);
		glVertex2f(-0.5f, -0.5f);
		glVertex2f(0.5f, -0.5f);
		glVertex2f(0.5f, 0.5f);
		glVertex2f(-0.5f, 0.5f);
		glEnd();
	}
}

void close()
{
	//Destroy window
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

int main(int argc, char *args[])
{
	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize sdl!\n");
	}
	else
	{
		//Main loop flag
		bool quit = false;

		//Event handler
		SDL_Event e;

		//Enable text input
		SDL_StartTextInput();

		//start audio
		if (!playSaw(argcM, argsM))
		{
			printf("Failed to initialize audio!\n");
		}

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
				//Handle keypress with current mouse position
				else if (e.type == SDL_TEXTINPUT)
				{
					int x = 0, y = 0;
					SDL_GetMouseState(&x, &y);
					handleKeys(e.text.text[0], x, y);
				}
			}

			//Render quad
			render();

			//Update screen
			SDL_GL_SwapWindow(gWindow);
		}

		//Disable text input
		SDL_StopTextInput();
	}

	//Free resources and close SDL
	cleanUp();
	close();

	return 0;
}
