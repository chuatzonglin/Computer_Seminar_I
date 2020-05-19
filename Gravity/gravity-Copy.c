/* gravity.c */
/* compile: cc -o gravity gravity.c -g -O2 -Wall -lglfw -lGLU -lGL -lX11 -lXrandr -lm */

/*Concept: All motion quantities(acceleration, velocity, position) are stored as a Tensor of rank 1. 
The coordinates are transformed from global coordinates to relative coordinates for acceleration calculation.*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <unistd.h>
#include <GL/glfw.h>

typedef struct{

	double x;
	double y;
} Vector;

typedef struct{
	int ID;
	char name[24];
	double mass;
	Vector position;
	Vector velocity;
	Vector acceleration;

} Point_Mass;

//Defining Constants
//timestep
#define dt 0.04
#define G  0.6
#define mass_1 9000
#define mass_2 4000
#define mass_3 1.5
#define mass_4 1.6

void simulate();
void motion_state(Vector velocity,Vector position,Vector acceleration);
void Acceleration(double abs_rel,Vector acceleration,Vector rel_21);
double RelativeDis(Vector position_1,Vector position_2,Vector rel_12);

/*Drawing Circles*/
void circle(double x, double y, double r);

/*Vector Operations*/
void sum_vec(Vector vec1,Vector vec2);
void vec_intializer(Vector vec, double x,double y);
void const_vec_mul(Vector vec,Vector result, double constant);
void printvec(Vector vec);



int main(void)
{
	//From sample code in class
	
	int width = 1000, height = 500; /* Window size */
	/* Initialize the graphic environment and open a window */
	glfwInit();
	glfwOpenWindow(width, height, 0, 0, 0, 0, 0, 0, GLFW_WINDOW);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); /* Do not convert coordinates except for projection purposes */

	//Simulation part are executed in the simulate() function
	simulate();

	glfwTerminate(); /* Close the window */
	return 0;
}

