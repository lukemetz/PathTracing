demo: cldemo.c
	gcc -o cldemo -std=gnu99 -Wall cldemo.c -lOpenCL -lm -lglfw -lGLEW -lGL -g
