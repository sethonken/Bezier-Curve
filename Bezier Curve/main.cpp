#include "GL/freeglut.h"
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

#define CTRL_POINT_DEFINE 1
#define CTRL_POINT_DELETE 2
#define CTRL_POINT_MOVE 3
#define DECASTELJAU_MOVE 4
#define CURVE 5
#define CURVE_EXTENDED 6
#define CONTROL_POLYGON 7
#define DECASTELJAU 8
#define CONVEX_HULL 9
#define CLEAR_ALL 10
#define EXIT 11

//structs
struct Point {
	double x = 0.0;
	double y = 0.0;
};

//global variables
int width = 600;
int height = 600;
int pointNumber = 0;
int movingPointIndex = -1;
int t_density = 10000;
double DeCasteljau_t_value = 0.5;
double DeCasteljau_x, DeCasteljau_y;
bool leftClicked = false;
bool controlPolygon = false;
bool curve = false;
bool curveExtended = false;
bool DeCasteljau = false;
bool convexHull = true;
int ctrl_point_state = CTRL_POINT_DEFINE;
Point points[32];

//global variables for w-window
int width2 = 300;
int height2 = 300;
double weights[30];
int windowOne, windowTwo;
bool callWindowOne = false;
bool rightClicked = false;
int redPoint = -1;

//functions
static void display();
static void display2();
void processMenuEvents(int option);
void processModeEvents(int option);
void processDrawEvents(int option);

