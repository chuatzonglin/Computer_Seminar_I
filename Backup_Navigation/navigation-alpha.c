/* compile: cc navigation.c -g -O2 -Wall -I/usr/include/freetype2 -lftgl -lglfw -lGLU -lGL -lX11 -lXrandr -lm -o navigation*/
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>
#include<fnmatch.h>
#include <unistd.h>
#include <GL/glfw.h>
#include <time.h>

//FreeType for displaying names in the map
#include <FTGL/ftgl.h>
#ifndef FONT_FILENAME
#define FONT_FILENAME "/usr/share/fonts/truetype/fonts-japanese-gothic.ttf"
#endif

//Defining control keys, origin and size of map
#define ORIGIN_X 0.0 
#define ORIGIN_Y 0.0
#define GLFW_KEY_Z   90
#define GLFW_KEY_X   88
#define GLFW_KEY_D   68
#define GLFW_KEY_A   65
#define GLFW_KEY_S   83
#define GLFW_KEY_W   87
#define GLFW_KEY_Q   81
#define GLFW_KEY_E   69
#define GLFW_KEY_R   82
#define GLFW_KEY_T   84


//Defining displacement steps
#define ds 0.02

FTGLfont *font;

//Creating specific structures to store our data
typedef struct{
	float x;
	float y;
} Coordinate;

typedef struct{
	int ID;
	Coordinate pos;
	float wait4;
	char name[50];
	int num_points;
	int connect[50];
} Crossing;

typedef struct{
	int ID;
	Coordinate rel;
	double abs_rel;
}Vector_info;

//Read Files
int readFile(Crossing *map_data);

//Map Manipulation
void OpenViewer(Crossing *map_data,int num_dat,int ID_choice_start,int ID_choice_end,int *pathID,int num_nodes);
void PlotMap(Crossing *map_data,int num_dat);
void showID_location(Crossing *map_data, int ID);
void show_vec(Coordinate vector,double x,double y);

//User input data
int check_input(char *input);
int searchcrossing(Crossing *map_data, int num_dat);
int searchcrossing_ID(Crossing *map_data, int num_dat);
int searchcrossing_Name(Crossing *map_data, int num_dat);
int searchcrossing_Coordinate(Crossing *map_data, int num_data);

//Data Analysis
void DistanceSorting(Crossing *map_data, Vector_info *pos_vec, int num_data, Coordinate target_point);
int djikstra(Crossing *map_data,int num,int start, int goal, int *pathID);
//int djikstra(Crossing *map_data, int num_data, int ID_choice_start, int ID_choice_end, int *pathID);

//Drawing Tools
void outtextxy(double x, double y, double orientation, char const *text, double scale);
void line(double x0, double y0, double x1, double y1);
void circle(double x, double y, double r);
void marker(double x, double y, double r);
void pointy(Coordinate vector, double x, double y);
void North(double x, double y, double scale);

int main(){
	Crossing map_data[100];
	char response[50];
	int num_nodes;

	//Reading data file
	int num_dat=readFile(map_data);

	int pathID[num_dat],temp[num_dat];

	//Finding Starting and Destination
	printf("\nPlease select your starting point\n");
	int ID_choice_start = searchcrossing(map_data,num_dat);
	if (ID_choice_start == 90){goto Exit;}
	printf("Starting ID %d\n",ID_choice_start);

	printf("\nPlease select your destination\n");
	int ID_choice_end = searchcrossing(map_data,num_dat);
	if (ID_choice_end == 90){goto Exit;}
	printf("Destination ID %d\n",ID_choice_end);

	printf("\nDo you want to add a stop in between? Y/n\n");
	scanf("%s",response);


	if (response[0] == 'Y'||response[0] == 'y')
	{
		int ID_choice_mid = searchcrossing(map_data,num_dat);
		if (ID_choice_mid == 90){goto Exit;}
		printf("Rest point ID %d\n",ID_choice_mid);
		num_nodes = djikstra(map_data,num_dat,ID_choice_start,ID_choice_mid,pathID);
			printf("Use:\n ASWD to pan\n QE to rotate\n ZX to zoom\n R to recenter\n T to toggle between views\n");
		//Showing destination location and map
		OpenViewer(map_data,num_dat,ID_choice_start,ID_choice_mid,pathID,num_nodes);
		//Continuation
		num_nodes = djikstra(map_data,num_dat,ID_choice_mid,ID_choice_end,pathID);
		//Showing destination location and map
		OpenViewer(map_data,num_dat,ID_choice_mid,ID_choice_end,pathID,num_nodes);
	}else if (response[0]=='N'||response[0]=='n')
	{
		num_nodes = djikstra(map_data,num_dat,ID_choice_start,ID_choice_end,pathID);
			printf("Use ASWD to pan\n QE to rotate\n ZX to zoom\n R to recenter\n T to toggle between views\n");
		//Showing destination location and map
		OpenViewer(map_data,num_dat,ID_choice_start,ID_choice_end,pathID,num_nodes);
	}else{goto Exit;}


	Exit:printf("Exiting Program\n");
	return 0;
}

