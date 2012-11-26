#include "draw_utils.h"

void make_window(int width, int height)
{
	if( !glfwInit() ) {
	    fprintf( stderr, "Failed to initialize GLFW\n" );
	    exit(-1);
	}
	// Open a window and create its OpenGL context
	if( !glfwOpenWindow(width, height, 0,0,0,0, 32,0, GLFW_WINDOW ) )
	{
	    fprintf( stderr, "Failed to open GLFW window\n" );
	    glfwTerminate();
	    exit(-1);
	}

	if (glewInit() != GLEW_OK) {
	    fprintf(stderr, "Failed to initialize GLEW\n");
	    exit(-1);
	}
	glfwSetWindowTitle("PathTracer" );
	glfwEnable( GLFW_STICKY_KEYS );
}

void navigation(float *origin, float *direction)
{
	bool up_press;
	bool down_press;
	bool left_press;
	bool right_press;
  	bool space_press;
	bool tab_press;
	bool W_press;
	bool S_press;
	bool A_press;
	bool D_press;
	bool Q_press;
	bool E_press;

	up_press = glfwGetKey( GLFW_KEY_UP ) == GLFW_PRESS;
	down_press = glfwGetKey( GLFW_KEY_DOWN ) == GLFW_PRESS;
	left_press = glfwGetKey( GLFW_KEY_LEFT ) == GLFW_PRESS;
	right_press = glfwGetKey( GLFW_KEY_RIGHT ) == GLFW_PRESS;
	space_press = glfwGetKey( GLFW_KEY_SPACE ) == GLFW_PRESS;
	tab_press = glfwGetKey( GLFW_KEY_TAB ) == GLFW_PRESS;

	W_press = glfwGetKey( 'W' ) == GLFW_PRESS;
	S_press = glfwGetKey( 'S' ) == GLFW_PRESS;
	A_press = glfwGetKey( 'A' ) == GLFW_PRESS;
	D_press = glfwGetKey( 'D' ) == GLFW_PRESS;
	Q_press = glfwGetKey( 'Q' ) == GLFW_PRESS;
	E_press = glfwGetKey( 'E' ) == GLFW_PRESS;

	if(up_press) {
			origin[1]=origin[1]+1.0f;
	}
	if(down_press) {
			origin[1]=origin[1]-1.0f;
	}
	if(left_press) {
			origin[0]=origin[0]-1.0f;
	}
	if(right_press) {
			origin[0]=origin[0]+1.0f;
	}
	if(space_press) {
			origin[2]=origin[2]+1.0f;
	}
	if(tab_press) {
			origin[2]=origin[2]-1.0f;
	}

	if(W_press) {
			direction[1]=direction[1]+.01f;
	}
	if(S_press) {
			direction[1]=direction[1]-.01f;
	}
	if(A_press) {
			direction[0]=direction[0]-.01f;
	}
	if(D_press) {
			direction[0]=direction[0]+.01f;
	}
	if(Q_press) {
			direction[2]=direction[2]+.01f;
	}
	if(E_press) {
			direction[2]=direction[2]-.01f;
	}
}

void fill_pixels(int *pixels, int width, int height, float *out_r, float *out_g, float *out_b)
{
	for (int i=0; i < width*height; ++i) {
		pixels[i] = to_int(out_r[i]) + ( to_int(out_g[i]) << 8 ) + ( to_int(out_b[i]) << 16 );
	}
}