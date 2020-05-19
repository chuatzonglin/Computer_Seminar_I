/* gravity.c */
/* compile: cc -o gravity gravity.c -g -O2 -Wall -lglfw -lGLU -lGL -lX11 -lXrandr -lm */

/*Concept: All motion quantities(acceleration, velocity, position) are stored as a Tensor of rank 1. 
The coordinates are transformed from global coordinates to relative coordinates for acceleration calculation.*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <unistd.h>
#include <GL/glfw.h>

//Defining Constants
//timestep
#define dt 0.1
#define G  6.67259*pow(10,-11)
#define mass_1 1*pow(10,14)
#define mass_2 1*pow(10,14)
//#define blackhole 1*pow(10,14)

void circle(double x, double y, double r);
void simulate();
void motion_state(double velocity[2],double position[2],double acceleration[2]);
void Acceleration(double abs_rel,double acceleration[2],double rel_21[2]);
void sum_vec(double vec1[2],double vec2[2]);
void vec_intializer(double vec[2], double X1,double X2);
void const_vec_mul(double vec1[2],double vec2[2], double constant);
void printvec(double vec[2]);
double RelativeDis(double position_1[2],double position_2[2],double rel_12[2],double rel_21[2]);

int main(void)
{
	//From sample code in class
	int width = 640, height = 400; /* Window size */

	/* Initialize the graphic environment and open a window */
	glfwInit();
	glfwOpenWindow(width, height, 0, 0, 0, 0, 0, 0, GLFW_WINDOW);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); /* Do not convert coordinates except for projection purposes */


	//Part modified are executed from simulate() function
	simulate();

	glfwTerminate(); /* Close the window */

	return 0;
}


/*Simulater*/
void simulate()
{
	int width = 640, height = 500; /* Window size */
	//double x = 200.0, y = 200.0; /* Central position of the ball */
	//double vx = 5.0, vy_i = 5.0, vy_f = 0; /* Speed of the ball */
	double r = 10.0; /* Radius of the ball */

	//double axis[2];

	//Initial Conditions
	double position_2[2];
	position_2[0]=200.0; position_2[1]=200.0;
	double position_1[2];
	position_1[0]=200.0; position_1[1]=100.0;
	double velocity_1[2];
	velocity_1[0]=2; velocity_1[1]=0;
	double velocity_2[2];
	vec_intializer(velocity_2,-2,0);

	double acceleration_1[2], acceleration_2[2];

	//Relative vec
	double rel_12[2],rel_21[2];
	//double rel_a1[2],rel_1a[2];
	//double rel_a2[2],rel_2a[2];

			while(1) {
		/* The program is ended if the user pushes Esc or closes the window */
		if (glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED)) 
			break;

		glfwGetWindowSize(&width, &height); /* Obtain the current window size */
		glViewport(0, 0, width, height); /* Use the whole window as a viewport */

		glMatrixMode(GL_PROJECTION); glLoadIdentity();
		glOrtho(0, width, 0, height, -1.0, 1.0); /* Project the space with this range to a viewport */

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f); 
		glClear(GL_COLOR_BUFFER_BIT); /* Paint the back buffer in black */

		//vec_intializer(axis,500,500);

		vec_intializer(acceleration_1,0,0);
		vec_intializer(acceleration_2,0,0);

		//System 1
		double abs_rel = RelativeDis(position_1,position_2,rel_12,rel_21);
		Acceleration(abs_rel,acceleration_1,rel_21);
		const_vec_mul(acceleration_1,acceleration_1,mass_2);
		Acceleration(abs_rel,acceleration_2,rel_12);
		const_vec_mul(acceleration_2,acceleration_2,mass_1);

		/*//System 2
		double abs_rel_a = RelativeDis(position_1,axis,rel_1a,rel_a1);
		Acceleration(abs_rel_a,acceleration_1,rel_a1);
		const_vec_mul(acceleration_1,acceleration_1,blackhole);

		double abs_rel_b = RelativeDis(position_2,axis,rel_2a,rel_a2);
		Acceleration(abs_rel_b,acceleration_2,rel_a2);
		const_vec_mul(acceleration_2,acceleration_2,blackhole);*/


		
		printvec(rel_21);
		printvec(acceleration_1);
		printvec(acceleration_2);
		printvec(velocity_1);
		printvec(velocity_2);
		printf("\n");
		
		motion_state(velocity_1,position_1,acceleration_1);
		motion_state(velocity_2,position_2,acceleration_2);
		/* Let the ball bounce if it hits one of the four sides of the window */
		/*if (x <= r || x >= width - r) 
			vx = -vx;
		if (y <= r || y >= height - r) 
			vy_i = -vy_i;*/

		glColor3d(1.0, 1.0, 1.0);
		circle(position_1[0], position_1[1], r); /* Draw a ball */
		circle(position_2[0], position_2[1], r);

		glfwSwapBuffers(); /* Switch between the front and back buffers */
		usleep(40 * 100); /* Wait about 40 milliseconds */
	}



}

void sum_vec(double vec1[2],double vec2[2]){
	for (int i = 0; i < 2; ++i)
	{
		vec1[i] = vec1[i] + vec2[i];
	}

}

void vec_intializer(double vec[2], double X1,double X2){
	vec[0] = X1;
	vec[1] = X1;
}

void const_vec_mul(double vec1[2],double vec2[2], double constant){
	for (int i = 0; i < 2; ++i)
	{
		vec2[i] = vec1[i]*constant;
	}
}

void printvec(double vec[2]){

		printf("(%lf, %lf)\n",vec[0],vec[1]);
}

//Relative coordinates and relative distance calculater
double RelativeDis(double position_1[2],double position_2[2],double rel_12[2],double rel_21[2]){

	for (int i = 0; i < 2; ++i)
	{
		rel_12[i] = position_2[i] - position_1[i];
		rel_21[i] = -rel_12[i];
	}
	double abs_rel = sqrt(pow(rel_12[0],2)+pow(rel_12[1],2));
	
	return abs_rel;
}

//acceleration calculater
void Acceleration(double abs_rel,double acceleration[2],double rel_21[2]){
	double abs_accel;
	double temp[2];
	abs_accel = -G/pow(abs_rel,3);
	const_vec_mul(rel_21,temp,abs_accel);
	sum_vec(acceleration,temp);
}
//Position Velocity Calculation
void motion_state(double velocity[2],double position[2],double acceleration[2]){
	double temp[2];

	const_vec_mul(acceleration,temp,dt);
	sum_vec(velocity,temp);
	const_vec_mul(velocity,temp,dt);
	sum_vec(position,temp);
}

/* Draw a circle */
void circle(double x, double y, double r)
{
	int const N = 24; /* Draw a circle using line segments by dividing the circumference into 24 pieces */
	int i;
	
	glBegin(GL_LINE_LOOP);
	for (i = 0; i < N; i++)
		glVertex2d(x + cos(2 * M_PI * i / N) * r, y + sin(2 * M_PI * i / N) * r);
	glEnd(); 
}