/*Reading Files*/
int readFile(Crossing *map_data){
	int num_dat;
	int num_points;

	FILE *fp;

	fp = fopen("map.dat","r");

	if(fp == NULL){
		printf("ERROR in opening File\n");
		return -1;
	}else if(fp != NULL){printf("File successfully opened.\n");/*printf("Printing List of Crossings...\n");*/}

	fscanf(fp,"%d \n",&num_dat);

	for (int i = 0; i < num_dat; ++i)
		{
			fscanf(fp, "%d,%f,%f,%f,%[^,],%d",&(map_data[i].ID),&(map_data[i].pos.x),&(map_data[i].pos.y),&(map_data[i].wait4),map_data[i].name,&(map_data[i].num_points));
				for (int j = 0; j < map_data[i].num_points; j++)
				{
					fscanf(fp, ",%d", &map_data[i].connect[j]);
				}
			/*printf("%d %.2f %.2f %.2f %s %d ",map_data[i].ID,map_data[i].pos.x,map_data[i].pos.y,map_data[i].wait4,map_data[i].name,map_data[i].num_points);
			for (int k = 0; k < map_data[i].num_points; k++)
				{
					printf("%d ", map_data[i].connect[k]);
				}
			printf("\n");
			*/
		}
	fclose(fp);

	return num_dat;

}

