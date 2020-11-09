#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}





void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  

/*
	//Начало рисования квадратика станкина
	double A[2] = { -4, -4 };
	double B[2] = { 4, -4 };
	double C[2] = { 4, 4 };
	double D[2] = { -4, 4 };

	glBindTexture(GL_TEXTURE_2D, texId);

	glColor3d(0.6, 0.6, 0.6);
	glBegin(GL_QUADS);

	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);

	glEnd();
	//конец рисования квадратика станкина
	*/
	/*

	double plane0 = 0;
	double plane1 = 2;

	double A0[] = { 2,0,plane0 };
	double B0[] = { 6,3,plane0 };
	double C0[] = { 0,1,plane0 };
	double D0[] = { -1,7,plane0 };
	double E0[] = { -5,3,plane0 };
	double F0[] = { -3, 2, plane0 };
	double G0[] = { -7,0,plane0 };
	double H0[] = { 2,-6,plane0 };
	double I0[] = { 5, -2, plane0 };

	double A1[] = { 2,0,plane1 };
	double B1[] = { 6,3,plane1 };
	double C1[] = { 0,1,plane1 };
	double D1[] = { -1,7,plane1 };
	double E1[] = { -5,3,plane1 };
	double F1[] = { -3, 2, plane1 };
	double G1[] = { -7,0,plane1 };
	double H1[] = { 2,-6,plane1 };
	double I1[] = { 5, -2, plane1 };

	double colorOne[] = { 0.41, 0.41, 0.41 };
	double colorTwo[] = { 0.45, 0.45, 0.45 };
	double colorThree[] = { 0.21, 0.21, 0.21 };
	double colorFour[] = { 0.31, 0.31, 0.31 };





	// Пол
#pragma region
	glNormal3d(0, 0, -1);
	glBegin(GL_TRIANGLES);

	glNormal3d(0, 0, -1);
	glColor3dv(colorOne);

	glNormal3d(0, 0, -1);
	glVertex3dv(A0);
	glVertex3dv(B0);
	glVertex3dv(C0);

	glNormal3d(0, 0, -1);
	glVertex3dv(C0);
	glVertex3dv(D0);
	glVertex3dv(F0);

	glNormal3d(0, 0, -1);
	glVertex3dv(F0);
	glVertex3dv(D0);
	glVertex3dv(E0);

	glNormal3d(0, 0, -1);
	glVertex3dv(F0);
	glVertex3dv(C0);
	glVertex3dv(G0);

	glNormal3d(0, 0, -1);
	glVertex3dv(G0);
	glVertex3dv(C0);
	glVertex3dv(A0);

	//glVertex3dv(G0);
	//glVertex3dv(A0);
	//glVertex3dv(H0);

	glNormal3d(0, 0, -1);
	glVertex3dv(H0);
	glVertex3dv(A0);
	glVertex3dv(I0);

	glEnd();

#pragma endregion



	// Потолок
#pragma region
	glNormal3d(0, 0, 1);
	glBegin(GL_TRIANGLES);

	glNormal3d(0, 0, 1);

	glColor3dv(colorTwo);
	
	glNormal3d(0, 0, 1);
	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(C1);

	glNormal3d(0, 0, 1);
	glVertex3dv(C1);
	glVertex3dv(D1);
	glVertex3dv(F1);

	glNormal3d(0, 0, 1);
	glVertex3dv(F1);
	glVertex3dv(D1);
	glVertex3dv(E1);

	glNormal3d(0, 0, 1);
	glVertex3dv(F1);
	glVertex3dv(C1);
	glVertex3dv(G1);

	glNormal3d(0, 0, 1);
	glVertex3dv(G1);
	glVertex3dv(C1);
	glVertex3dv(A1);

	//glVertex3dv(G1);
	//glVertex3dv(A1);
	//glVertex3dv(H1);

	glNormal3d(0, 0, 1);
	glVertex3dv(H1);
	glVertex3dv(A1);
	glVertex3dv(I1);

	glEnd();

#pragma endregion

	// Стена
#pragma region 
	glBegin(GL_QUADS);

	glColor3dv(colorThree);

	glNormal3d(6, -8, 0);
	glVertex3dv(A0);
	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(B0);

	glNormal3d(6, 1, 0);
	glVertex3dv(C0);
	glVertex3dv(C1);
	glVertex3dv(D1);
	glVertex3dv(D0);
	
	glNormal3d(-2, -4, 0);
	glVertex3dv(E0);
	glVertex3dv(E1);
	glVertex3dv(F1);
	glVertex3dv(F0);

	//glVertex3dv(G0);
	//glVertex3dv(G1);
	//glVertex3dv(H1);
	//glVertex3dv(H0);

	glColor3dv(colorFour);

	glNormal3d(-4, 12, 0);
	glVertex3dv(B0);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glVertex3dv(C0);

	glNormal3d(4, 6, 0);
	glVertex3dv(I0);
	glVertex3dv(I1);
	glVertex3dv(A1);
	glVertex3dv(A0);

	//glVertex3dv(D0);
	//glVertex3dv(D1);
	//glVertex3dv(E1);
	//glVertex3dv(E0);

	glNormal3d(-4, 8, 0);
	glVertex3dv(F0);
	glVertex3dv(F1);
	glVertex3dv(G1);
	glVertex3dv(G0);

	glNormal3d(8, -6, 0);
	glVertex3dv(H0);
	glVertex3dv(H1);
	glVertex3dv(I1);
	glVertex3dv(I0);


	glEnd();

#pragma endregion


	// пол выпуклости
#pragma region
	glBegin(GL_TRIANGLE_FAN);
	float x = -3;
	float y = 5;
	float r = sqrt(8);
	float Pi = 3.14159265358979323846;


	float start = Pi / 4 - Pi / 360;
	float end = 5 * Pi / 4 + Pi / 360;

	glNormal3d(0, 0, -1);

	glColor3dv(colorOne);
	glVertex3f(x, y, 0); // вершина

	for (float a = start; a <= end; a += 0.01) {

		glVertex2f(x - sin(a) * r, y - cos(a) * r);
	}

	glEnd();
#pragma endregion

	//  потолок выпуклости
#pragma region

	glPushMatrix();
	glTranslatef(0, 0, 2);

	glBegin(GL_TRIANGLE_FAN);

	glNormal3d(0, 0, 1);
	glColor3dv(colorTwo);
	glVertex2f(x, y); // вершина

	for (float a = start; a <= end; a += 0.01) {
		glNormal3d(0, 0, 1);
		glVertex2f(x - sin(a) * r, y - cos(a) * r);
	}

	glEnd();
	glPopMatrix();
#pragma endregion

	// стена выпуклости
#pragma region
	glBegin(GL_QUADS);
	x = -3;
	y = 5;
	r = sqrt(8);
	Pi = 3.14159265358979323846;


	start = Pi / 4;
	end = 5 * Pi / 4;

	double vektor1[3];
	double vektor2[3];
	double n[] = { 0, 0, 0 };
	double vx1, vy1, vz1, vx2, vy2, vz2, ax, bx, ay, by, az, bz, cx, cy, cz, nx, ny, nz;

	glColor3dv(colorFour);

	for (float a = start; a <= end - 0.01; a += 0.01) {


		ax = x - sin(a) * r;
		ay = y - cos(a) * r;
		az = plane0;

		bx = x - sin(a) * r;
		by = y - cos(a) * r;
		bz = plane1;

		cx = x - sin(a + 0.01) * r;
		cy = y - cos(a + 0.01) * r;
		cz = plane1;

		vx1 = ax - bx;
		vy1 = ay - by;
		vz1 = az - bz;
		vx2 = bx - cx;
		vy2 = by - cy;
		vz2 = bz - cz;

		nx = vy1 * vz2 - vz1 * vy2;
		ny = vz1 * vx2 - vx1 * vz2;
		nz = vx1 * vy2 - vy1 * vx2;

		glNormal3d(nx, ny, nz);

		glVertex3f(x - sin(a) * r, y - cos(a) * r, plane0); //a
		glVertex3f(x - sin(a) * r, y - cos(a) * r, plane1); //b
		glVertex3f(x - sin(a + 0.01) * r, y - cos(a + 0.01) * r, plane1); //c
		glVertex3f(x - sin(a + 0.01) * r, y - cos(a + 0.01) * r, plane0);
	}

	glEnd();
#pragma endregion


	// Пол вогнутости
#pragma region
	glBegin(GL_TRIANGLES);
	x = -10.1667;
	y = -14.5;
	r = 14.85176;
	Pi = 3.14159265358979323846;


	start = Pi * 34.89879 / 180;
	end = Pi * 77.870 / 180;

	glColor3dv(colorOne);

	glNormal3d(0, 0, -1);
	for (float a = start; a <= end - 0.05; a += 0.005) {

		glVertex2f(x + cos(a) * r, y + sin(a) * r);
		glVertex2f(x + cos(a + 0.05) * r, y + sin(a + 0.05) * r);
		glVertex2f(2, 0);
	}

	glEnd();
#pragma endregion


	// потолок вогнутости
#pragma region
	glPushMatrix();
	glTranslatef(0, 0, 2);

	glBegin(GL_TRIANGLES);

	x = -10.1667;
	y = -14.5;
	r = 14.85176;
	Pi = 3.14159265358979323846;


	start = Pi * 34.89879 / 180;
	end = Pi * 77.870 / 180;

	glColor3dv(colorTwo);
	glNormal3d(0, 0, 1);

	for (float a = start; a <= end - 0.05; a += 0.005) {

		glVertex2f(x + cos(a) * r, y + sin(a) * r);
		glVertex2f(x + cos(a + 0.05) * r, y + sin(a + 0.05) * r);
		glVertex2f(2, 0);
	}

	glEnd();
	glPopMatrix();
#pragma endregion

	// Стена вогнутости
#pragma region
	glBegin(GL_QUADS);
	x = -10.1667;
	y = -14.5;
	r = 14.85176;
	Pi = 3.14159265358979323846;


	start = Pi * 34.89879 / 180;
	end = Pi * 77.870 / 180;

	glColor3dv(colorThree);

	for (float a = start; a <= end - 0.05; a += 0.005) {

		ax = x + cos(a) * r;
		ay = y + sin(a) * r;
		az = plane0;

		bx = x + cos(a) * r;
		by = y + sin(a) * r;
		bz = plane1;

		cx = x + cos(a + 0.05) * r;
		cy = y + sin(a + 0.05) * r;
		cz = plane1;

		vx1 = ax - bx;
		vy1 = ay - by;
		vz1 = az - bz;
		vx2 = bx - cx;
		vy2 = by - cy;
		vz2 = bz - cz;

		nx = vy1 * vz2 - vz1 * vy2;
		ny = vz1 * vx2 - vx1 * vz2;
		nz = vx1 * vy2 - vy1 * vx2;

		glNormal3d(nx, ny, nz);

		glVertex3f(x + cos(a) * r, y + sin(a) * r, plane0);
		glVertex3f(x + cos(a) * r, y + sin(a) * r, plane1);
		glVertex3f(x + cos(a + 0.05) * r, y + sin(a + 0.05) * r, plane1);
		glVertex3f(x + cos(a + 0.05) * r, y + sin(a + 0.05) * r, plane0);

	}

	glEnd();

#pragma endregion
	*/


		glBegin(GL_TRIANGLES);
		glColor3d(0, 1, 0);

		glNormal3b(0, 0, -1);
		glVertex3d(0, 0, 0);
		glVertex3d(3, 5, 0);
		glVertex3d(6, 3, 0);

		glVertex3d(3, 5, 0);
		glVertex3d(6, 3, 0);
		glVertex3d(9, 4, 0);

		glVertex3d(9, 4, 0);
		glVertex3d(6, 3, 0);
		glVertex3d(12, 0, 0);

		glVertex3d(12, 0, 0);
		glVertex3d(9, 4, 0);
		glVertex3d(11, 6, 0);

		glVertex3d(12, 0, 0);
		glVertex3d(6, 3, 0);
		glVertex3d(1, -4, 0);


		glNormal3b(0, 0, 1);
		glVertex3d(0, 0, 5);
		glVertex3d(3, 5, 5);
		glVertex3d(6, 3, 5);

		glVertex3d(3, 5, 5);
		glVertex3d(6, 3, 5);
		glVertex3d(9, 4, 5);

		glVertex3d(9, 4, 5);
		glVertex3d(6, 3, 5);
		glVertex3d(12, 0, 5);

		glVertex3d(12, 0, 5);
		glVertex3d(9, 4, 5);
		glVertex3d(11, 6, 5);

		glVertex3d(12, 0, 5);
		glVertex3d(6, 3, 5);
		glVertex3d(1, -4, 5);

		glEnd();

		glBegin(GL_QUADS);

		glColor3d(1, 0, 0);

		glNormal3b(5, 30, 0);
		glVertex3d(9, 4, 0);
		glVertex3d(9, 4, 5);
		glVertex3d(3, 5, 5);
		glVertex3d(3, 5, 0);

		glColor3d(0, 0, 1);

		glNormal3b(-10, 10, 0);
		glVertex3d(9, 4, 0);
		glVertex3d(9, 4, 5);
		glVertex3d(11, 6, 5);
		glVertex3d(11, 6, 0);

		glColor3d(1, 0, 0);

		glNormal3b(30, 5, 0);
		glVertex3d(11, 6, 0);
		glVertex3d(11, 6, 5);
		glVertex3d(12, 0, 5);
		glVertex3d(12, 0, 0);

		glColor3d(0, 0, 1);

		glNormal3b(20, -55, 0);
		glVertex3d(12, 0, 0);
		glVertex3d(12, 0, 5);
		glVertex3d(1, -4, 5);
		glVertex3d(1, -4, 0);

		glColor3d(1, 0, 0);

		glNormal3b(-35, 25, 0);
		glVertex3d(1, -4, 0);
		glVertex3d(1, -4, 5);
		glVertex3d(6, 3, 5);
		glVertex3d(6, 3, 0);

		glColor3d(0, 0, 1);
		
		glNormal3b(15, -30, 0);
		glVertex3d(6, 3, 0);
		glVertex3d(6, 3, 5);
		glVertex3d(0, 0, 5);
		glVertex3d(0, 0, 0);

		glEnd();

		glBegin(GL_TRIANGLE_FAN);
		glNormal3b(0, 0, -1);
		glVertex3d(1.5, 2.5, 0);
		for (float a = -2.11; a <= 1.03; a += 0.01)
		{
			glVertex2d(1.5 - cos(a) * 2.91547595, 2.5 - sin(a) * 2.91547595);
		}
		glEnd();

		glBegin(GL_TRIANGLE_FAN);
		glNormal3b(0, 0, 1);
		glVertex3d(1.5, 2.5, 5);
		for (float a = -2.11; a <= 1.03; a += 0.01)
		{
			glVertex3d(1.5 - cos(a) * 2.91547595, 2.5 - sin(a) * 2.91547595, 5);
		}
		glEnd();

		double n[] = { 0, 0, 0 };
		double vx1, vy1, vz1, vx2, vy2, vz2, ax, bx, ay, by, az, bz, cx, cy, cz, nx, ny, nz, r, x, y;

		glBegin(GL_QUAD_STRIP);
		for (float a = -2.11; a <= 1.03; a += 0.01)
		{

			
		
			

			ax = 1.5 - cos(a) * 2.91547595;
			ay = 2.5 - sin(a) * 2.91547595;
			az = 0;

			bx = 1.5 - cos(a) * 2.91547595;
			by = 2.5 - sin(a) * 2.91547595;
			bz = 5;

			cx = 1.5 - cos(a + 0.01) * 2.91547595;
			cy = 2.5 - sin(a + 0.01) * 2.91547595;
			cz = 5;

			vx1 = ax - bx;
			vy1 = ay - by;
			vz1 = az - bz;
			vx2 = bx - cx;
			vy2 = by - cy;
			vz2 = bz - cz;

			nx = vy1 * vz2 - vz1 * vy2;
			ny = vz1 * vx2 - vx1 * vz2;
			nz = vx1 * vy2 - vy1 * vx2;

			glNormal3d(-nx, -ny, nz);

			glVertex3d(1.5 - cos(a) * 2.91547595, 2.5 - sin(a) * 2.91547595, 0);
			glVertex3d(1.5 - cos(a) * 2.91547595, 2.5 - sin(a) * 2.91547595, 5);
		}
		glEnd();

   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}