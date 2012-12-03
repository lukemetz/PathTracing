demo: cldemo.c draw_utils.c draw_utils.h general_utils.h general_utils.c
	gcc -o cldemo -std=gnu99 -Wall cldemo.c draw_utils.c general_utils.c -lOpenCL -lm -lglfw -lGLEW -lGL -g

orion_demo: cldemo.c
	gcc -o cldemo -std=gnu99 -Wall cldemo.c -lOpenCL -lm -lglfw -lGLEW -lGL -g -I/usr/include/nvidia-current