//Map manipulation
void OpenViewer(Crossing *map_data,int num_dat,int ID_choice_start,int ID_choice_end,int *pathID,int num_nodes){

	/*Seed for random number generator to simulate traffic*/
	srand(time(0));
	int traffic,traffic_light;

	/*Variables for window*/
	int width, height;
	/*Viewer Size will be adjusted when zooming in or out*/
	double REAL_SIZE_X = 8.0;
	double REAL_SIZE_Y = 8.0;
	/*Projected origin of the map*/
	Coordinate origin;

	/*Indicators of Mode*/
	int first_map_view = 0;
	int Pan_keys = 0, Orientation_keys = 0;
	/*Information of Panning and Rotating*/
	double orientation;Coordinate pan_info;

	/*Starting and moving points*/
	Coordinate starting_moving_point = map_data[ID_choice_start].pos;

	/* Read and configure the font used for drawing strings */
	font = ftglCreateExtrudeFont(FONT_FILENAME);
	if (font == NULL) {
		perror(FONT_FILENAME);
		fprintf(stderr, "Could not load font\n");
		exit(1);}
	ftglSetFontFaceSize(font, 16, 16);
	ftglSetFontDepth(font, 0.01);
	ftglSetFontOutset(font, 0, 0.01);
	ftglSetFontCharMap(font, ft_encoding_unicode);

	char txt_box[64] ;

	/*Initializing moving steps*/
	/*Information stored in each row, [x-steps, y-steps, relative distance, number of steps]*/	
	double moving_info[num_dat][4];
	int moving_counter = 0;/*Will be used to change the rows of the moving_info when the point has reached a new crossing*/
	int i;/*Final value will be used as an indicator to inhibit the moving process*/
	for (i = 0; i < num_dat; i++)
	{
		printf("%d. %d\n", i, pathID[i]);
		if (pathID[i] == ID_choice_end||ID_choice_start==ID_choice_end)
		{
			moving_info[i][0] = 0;
			moving_info[i][1] = 0;
			break;
		}
		moving_info[i][0] = map_data[pathID[i+1]].pos.x - map_data[pathID[i]].pos.x;
		moving_info[i][1] = map_data[pathID[i+1]].pos.y - map_data[pathID[i]].pos.y;
		moving_info[i][2] = sqrt(pow(moving_info[i][0],2)+pow(moving_info[i][1],2));
		moving_info[i][3] = moving_info[i][2]/ds;
		moving_info[i][0] = moving_info[i][0]/moving_info[i][3];
		moving_info[i][1] = moving_info[i][1]/moving_info[i][3];
		moving_info[i][3] = floor(moving_info[i][3]);
	}


	/* Initialize the graphic environment and open a window */
	glfwInit();
	glfwOpenWindow(1080, 1920, 0, 0, 0, 0, 0, 0, GLFW_WINDOW);	

	/*Infinity Loop*/
		printf("\n\n");
		int frame_ticker;//Logging the number of frames swapped to be compared with moving_info[][4]
		double scale = 0.01;
		double stopwatch = 0;
		double velocity = 0;
		double temp = 0;

	for (frame_ticker = 1;;frame_ticker++){

		/* End when Esc is pressed or the window is closed */
		if (glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED))
		  break;

		stopwatch = clock();
			if (Pan_keys == 0 && Orientation_keys ==0)
			{
				/*Saving the panning parameters to be used in panning mode/rotation mode*/
				origin = starting_moving_point;
				if (first_map_view%2 == 0){orientation = atan2(moving_info[moving_counter][0],moving_info[moving_counter][1])*180/3.142;pan_info.x = moving_info[moving_counter][0]*10; pan_info.y = moving_info[moving_counter][1]*10;}
				else if (first_map_view%2==1){orientation = 0;pan_info.x = 0; pan_info.y = 0.2;}

				if (map_data[ID_choice_end].pos.x == starting_moving_point.x && map_data[ID_choice_end].pos.y == starting_moving_point.y){pan_info.x = 0;pan_info.y = 0.2;}

				/* Based on the position of the moving point, project the space within the range of
				 REAL_SIZE_X * REAL_SIZE_Y to the viewport */
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glRotatef(orientation,0.0,0.0,1.0);
				glOrtho(origin.x + REAL_SIZE_X * -0.5, origin.x + REAL_SIZE_X * 0.5,
				  origin.y + REAL_SIZE_Y * -0.5,origin.y + REAL_SIZE_Y * 0.5,
				  -1.0, 1.0);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity(); /* Do not perform other coordinate conversion */
				
			}else if (Orientation_keys == 1 && Pan_keys == 0)
			{
				origin = starting_moving_point;

				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glRotatef(orientation,0.0,0.0,1.0);
				glOrtho(origin.x + REAL_SIZE_X * -0.5, origin.x + REAL_SIZE_X * 0.5,
				  origin.y + REAL_SIZE_Y * -0.5,origin.y + REAL_SIZE_Y * 0.5,
				  -1.0, 1.0);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity(); /* Do not perform other coordinate conversion */	
			}else if (Pan_keys == 1)
			{
				/* Based on (ORIGIN_X, ORIGIN_Y), project the space within the range of
				 REAL_SIZE_X * REAL_SIZE_Y to the viewport */
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glRotatef(orientation,0.0,0.0,1.0);
				glOrtho(origin.x + REAL_SIZE_X * -0.5, origin.x + REAL_SIZE_X * 0.5,
				  origin.y + REAL_SIZE_Y * -0.5,origin.y + REAL_SIZE_Y * 0.5,
				  -1.0, 1.0);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity(); /* Do not perform other coordinate conversion */
			}


			/* Keyboard controls for zoom and pan */
			//Zoom
			if (glfwGetKey(GLFW_KEY_X) || !glfwGetWindowParam(GLFW_OPENED)){glScalef(0.9f, 0.9f, 0.9f && Pan_keys == 0);scale = scale/0.9;REAL_SIZE_X=REAL_SIZE_X/0.9;REAL_SIZE_Y=REAL_SIZE_Y/0.9;}
			if (glfwGetKey(GLFW_KEY_Z) || !glfwGetWindowParam(GLFW_OPENED)){glScalef(1.1f, 1.1f, 1.1f && Pan_keys == 0);scale = scale*0.9;REAL_SIZE_X=REAL_SIZE_X*0.9;REAL_SIZE_Y=REAL_SIZE_Y*0.9;}
			

			//Pan
			if (glfwGetKey(GLFW_KEY_D) || !glfwGetWindowParam(GLFW_OPENED)){origin.y -= pan_info.x;origin.x +=pan_info.y;Pan_keys = 1;}
			if (glfwGetKey(GLFW_KEY_A) || !glfwGetWindowParam(GLFW_OPENED)){origin.y += pan_info.x;origin.x -=pan_info.y;Pan_keys = 1;}
			if (glfwGetKey(GLFW_KEY_W) || !glfwGetWindowParam(GLFW_OPENED)){origin.x += pan_info.x;origin.y +=pan_info.y;Pan_keys = 1;}
			if (glfwGetKey(GLFW_KEY_S) || !glfwGetWindowParam(GLFW_OPENED)){origin.x -= pan_info.x;origin.y -=pan_info.y;Pan_keys = 1;}
			if (glfwGetKey(GLFW_KEY_Q) || !glfwGetWindowParam(GLFW_OPENED)){orientation += 1;pan_info.x = pan_info.x*cos(0.017453292) + pan_info.y*sin(0.017453292);pan_info.y = pan_info.y*cos(0.017453292) - pan_info.x*sin(0.017453292);Orientation_keys = 1;}
			if (glfwGetKey(GLFW_KEY_E) || !glfwGetWindowParam(GLFW_OPENED)){orientation -= 1;pan_info.x = pan_info.x*cos(-0.017453292) + pan_info.y*sin(-0.017453292);pan_info.y = pan_info.y*cos(-0.017453292) - pan_info.x*sin(-0.017453292);Orientation_keys = 1;}
			
			//Toggle/Return Mode
			if ((glfwGetKey(GLFW_KEY_R) || !glfwGetWindowParam(GLFW_OPENED)) && (Pan_keys == 1 || Orientation_keys == 1)){Pan_keys = 0; Orientation_keys = 0;}
			if (glfwGetKey(GLFW_KEY_T) || !glfwGetWindowParam(GLFW_OPENED)){first_map_view++;}	

		glfwGetWindowSize(&width, &height); /* Get the size of the current window */
		glViewport(0, 0, width, height); /* Set the whole window as a viewport */
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT); /* Fill the back buffer with black */

			/*All manipulation of information*/
				/* Draw all information on the back buffer */
				//Plotting map and roads
				PlotMap(map_data,num_dat);
				//Marking destination
				showID_location(map_data, ID_choice_end);

				North(origin.x + pan_info.x*18*scale/0.01 + (pan_info.y*18*scale/0.01),origin.y + pan_info.y*18*scale/0.01 - (pan_info.x*18*scale/0.01), scale);

				//Colouring the route and display name of intersecting crossings in green
				glColor3d(0.0,1.0,0.0);
				for (int i = 0; i < num_nodes; ++i)
				{
					if (i<num_nodes-1)
					{
						line(map_data[pathID[i]].pos.x,map_data[pathID[i]].pos.y,
						map_data[pathID[i+1]].pos.x,map_data[pathID[i+1]].pos.y);
					}
					outtextxy(map_data[pathID[i]].pos.x + 0.1,map_data[pathID[i]].pos.y + 0.1,-orientation,map_data[pathID[i]].name,scale);

				}

				//Colouring the current route and display name of incoming intersecting crossings in red
				glColor3d(1.0,0.0,0.0);
				if(num_nodes > moving_counter + 1){
					glLineWidth(2.0);
					line(starting_moving_point.x,starting_moving_point.y,
						map_data[pathID[moving_counter+1]].pos.x,map_data[pathID[moving_counter+1]].pos.y);
					outtextxy(map_data[pathID[moving_counter+1]].pos.x + 0.1,map_data[pathID[moving_counter+1]].pos.y + 0.1,-orientation,map_data[pathID[moving_counter+1]].name,scale);
					outtextxy(origin.x + pan_info.x*16*scale/0.01 - (pan_info.y*18*scale/0.01),origin.y + pan_info.y*16*scale/0.01 + (pan_info.x*18*scale/0.01),-orientation,map_data[pathID[moving_counter+1]].name,scale);
					outtextxy(origin.x + pan_info.x*15.5*scale/0.01 - (pan_info.y*17*scale/0.01),origin.y + pan_info.y*15.5*scale/0.01 + (pan_info.x*17*scale/0.01),-orientation + 180,"V",scale);
					glColor3d(0.0,1.0,0.0);
					outtextxy(origin.x + pan_info.x*15*scale/0.01 - (pan_info.y*17*scale/0.01),origin.y + pan_info.y*15*scale/0.01 + (pan_info.x*17*scale/0.01),-orientation + 180,"V",scale);
					outtextxy(origin.x + pan_info.x*13.5*scale/0.01 - (pan_info.y*18*scale/0.01),origin.y + pan_info.y*13.5*scale/0.01 + (pan_info.x*18*scale/0.01),-orientation,map_data[pathID[moving_counter]].name,scale);

				}

				//Displaying moving/current position
				show_vec(starting_moving_point,moving_info[moving_counter][0],moving_info[moving_counter][1]);


				/*Manipulation of starting_moving_point*/
				starting_moving_point.x = (starting_moving_point.x + moving_info[moving_counter][0]);
				starting_moving_point.y = (starting_moving_point.y + moving_info[moving_counter][1]);

				if (moving_counter != (num_nodes -1) && moving_counter != (num_nodes -2) && !Pan_keys && ID_choice_start != ID_choice_end)
				{
					/*Turning Information Display*/
					glColor3d(0.5,1.0,0);
					if (atan2(moving_info[moving_counter+1][1],moving_info[moving_counter+1][0]) - atan2(moving_info[moving_counter][1],moving_info[moving_counter][0])>0)
					{
						outtextxy(origin.x + pan_info.x*18.5*scale/0.01 - (pan_info.y*8*scale/0.01),origin.y + pan_info.y*18.5*scale/0.01 + (pan_info.x*8*scale/0.01),-orientation,"  Turn Left in",0.015*scale/0.01);

					}else if (atan2(moving_info[moving_counter+1][1],moving_info[moving_counter+1][0]) - atan2(moving_info[moving_counter][1],moving_info[moving_counter][0])<0)
					{
						outtextxy(origin.x + pan_info.x*18.5*scale/0.01 - (pan_info.y*8*scale/0.01),origin.y + pan_info.y*18.5*scale/0.01 + (pan_info.x*8*scale/0.01),-orientation,"Turn Right in",0.015*scale/0.01);
					}
					snprintf(txt_box,5,"%f",hypot(map_data[pathID[moving_counter+1]].pos.x - starting_moving_point.x,map_data[pathID[moving_counter+1]].pos.y - starting_moving_point.y));
					outtextxy(origin.x + pan_info.x*18.5*scale/0.01 - (pan_info.y*0.5*scale/0.01),origin.y + pan_info.y*18.5*scale/0.01 + (pan_info.x*0.5*scale/0.01),-orientation,txt_box,0.015*scale/0.01);

				}

				

				/*Repositioning the starting_moving point to the exact crossing position*/
				if (frame_ticker == moving_info[moving_counter][3])
				{	
					moving_counter++;
					starting_moving_point.x = map_data[pathID[moving_counter]].pos.x;
					starting_moving_point.y = map_data[pathID[moving_counter]].pos.y;
					frame_ticker = 0;//Resetting the logging	

					traffic_light=rand()%map_data[pathID[moving_counter]].num_points;
					if(traffic_light != 0)
					{
					glColor3d(1.0,0.0,0.0);
					marker(starting_moving_point.x,starting_moving_point.y,0.005);
					glEnd();			
					}
					if(traffic_light == 0)
					{
					glColor3d(0.0,1.0,0.0);
					marker(starting_moving_point.x,starting_moving_point.y,0.005);
					glEnd();					
					}
				}

				if (!(starting_moving_point.x == map_data[ID_choice_end].pos.x && starting_moving_point.y == map_data[ID_choice_end].pos.y))
				{
					/*Displaying cruising speed*/
					if (traffic_light != 0 || (starting_moving_point.x == map_data[ID_choice_end].pos.x && starting_moving_point.y == map_data[ID_choice_end].pos.y)){velocity = 0;}		
					snprintf(txt_box,5,"%f",velocity);
					glColor3d(0.9,0.9,1.0);
					outtextxy(origin.x + pan_info.x*19*scale/0.01 - (pan_info.y*18*scale/0.01),origin.y + pan_info.y*19*scale/0.01 + (pan_info.x*18*scale/0.01),-orientation,"Cruising Speed",0.01*scale/0.01);
					outtextxy(origin.x + pan_info.x*18*scale/0.01 - (pan_info.y*18*scale/0.01),origin.y + pan_info.y*18*scale/0.01 + (pan_info.x*18*scale/0.01),-orientation,txt_box,0.01*scale/0.01);
				}
		glColor3d(1.0,1.0,1.0);
		outtextxy(origin.x - pan_info.x*19*scale/0.01 + (pan_info.y*10*scale/0.01),origin.y - pan_info.y*19*scale/0.01 - (pan_info.x*10*scale/0.01),-orientation,"Press ESC to Exit or Continue...",0.008*scale/0.01);
		glColor3d(0.0,1.0,0.0);
		outtextxy(origin.x - pan_info.x*19*scale/0.01 - (pan_info.y*19*scale/0.01),origin.y - pan_info.y*19*scale/0.01 + (pan_info.x*19*scale/0.01),-orientation,"Kobra Maps",0.008*scale/0.01);
		glfwSwapBuffers(); /* Swap the front buffer with the back buffer */
		if(traffic_light!=0)
		{
			usleep(traffic_light*1000*1000);
			traffic_light=0;
		}	
			
		/* crude simulation of traffic */
		traffic = (rand()%10) + 10;                       
		usleep(traffic * 100);
		stopwatch = (clock()-stopwatch)/CLOCKS_PER_SEC ;
		velocity = 0.02/stopwatch;
		
		}
	glfwTerminate();

}

