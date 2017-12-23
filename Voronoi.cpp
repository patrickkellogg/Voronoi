// Voronoi.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <gl\gl.h>			//Header File For The OpenGL32 Library
#include <gl\glu.h>			//Header File For The GLu32 Library
#include <gl\glaux.h>		//Header File For The Glaux Library
#include <stdlib.h>			//Standard Header File For C++
#include <math.h>			//Math Function Header File For C++
#include <time.h>			//Use this to seed the random number generator
#include <limits.h>			//Provides INT_MAX value

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;						// current instance
TCHAR szTitle[MAX_LOADSTRING];			// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];	// The title bar text

HWND g_hwndClient;
HDC g_dcClient;
HGLRC g_hGLContext;						// required GLContext

//Other required classes
class GLfloatPoint{
public:

	float x, y;
};
class GLfloatVect{
public:

	float x, y;
};
class GLfloatEdge{
public:

	GLfloatPoint glpnt1, glpnt2;
};
class GLfloatTriangle{
public:

	GLfloatPoint glpnt1, glpnt2, glpnt3;
};

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void				DrawDot(GLfloatPoint glpntIn);
void				DrawLine(GLfloatPoint glpnt1, GLfloatPoint glpnt2);
bool				LinesIntersect(GLfloatPoint glpnta, GLfloatPoint glpntb, GLfloatPoint glpntc, GLfloatPoint glpntd);

//More global variables
#define MAX_POINTS 100
//MAX_EDGES must be (3*MAX_POINTS-6)
#define MAX_EDGES 294
GLfloatPoint glpntVertices[MAX_POINTS];
int nNumPoints = 0;
GLfloatEdge glpntEdges[MAX_EDGES];
int nNumEdges = 0;
//MAX_TRIANGLES = (MAX_POINTS-3)*3 + 1 ???
#define MAX_TRIANGLES 874
GLfloatTriangle glpntTriangles[MAX_TRIANGLES];
int nNumTriangles = 0;
GLfloatPoint glpntCenters[MAX_TRIANGLES];

BOOL SetWindowPixelFormat(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pixelDesc;

    pixelDesc.nSize     = sizeof(PIXELFORMATDESCRIPTOR);
    pixelDesc.nVersion = 1;
    pixelDesc.dwFlags   =

        PFD_DRAW_TO_WINDOW |// Format Must Support Window
        PFD_SUPPORT_OPENGL |// Format Must Support OpenGL
        PFD_DOUBLEBUFFER, // Must Support Double Buffering
        PFD_TYPE_RGBA,  // Request An RGBA Format
        16,     // Select A 16 Bit Color Depth
        0, 0, 0, 0, 0, 0, // Color Bits Ignored
        0,     // No Alpha Buffer
        0,     // Shift Bit Ignored
        0,     // No Accumulation Buffer
        0, 0, 0, 0,   // Accumulation Bits Ignored
        16,     // 16Bit Z-Buffer (Depth Buffer)
        0,     // No Stencil Buffer
        0,     // No Auxiliary Buffer
        PFD_MAIN_PLANE,  // Main Drawing Layer
        0,     // Reserved
        0, 0, 0;   // Layer Masks Ignored

    int iGLPixelIndex = ChoosePixelFormat(hDC, &pixelDesc);

    if(SetPixelFormat(hDC, iGLPixelIndex, &pixelDesc))
    {
        return false;
    }

    return TRUE;
}

void DrawGLScene()
{
	//Draw the points
	for (int i = 0; i < nNumPoints; i++) {
		DrawDot(glpntVertices[i]);
	}
	//Draw the triagles
	for (int j = 0; j < nNumTriangles; j++) {
		DrawLine(glpntTriangles[j].glpnt1, glpntTriangles[j].glpnt2);
		DrawLine(glpntTriangles[j].glpnt2, glpntTriangles[j].glpnt3);
		DrawLine(glpntTriangles[j].glpnt1, glpntTriangles[j].glpnt3);
	}
	//Make sure it gets drawn
	glFlush();
	SwapBuffers(g_dcClient);
}	

void ResizeScene(int iWidth, int iHeigth)
{
	// prevent a divide by zero
	if(0 == iHeigth)	
		iHeigth = 1;

	glClear(GL_COLOR_BUFFER_BIT);
	SwapBuffers(g_dcClient);
	glViewport(0, 0, iWidth, iHeigth);
//	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0,iWidth,0.0,iHeigth);
//	SwapBuffers(g_dcClient);
}

BOOL CreateViewGLContext(HDC hDC)
{
	g_hGLContext = wglCreateContext(hDC);
	if(NULL == g_hGLContext)
		return FALSE;

	if(FALSE == wglMakeCurrent(hDC, g_hGLContext))
		return FALSE;

	return TRUE;
}

void InitGl(int iWidth, int iHeigth)
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glColor3f(0.0f,0.0f,0.0f);
	glPointSize(4.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0,iWidth,0.0,iHeigth);
	glClear(GL_COLOR_BUFFER_BIT);
	SwapBuffers(g_dcClient);
}

void AddEdge(GLfloatPoint glpntIn1, GLfloatPoint glpntIn2)
{
	//See which point is first (i.e. has the lowest x)
	if (glpntIn1.x == glpntIn2.x) {
		if (glpntIn1.y < glpntIn2.y) {
			//Add the edge
			glpntEdges[nNumEdges].glpnt1.x = glpntIn1.x;
			glpntEdges[nNumEdges].glpnt1.y = glpntIn1.y;
			glpntEdges[nNumEdges].glpnt2.x = glpntIn2.x;
			glpntEdges[nNumEdges].glpnt2.y = glpntIn2.y;
			//Increment the number of edges
			nNumEdges++;
			return;
		} else if (glpntIn1.y < glpntIn2.y) {
			//Add the edge
			glpntEdges[nNumEdges].glpnt1.x = glpntIn2.x;
			glpntEdges[nNumEdges].glpnt1.y = glpntIn2.y;
			glpntEdges[nNumEdges].glpnt2.x = glpntIn1.x;
			glpntEdges[nNumEdges].glpnt2.y = glpntIn1.y;
			//Increment the number of edges
			nNumEdges++;
			return;
		} else {
			//Error
			return;
		}
	} else if (glpntIn1.x < glpntIn2.x) {
		//Point In1 is smaller
		//Add the edge
		glpntEdges[nNumEdges].glpnt1.x = glpntIn1.x;
		glpntEdges[nNumEdges].glpnt1.y = glpntIn1.y;
		glpntEdges[nNumEdges].glpnt2.x = glpntIn2.x;
		glpntEdges[nNumEdges].glpnt2.y = glpntIn2.y;
		//Increment the number of edges
		nNumEdges++;
		return;
	} else {
		//Point In2 is smaller
		//Add the edge
		glpntEdges[nNumEdges].glpnt1.x = glpntIn2.x;
		glpntEdges[nNumEdges].glpnt1.y = glpntIn2.y;
		glpntEdges[nNumEdges].glpnt2.x = glpntIn1.x;
		glpntEdges[nNumEdges].glpnt2.y = glpntIn1.y;
		//Increment the number of edges
		nNumEdges++;
		return;
	}
}