/*Simulater*/
void simulate()
{
	int width = 1000, height = 500; /* Window size */
	double r = 5.0; /* Radius of the planet */

	//Initial Conditions
	Vector position_1,position_2,position_3,position_4;
	vec_intializer(position_1,440.0,250.0);
	vec_intializer(position_2,455.0,250.0);
	vec_intializer(position_3,800.0,100.0);
	vec_intializer(position_4,300.0,250.0);

	Vector velocity_1,velocity_2,velocity_3,velocity_4;
	vec_intializer(velocity_1,0,-15);
	vec_intializer(velocity_2,0,30);
	vec_intializer(velocity_3,-100,-100);
	vec_intializer(velocity_4,0,-400);


	//Acceleration vectors
	Vector acceleration_1, acceleration_2, acceleration_3,acceleration_4;
	
	//Relative positions rel_ab ==> position of a relative to b
	Vector rel_12,rel_21;
	Vector rel_13,rel_31;
	Vector rel_23,rel_32;
	Vector rel_14,rel_41;
	Vector rel_24,rel_42;
	Vector rel_43,rel_34;


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


						//Resetting the acceleration vector each iteration
						vec_intializer(acceleration_1,0,0);
						vec_intializer(acceleration_2,0,0);
						vec_intializer(acceleration_3,0,0);
						vec_intializer(acceleration_4,0,0);


						/*Each System Calulates the Acceleration between two selected masses, 
						the total acceleration is the sum of acceleration exerted by all bodies*/

						//System 1
						double abs_rel_12 = RelativeDis(position_1,position_2,rel_12);
						const_vec_mul(rel_12,rel_21,-1);
						Acceleration(abs_rel_12,acceleration_1,rel_21);
						const_vec_mul(acceleration_1,acceleration_1,mass_2);
						Acceleration(abs_rel_12,acceleration_2,rel_12);
						const_vec_mul(acceleration_2,acceleration_2,mass_1);

						//System 2
						double abs_rel_13 = RelativeDis(position_1,position_3,rel_13);
						const_vec_mul(rel_13,rel_31,-1);
						Acceleration(abs_rel_13,acceleration_1,rel_31);
						const_vec_mul(acceleration_1,acceleration_1,mass_3);
						Acceleration(abs_rel_13,acceleration_3,rel_13);
						const_vec_mul(acceleration_3,acceleration_3,mass_1);

						//System 3
						double abs_rel_23 = RelativeDis(position_2,position_3,rel_23);
						const_vec_mul(rel_23,rel_32,-1);
						Acceleration(abs_rel_23,acceleration_2,rel_32);
						const_vec_mul(acceleration_2,acceleration_2,mass_3);
						Acceleration(abs_rel_23,acceleration_3,rel_23);
						const_vec_mul(acceleration_3,acceleration_3,mass_2);

						//System 4
						double abs_rel_14 = RelativeDis(position_1,position_4,rel_14);
						const_vec_mul(rel_14,rel_41,-1);
						Acceleration(abs_rel_14,acceleration_1,rel_41);
						const_vec_mul(acceleration_1,acceleration_1,mass_4);
						Acceleration(abs_rel_14,acceleration_4,rel_14);
						const_vec_mul(acceleration_4,acceleration_4,mass_1);
						
						//System 5
						double abs_rel_24 = RelativeDis(position_2,position_4,rel_24);
						const_vec_mul(rel_24,rel_42,-1);
						Acceleration(abs_rel_24,acceleration_2,rel_42);
						const_vec_mul(acceleration_2,acceleration_2,mass_4);
						Acceleration(abs_rel_24,acceleration_4,rel_24);
						const_vec_mul(acceleration_4,acceleration_4,mass_2);

						//System 6
						double abs_rel_43 = RelativeDis(position_4,position_3,rel_43);
						const_vec_mul(rel_43,rel_34,-1);
						Acceleration(abs_rel_43,acceleration_4,rel_34);
						const_vec_mul(acceleration_4,acceleration_4,mass_3);
						Acceleration(abs_rel_43,acceleration_3,rel_43);
						const_vec_mul(acceleration_3,acceleration_3,mass_4);


						//Print Values to set parameters
						/*printvec(position_4);
						printvec(acceleration_1);
						printvec(acceleration_4);
						printvec(velocity_1);
						printvec(velocity_4);
						printf("\n");*/

						//Velocity and position of each body is calculated by the acceleration obtained above						
						motion_state(velocity_1,position_1,acceleration_1);
						motion_state(velocity_2,position_2,acceleration_2);
						motion_state(velocity_3,position_3,acceleration_3);
						motion_state(velocity_4,position_4,acceleration_4);


				glColor3d(1.0, 1.0, 1.0);
				//Printing position of Planets

				circle(position_1.x, position_1.y, r);
				circle(position_2.x, position_2.y, r);
				circle(position_3.x, position_3.y, r/2);
				circle(position_4.x, position_4.y, r/2);

				glfwSwapBuffers(); /* Switch between the front and back buffers */
				usleep(40*1000); /* Wait about 40 milliseconds */
			}



}

void sum_vec(Vector vec1,Vector vec2){
	vec1.x = vec1.x + vec2.x;
	vec1.y = vec1.y + vec2.y;
}

void vec_intializer(Vector vec, double x,double y){
	vec.x = x;
	vec.y = y;
}

void const_vec_mul(Vector vec,Vector result, double constant){
	result.x = vec.x*constant;
	result.y = vec.y*constant;
}

void printvec(Vector vec){

		printf("(%lf, %lf)\n",vec.x,vec.y);
}

//Relative coordinates and relative distance calculater
double RelativeDis(Vector position_1,Vector position_2,Vector rel_12){

	rel_12.x = position_2.x - position_1.x;
	rel_12.y = position_2.y - position_1.y;

	double abs_rel = sqrt(pow(rel_12.x,2)+pow(rel_12.y,2));
	
	return abs_rel;
}

//acceleration calculater
void Acceleration(double abs_rel, Vector acceleration,Vector rel_21){
	
	double abs_accel;
	Vector temp;
	abs_accel = -G/pow(abs_rel,3);
	const_vec_mul(rel_21,temp,abs_accel);
	sum_vec(acceleration,temp);
}
//Position Velocity Calculation
void motion_state(Vector velocity,Vector position,Vector acceleration){
	Vector temp;

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