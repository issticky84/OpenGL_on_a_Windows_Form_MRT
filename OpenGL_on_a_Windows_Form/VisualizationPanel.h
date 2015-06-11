#pragma once

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "ReadCSV.h"
#include "RECTANGLE.h"
#include "mtxlib.h"
#include <sstream>
#include "Preprocessing_Data.h"


#define generic GenericFromFreeTypeLibrary
#include <FTGL/ftgl.h>
#undef generic

using namespace System::Windows::Forms;
#define FONT_FILE "arial.ttf"


namespace OpenGLForm{
	//extern  vector<float>  draw_color;
	extern	ReadCSV read_csv;
	extern Preprocessing_Data preprocessing_data;
	extern	vector<RECTANGLE*> recycle;
	extern char quote[24][80];
	extern char title[4][80];
	extern char five_minutes[12][80];
	extern vector<vector4> histogram_position_table;
	extern vector<int> histogram_index;
	extern vector< vector<vector4> > raw_data_position_table;
	extern vector<int> raw_data_index;
	extern vector < vector<int> > raw_data_index_2D;
	extern float scale_x[3],scale_y[3],scale_z[3];
	extern float scale_size[3],scale_factor[3];
	extern int last_X[3],last_Y[3];
	extern float move_x[3],move_y[3],move_z[3];
	extern int windowWidth[3],windowHeight[3];
	extern FTPixmapFont font;
	extern char* text;
	extern double ModelViewMatrix2[16];
	extern int select_histogram_index;
	extern bool select_histogram_flag;
	extern int counter;

	public ref class VisualizationPanel: public NativeWindow{
	public:
			VisualizationPanel(Form ^ parentForm,Panel ^ parentPanel, GLsizei iWidth, GLsizei iHeight,ReadCSV read_csv_ref,Preprocessing_Data preprocessing_data_ref);
			//virtual System::Void Render(System::Void){}
			virtual System::Void Render(int,int){}
			~VisualizationPanel(System::Void);
	protected:
			HDC m_hDC;
			HGLRC m_hglrc;
			GLfloat	rtri;				// Angle for the triangle
			GLfloat	rquad;				// Angle for the quad
			GLint MySetPixelFormat(HDC hdc);
			bool InitGL(GLvoid);
			GLvoid ReSizeGLScene(GLsizei width, GLsizei height);
			System::Void DrawRectWithOpenGL(RECTANGLE* pRect,vector<float>  draw_color);
			System::Void SwapOpenGLBuffers(System::Void);
			System::Void wglmakecur();
			double round(double);
			Form ^ parentForm;
	};

}