void AddTriangle(GLfloatPoint glpnta, GLfloatPoint glpntb, GLfloatPoint glpntc) {
	//Create a temp holder
	GLfloatPoint glpntTemp;

	//See which one's bigger by doing three swaps
	//Swap a and b
	if (glpnta.x > glpntb.x) {
		//Swap them
		glpntTemp.x = glpnta.x;
		glpntTemp.y = glpnta.y;
		glpnta.x = glpntb.x;
		glpnta.y = glpntb.y;
		glpntb.x = glpntTemp.x;
		glpntb.y = glpntTemp.y;
	} else if (glpnta.x == glpntb.x) {
		if (glpnta.y > glpntb.y) {
			//Swap them
			glpntTemp.x = glpnta.x;
			glpntTemp.y = glpnta.y;
			glpnta.x = glpntb.x;
			glpnta.y = glpntb.y;
			glpntb.x = glpntTemp.x;
			glpntb.y = glpntTemp.y;
		}
	}
	//Swap b and c
	if (glpntb.x > glpntc.x) {
		//Swap them
		glpntTemp.x = glpntb.x;
		glpntTemp.y = glpntb.y;
		glpntb.x = glpntc.x;
		glpntb.y = glpntc.y;
		glpntc.x = glpntTemp.x;
		glpntc.y = glpntTemp.y;
	} else if (glpnta.x == glpntb.x) {
		if (glpntb.y > glpntc.y) {
			//Swap them
			glpntTemp.x = glpntb.x;
			glpntTemp.y = glpntb.y;
			glpntb.x = glpntc.x;
			glpntb.y = glpntc.y;
			glpntc.x = glpntTemp.x;
			glpntc.y = glpntTemp.y;
		}
	}
	//Swap a and b
	if (glpnta.x > glpntb.x) {
		//Swap them
		glpntTemp.x = glpnta.x;
		glpntTemp.y = glpnta.y;
		glpnta.x = glpntb.x;
		glpnta.y = glpntb.y;
		glpntb.x = glpntTemp.x;
		glpntb.y = glpntTemp.y;
	} else if (glpnta.x == glpntb.x) {
		if (glpnta.y > glpntb.y) {
			//Swap them
			glpntTemp.x = glpnta.x;
			glpntTemp.y = glpnta.y;
			glpnta.x = glpntb.x;
			glpnta.y = glpntb.y;
			glpntb.x = glpntTemp.x;
			glpntb.y = glpntTemp.y;
		}
	}
	//Insert the new triangle
	glpntTriangles[nNumTriangles].glpnt1.x = glpnta.x;
	glpntTriangles[nNumTriangles].glpnt1.y = glpnta.y;
	glpntTriangles[nNumTriangles].glpnt2.x = glpntb.x;
	glpntTriangles[nNumTriangles].glpnt2.y = glpntb.y;
	glpntTriangles[nNumTriangles].glpnt3.x = glpntc.x;
	glpntTriangles[nNumTriangles].glpnt3.y = glpntc.y;
	nNumTriangles++;
}

bool SharedEdge(GLfloatTriangle gltriangle1, GLfloatTriangle gltriangle2) {

	//See if ab matches de
	if ((gltriangle1.glpnt1.x == gltriangle2.glpnt1.x) && (gltriangle1.glpnt1.y == gltriangle2.glpnt1.y)
		&& (gltriangle1.glpnt2.x == gltriangle2.glpnt2.x) && (gltriangle1.glpnt2.y == gltriangle2.glpnt2.y)) {
		return(1);
	}
	//See if ab matches ef
	if ((gltriangle1.glpnt1.x == gltriangle2.glpnt2.x) && (gltriangle1.glpnt1.y == gltriangle2.glpnt2.y)
		&& (gltriangle1.glpnt2.x == gltriangle2.glpnt3.x) && (gltriangle1.glpnt2.y == gltriangle2.glpnt3.y)) {
		return(1);
	}
	//See if ab matches df
	if ((gltriangle1.glpnt1.x == gltriangle2.glpnt1.x) && (gltriangle1.glpnt1.y == gltriangle2.glpnt1.y)
		&& (gltriangle1.glpnt2.x == gltriangle2.glpnt3.x) && (gltriangle1.glpnt2.y == gltriangle2.glpnt3.y)) {
		return(1);
	}
	//See if bc matches de
	if ((gltriangle1.glpnt2.x == gltriangle2.glpnt1.x) && (gltriangle1.glpnt2.y == gltriangle2.glpnt1.y)
		&& (gltriangle1.glpnt3.x == gltriangle2.glpnt2.x) && (gltriangle1.glpnt3.y == gltriangle2.glpnt2.y)) {
		return(1);
	}
	//See if bc matches ef
	if ((gltriangle1.glpnt2.x == gltriangle2.glpnt2.x) && (gltriangle1.glpnt2.y == gltriangle2.glpnt2.y)
		&& (gltriangle1.glpnt3.x == gltriangle2.glpnt3.x) && (gltriangle1.glpnt3.y == gltriangle2.glpnt3.y)) {
		return(1);
	}
	//See if bc matches df
	if ((gltriangle1.glpnt2.x == gltriangle2.glpnt1.x) && (gltriangle1.glpnt2.y == gltriangle2.glpnt1.y)
		&& (gltriangle1.glpnt3.x == gltriangle2.glpnt3.x) && (gltriangle1.glpnt3.y == gltriangle2.glpnt3.y)) {
		return(1);
	}
	//See if ac matches de
	if ((gltriangle1.glpnt1.x == gltriangle2.glpnt1.x) && (gltriangle1.glpnt1.y == gltriangle2.glpnt1.y)
		&& (gltriangle1.glpnt3.x == gltriangle2.glpnt2.x) && (gltriangle1.glpnt3.y == gltriangle2.glpnt2.y)) {
		return(1);
	}
	//See if ac matches ef
	if ((gltriangle1.glpnt1.x == gltriangle2.glpnt2.x) && (gltriangle1.glpnt1.y == gltriangle2.glpnt2.y)
		&& (gltriangle1.glpnt3.x == gltriangle2.glpnt3.x) && (gltriangle1.glpnt3.y == gltriangle2.glpnt3.y)) {
		return(1);
	}
	//See if ac matches df
	if ((gltriangle1.glpnt1.x == gltriangle2.glpnt1.x) && (gltriangle1.glpnt1.y == gltriangle2.glpnt1.y)
		&& (gltriangle1.glpnt3.x == gltriangle2.glpnt3.x) && (gltriangle1.glpnt3.y == gltriangle2.glpnt3.y)) {
		return(1);
	}

	//Otherwise, we are different
	return(0);
}

