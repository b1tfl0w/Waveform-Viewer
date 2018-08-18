# Waveform-Viewer
A simple application which draws a waveform from mic input signal or saw oscillator using SDL2, opengl2 and RtAudio.

Libraries used:
SDL2 https://www.libsdl.org/ 

RtAudio https://www.music.mcgill.ca/~gary/rtaudio/index.html 

OpenGL https://www.opengl.org/ 

//Install SDL2 and RtAudio Development libraries:

sudo apt install librtaudio-dev libsdl2-dev

//compile:

g++ -g -O0 -Wall -o program main.cpp -I/usr/include/SDL2 -I /usr/include/GL -I /usr/include -I /usr/include/rtaudio -lSDL2 -lSDL2main -lGL -lGLU -lrtaudio

//run the program:

./program

press 1 for saw wave output, and 2 for input to output.

![Alt text](waveform-viewer-screenshot.jpg?raw=true "Screenshot")