//Map manipulation
void PlotMap(Crossing *map_data,int num_dat){
	//Plotting Crossing Positions
	glColor3d(1.0,1.0,1.0);
	for (int i = 0; i < num_dat; ++i)
	{
		marker(map_data[i].pos.x,map_data[i].pos.y,0.003);
		//outtextxy(map_data[i].pos.x+0.1,map_data[i].pos.y,map_data[i].name);
	}
	glEnd();

	//Plotting Connecting Roads
	int ID_temp;
	Coordinate pos_temp;
	glColor3d(1.0,1.0,1.0);
	for (int i = 0; i < num_dat; ++i)
	{
		for (int k = 0; k < map_data[i].num_points; k++)
		{
			ID_temp=map_data[i].connect[k];
			pos_temp.x=map_data[ID_temp].pos.x;
			pos_temp.y=map_data[ID_temp].pos.y;

			line(map_data[i].pos.x,map_data[i].pos.y,pos_temp.x,pos_temp.y);
		}
	}
	glEnd();
}
//Display inputed ID Coordinates
void showID_location(Crossing *map_data, int ID)
{
	glColor3d(2.0,0.0,0.5);
		marker(map_data[ID].pos.x,map_data[ID].pos.y,0.005);
	glEnd();
}

//Display inputed Vector Coordinates
void show_vec(Coordinate vector,double x,double y)
{
	glColor3d(0.0,1.0,2.0);
		pointy(vector,x,y);
	glEnd();
}