bool IsInside(GLfloatPoint glpntQ, GLfloatTriangle gltriangleBoundary) {

	//See if the point is on the triangle
	if ((glpntQ.x==gltriangleBoundary.glpnt1.x)&&(glpntQ.y==gltriangleBoundary.glpnt1.y)) {
		return 0;
	}
	if ((glpntQ.x==gltriangleBoundary.glpnt2.x)&&(glpntQ.y==gltriangleBoundary.glpnt2.y)) {
		return 0;
	}
	if ((glpntQ.x==gltriangleBoundary.glpnt3.x)&&(glpntQ.y==gltriangleBoundary.glpnt3.y)) {
		return 0;
	}

	//Create some storage vectors
	GLfloatPoint glpntABmid;
	GLfloatPoint glpntBCmid;
	GLfloatPoint glpntACmid;
	GLfloatVect glvectABtemp;
	GLfloatVect glvectBCtemp;
	GLfloatVect glvectACtemp;
	GLfloatVect glvectQABtemp;
	GLfloatVect glvectQBCtemp;
	GLfloatVect glvectQACtemp;
	GLfloatVect glvectABperp;
	GLfloatVect glvectBCperp;
	GLfloatVect glvectACperp;
	GLfloatVect glvectQA;
	GLfloatVect glvectQB;
	GLfloatVect glvectQC;
	GLfloatVect glvect1norm;
	GLfloatVect glvect2norm;
	double d1norm;
	double d2norm;
	double dDotProduct;

	//Find the midpoints
	glpntABmid.x = ((gltriangleBoundary.glpnt1.x)+(gltriangleBoundary.glpnt2.x))/2;
	glpntABmid.y = ((gltriangleBoundary.glpnt1.y)+(gltriangleBoundary.glpnt2.y))/2;
	glpntBCmid.x = ((gltriangleBoundary.glpnt2.x)+(gltriangleBoundary.glpnt3.x))/2;
	glpntBCmid.y = ((gltriangleBoundary.glpnt2.y)+(gltriangleBoundary.glpnt3.y))/2;
	glpntACmid.x = ((gltriangleBoundary.glpnt1.x)+(gltriangleBoundary.glpnt3.x))/2;
	glpntACmid.y = ((gltriangleBoundary.glpnt1.y)+(gltriangleBoundary.glpnt3.y))/2;

	//Find perpendicular vectors
	glvectABtemp.x = (gltriangleBoundary.glpnt2.y) - glpntABmid.y;
	glvectABtemp.y = glpntABmid.x - (gltriangleBoundary.glpnt2.x);
	glvectQABtemp.x = gltriangleBoundary.glpnt3.x - glpntABmid.x;
	glvectQABtemp.y = gltriangleBoundary.glpnt3.y - glpntABmid.x;
	d1norm = sqrt(fabs((glvectABtemp.x*glvectABtemp.x) + (glvectABtemp.y*glvectABtemp.y)));
	d2norm = sqrt(fabs((glvectQABtemp.x*glvectQABtemp.x) + (glvectQABtemp.y*glvectQABtemp.y)));
	glvect1norm.x = float(glvectABtemp.x/d1norm);
	glvect1norm.y = float(glvectABtemp.y/d1norm);
	glvect2norm.x = float(glvectQABtemp.x/d2norm);
	glvect2norm.y = float(glvectQABtemp.y/d2norm);
	dDotProduct = (glvect1norm.x * glvect2norm.x) + (glvect1norm.y * glvect2norm.y);
	if (dDotProduct > 0) {
		//Go in the opposite direction
		glvectABperp.x = 0 - glvectABtemp.x;
		glvectABperp.y = 0 - glvectABtemp.y;
	} else {
		//It's ok
		glvectABperp.x = glvectABtemp.x;
		glvectABperp.y = glvectABtemp.y;
	}
	glvectBCtemp.x = (gltriangleBoundary.glpnt3.y) - glpntBCmid.y;
	glvectBCtemp.y = glpntBCmid.x - (gltriangleBoundary.glpnt3.x);
	glvectQBCtemp.x = gltriangleBoundary.glpnt1.x - glpntBCmid.x;
	glvectQBCtemp.y = gltriangleBoundary.glpnt1.y - glpntBCmid.y;
	d1norm = sqrt(fabs((glvectBCtemp.x*glvectBCtemp.x) + (glvectBCtemp.y*glvectBCtemp.y)));
	d2norm = sqrt(fabs((glvectQBCtemp.x*glvectQBCtemp.x) + (glvectQBCtemp.y*glvectQBCtemp.y)));
	glvect1norm.x = float(glvectBCtemp.x/d1norm);
	glvect1norm.y = float(glvectBCtemp.y/d1norm);
	glvect2norm.x = float(glvectQBCtemp.x/d2norm);
	glvect2norm.y = float(glvectQBCtemp.y/d2norm);
	dDotProduct = (glvect1norm.x * glvect2norm.x) + (glvect1norm.y * glvect2norm.y);
	if (dDotProduct > 0) {
		//Go in the opposite direction
		glvectBCperp.x = 0 - glvectBCtemp.x;
		glvectBCperp.y = 0 - glvectBCtemp.y;
	} else {
		//It's ok
		glvectBCperp.x = glvectBCtemp.x;
		glvectBCperp.y = glvectBCtemp.y;
	}
	glvectACtemp.x = (gltriangleBoundary.glpnt1.y) - glpntACmid.y;
	glvectACtemp.y = glpntACmid.x - (gltriangleBoundary.glpnt1.x);
	glvectQACtemp.x = gltriangleBoundary.glpnt2.x - glpntACmid.x;
	glvectQACtemp.y = gltriangleBoundary.glpnt2.y - glpntACmid.y;
	d1norm = sqrt(fabs((glvectACtemp.x*glvectACtemp.x) + (glvectACtemp.y*glvectACtemp.y)));
	d2norm = sqrt(fabs((glvectQACtemp.x*glvectQACtemp.x) + (glvectQACtemp.y*glvectQACtemp.y)));
	glvect1norm.x = float(glvectACtemp.x/d1norm);
	glvect1norm.y = float(glvectACtemp.y/d1norm);
	glvect2norm.x = float(glvectQACtemp.x/d2norm);
	glvect2norm.y = float(glvectQACtemp.y/d2norm);
	dDotProduct = (glvect1norm.x * glvect2norm.x) + (glvect1norm.y * glvect2norm.y);
	if (dDotProduct > 0) {
		//Go in the opposite direction
		glvectACperp.x = 0 - glvectACtemp.x;
		glvectACperp.y = 0 - glvectACtemp.y;
	} else {
		//It's ok
		glvectACperp.x = glvectACtemp.x;
		glvectACperp.y = glvectACtemp.y;
	}

	//Find the Q vectors
	glvectQA.x = glpntQ.x - gltriangleBoundary.glpnt1.x;
	glvectQA.y = glpntQ.y - gltriangleBoundary.glpnt1.y;
	glvectQB.x = glpntQ.x - gltriangleBoundary.glpnt2.x;
	glvectQB.y = glpntQ.y - gltriangleBoundary.glpnt2.y;
	glvectQC.x = glpntQ.x - gltriangleBoundary.glpnt3.x;
	glvectQC.y = glpntQ.y - gltriangleBoundary.glpnt3.y;

	//See if the point is within the boundary
	//(i.e. the angle between the Qxy and all perps is greater than 90
	//If any dotproduct is less than zero, the point is outside the triangle

	//Test QA vs. glvectABperp
	d1norm = sqrt(fabs((glvectQA.x*glvectQA.x) + (glvectQA.y*glvectQA.y)));
	d2norm = sqrt(fabs((glvectABperp.x*glvectABperp.x) + (glvectABperp.y*glvectABperp.y)));
	glvect1norm.x = float(glvectQA.x/d1norm);
	glvect1norm.y = float(glvectQA.y/d1norm);
	glvect2norm.x = float(glvectABperp.x/d2norm);
	glvect2norm.y = float(glvectABperp.y/d2norm);
	dDotProduct = (glvect1norm.x * glvect2norm.x) + (glvect1norm.y * glvect2norm.y);
	if (dDotProduct > 0) {
		return 0;
	}
	//Test QA vs. glvectACperp
	d1norm = sqrt(fabs((glvectQA.x*glvectQA.x) + (glvectQA.y*glvectQA.y)));
	d2norm = sqrt(fabs((glvectACperp.x*glvectACperp.x) + (glvectACperp.y*glvectACperp.y)));
	glvect1norm.x = float(glvectQA.x/d1norm);
	glvect1norm.y = float(glvectQA.y/d1norm);
	glvect2norm.x = float(glvectACperp.x/d2norm);
	glvect2norm.y = float(glvectACperp.y/d2norm);
	dDotProduct = (glvect1norm.x * glvect2norm.x) + (glvect1norm.y * glvect2norm.y);
	if (dDotProduct > 0) {
		return 0;
	}
	//Test QB vs. glvectABperp
	d1norm = sqrt(fabs((glvectQB.x*glvectQB.x) + (glvectQB.y*glvectQB.y)));
	d2norm = sqrt(fabs((glvectABperp.x*glvectABperp.x) + (glvectABperp.y*glvectABperp.y)));
	glvect1norm.x = float(glvectQB.x/d1norm);
	glvect1norm.y = float(glvectQB.y/d1norm);
	glvect2norm.x = float(glvectABperp.x/d2norm);
	glvect2norm.y = float(glvectABperp.y/d2norm);
	dDotProduct = (glvect1norm.x * glvect2norm.x) + (glvect1norm.y * glvect2norm.y);
	if (dDotProduct > 0) {
		return 0;
	}
	//Test QB vs. glvectBCperp
	d1norm = sqrt(fabs((glvectQB.x*glvectQB.x) + (glvectQB.y*glvectQB.y)));
	d2norm = sqrt(fabs((glvectBCperp.x*glvectBCperp.x) + (glvectBCperp.y*glvectBCperp.y)));
	glvect1norm.x = float(glvectQB.x/d1norm);
	glvect1norm.y = float(glvectQB.y/d1norm);
	glvect2norm.x = float(glvectBCperp.x/d2norm);
	glvect2norm.y = float(glvectBCperp.y/d2norm);
	dDotProduct = (glvect1norm.x * glvect2norm.x) + (glvect1norm.y * glvect2norm.y);
	if (dDotProduct > 0) {
		return 0;
	}
	//Test QC vs. glvectBCperp
	d1norm = sqrt(fabs((glvectQC.x*glvectQC.x) + (glvectQC.y*glvectQC.y)));
	d2norm = sqrt(fabs((glvectBCperp.x*glvectBCperp.x) + (glvectBCperp.y*glvectBCperp.y)));
	glvect1norm.x = float(glvectQC.x/d1norm);
	glvect1norm.y = float(glvectQC.y/d1norm);
	glvect2norm.x = float(glvectBCperp.x/d2norm);
	glvect2norm.y = float(glvectBCperp.y/d2norm);
	dDotProduct = (glvect1norm.x * glvect2norm.x) + (glvect1norm.y * glvect2norm.y);
	if (dDotProduct > 0) {
		return 0;
	}
	//Test QC vs. glvectACperp
	d1norm = sqrt(fabs((glvectQC.x*glvectQC.x) + (glvectQC.y*glvectQC.y)));
	d2norm = sqrt(fabs((glvectACperp.x*glvectACperp.x) + (glvectACperp.y*glvectACperp.y)));
	glvect1norm.x = float(glvectQC.x/d1norm);
	glvect1norm.y = float(glvectQC.y/d1norm);
	glvect2norm.x = float(glvectACperp.x/d2norm);
	glvect2norm.y = float(glvectACperp.y/d2norm);
	dDotProduct = (glvect1norm.x * glvect2norm.x) + (glvect1norm.y * glvect2norm.y);
	if (dDotProduct > 0) {
		return 0;
	}

	//The point is inside the triangle
	return 1;
}

