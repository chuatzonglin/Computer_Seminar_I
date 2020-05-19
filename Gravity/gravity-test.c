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
#define dt 0.04
#define G  0.6
#define mass_1 9000
#define mass_2 4000
#define mass_3 1.5
#define mass_4 1.6



void simulate();
void motion_state(double velocity[2],double position[2],double acceleration[2]);
void Acceleration(double abs_rel,double acceleration[2],double rel_21[2]);
double RelativeDis(double position_1[2],double position_2[2],double rel_12[2]);

/*Drawing Circles*/
void circle(double x, double y, double r);

/*Vector Operations*/
void sum_vec(double vec1[2],double vec2[2]);
void vec_intializer(double vec[2], double X1,double X2);
void const_vec_mul(double vec[2],double result[2], double constant);
void printvec(double vec[2]);



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
	double position_1[2],position_2[2],position_3[2],position_4[2];
	vec_intializer(position_1,440.0,250.0);
	vec_intializer(position_2,455.0,250.0);
	vec_intializer(position_3,800.0,100.0);
	vec_intializer(position_4,300.0,250.0);

	double velocity_1[2],velocity_2[2],velocity_3[2],velocity_4[2];
	vec_intializer(velocity_1,0,-15);
	vec_intializer(velocity_2,0,30);
	vec_intializer(velocity_3,-100,-100);
	vec_intializer(velocity_4,0,-400);


	//Acceleration vectors
	double acceleration_1[2], acceleration_2[2], acceleration_3[2],acceleration_4[2];
	
	//Relative positions rel_ab ==> position of a relative to b
	double rel_12[2],rel_21[2];
	double rel_13[2],rel_31[2];
	double rel_23[2],rel_32[2];
	double rel_14[2],rel_41[2];
	double rel_24[2],rel_42[2];
	double rel_43[2],rel_34[2];


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

				circle(position_1[0], position_1[1], r);
				circle(position_2[0], position_2[1], r);
				circle(position_3[0], position_3[1], r/2);
				circle(position_4[0], position_4[1], r/2);

				glfwSwapBuffers(); /* Switch between the front and back buffers */
				usleep(40*1000); /* Wait about 40 milliseconds */
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
	vec[1] = X2;
}

void const_vec_mul(double vec[2],double result[2], double constant){
	for (int i = 0; i < 2; ++i)
	{
		result[i] = vec[i]*constant;
	}
}

void printvec(double vec[2]){

		printf("(%lf, %lf)\n",vec[0],vec[1]);
}

//Relative coordinates and relative distance calculater
double RelativeDis(double position_1[2],double position_2[2],double rel_12[2]){

	for (int i = 0; i < 2; ++i)
	{
		rel_12[i] = position_2[i] - position_1[i];
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