/*User Entry Data*/
//Search Functions
int searchcrossing(Crossing *map_data, int num_dat){
	int choice,ID_choice;

	printf("Please select searching method,\n1.ID 2.Name 3.Coordinate 4.Exit\n");
	scanf("%d",&choice);

	//Checking Input method
	if (choice==1){ID_choice = searchcrossing_ID(map_data,num_dat);
	}else if (choice==2){ID_choice = searchcrossing_Name(map_data,num_dat);
	}else if (choice==3){ID_choice = searchcrossing_Coordinate(map_data,num_dat);
	}else{ID_choice = 90;}

	return ID_choice;
}

int searchcrossing_ID(Crossing *map_data, int num_dat)
{
	int ID_choice;
	while(1){
		printf("Please input ID of location: ");
		scanf("%d",&ID_choice);
		if (ID_choice >= 0  && ID_choice <= 89){break;}
		else{
			printf("ERROR\n");
			ID_choice = 90;
			break;
		}
	}
	//printf("ID of destination %d\n",ID_choice);	
	return ID_choice;
}

//Compares string using fnmatch)
int searchcrossing_Name(Crossing *map_data, int num_data)
{
	char buff[256];

	int ID_choice;
	int matched_ID[90];
	int counter;
	
	while(1){
		counter = 0;
		printf("For partial search please insert '*' after name of location\n");
		printf("Input the name of the crossing=>");
		scanf("%s",buff);


		printf("Finding closest matching...\n\n");

		for(int i = 0; i<num_data;i++){

		 	if(fnmatch(buff,map_data[i].name,0)==0){
		 		counter++;
		 		printf("%d %s [%f, %f]\n",counter,map_data[i].name,map_data[i].pos.x,map_data[i].pos.y);
		 		matched_ID[counter] = i;
		 		}
		 	}
		 if(counter == 0){
		 	printf("No crossing found\nPlease try again\n\n");
		 	continue;
		}else if (counter > 0){
		 	printf("\nPlease select your desired Crossing\n");
		 	scanf("%d",&ID_choice);
		 	if (ID_choice > 0 &&ID_choice <= counter)
		 	{
		 		ID_choice = matched_ID[ID_choice];
		 		break;
		 	}else{
			printf("ERROR\n");
			ID_choice = 90;
			break;
			}
		 	
		 }

	}
	return ID_choice;

}