bool NotFixedTriangles()
{
	//If a triangle is inside another triangle, delete the outer triangle
	for (int i = 0; i < (nNumTriangles-1); i++) {
		for (int j = (i+1); j < nNumTriangles; j++) {
			if (SharedEdge(glpntTriangles[i], glpntTriangles[j])) {
				//Test if point1 of triangle 1 is inside triangle 2
				if (IsInside(glpntTriangles[i].glpnt1, glpntTriangles[j])) {
					//Delete triangle 2
					for (int x = j; x < (nNumTriangles-1); x++) {
						glpntTriangles[x].glpnt1.x = glpntTriangles[x+1].glpnt1.x;
						glpntTriangles[x].glpnt1.y = glpntTriangles[x+1].glpnt1.y;
						glpntTriangles[x].glpnt2.x = glpntTriangles[x+1].glpnt2.x;
						glpntTriangles[x].glpnt2.y = glpntTriangles[x+1].glpnt2.y;
						glpntTriangles[x].glpnt3.x = glpntTriangles[x+1].glpnt3.x;
						glpntTriangles[x].glpnt3.y = glpntTriangles[x+1].glpnt3.y;
					}
					nNumTriangles--;
					return(1);
				//Test if point2 of triangle 1 is inside triangle 2
				} else if (IsInside(glpntTriangles[i].glpnt2, glpntTriangles[j])) {
					//Delete triangle 2
					for (int x = j; x < (nNumTriangles-1); x++) {
						glpntTriangles[x].glpnt1.x = glpntTriangles[x+1].glpnt1.x;
						glpntTriangles[x].glpnt1.y = glpntTriangles[x+1].glpnt1.y;
						glpntTriangles[x].glpnt2.x = glpntTriangles[x+1].glpnt2.x;
						glpntTriangles[x].glpnt2.y = glpntTriangles[x+1].glpnt2.y;
						glpntTriangles[x].glpnt3.x = glpntTriangles[x+1].glpnt3.x;
						glpntTriangles[x].glpnt3.y = glpntTriangles[x+1].glpnt3.y;
					}
					nNumTriangles--;
					return(1);
				//Test if point3 of triangle 1 is inside triangle 2
				} else if (IsInside(glpntTriangles[i].glpnt3, glpntTriangles[j])) {
					//Delete triangle 2
					for (int x = j; x < (nNumTriangles-1); x++) {
						glpntTriangles[x].glpnt1.x = glpntTriangles[x+1].glpnt1.x;
						glpntTriangles[x].glpnt1.y = glpntTriangles[x+1].glpnt1.y;
						glpntTriangles[x].glpnt2.x = glpntTriangles[x+1].glpnt2.x;
						glpntTriangles[x].glpnt2.y = glpntTriangles[x+1].glpnt2.y;
						glpntTriangles[x].glpnt3.x = glpntTriangles[x+1].glpnt3.x;
						glpntTriangles[x].glpnt3.y = glpntTriangles[x+1].glpnt3.y;
					}
					nNumTriangles--;
					return(1);
				//Test if point1 of triangle 2 is inside triangle 1
				} else if (IsInside(glpntTriangles[j].glpnt1, glpntTriangles[i])) {
					//Delete triangle 1
					for (int x = i; x < (nNumTriangles-1); x++) {
						glpntTriangles[x].glpnt1.x = glpntTriangles[x+1].glpnt1.x;
						glpntTriangles[x].glpnt1.y = glpntTriangles[x+1].glpnt1.y;
						glpntTriangles[x].glpnt2.x = glpntTriangles[x+1].glpnt2.x;
						glpntTriangles[x].glpnt2.y = glpntTriangles[x+1].glpnt2.y;
						glpntTriangles[x].glpnt3.x = glpntTriangles[x+1].glpnt3.x;
						glpntTriangles[x].glpnt3.y = glpntTriangles[x+1].glpnt3.y;
					}
					nNumTriangles--;
					return(1);
				//Test if point2 of triangle 2 is inside triangle 1
				} else if (IsInside(glpntTriangles[j].glpnt2, glpntTriangles[i])) {
					//Delete triangle 1
					for (int x = i; x < (nNumTriangles-1); x++) {
						glpntTriangles[x].glpnt1.x = glpntTriangles[x+1].glpnt1.x;
						glpntTriangles[x].glpnt1.y = glpntTriangles[x+1].glpnt1.y;
						glpntTriangles[x].glpnt2.x = glpntTriangles[x+1].glpnt2.x;
						glpntTriangles[x].glpnt2.y = glpntTriangles[x+1].glpnt2.y;
						glpntTriangles[x].glpnt3.x = glpntTriangles[x+1].glpnt3.x;
						glpntTriangles[x].glpnt3.y = glpntTriangles[x+1].glpnt3.y;
					}
					nNumTriangles--;
					return(1);
				//Test if point3 of triangle 2 is inside triangle 1
				} else if (IsInside(glpntTriangles[j].glpnt3, glpntTriangles[i])) {
					//Delete triangle 1
					for (int x = i; x < (nNumTriangles-1); x++) {
						glpntTriangles[x].glpnt1.x = glpntTriangles[x+1].glpnt1.x;
						glpntTriangles[x].glpnt1.y = glpntTriangles[x+1].glpnt1.y;
						glpntTriangles[x].glpnt2.x = glpntTriangles[x+1].glpnt2.x;
						glpntTriangles[x].glpnt2.y = glpntTriangles[x+1].glpnt2.y;
						glpntTriangles[x].glpnt3.x = glpntTriangles[x+1].glpnt3.x;
						glpntTriangles[x].glpnt3.y = glpntTriangles[x+1].glpnt3.y;
					}
					nNumTriangles--;
					return(1);
				}
			}
		}
	}
	return(0);
}

void FixTriangles() 
{
	bool fNotFixedYet = true;

	//Loop until the cleanup routine says we're done
	while (fNotFixedYet) {
		fNotFixedYet = NotFixedTriangles();
	}
}

//See if the area is convex
bool IsConvex(GLfloatPoint glpntA, GLfloatPoint glpntB, GLfloatPoint glpntC, GLfloatPoint glpntD)
{
	if (LinesIntersect(glpntA, glpntB, glpntC, glpntD)) {
		return(1);
	} else {
		return(0);
	}
}