void createGLUTMenus() {
	//submenu for mode
	int subMenuMode = glutCreateMenu(processModeEvents);
	glutAddMenuEntry("Define ctrl point", CTRL_POINT_DEFINE);
	glutAddMenuEntry("Delete ctrl point", CTRL_POINT_DELETE);
	glutAddMenuEntry("Move ctrl point", CTRL_POINT_MOVE);
	glutAddMenuEntry("Move DeCasteljau", DECASTELJAU_MOVE);

	//submenu for draw
	int subMenuDraw = glutCreateMenu(processDrawEvents);
	glutAddMenuEntry("Curve", CURVE);
	glutAddMenuEntry("Curve extended", CURVE_EXTENDED);
	glutAddMenuEntry("Control polygon", CONTROL_POLYGON);
	glutAddMenuEntry("DeCasteljau", DECASTELJAU);
	glutAddMenuEntry("Convex hull", CONVEX_HULL);

	//main menu
	int menu = glutCreateMenu(processMenuEvents);
	glutAddSubMenu("Mode", subMenuMode);
	glutAddSubMenu("Draw", subMenuDraw);
	glutAddMenuEntry("Clear all", CLEAR_ALL);
	glutAddMenuEntry("Exit", EXIT);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void processMenuEvents(int option) {
	switch (option) {
	case CLEAR_ALL:
		//initialize all weights to 1
		for (int i = 0; i < 30; i++) {
			weights[i] = 1.0;
		}
		redPoint = -1;
		ctrl_point_state = CTRL_POINT_DEFINE;
		DeCasteljau_t_value = 0.5;
		pointNumber = 0;
		display();
		break;
	case EXIT:
		exit(0);
	}
}

void processModeEvents(int option) {
	switch (option) {
	case CTRL_POINT_DEFINE:
		ctrl_point_state = CTRL_POINT_DEFINE;
		break;
	case CTRL_POINT_DELETE:
		ctrl_point_state = CTRL_POINT_DELETE;
		break;
	case CTRL_POINT_MOVE:
		ctrl_point_state = CTRL_POINT_MOVE;
		break;
	case DECASTELJAU_MOVE:
		ctrl_point_state = DECASTELJAU_MOVE;
		break;
	}
	display();
}

void processDrawEvents(int option) {
	switch (option) {
	case CURVE:
		if (curveExtended && curve) {
			curve = true;
			curveExtended = false;
		}
		else
			curve = !curve;
		break;
	case CURVE_EXTENDED:
		curveExtended = !curveExtended;
		break;
	case CONTROL_POLYGON:
		controlPolygon = !controlPolygon;
		break;
	case DECASTELJAU:
		DeCasteljau = !DeCasteljau;
		break;
	case CONVEX_HULL:
		convexHull = !convexHull;
		break;
	}
	display();
}

static void mouse_active_move(int x, int y) {
	double widthd = width;
	double heightd = height;
	double pointx = (x / widthd) * 2 - 1;
	double pointy = ((heightd - y) / heightd) * 2 - 1;

	//check if point needs to be moved
	if (ctrl_point_state == CTRL_POINT_MOVE && leftClicked && movingPointIndex != -1) {
		points[movingPointIndex].x = pointx;
		points[movingPointIndex].y = pointy;
	}

	if (ctrl_point_state == DECASTELJAU_MOVE && leftClicked && movingPointIndex != -1) {
		if (DeCasteljau_x - pointx < 0) {
			DeCasteljau_t_value += 0.004;
		}
		else {
			DeCasteljau_t_value -= 0.004;
		}
	}
	display();
}

static void mouse_press(int button, int state, int x, int y) {
	double widthd = width;
	double heightd = height;
	double pointx = (x / widthd) * 2 - 1;
	double pointy = ((heightd - y) / heightd) * 2 - 1;

	if (button == 0 && state == 0) {
		leftClicked = true;

		if (ctrl_point_state == CTRL_POINT_MOVE) {
			double pointRange = 0.02;
			//set index to -1 incase mouse click is not on point
			movingPointIndex = -1;
			//check if ctrl point is close to cursor
			for (int i = 0; i < pointNumber; i++) {
				if ((points[i].x - pointx <= pointRange && points[i].x - pointx >= 0) ||
					(points[i].x - pointx >= -pointRange && points[i].x - pointx <= 0)) {
					if ((points[i].y - pointy <= pointRange && points[i].y - pointy >= 0) ||
						(points[i].y - pointy >= -pointRange && points[i].y - pointy <= 0)) {
						//point click is in range of a ctrl point, mark index
						movingPointIndex = i;
						break;
					}
				}
			}
		}

		if (ctrl_point_state == DECASTELJAU_MOVE) {
			double pointRange = 0.02;
			//set index to -1 incase mouse click is not on point
			movingPointIndex = -1;
			//check if DeCasteljau t-value is close to cursor
			if ((DeCasteljau_x - pointx <= pointRange && DeCasteljau_x - pointx >= 0) ||
				(DeCasteljau_x - pointx >= -pointRange && DeCasteljau_x - pointx <= 0)) {
				if ((DeCasteljau_y - pointy <= pointRange && DeCasteljau_y - pointy >= 0) ||
					(DeCasteljau_y - pointy >= -pointRange && DeCasteljau_y - pointy <= 0)) {
					//point click is in range of a ctrl point, mark index
					movingPointIndex = 1;
				}
			}
		}
	}
	if (button == 0 && state == 1) { //left button released
		leftClicked = false;

		switch (ctrl_point_state) {
		case CTRL_POINT_DEFINE:
			if (pointNumber == 32)
				break;

			points[pointNumber].x = pointx;
			points[pointNumber].y = pointy;

			pointNumber++;
			break;
		case CTRL_POINT_DELETE:
			double pointRange = 0.02;
			//check if ctrl point is close to cursor
			for (int i = 0; i < pointNumber; i++) {
				if ((points[i].x - pointx <= pointRange && points[i].x - pointx >= 0) ||
					(points[i].x - pointx >= -pointRange && points[i].x - pointx <= 0)) {
					if ((points[i].y - pointy <= pointRange && points[i].y - pointy >= 0) ||
						(points[i].y - pointy >= -pointRange && points[i].y - pointy <= 0)) {
						//point click is in range of a ctrl point, remove it from points array by overwriting
						for (int j = i; j < pointNumber; j++) {
							if (j + 1 == pointNumber) {
								points[j].x = 0;
								points[j].y = 0;
								weights[j - 2] = 1;
								weights[j - 1] = 1;
							}
							else {
								points[j].x = points[j + 1].x;
								points[j].y = points[j + 1].y;
								//change weights as well
								if (j > 0) {
									weights[j - 1] = weights[j];
								}
								else {
									weights[j] = weights[j + 1];
								}
							}
						}
						pointNumber--;
						break;
					}
				}
			}
		}
		display();
	}
}

static void mouse_active_move2(int x, int y) {
	double widthd = width2;
	double heightd = height2;
	double pointx = (x / widthd) * 2 - 1;
	double pointy = ((heightd - y) / heightd) * 2 - 1;

	//mainframe variables
	double heightMax = 0.8;
	double heightMin = -0.8;
	double widthMax = 0.9;
	double widthMin = -0.9;
	double barZeroX = -0.5;

	//move weight boxes
	if (pointNumber > 2 && leftClicked && movingPointIndex != -1) {
		double height = pointy;
		if (height > heightMax) {
			height = 10;
		}
		else if (height < heightMin) {
			height = -2;
		}
		else {
			height = -2 + (height - heightMin) * 12.0 / (heightMax - heightMin);
		}
		weights[movingPointIndex] = height;
		callWindowOne = true;
		display2();
	}
}

static void mouse_passive(int x, int y) {
	double widthd = width2;
	double heightd = height2;
	double pointx = (x / widthd) * 2 - 1;
	double pointy = ((heightd - y) / heightd) * 2 - 1;

	//mainframe variables
	double heightMax = 0.8;
	double heightMin = -0.8;
	double widthMax = 0.9;
	double widthMin = -0.9;
	double barZeroX = -0.5;

	//check if red point needs to stay
	if (pointNumber > 2 && rightClicked == true) {
		movingPointIndex = -1;
		int boxes = pointNumber - 2;
		double boxWidth = (widthMax - barZeroX) / boxes;
		double space = boxWidth / 10.0;
		boxWidth -= space;

		double xPos = barZeroX;
		for (int i = 0; i < boxes; i++) {
			xPos += space;
			if (pointx > xPos) {
				xPos += boxWidth;
				if (pointx < xPos) {
					//point click is in range of a weight bar, mark index
					movingPointIndex = i;
					if (redPoint - 1 != movingPointIndex) {
						redPoint = -1;
						rightClicked = false;
						callWindowOne = true;
						display2();
					}
					break;
				}
			}
		}
		if (movingPointIndex == -1) {
			redPoint = -1;
			rightClicked = false;
			callWindowOne = true;
			display2();
		}
	}
}

static void mouse_press2(int button, int state, int x, int y) {
	double widthd = width2;
	double heightd = height2;
	double pointx = (x / widthd) * 2 - 1;
	double pointy = ((heightd - y) / heightd) * 2 - 1;

	if (button == 0 && state == 0) {
		leftClicked = true;

		//mainframe variables
		double heightMax = 0.8;
		double heightMin = -0.8;
		double widthMax = 0.9;
		double widthMin = -0.9;
		double barZeroX = -0.5;

		//set index to -1 incase mouse is not on weight bar
		movingPointIndex = -1;

		//check if weight bar is close to cursor
		if (pointNumber > 2) {
			int boxes = pointNumber - 2;
			double boxWidth = (widthMax - barZeroX) / boxes;
			double space = boxWidth / 10.0;
			boxWidth -= space;

			double xPos = barZeroX;
			for (int i = 0; i < boxes; i++) {
				xPos += space;
				if (pointx > xPos) {
					xPos += boxWidth;
					if (pointx < xPos) {
						//point click is in range of a weight bar, mark index
						movingPointIndex = i;
						if (redPoint - 1 != movingPointIndex) {
							redPoint = -1;
						}
						break;
					}
				}
			}
		}
	}
	else if (button == 0 && state == 1) { //left button released
		leftClicked = false;
	}
	else if (button == GLUT_RIGHT_BUTTON && state == 0) { //right button clicked
		rightClicked = true;
		redPoint = -1;

		//mainframe variables
		double heightMax = 0.8;
		double heightMin = -0.8;
		double widthMax = 0.9;
		double widthMin = -0.9;
		double barZeroX = -0.5;

		//determine if on box
		if (pointNumber > 2) {
			int boxes = pointNumber - 2;
			double boxWidth = (widthMax - barZeroX) / boxes;
			double space = boxWidth / 10.0;
			boxWidth -= space;

			double xPos = barZeroX;
			for (int i = 0; i < boxes; i++) {
				xPos += space;
				if (pointx > xPos) {
					xPos += boxWidth;
					if (pointx < xPos) {
						//point click is in range of a weight bar, mark index
						redPoint = i + 1;
						callWindowOne = true;
						display2();
						break;
					}
				}
			}
		}

	}
}

static void display() {
	glClear(GL_COLOR_BUFFER_BIT);

	//convex hull
	if (convexHull) {
		Point hull[32];
		int hullIndex = 0;
		//find lowest x
		int pos = -1;
		double smallest = 2;
		for (int i = 0; i < pointNumber; i++) {
			if (points[i].x < smallest) {
				smallest = points[i].x;
				pos = i;
			}
		}
		hull[hullIndex].x = points[pos].x;
		hull[hullIndex].y = points[pos].y;
		hullIndex++;

		glColor3f(0.1f, 0.1f, 0.6f);
		glBegin(GL_POLYGON);
		for (int i = 0; i < hullIndex; i++) {
			glVertex2d(points[i].x, points[i].y);
		}
		glEnd();
	}

	//control polygon
	if (controlPolygon) {
		glColor3f(1.0f, 1.0f, 0.0f);
		for (int i = 0; i < pointNumber; i++) {
			glBegin(GL_LINES);
			glVertex2d(points[i].x, points[i].y);
			if (i + 1 != pointNumber)
				glVertex2d(points[i + 1].x, points[i + 1].y);
			glEnd();
		}
	}

	//curve
	if (curve || curveExtended) {
		glBegin(GL_LINES);
		glColor3f(0.0f, 1.0f, 0.0f);
		glPointSize(2);
		//array size
		int p = pointNumber;
		int arraySize = 0;
		while (p > 0) {
			p--;
			arraySize += p;
		}
		Point curvePoints[496];
		int curvePointsIndex = 0;
		Point lastPoint;

		//loop for t-values (range 0 - 1)
		for (int i = 0; i <= t_density; i++) {
			double t_value = i * 1.0;
			t_value /= t_density;

			if (curveExtended) {
				t_value = (t_value * 11) - 5;
			}

			//point tree
			int loopLength = pointNumber - 1;
			for (int j = 0; j < loopLength; j++) {
				double weight = 0;
				double weightNext = 0;
				if (j != 0) {
					if (weights[j - 1] < 0) {
						weight = (weights[j - 1]) * (weights[j - 1]) * (weights[j - 1]);
					}
					else if (weights[j - 1] > 0 && weights[j - 1] < 1) {
						weight = (weights[j - 1]) + (weights[j - 1]) * 0.5;
					}
					else if (weights[j - 1] >= 1) {
						weight = (weights[j - 1] * 0.09 + .91);
					}
				}
				if (j != loopLength - 1) {
					if (weights[j] < 0) {
						weightNext = (weights[j]) * (weights[j]) * (weights[j]);
					}
					else if (weights[j] > 0 && weights[j] < 1) {
						weight = (weights[j]) + (weights[j]) * 0.5;;
					}
					else if (weights[j] >= 1) {
						weightNext = (weights[j] * 0.09 + .91);
					}
				}

				if (loopLength == 1) {
					curvePoints[curvePointsIndex].x = points[j].x + t_value * (points[j + 1].x - points[j].x);
					curvePoints[curvePointsIndex].y = points[j].y + t_value * (points[j + 1].y - points[j].y);
				}
				else if (j == 0) {
					double nextPointX = points[j + 1].x;
					double nextPointY = points[j + 1].y;
					double midPointX = (points[j + 2].x - (points[j + 2].x - points[j].x) / 2);
					double midPointY = (points[j + 2].y - (points[j + 2].y - points[j].y) / 2);
					double nextPointX_distance = midPointX - (midPointX - nextPointX) * weightNext;
					double nextPointY_distance = midPointY - (midPointY - nextPointY) * weightNext;

					curvePoints[curvePointsIndex].x = points[j].x + t_value * (nextPointX_distance - points[j].x);
					curvePoints[curvePointsIndex].y = points[j].y + t_value * (nextPointY_distance - points[j].y);
				}
				else if (j == loopLength - 1) {
					double currPointX = points[j].x;
					double currPointY = points[j].y;
					double midPointX = (points[j + 1].x - (points[j + 1].x - points[j - 1].x) / 2);
					double midPointY = (points[j + 1].y - (points[j + 1].y - points[j - 1].y) / 2);
					double currPointX_distance = midPointX + (currPointX - midPointX) * weight;
					double currPointY_distance = midPointY + (currPointY - midPointY) * weight;

					curvePoints[curvePointsIndex].x = currPointX_distance + t_value * (points[j + 1].x - currPointX_distance);
					curvePoints[curvePointsIndex].y = currPointY_distance + t_value * (points[j + 1].y - currPointY_distance);
				}
				else {
					double midPointX = (points[j + 1].x - (points[j + 1].x - points[j - 1].x) / 2);
					double midPointY = (points[j + 1].y - (points[j + 1].y - points[j - 1].y) / 2);
					double currPointX = points[j].x;
					double currPointY = points[j].y;
					double currPointX_distance = midPointX + (currPointX - midPointX) * weight;
					double currPointY_distance = midPointY + (currPointY - midPointY) * weight;
					double nextPointX = points[j + 1].x;
					double nextPointY = points[j + 1].y;
					double nextPointX_distance = midPointX - (midPointX - nextPointX) * weightNext;
					double nextPointY_distance = midPointY - (midPointY - nextPointY) * weightNext;

					curvePoints[curvePointsIndex].x = currPointX_distance + t_value * (nextPointX_distance - currPointX_distance);
					curvePoints[curvePointsIndex].y = currPointY_distance + t_value * (nextPointY_distance - currPointY_distance);
				}
				curvePointsIndex++;
			}

			int startIndex = 0;
			int increment = pointNumber - 1;
			while (curvePointsIndex < arraySize) {
				loopLength--;
				for (int j = 0; j < loopLength; j++) {
					curvePoints[curvePointsIndex].x = curvePoints[startIndex + j].x + t_value * (curvePoints[startIndex + j + 1].x - curvePoints[startIndex + j].x);
					curvePoints[curvePointsIndex].y = curvePoints[startIndex + j].y + t_value * (curvePoints[startIndex + j + 1].y - curvePoints[startIndex + j].y);
					curvePointsIndex++;
				}
				startIndex += increment;
				increment--;
			}

			//draw final point
			if (i != 0) {
				glVertex2d(lastPoint.x, lastPoint.y);
				glVertex2d(curvePoints[arraySize - 1].x, curvePoints[arraySize - 1].y);
			}
			lastPoint.x = curvePoints[arraySize - 1].x;
			lastPoint.y = curvePoints[arraySize - 1].y;
			curvePointsIndex = 0;
		}
		glEnd();
	}

	//draw points
	glPointSize(4);
	for (int i = 0; i < pointNumber; i++) {
		glColor3f(0.8f, 0.8f, 0.0f);
		glBegin(GL_POINTS);
		if (redPoint == i) {
			glColor3f(1.0f, 0.0f, 0.0f);
		}
		glVertex2d(points[i].x, points[i].y);
		glEnd();
	}

	//DeCasteljau
	if (DeCasteljau) {
		//array size
		int p = pointNumber;
		int arraySize = 0;
		while (p > 0) {
			p--;
			arraySize += p;
		}
		Point curvePoints[496];
		int curvePointsIndex = 0;

		if (curveExtended) {
			if (DeCasteljau_t_value > 6)
				DeCasteljau_t_value = 6;
			else if (DeCasteljau_t_value < -5)
				DeCasteljau_t_value = -5;
		}
		else {
			if (DeCasteljau_t_value > 1)
				DeCasteljau_t_value = 1;
			else if (DeCasteljau_t_value < 0)
				DeCasteljau_t_value = 0;
		}

		double t_value = DeCasteljau_t_value;

		//point tree
		int loopLength = pointNumber - 1;
		for (int j = 0; j < loopLength; j++) {
			double weight = 0;
			double weightNext = 0;
			if (j != 0) {
				if (weights[j - 1] < 0) {
					weight = (weights[j - 1]) * (weights[j - 1]) * (weights[j - 1]);
				}
				else if (weights[j - 1] > 0 && weights[j - 1] < 1) {
					weight = (weights[j - 1]) + (weights[j - 1]) * 0.5;
				}
				else if (weights[j - 1] >= 1) {
					weight = (weights[j - 1] * 0.09 + .91);
				}
			}
			if (j != loopLength - 1) {
				if (weights[j] < 0) {
					weightNext = (weights[j]) * (weights[j]) * (weights[j]);
				}
				else if (weights[j] > 0 && weights[j] < 1) {
					weight = (weights[j]) + (weights[j]) * 0.5;;
				}
				else if (weights[j] >= 1) {
					weightNext = (weights[j] * 0.09 + .91);
				}
			}

			if (loopLength == 1) {
				curvePoints[curvePointsIndex].x = points[j].x + t_value * (points[j + 1].x - points[j].x);
				curvePoints[curvePointsIndex].y = points[j].y + t_value * (points[j + 1].y - points[j].y);
			}
			else if (j == 0) {
				double nextPointX = points[j + 1].x;
				double nextPointY = points[j + 1].y;
				double midPointX = (points[j + 2].x - (points[j + 2].x - points[j].x) / 2);
				double midPointY = (points[j + 2].y - (points[j + 2].y - points[j].y) / 2);
				double nextPointX_distance = midPointX - (midPointX - nextPointX) * weightNext;
				double nextPointY_distance = midPointY - (midPointY - nextPointY) * weightNext;

				curvePoints[curvePointsIndex].x = points[j].x + t_value * (nextPointX_distance - points[j].x);
				curvePoints[curvePointsIndex].y = points[j].y + t_value * (nextPointY_distance - points[j].y);
			}
			else if (j == loopLength - 1) {
				double currPointX = points[j].x;
				double currPointY = points[j].y;
				double midPointX = (points[j + 1].x - (points[j + 1].x - points[j - 1].x) / 2);
				double midPointY = (points[j + 1].y - (points[j + 1].y - points[j - 1].y) / 2);
				double currPointX_distance = midPointX + (currPointX - midPointX) * weight;
				double currPointY_distance = midPointY + (currPointY - midPointY) * weight;

				curvePoints[curvePointsIndex].x = currPointX_distance + t_value * (points[j + 1].x - currPointX_distance);
				curvePoints[curvePointsIndex].y = currPointY_distance + t_value * (points[j + 1].y - currPointY_distance);
			}
			else {
				double midPointX = (points[j + 1].x - (points[j + 1].x - points[j - 1].x) / 2);
				double midPointY = (points[j + 1].y - (points[j + 1].y - points[j - 1].y) / 2);
				double currPointX = points[j].x;
				double currPointY = points[j].y;
				double currPointX_distance = midPointX + (currPointX - midPointX) * weight;
				double currPointY_distance = midPointY + (currPointY - midPointY) * weight;
				double nextPointX = points[j + 1].x;
				double nextPointY = points[j + 1].y;
				double nextPointX_distance = midPointX - (midPointX - nextPointX) * weightNext;
				double nextPointY_distance = midPointY - (midPointY - nextPointY) * weightNext;

				curvePoints[curvePointsIndex].x = currPointX_distance + t_value * (nextPointX_distance - currPointX_distance);
				curvePoints[curvePointsIndex].y = currPointY_distance + t_value * (nextPointY_distance - currPointY_distance);
			}
			//draw point
			glColor3f(0.93f, 1.0f, 1.0f);
			glPointSize(4);
			glBegin(GL_POINTS);
			glVertex2d(curvePoints[curvePointsIndex].x, curvePoints[curvePointsIndex].y);
			glEnd();

			//draw line
			if (j != 0) {
				glColor3f(0.0f, 1.0f, 1.0f);
				glBegin(GL_LINES);
				glVertex2d(curvePoints[curvePointsIndex - 1].x, curvePoints[curvePointsIndex - 1].y);
				glVertex2d(curvePoints[curvePointsIndex].x, curvePoints[curvePointsIndex].y);
				glEnd();
			}

			curvePointsIndex++;
		}

		int startIndex = 0;
		int increment = pointNumber - 1;
		while (curvePointsIndex < arraySize) {
			loopLength--;
			for (int j = 0; j < loopLength; j++) {
				curvePoints[curvePointsIndex].x = curvePoints[startIndex + j].x + t_value * (curvePoints[startIndex + j + 1].x - curvePoints[startIndex + j].x);
				curvePoints[curvePointsIndex].y = curvePoints[startIndex + j].y + t_value * (curvePoints[startIndex + j + 1].y - curvePoints[startIndex + j].y);
				//draw point
				glColor3f(0.93f, 1.0f, 1.0f);
				glPointSize(4);
				glBegin(GL_POINTS);
				glVertex2d(curvePoints[curvePointsIndex].x, curvePoints[curvePointsIndex].y);
				glEnd();

				//draw line
				if (j != 0) {
					glColor3f(0.0f, 1.0f, 1.0f);
					glBegin(GL_LINES);
					glVertex2d(curvePoints[curvePointsIndex - 1].x, curvePoints[curvePointsIndex - 1].y);
					glVertex2d(curvePoints[curvePointsIndex].x, curvePoints[curvePointsIndex].y);
					glEnd();
				}

				curvePointsIndex++;
			}
			startIndex += increment;
			increment--;
		}

		//draw final point
		glPointSize(4);
		glColor3f(1.2f, 0.2f, 1.0f);
		glBegin(GL_POINTS);
		glVertex2d(curvePoints[arraySize - 1].x, curvePoints[arraySize - 1].y);
		glEnd();

		DeCasteljau_x = curvePoints[arraySize - 1].x;
		DeCasteljau_y = curvePoints[arraySize - 1].y;
		curvePointsIndex = 0;
	}

	//SwapBuffers
	glutSwapBuffers();

	glutSetWindow(windowTwo);

	if (callWindowOne) {
		callWindowOne = false;
	}
	else {
		display2();
	}
}

static void display2() {
	glClear(GL_COLOR_BUFFER_BIT);

	//draw mainframe
	double heightMax = 0.8;
	double heightMin = -0.8;
	double widthMax = 0.9;
	double widthMin = -0.9;
	double barZeroX = -0.5;

	glColor3f(0.0f, 0.0f, 0.0f);
	glLineWidth(2);
	glBegin(GL_LINES);
	glVertex2d(widthMin, heightMin + 1.6 * 1 / 6.0);
	glVertex2d(widthMax, heightMin + 1.6 * 1 / 6.0);
	glVertex2d(barZeroX, heightMin);
	glVertex2d(barZeroX, heightMax);
	glVertex2d(barZeroX - 0.2, heightMin);
	glVertex2d(barZeroX, heightMin);
	glVertex2d(barZeroX - 0.2, heightMin + 1.6 * 2 / 6.0);
	glVertex2d(barZeroX, heightMin + 1.6 * 2 / 6.0);
	glVertex2d(barZeroX - 0.2, heightMin + 1.6 * 3 / 6.0);
	glVertex2d(barZeroX, heightMin + 1.6 * 3 / 6.0);
	glVertex2d(barZeroX - 0.2, heightMin + 1.6 * 4 / 6.0);
	glVertex2d(barZeroX, heightMin + 1.6 * 4 / 6.0);
	glVertex2d(barZeroX - 0.2, heightMin + 1.6 * 5 / 6.0);
	glVertex2d(barZeroX, heightMin + 1.6 * 5 / 6.0);
	glVertex2d(widthMin, heightMin + 1.6);
	glVertex2d(barZeroX, heightMin + 1.6);
	glEnd();

	//draw weight boxes
	if (pointNumber > 2) {
		int boxes = pointNumber - 2;
		double boxWidth = (widthMax - barZeroX) / boxes;
		double space = boxWidth / 10.0;
		boxWidth -= space;

		double xPos = barZeroX;
		for (int i = 0; i < boxes; i++) {
			xPos += space;
			glBegin(GL_POLYGON);
			glColor3f(0.9f, 0.8f, 0.3f); //yellow
			double weightHight = heightMin + ((weights[i] + 2) / 12.0) * (heightMax - heightMin);
			if (weightHight < heightMin + 1.6 * 1 / 6.0) {
				glColor3f(1.0f, 0.2f, 0.2f); //red
			}
			glVertex2d(xPos, heightMin + 1.6 * 1 / 6.0);
			glVertex2d(xPos, weightHight);
			xPos += boxWidth;
			glVertex2d(xPos, weightHight);
			glVertex2d(xPos, heightMin + 1.6 * 1 / 6.0);
			glEnd();
		}
	}
	//SwapBuffers
	glutSwapBuffers();

	glutSetWindow(windowOne);
	if (callWindowOne) {
		display();
	}
}

void reshape(int w, int h) {
	width = w;
	height = h;

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void reshape2(int w, int h) {
	width2 = w;
	height2 = h;

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void init(void) {
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float s = width / 2.0;
	if (height < width) s = height / 2;
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void init2(void) {
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float s = width / 2.0;
	if (height < width) s = height / 2;
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	//initialize all weights to 1
	for (int i = 0; i < 30; i++) {
		weights[i] = 1.0;
	}

	glutInit(&__argc, __argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

	//w window
	glutInitWindowSize(width / 2.0, height / 2.0);
	glutInitWindowPosition(500, 100);
	windowTwo = glutCreateWindow("W-window");
	init2();
	glutReshapeFunc(reshape2);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glutDisplayFunc(display2);
	glutMouseFunc(mouse_press2);
	glutMotionFunc(mouse_active_move2);
	glutPassiveMotionFunc(mouse_passive);

	//g window
	glutInitWindowSize(width, height);
	glutInitWindowPosition(800, 100);
	windowOne = glutCreateWindow("G-window");
	init();
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);

	//mouse callbacks
	createGLUTMenus();
	glutMouseFunc(mouse_press);
	glutMotionFunc(mouse_active_move);

	glutMainLoop();
	return 0;
}