//Finds closest crossing for inputted coordinates
int searchcrossing_Coordinate(Crossing *map_data, int num_data)
{
	Coordinate position_choice;
	Vector_info pos_vec[num_data];
	int ID_choice;

	
		start: 
		printf("Please input coordinates of location: \nX ==> ");
		scanf("%f",&(position_choice.x));

		printf("Y ==> ");
		scanf("%f",&(position_choice.y));

		DistanceSorting(map_data, pos_vec, num_data, position_choice);

		printf("\nPrinting the 3 closest crossings\n");

		/*for (int i = 0; i < 3; ++i)
		{
			printf("%d %s [%.2f,%.2f]\n",i+1,map_data[pos_vec[i].ID].name,map_data[pos_vec[i].ID].pos.x,map_data[pos_vec[i].ID].pos.y);
		}*/
		printf("4. Reenter your coordinates\n");
		printf("\nPlease select your desired crossing\n");
		scanf("%d",&ID_choice);
		while(1){
			if (ID_choice >=1 && ID_choice <=3){
			ID_choice = pos_vec[ID_choice-1].ID;
			break;
			}else if(ID_choice == 4){
				goto start;
			}else{
				printf("\nERROR\n");
				ID_choice = 90;
				return 90;
				break;
			}
		}

	return ID_choice;
}