//See if we should swap
bool ShouldSwap(GLfloatPoint glpntA, GLfloatPoint glpntB, GLfloatPoint glpntC, GLfloatPoint glpntD, int i, int j)
{
	//Don't even think of swapping if the shape is not convex
	if (!(IsConvex(glpntA, glpntC, glpntB, glpntD))) {
		return(0);
	}

	//Create some temp variables
	float fSmallest1 = 1;
	float fSmallest2 = 1;
	float fTester;
	float fNorm1;
	float fNorm2;
	int nCounter = 0;

	//Create some storage for the vectors
	GLfloatVect glvect1;
	GLfloatVect glvect2;

	//We are comparing (ABC,ACD) to (ABD,BCD)
	//Look at ABC
	glvect1.x = glpntA.x - glpntB.x;
	glvect1.y = glpntA.y - glpntB.y;
	glvect2.x = glpntC.x - glpntB.x;
	glvect2.y = glpntC.y - glpntB.y;
	fNorm1 = float(sqrt((glvect1.x*glvect1.x)+(glvect1.y*glvect1.y)));
	fNorm2 = float(sqrt((glvect2.x*glvect2.x)+(glvect2.y*glvect2.y)));
	glvect1.x = glvect1.x/fNorm1;
	glvect1.y = glvect1.y/fNorm1;
	glvect2.x = glvect2.x/fNorm2;
	glvect2.y = glvect2.y/fNorm2;
	fTester = float(fabs( ( (float(glvect1.x))*(float(glvect2.x)) ) + ( (float(glvect1.y))*(float(glvect2.y)) ) ) );
	if (fTester < fSmallest1) {
		fSmallest1 = fTester;
	}
	//Look at BCA
	glvect1.x = glpntB.x - glpntC.x;
	glvect1.y = glpntB.y - glpntC.y;
	glvect2.x = glpntA.x - glpntC.x;
	glvect2.y = glpntA.y - glpntC.y;
	fNorm1 = float(sqrt((glvect1.x*glvect1.x)+(glvect1.y*glvect1.y)));
	fNorm2 = float(sqrt((glvect2.x*glvect2.x)+(glvect2.y*glvect2.y)));
	glvect1.x = glvect1.x/fNorm1;
	glvect1.y = glvect1.y/fNorm1;
	glvect2.x = glvect2.x/fNorm2;
	glvect2.y = glvect2.y/fNorm2;
	fTester = float(fabs( ( (float(glvect1.x))*(float(glvect2.x)) ) + ( (float(glvect1.y))*(float(glvect2.y)) ) ) );
	if (fTester < fSmallest1) {
		fSmallest1 = fTester;
	}
	//Look at CAB
	glvect1.x = glpntC.x - glpntA.x;
	glvect1.y = glpntC.y - glpntA.y;
	glvect2.x = glpntB.x - glpntA.x;
	glvect2.y = glpntB.y - glpntA.y;
	fNorm1 = float(sqrt((glvect1.x*glvect1.x)+(glvect1.y*glvect1.y)));
	fNorm2 = float(sqrt((glvect2.x*glvect2.x)+(glvect2.y*glvect2.y)));
	glvect1.x = glvect1.x/fNorm1;
	glvect1.y = glvect1.y/fNorm1;
	glvect2.x = glvect2.x/fNorm2;
	glvect2.y = glvect2.y/fNorm2;
	fTester = float(fabs( ( (float(glvect1.x))*(float(glvect2.x)) ) + ( (float(glvect1.y))*(float(glvect2.y)) ) ) );
	if (fTester < fSmallest1) {
		fSmallest1 = fTester;
	}
	//Look at ACD
	glvect1.x = glpntA.x - glpntC.x;
	glvect1.y = glpntA.y - glpntC.y;
	glvect2.x = glpntD.x - glpntC.x;
	glvect2.y = glpntD.y - glpntC.y;
	fNorm1 = float(sqrt((glvect1.x*glvect1.x)+(glvect1.y*glvect1.y)));
	fNorm2 = float(sqrt((glvect2.x*glvect2.x)+(glvect2.y*glvect2.y)));
	glvect1.x = glvect1.x/fNorm1;
	glvect1.y = glvect1.y/fNorm1;
	glvect2.x = glvect2.x/fNorm2;
	glvect2.y = glvect2.y/fNorm2;
	fTester = float(fabs( ( (float(glvect1.x))*(float(glvect2.x)) ) + ( (float(glvect1.y))*(float(glvect2.y)) ) ) );
	if (fTester < fSmallest1) {
		fSmallest1 = fTester;
	}
	//Look at CDA
	glvect1.x = glpntC.x - glpntD.x;
	glvect1.y = glpntC.y - glpntD.y;
	glvect2.x = glpntA.x - glpntD.x;
	glvect2.y = glpntA.y - glpntD.y;
	fNorm1 = float(sqrt((glvect1.x*glvect1.x)+(glvect1.y*glvect1.y)));
	fNorm2 = float(sqrt((glvect2.x*glvect2.x)+(glvect2.y*glvect2.y)));
	glvect1.x = glvect1.x/fNorm1;
	glvect1.y = glvect1.y/fNorm1;
	glvect2.x = glvect2.x/fNorm2;
	glvect2.y = glvect2.y/fNorm2;
	fTester = float(fabs( ( (float(glvect1.x))*(float(glvect2.x)) ) + ( (float(glvect1.y))*(float(glvect2.y)) ) ) );
	if (fTester < fSmallest1) {
		fSmallest1 = fTester;
	}
	//Look at DAC
	glvect1.x = glpntD.x - glpntA.x;
	glvect1.y = glpntD.y - glpntA.y;
	glvect2.x = glpntC.x - glpntA.x;
	glvect2.y = glpntC.y - glpntA.y;
	fNorm1 = float(sqrt((glvect1.x*glvect1.x)+(glvect1.y*glvect1.y)));
	fNorm2 = float(sqrt((glvect2.x*glvect2.x)+(glvect2.y*glvect2.y)));
	glvect1.x = glvect1.x/fNorm1;
	glvect1.y = glvect1.y/fNorm1;
	glvect2.x = glvect2.x/fNorm2;
	glvect2.y = glvect2.y/fNorm2;
	fTester = float(fabs( ( (float(glvect1.x))*(float(glvect2.x)) ) + ( (float(glvect1.y))*(float(glvect2.y)) ) ) );
	if (fTester < fSmallest1) {
		fSmallest1 = fTester;
	}

	//Now compare the next four triangles
	//Look at ABD
	glvect1.x = glpntA.x - glpntB.x;
	glvect1.y = glpntA.y - glpntB.y;
	glvect2.x = glpntD.x - glpntB.x;
	glvect2.y = glpntD.y - glpntB.y;
	fNorm1 = float(sqrt((glvect1.x*glvect1.x)+(glvect1.y*glvect1.y)));
	fNorm2 = float(sqrt((glvect2.x*glvect2.x)+(glvect2.y*glvect2.y)));
	glvect1.x = glvect1.x/fNorm1;
	glvect1.y = glvect1.y/fNorm1;
	glvect2.x = glvect2.x/fNorm2;
	glvect2.y = glvect2.y/fNorm2;
	fTester = float(fabs( ( (float(glvect1.x))*(float(glvect2.x)) ) + ( (float(glvect1.y))*(float(glvect2.y)) ) ) );
	if (fTester < fSmallest2) {
		fSmallest2 = fTester;
	}
	//Look at BDA
	glvect1.x = glpntB.x - glpntD.x;
	glvect1.y = glpntB.y - glpntD.y;
	glvect2.x = glpntA.x - glpntD.x;
	glvect2.y = glpntA.y - glpntD.y;
	fNorm1 = float(sqrt((glvect1.x*glvect1.x)+(glvect1.y*glvect1.y)));
	fNorm2 = float(sqrt((glvect2.x*glvect2.x)+(glvect2.y*glvect2.y)));
	glvect1.x = glvect1.x/fNorm1;
	glvect1.y = glvect1.y/fNorm1;
	glvect2.x = glvect2.x/fNorm2;
	glvect2.y = glvect2.y/fNorm2;
	fTester = float(fabs( ( (float(glvect1.x))*(float(glvect2.x)) ) + ( (float(glvect1.y))*(float(glvect2.y)) ) ) );
	if (fTester < fSmallest2) {
		fSmallest2 = fTester;
	}
	//Look at DAB
	glvect1.x = glpntD.x - glpntA.x;
	glvect1.y = glpntD.y - glpntA.y;
	glvect2.x = glpntB.x - glpntA.x;
	glvect2.y = glpntB.y - glpntA.y;
	fNorm1 = float(sqrt((glvect1.x*glvect1.x)+(glvect1.y*glvect1.y)));
	fNorm2 = float(sqrt((glvect2.x*glvect2.x)+(glvect2.y*glvect2.y)));
	glvect1.x = glvect1.x/fNorm1;
	glvect1.y = glvect1.y/fNorm1;
	glvect2.x = glvect2.x/fNorm2;
	glvect2.y = glvect2.y/fNorm2;
	fTester = float(fabs( ( (float(glvect1.x))*(float(glvect2.x)) ) + ( (float(glvect1.y))*(float(glvect2.y)) ) ) );
	if (fTester < fSmallest2) {
		fSmallest2 = fTester;
	}
	//Look at BCD
	glvect1.x = glpntB.x - glpntC.x;
	glvect1.y = glpntB.y - glpntC.y;
	glvect2.x = glpntD.x - glpntC.x;
	glvect2.y = glpntD.y - glpntC.y;
	fNorm1 = float(sqrt((glvect1.x*glvect1.x)+(glvect1.y*glvect1.y)));
	fNorm2 = float(sqrt((glvect2.x*glvect2.x)+(glvect2.y*glvect2.y)));
	glvect1.x = glvect1.x/fNorm1;
	glvect1.y = glvect1.y/fNorm1;
	glvect2.x = glvect2.x/fNorm2;
	glvect2.y = glvect2.y/fNorm2;
	fTester = float(fabs( ( (float(glvect1.x))*(float(glvect2.x)) ) + ( (float(glvect1.y))*(float(glvect2.y)) ) ) );
	if (fTester < fSmallest2) {
		fSmallest2 = fTester;
	}
	//Look at CDB
	glvect1.x = glpntC.x - glpntD.x;
	glvect1.y = glpntC.y - glpntD.y;
	glvect2.x = glpntB.x - glpntD.x;
	glvect2.y = glpntB.y - glpntD.y;
	fNorm1 = float(sqrt((glvect1.x*glvect1.x)+(glvect1.y*glvect1.y)));
	fNorm2 = float(sqrt((glvect2.x*glvect2.x)+(glvect2.y*glvect2.y)));
	glvect1.x = glvect1.x/fNorm1;
	glvect1.y = glvect1.y/fNorm1;
	glvect2.x = glvect2.x/fNorm2;
	glvect2.y = glvect2.y/fNorm2;
	fTester = float(fabs( ( (float(glvect1.x))*(float(glvect2.x)) ) + ( (float(glvect1.y))*(float(glvect2.y)) ) ) );
	if (fTester < fSmallest2) {
		fSmallest2 = fTester;
	}
	//Look at DBC
	glvect1.x = glpntD.x - glpntB.x;
	glvect1.y = glpntD.y - glpntB.y;
	glvect2.x = glpntC.x - glpntB.x;
	glvect2.y = glpntC.y - glpntB.y;
	fNorm1 = float(sqrt((glvect1.x*glvect1.x)+(glvect1.y*glvect1.y)));
	fNorm2 = float(sqrt((glvect2.x*glvect2.x)+(glvect2.y*glvect2.y)));
	glvect1.x = glvect1.x/fNorm1;
	glvect1.y = glvect1.y/fNorm1;
	glvect2.x = glvect2.x/fNorm2;
	glvect2.y = glvect2.y/fNorm2;
	fTester = float(fabs( ( (float(glvect1.x))*(float(glvect2.x)) ) + ( (float(glvect1.y))*(float(glvect2.y)) ) ) );
	if (fTester < fSmallest2) {
		fSmallest2 = fTester;
	}

	//See if we should swap
	if (fSmallest1 < fSmallest2) {
		//Everything's ok, just return
		return (0);
	}

	//Do the swap by deleting the first edge and adding the second
	GLfloatPoint glpnt1;
	GLfloatPoint glpnt2;
	GLfloatPoint glpnt3;
	GLfloatPoint glpnt4;
	if (glpntA.x == glpntC.x) {
		if (glpntA.y < glpntC.y) {
			glpnt1.x = glpntA.x;
			glpnt1.y = glpntA.y;
			glpnt2.x = glpntC.x;
			glpnt2.y = glpntC.y;
		} else {
			glpnt1.x = glpntC.x;
			glpnt1.y = glpntC.y;
			glpnt2.x = glpntA.x;
			glpnt2.y = glpntA.y;
		}
	} else if (glpntA.x < glpntC.x) {
		glpnt1.x = glpntA.x;
		glpnt1.y = glpntA.y;
		glpnt2.x = glpntC.x;
		glpnt2.y = glpntC.y;
	} else {
		glpnt1.x = glpntC.x;
		glpnt1.y = glpntC.y;
		glpnt2.x = glpntA.x;
		glpnt2.y = glpntA.y;
	}
	if (glpntB.x == glpntD.x) {
		if (glpntB.y < glpntD.y) {
			glpnt3.x = glpntB.x;
			glpnt3.y = glpntB.y;
			glpnt4.x = glpntD.x;
			glpnt4.y = glpntD.y;
		} else {
			glpnt3.x = glpntD.x;
			glpnt3.y = glpntD.y;
			glpnt4.x = glpntB.x;
			glpnt4.y = glpntB.y;
		}
	} else if (glpntB.x < glpntD.x) {
		glpnt3.x = glpntB.x;
		glpnt3.y = glpntB.y;
		glpnt4.x = glpntD.x;
		glpnt4.y = glpntD.y;
	} else {
		glpnt3.x = glpntD.x;
		glpnt3.y = glpntD.y;
		glpnt4.x = glpntB.x;
		glpnt4.y = glpntB.y;
	}

	//Find the edge
	nCounter = 0;
	for (int x = 0; x < nNumEdges; x++) {
		//Copy the edge
		if (((glpntEdges[x].glpnt1.x == glpnt1.x) && (glpntEdges[x].glpnt1.y == glpnt1.y)) && ((glpntEdges[x].glpnt2.x == glpnt2.x) && (glpntEdges[x].glpnt2.y == glpnt2.y))) {
		} else {
			glpntEdges[nCounter].glpnt1.x = glpntEdges[x].glpnt1.x;
			glpntEdges[nCounter].glpnt1.y = glpntEdges[x].glpnt1.y;
			glpntEdges[nCounter].glpnt2.x = glpntEdges[x].glpnt2.x;
			glpntEdges[nCounter].glpnt2.y = glpntEdges[x].glpnt2.y;
			nCounter++;
		}
	}
	nNumEdges--;
	//Add the new edge
	AddEdge(glpnt3, glpnt4);

	//Delete both i and j triangles
	nCounter = 0;
	for (int y = 0; y < nNumTriangles; y++) {
		if ((nCounter == i) || (nCounter == j)) {
		} else {
			//Copy the triangle
			glpntTriangles[nCounter].glpnt1.x = glpntTriangles[y].glpnt1.x;
			glpntTriangles[nCounter].glpnt1.y = glpntTriangles[y].glpnt1.y;
			glpntTriangles[nCounter].glpnt2.x = glpntTriangles[y].glpnt2.x;
			glpntTriangles[nCounter].glpnt2.y = glpntTriangles[y].glpnt2.y;
			glpntTriangles[nCounter].glpnt3.x = glpntTriangles[y].glpnt3.x;
			glpntTriangles[nCounter].glpnt3.y = glpntTriangles[y].glpnt3.y;
			nCounter++;
		}
	}
	nNumTriangles--;
	nNumTriangles--;
	//Add the two new triangles
	AddTriangle(glpntD, glpntA, glpntB);
	AddTriangle(glpntB, glpntC, glpntD);

	//We did a swap
	return(1);
}