/*Data Analysis*/

/*Sorts the distances from inputted Coordinates*/
void DistanceSorting(Crossing *map_data, Vector_info *pos_vec, int num_data, Coordinate target_point){
	double temp;

	for (int i = 0; i < num_data; i++)
	{
		temp = hypot(target_point.x - map_data[i].pos.x,target_point.y-map_data[i].pos.y);
		pos_vec[i].ID = map_data[i].ID;
		pos_vec[i].rel.x = map_data[i].pos.x - target_point.x;
		pos_vec[i].rel.y = map_data[i].pos.y - target_point.y;
		pos_vec[i].abs_rel = temp;

		for (int j = 0; j < i-1; j++)
		{
			if (temp < pos_vec[j].abs_rel)
			{
				for (int k = i; k > j; k--)
				{
					pos_vec[k] = pos_vec[k-1];
				}
				pos_vec[j].ID = map_data[i].ID;
				pos_vec[j].rel.x = map_data[i].pos.x - target_point.x;
				pos_vec[j].rel.y = map_data[i].pos.y - target_point.y;
				pos_vec[j].abs_rel = temp;
				break;
			}
		}

	}
}

//Finds the IDs to pass through
int PathFinder_beta(Crossing *map_data, int ID_choice_start, int ID_choice_end, int *pathID){
	int ID_temp;
	int counter = 1;
	double min_dis;

	pathID[0]=ID_choice_start;

	while(1){
		min_dis = 10000;
		for (int i = 0; i < map_data[ID_choice_start].num_points; ++i){
			ID_temp = map_data[ID_choice_start].connect[i];
			if (map_data[ID_choice_start].connect[i] == ID_choice_end)
			{
				pathID[counter] = ID_choice_end;
				break;

			}else if (min_dis > hypot(map_data[ID_temp].pos.x-map_data[ID_choice_end].pos.x,map_data[ID_temp].pos.y-map_data[ID_choice_end].pos.y))
			{
				min_dis = hypot(map_data[ID_temp].pos.x-map_data[ID_choice_end].pos.x,map_data[ID_temp].pos.y-map_data[ID_choice_end].pos.y);
				pathID[counter] = ID_temp;
			}
		}
		ID_choice_start = pathID[counter];

		if (pathID[counter] == ID_choice_end)
		{
			return 1;

		}else if (counter > 1 && pathID[counter] == pathID[counter-2]){
			printf("ERROR\nCan't find path\n");
			return -1;
		}
		counter++;

	}
}
typedef struct{
	int id;
	int previous;
	double distance;
	int status;
}djik_info;	