bool NotFixedMesh()
{
	//Set a flag to see if a swap took place
	bool fDidntSwap = true;

	//Find a neighbor triangle and see if we should swap
	for (int i = 0; i < (nNumTriangles-1); i++) {
		for (int j = (i+1); j < nNumTriangles; j++) {

			//Create some temp triangles		
			GLfloatTriangle gltriangle1 = glpntTriangles[i];
			GLfloatTriangle gltriangle2 = glpntTriangles[j];

			//See if ab matches de
			if ((gltriangle1.glpnt1.x == gltriangle2.glpnt1.x) && (gltriangle1.glpnt1.y == gltriangle2.glpnt1.y)
				&& (gltriangle1.glpnt2.x == gltriangle2.glpnt2.x) && (gltriangle1.glpnt2.y == gltriangle2.glpnt2.y)) {
				//Error?
			}
			//See if ab matches ef
			if ((gltriangle1.glpnt1.x == gltriangle2.glpnt2.x) && (gltriangle1.glpnt1.y == gltriangle2.glpnt2.y)
				&& (gltriangle1.glpnt2.x == gltriangle2.glpnt3.x) && (gltriangle1.glpnt2.y == gltriangle2.glpnt3.y)) {
				if (ShouldSwap(gltriangle1.glpnt1, gltriangle2.glpnt1, gltriangle1.glpnt2, gltriangle1.glpnt3, i, j)) {
					return(1);
				}
			}
			//See if ab matches df
			if ((gltriangle1.glpnt1.x == gltriangle2.glpnt1.x) && (gltriangle1.glpnt1.y == gltriangle2.glpnt1.y)
				&& (gltriangle1.glpnt2.x == gltriangle2.glpnt3.x) && (gltriangle1.glpnt2.y == gltriangle2.glpnt3.y)) {
				if (ShouldSwap(gltriangle1.glpnt1, gltriangle2.glpnt2, gltriangle1.glpnt2, gltriangle1.glpnt3, i, j)) {
					return(1);
				}
			}
			//See if bc matches de
			if ((gltriangle1.glpnt2.x == gltriangle2.glpnt1.x) && (gltriangle1.glpnt2.y == gltriangle2.glpnt1.y)
				&& (gltriangle1.glpnt3.x == gltriangle2.glpnt2.x) && (gltriangle1.glpnt3.y == gltriangle2.glpnt2.y)) {
				if (ShouldSwap(gltriangle1.glpnt2, gltriangle2.glpnt3, gltriangle1.glpnt3, gltriangle1.glpnt1, i, j)) {
					return(1);
				}
			}
			//See if bc matches ef
			if ((gltriangle1.glpnt2.x == gltriangle2.glpnt2.x) && (gltriangle1.glpnt2.y == gltriangle2.glpnt2.y)
				&& (gltriangle1.glpnt3.x == gltriangle2.glpnt3.x) && (gltriangle1.glpnt3.y == gltriangle2.glpnt3.y)) {
				//Error?
			}
			//See if bc matches df
			if ((gltriangle1.glpnt2.x == gltriangle2.glpnt1.x) && (gltriangle1.glpnt2.y == gltriangle2.glpnt1.y)
				&& (gltriangle1.glpnt3.x == gltriangle2.glpnt3.x) && (gltriangle1.glpnt3.y == gltriangle2.glpnt3.y)) {
				if (ShouldSwap(gltriangle1.glpnt2, gltriangle2.glpnt2, gltriangle1.glpnt3, gltriangle1.glpnt1, i, j)) {
					return(1);
				}
			}
			//See if ac matches de
			if ((gltriangle1.glpnt1.x == gltriangle2.glpnt1.x) && (gltriangle1.glpnt1.y == gltriangle2.glpnt1.y)
				&& (gltriangle1.glpnt3.x == gltriangle2.glpnt2.x) && (gltriangle1.glpnt3.y == gltriangle2.glpnt2.y)) {
				if (ShouldSwap(gltriangle1.glpnt1, gltriangle2.glpnt3, gltriangle1.glpnt3, gltriangle1.glpnt2, i, j)) {
					return(1);
				}
			}
			//See if ac matches ef
			if ((gltriangle1.glpnt1.x == gltriangle2.glpnt2.x) && (gltriangle1.glpnt1.y == gltriangle2.glpnt2.y)
				&& (gltriangle1.glpnt3.x == gltriangle2.glpnt3.x) && (gltriangle1.glpnt3.y == gltriangle2.glpnt3.y)) {
				if (ShouldSwap(gltriangle1.glpnt1, gltriangle2.glpnt1, gltriangle1.glpnt3, gltriangle1.glpnt2, i, j)) {
					return(1);
				}
			}
			//See if ac matches df
			if ((gltriangle1.glpnt1.x == gltriangle2.glpnt1.x) && (gltriangle1.glpnt1.y == gltriangle2.glpnt1.y)
				&& (gltriangle1.glpnt3.x == gltriangle2.glpnt3.x) && (gltriangle1.glpnt3.y == gltriangle2.glpnt3.y)) {
				if (ShouldSwap(gltriangle1.glpnt1, gltriangle1.glpnt2, gltriangle1.glpnt3, gltriangle2.glpnt2, i, j)) {
					return(1);
				}
			}
		}
	}
	return(0);
}


void FixMesh() 
{
	bool fNotFixedYet = true;

	//Loop until the cleanup routine says we're done
	while (fNotFixedYet) {
		fNotFixedYet = NotFixedMesh();
	}
}

void RebuildTriangles()
{
	//Clear the counter
	nNumTriangles = 0;

	//Set some variables
	GLfloatPoint glpnt1;
	GLfloatPoint glpnt2;
	GLfloatPoint glpnt3;
	bool fNotFound = true;

	//Loop through all the edges
	for (int i = 0; i < (nNumEdges - 2); i++) {

		//Try to find a matching edge
		for (int j = (i+1); j < (nNumEdges - 1); j++) {

			fNotFound = true;
			//See if the new edge (cd) forms a(bc)d a/bc\d
			if ((glpntEdges[i].glpnt2.x==glpntEdges[j].glpnt1.x)&&(glpntEdges[i].glpnt2.y==glpntEdges[j].glpnt1.y)) {

				//Set the new point
				glpnt1.x = glpntEdges[i].glpnt1.x;
				glpnt1.y = glpntEdges[i].glpnt1.y;
				glpnt2.x = glpntEdges[i].glpnt2.x;
				glpnt2.y = glpntEdges[i].glpnt2.y;
				glpnt3.x = glpntEdges[j].glpnt2.x;
				glpnt3.y = glpntEdges[j].glpnt2.y;
				fNotFound = false;

			//See if the new edge (cd) forms c(da)b c\da/b
			} else if ((glpntEdges[i].glpnt1.x==glpntEdges[j].glpnt2.x)&&(glpntEdges[i].glpnt1.y==glpntEdges[j].glpnt2.y)) {

				//Set the new point
				glpnt1.x = glpntEdges[j].glpnt1.x;
				glpnt1.y = glpntEdges[j].glpnt1.y;
				glpnt2.x = glpntEdges[j].glpnt2.x;
				glpnt2.y = glpntEdges[j].glpnt2.y;
				glpnt3.x = glpntEdges[i].glpnt2.x;
				glpnt3.y = glpntEdges[i].glpnt2.y;
				fNotFound = false;

			//See if the new edge (cd) forms (ac)/bd or (ac)/db
			} else if ((glpntEdges[i].glpnt1.x==glpntEdges[j].glpnt1.x)&&(glpntEdges[i].glpnt1.y==glpntEdges[j].glpnt1.y)) {

				//Set the new point
				glpnt1.x = glpntEdges[i].glpnt2.x;
				glpnt1.y = glpntEdges[i].glpnt2.y;
				glpnt2.x = glpntEdges[i].glpnt1.x;
				glpnt2.y = glpntEdges[i].glpnt1.y;
				glpnt3.x = glpntEdges[j].glpnt2.x;
				glpnt3.y = glpntEdges[j].glpnt2.y;
				fNotFound = false;

			//See if the new edge (cd) forms ac\(bd) or ca/(bd)
			} else if ((glpntEdges[i].glpnt2.x==glpntEdges[j].glpnt2.x)&&(glpntEdges[i].glpnt2.y==glpntEdges[j].glpnt2.y)) {

				//Set the new point
				glpnt1.x = glpntEdges[i].glpnt1.x;
				glpnt1.y = glpntEdges[i].glpnt1.y;
				glpnt2.x = glpntEdges[i].glpnt2.x;
				glpnt2.y = glpntEdges[i].glpnt2.y;
				glpnt3.x = glpntEdges[j].glpnt1.x;
				glpnt3.y = glpntEdges[j].glpnt1.y;
				fNotFound = false;

			}
			
			//See if we've found a triad
			if (!fNotFound) {
				//Loop through the remaining edges trying to find a completer
				for (int k = (j+1); k < nNumEdges; k++) {
					//See if this edge is the one
					if ((glpnt1.x==glpntEdges[k].glpnt1.x)&&(glpnt1.y==glpntEdges[k].glpnt1.y)&&(glpnt3.x==glpntEdges[k].glpnt2.x)&&(glpnt3.y==glpntEdges[k].glpnt2.y)) {
						//Add the triangle
						AddTriangle(glpnt1, glpnt2, glpnt3);
						break;
					} else if ((glpnt1.x==glpntEdges[k].glpnt2.x)&&(glpnt1.y==glpntEdges[k].glpnt2.y)&&(glpnt3.x==glpntEdges[k].glpnt1.x)&&(glpnt3.y==glpntEdges[k].glpnt1.y)) {
						//Add the triangle
						AddTriangle(glpnt1, glpnt2, glpnt3);
						break;
					}
				}
			}
		}
	}
}