int djikstra(Crossing *map_data,int num,int start, int goal, int *pathID)
{
	if (start == goal)
	{
		pathID[0] = start;
		return 0;
	}
	djik_info djik[100];
	int i,j,k,ref;
	double p,minr;
	for(i=0;i<num;i++)
	{
		djik[i].distance=10000;
		djik[i].status=0;
		djik[i].previous=-1;
	}
	djik[start].id=start;
	djik[start].distance=0;



	k=start;
	do
	{
		djik[k].status=1;
		for(j=0;j<map_data[k].num_points;j++)      //loop for checking all the data num_points
		{	
			djik[map_data[k].connect[j]].id=map_data[k].connect[j];
			p=hypot(map_data[k].pos.x-map_data[map_data[k].connect[j]].pos.x,map_data[k].pos.y-map_data[map_data[k].connect[j]].pos.y);
			if(djik[map_data[k].connect[j]].distance>djik[k].distance+p)
			{
				djik[map_data[k].connect[j]].distance=djik[k].distance+p; //calculating minimum distance between data point from the entered values
				djik[map_data[k].connect[j]].previous=k;
			}


		}
		minr=10000;
		for(i=0;i<num;i++)
		{
			for(j=0;j<map_data[i].num_points;j++)
			{
				if((minr>djik[map_data[i].connect[j]].distance) && (djik[map_data[i].connect[j]].distance>0.0) && (djik[map_data[i].connect[j]].status==0))
				{
					minr=djik[map_data[i].connect[j]].distance;
					ref=map_data[i].connect[j];


				}

			}
		}




		k=ref;


	}while(djik[goal].status==0);
	int v=0;
	int z=goal;

	do
	{
		z=djik[z].previous;
		v+=1;

	}while(z!=start);

	pathID[v] = goal;
	for(i=v-1;i>0;i--)
	{
		pathID[i]=djik[pathID[i+1]].previous;
	}
	pathID[0]=start;
	return v+1;

}
/*
int dijkstra(Crossing *map_data, int n, int ID_choice_start, int ID_choice_end, int *pathID)
{
 
	int cost[n][n],distance[n],pred[n];
	int visited[n],count,min_dis,nextnode;
	int ID_temp;
	
	//pred[] stores the predecessor of each node
	//count gives the number of nodes seen so far
	//create the cost matrix
	int i,j;
	for (i = 0; i < n; ++i)
	{
		for (j = 0; j < n; ++j)
		{
			cost[i][j] = 10000;
		}
	}

	for (i = 0; i < n; ++i)
	{
		for (j = 0; j < map_data[i].num_points; ++j)
		{
			ID_temp = map_data[i].connect[j];
			cost[i][ID_temp] = hypot(map_data[i].pos.x-map_data[ID_temp].pos.x,map_data[i].pos.y-map_data[ID_temp].pos.y);
		}
	}
	
	//initialize pred[],distance[] and visited[]
	for(i=0;i<n;i++)
	{
		distance[i]=cost[ID_choice_start][i];
		pred[i]=ID_choice_start;
		visited[i]=0;
	}
	
	distance[ID_choice_start]=0;
	visited[ID_choice_start]=1;
	count=1;
	
	while(count<n-1)
	{
		min_dis=10000;
		
		//nextnode gives the node at minimum distance
		for(i=0;i<n;i++)
			if(distance[i]<min_dis&&!visited[i])
			{
				min_dis=distance[i];
				nextnode=i;
			}
			
			//check if a better path exists through nextnode			
			visited[nextnode]=1;
			for(i=0;i<n;i++)
				if(!visited[i])
					if(min_dis+cost[nextnode][i]<distance[i])
					{
						distance[i]=min_dis+cost[nextnode][i];
						pred[i]=nextnode;
					}
		count++;
	}
 
	//Saving the path
	count = 1;
	int temp[n];
	for(i=0;i<n;i++)
		if(i!=ID_choice_start && i == ID_choice_end)
		{
			temp [0] = i;
			j=i;
			do
			{
				j=pred[j];
				temp[count]=j;
				count++; 
			}while(j!=ID_choice_start);
	}
	for (i = 0; i < count; ++i)
	{
		pathID[count-1-i] = temp[i];
	}
	return count;
}
*/

/*Drawing Tools*/
/*Text Printing*/
void outtextxy(double x, double y, double orientation, char const *text, double scale){

	glPushMatrix();
	glTranslated(x,y,0.0);
	glRotatef(orientation,0,0,1);
	glScaled(scale,scale,scale);
	ftglRenderFont(font,text,FTGL_RENDER_ALL);
	glPopMatrix();
}
/* Draw a line segment */
void line(double x0, double y0, double x1, double y1){
  glBegin(GL_LINES);
  glVertex2d(x0,y0);
  glVertex2d(x1,y1);
  glEnd();
}
/* Draw a circle */
void circle(double x, double y, double r){
   //Draw a circle using line segments by dividing the circumference into 24 pieces
  int const N = 24;
  int i;

  glBegin(GL_LINE_LOOP);
  for (i=0;i<N;i++)
    glVertex2d(x+cos(2*M_PI*i/N)*r, y+sin(2*M_PI*i/N)*r);
  glEnd();
}
/*Filled Circle*/
void marker(double x, double y, double r){
	for (int i = 0; i < 20; ++i)
	{
		glVertex2i(x,y);
		circle(x,y,r*i);
	}
}
void pointy(Coordinate vector, double x, double y)
{
	x=4*x; y=4*y;
	glLineWidth(6);
	glBegin(GL_LINE_LOOP);
	glVertex2d(vector.x,vector.y);
	glVertex2d(vector.x-3*x+y,vector.y-3*y-x);
	glVertex2d(vector.x-1.5*x,vector.y-1.5*y);
	glVertex2d(vector.x-3*x-y,vector.y-3*y+x);
	glEnd();
	glLineWidth(2.0);
}
void North(double x, double y, double scale){

	glColor3d(0.1,0.2,1.0);
	scale = scale/1.5;
	line(x - 0.25*scale/0.01, y + 0.1*scale/0.01, x + 0.25*scale/0.01, y+ 0.1*scale/0.01);
	line(x, y - 0.5*scale/0.01,x, y + 0.5*scale/0.01);
	line(x, y + 0.5*scale/0.01,x- 0.0625*scale/0.01, y + 0.375*scale/0.01);
	line(x, y + 0.5*scale/0.01,x+ 0.0625*scale/0.01, y + 0.375*scale/0.01);
	circle(x,y+ 0.1*scale/0.01,0.1*scale/0.01);
}