void AddPoint(GLfloatPoint l_pointAdd)
{
	//Make sure we have space avaiable
	if (nNumPoints < MAX_POINTS) {
		//Add the point to the array
		glpntVertices[nNumPoints].x = l_pointAdd.x;
		glpntVertices[nNumPoints].y = l_pointAdd.y;
		//Increment the counter
		nNumPoints++;
		//Draw the new point
		DrawDot(l_pointAdd);

		//If there are only two points, connect them
		if (nNumPoints == 2) {
			//Add the edge to the list
			AddEdge(glpntVertices[0], l_pointAdd);
		} else {
			//See if we should add this point to the edge list
			for (int i = 0; i < (nNumPoints-1); i++) {
				bool fClear = true;
				for (int j = 0; ((j < nNumEdges) && (fClear)); j++) {
					if (LinesIntersect(l_pointAdd, glpntVertices[i], glpntEdges[j].glpnt1, glpntEdges[j].glpnt2)) {
						fClear = false;
					}
				}
				if (fClear) {
					//Add the edge to the list
					AddEdge(glpntVertices[i], l_pointAdd);
				}
			}
		}

		//Recalculate the number of trangles
		RebuildTriangles();

		//Fix the number of triangles
		FixTriangles();

		//Fix the mesh
		FixMesh();

		//Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		//Redraw the scene
		DrawGLScene();

		//Make sure it gets drawn
		glFlush();
		SwapBuffers(g_dcClient);
	}
}

GLfloatPoint tricircumcenter(GLfloatPoint a, GLfloatPoint b, GLfloatPoint c) 
{
	//"Borrowed" code from http://www.ics.uci.edu/~eppstein/junkyard/circumcenter.html
	GLfloatPoint circumcenter;
	double xba, yba, xca, yca;
	double balength, calength;
	double denominator;
	double xcirca, ycirca;

	/* Use coordinates relative to point 'a' of the triangle. */
	xba = b.x - a.x;
	yba = b.y - a.y;
	xca = c.x - a.x;
	yca = c.y - a.y;
	/* Squares of lengths of the edges incident to 'a'. */
	balength = ((xba * xba) + (yba * yba));
	calength = ((xca * xca) + (yca * yca));

	/* Take your chances with floating-point roundoff. */
	denominator = 0.5 / ((xba * yca) - (yba * xca));

	/* Calculate offset (from 'a') of circumcenter. */
	xcirca = (((yca * balength) - (yba * calength)) * denominator);  
	ycirca = (((xba * calength) - (xca * balength)) * denominator);  
	//circumcenter.x = xcirca;
	//circumcenter.y = ycirca;
	circumcenter.x = float(xcirca + a.x);
	circumcenter.y = float(ycirca + a.y);
	return(circumcenter);
}

void FindCenters()
{

	//Temp variable
	GLfloatPoint glpntNew;

	//Loop through all the triangles
	for (int i = 0; i < (nNumTriangles); i++) {
		
		//Find the circumcenter
		glpntNew = tricircumcenter(glpntTriangles[i].glpnt1, glpntTriangles[i].glpnt2, glpntTriangles[i].glpnt3);

		//Save the center
		glpntCenters[i].x = glpntNew.x;
		glpntCenters[i].y = glpntNew.y;
	}
}

void DrawVoronoi()
{
	//Find the centers of every triangle
	FindCenters();

	//Change the color
	glColor3f(1.0f,0.0f,0.0f);

	//Plot them
	for (int i = 0; i < nNumTriangles; i++) {
		DrawDot(glpntCenters[i]);
	}
	//Draw the Voronoi
	for (int j = 0; j < (nNumTriangles-1); j++) {
		for (int k = (j+1); k < nNumTriangles; k++) {

			//See if these two triangles share a common border
			if (SharedEdge(glpntTriangles[j], glpntTriangles[k])) {
				//Draw and edge betwen their two centers
				DrawLine(glpntCenters[j], glpntCenters[k]);
			}
		}
	}

	//Change the color back
	glColor3f(0.0f,0.0f,0.0f);

	//Make sure it gets drawn
	glFlush();
	SwapBuffers(g_dcClient);

}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	//Local variables
	MSG msg;

	//Initialize the string for the title
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_VORONOI, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	//Initialize the app
	if(!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	//Force a WM_PAINT message
	InvalidateRect(g_hwndClient, NULL, FALSE);

	while(GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_VORONOI);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_VORONOI;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if(!hWnd)
    {
        return FALSE;
    }

	//Show the Window
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

	//Copy the information globally
    g_hwndClient = hWnd;
    g_dcClient  = GetDC(g_hwndClient);

	//Create Windows pixel information
    BOOL bSuccess;
    bSuccess = SetWindowPixelFormat(g_dcClient);
    bSuccess = CreateViewGLContext(g_dcClient);

	//Create a rect object and initialize it
    RECT rect;
    GetClientRect(g_hwndClient, &rect);
    int iWidth = rect.right - rect.left;
    int iHeight = rect.bottom - rect.top;
    InitGl(iWidth, iHeight);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			switch (wmId)
			{
				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;

		case WM_PAINT:
			DrawGLScene();
			break;

		case WM_LBUTTONDOWN:
			//Set the new point
			GLfloatPoint l_pointNew;
			RECT rect; 
			GetClientRect(hWnd, &rect);
			l_pointNew.x = float(LOWORD(lParam)); 
			l_pointNew.y = rect.bottom - rect.top - float(HIWORD(lParam)); 
			AddPoint(l_pointNew);
			break;

		case WM_RBUTTONDOWN:
			DrawVoronoi();
			break;

		case WM_MOUSEMOVE:
			break;

		case WM_DESTROY:
			//Make the rendering context not current
			if(wglGetCurrentContext()!=NULL) 
				wglMakeCurrent(NULL, NULL);

			//Delete the context
			if(g_hGLContext!=NULL)
			{
				wglDeleteContext(g_hGLContext);
				g_hGLContext = NULL;
			}
			PostQuitMessage(0);
			break;

		case WM_SIZE:
			GetClientRect(hWnd, &rect);
			ResizeScene(rect.right - rect.left,rect.bottom - rect.top);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

//Draw dot at float point {x, y}
void DrawDot(GLfloatPoint glpntIn)
{
	glBegin(GL_POINTS);
		glVertex2f(glpntIn.x, glpntIn.y);
	glEnd();
}

//Draw line from {x1,y1} to {x2,y2}
void DrawLine(GLfloatPoint glpnt1, GLfloatPoint glpnt2)
{
	glBegin(GL_LINES);
		glVertex2f(glpnt1.x, glpnt1.y);
		glVertex2f(glpnt2.x, glpnt2.y);
	glEnd();
}

//Return 1 if the lines intersect
bool LinesIntersect(GLfloatPoint glpnta, GLfloatPoint glpntb, GLfloatPoint glpntc, GLfloatPoint glpntd)
{
	//Set up some temp variables
	float tester;
	float t;
	float u;
	GLfloatVect glvectb;
	GLfloatVect glvectc;
	GLfloatVect glvectd;
	GLfloatPoint glpntdparallel;
	GLfloatPoint glpntbparallel;

	//Find all glvect vectors
	glvectb.x = (glpntb.x-glpnta.x);
	glvectb.y = (glpntb.y-glpnta.y);
	glvectc.x = (glpntc.x-glpnta.x);
	glvectc.y = (glpntc.y-glpnta.y);
	glvectd.x = (glpntd.x-glpntc.x);
	glvectd.y = (glpntd.y-glpntc.y);

	//Find dparallel and bparallel
	glpntdparallel.y = glvectd.x;
	glpntdparallel.x = (0-glvectd.y);
	glpntbparallel.y = glvectb.x;
	glpntbparallel.x = (0-glvectb.y);

	//Find dparallel dot b
	tester = (glpntdparallel.x*glvectb.x)+(glpntdparallel.y*glvectb.y);

	//See if the two parent lines are parallel
	if (tester == 0) {
		return(0);
	}

	//Make sure t is betwen 0 and 1
	t = (((glpntdparallel.x*glvectc.x)+(glpntdparallel.y*glvectc.y))/tester);
	u = (((glpntbparallel.x*glvectc.x)+(glpntbparallel.y*glvectc.y))/tester);
	if (((t > 0) && (t < 1)) && ((u > 0) && (u < 1))) {
		return (1);
	} else {
		return(0);
	}

	return(0);
}

