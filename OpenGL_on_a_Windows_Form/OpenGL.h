#pragma once

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "ReadCSV.h"
#include "RECTANGLE.h"

using namespace System::Windows::Forms;

vector<float> draw_color;

namespace OpenGLForm 
{
	ReadCSV read_csv;
	vector<RECTANGLE*> recycle;
	float scale_x=0.0,scale_y=0.0,scale_z=0.0;
	const float scale_size = 0.05;
	public ref class COpenGL: public System::Windows::Forms::NativeWindow
	{
	public:
		

		COpenGL(System::Windows::Forms::Form ^ parentForm,System::Windows::Forms::Panel ^ parentPanel, GLsizei iWidth, GLsizei iHeight,ReadCSV read_csv_ref)
		{
			CreateParams^ cp = gcnew CreateParams;
			
			recycle.reserve(0);
			read_csv = read_csv_ref;
			// Set the position on the form
			cp->X = 0;
			cp->Y = 0;
			cp->Height = iHeight;
			cp->Width = iWidth;

			// Specify the form as the parent.
			cp->Parent = parentForm->Handle;

			// Create as a child of the specified parent and make OpenGL compliant (no clipping)
			cp->Style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

			// Create the actual window
			this->CreateHandle(cp);
			parentForm->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &COpenGL::COGL_MouseClick);
			parentForm->MouseClick += gcnew System::Windows::Forms::MouseEventHandler( this, &COpenGL::panel1_MouseDown );
			parentForm->MouseWheel += gcnew System::Windows::Forms::MouseEventHandler( this, &COpenGL::COGL_Scroll );
			m_hDC = GetDC((HWND)parentPanel->Handle.ToPointer());


			if(m_hDC)
			{
				MySetPixelFormat(m_hDC);
				ReSizeGLScene(iWidth, iHeight);
				InitGL();
			}

			rtri = 0.0f;
			rquad = 0.0f;

		}
		void  panel1_MouseDown( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e ){
			System::Windows::Forms::MessageBox::Show("a");
		}
		void COGL_MouseClick( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e ){
			System::Windows::Forms::MessageBox::Show(e->Location.ToString());
	
		}

		void COGL_Scroll( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e ){
			//System::Windows::Forms::MessageBox::Show(e->Location.ToString());
			if (e->Delta < 0){
				scale_x+=scale_size;
				scale_y+=scale_size;
				scale_z+=scale_size;
			}else{
				scale_x-=scale_size;
				scale_y-=scale_size;
				scale_z-=scale_size;
			
			}
		}


		System::Void DrawRectWithOpenGL(RECTANGLE* pRect)
		{
			glPushMatrix();  //Make sure our transformations don't affect any other transformations in other code
			glTranslatef(pRect->x, pRect->y,0);  //Translate rectangle to its assigned x and y position
			//Put other transformations here
			
			//glBegin(GL_TRIANGLES);
			glBegin(GL_QUADS);   //We want to draw a quad, i.e. shape with four sides
				glColor3f(draw_color[0],draw_color[1],draw_color[2]); //Set the colour to red 
				glVertex2f(0, 0);   //Draw the four corners of the rectangle
				glVertex2f(0, pRect->h);
				glVertex2f(pRect->w, pRect->h);
				glVertex2f(pRect->w, 0);       
			glEnd();
			glPopMatrix();
		}

		System::Void DrawHistogramVisualization(System::Void)
		{
			draw_color.resize(3);
			if(recycle.size()!=0){
				for(int i=0;i<recycle.size();i++)
					delete recycle.at(i);
				recycle.clear();
			}
			int windowSize[2];
			windowSize[0] = 600; 
			windowSize[1] = 800;
			glClearColor(0.0, 0.0, 0.0, 0.0);  //Set the cleared screen colour to black
			glViewport(0, 0, windowSize[0], windowSize[1]);   //This sets up the viewport so that the coordinates (0, 0) are at the top left of the window		
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, windowSize[0], windowSize[1], 0, -10, 10);

			//Back to the modelview so we can draw stuff 
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the screen and depth buffer

			glTranslatef(0.0,0.0,0.0);
			glScalef(0.6+scale_x,0.6+scale_y,0.6+scale_z);

			RECTANGLE *rect;
			RECTANGLE *line;

			int y_coord = 1400;
			int pixels;
			int current_hour;
			int last_hour = -1;
			int t=0;
			for(int i=0;i<read_csv.position_data.size()-1;++i)
			{
				current_hour = read_csv.hour_data[i][1];
				if(current_hour!=last_hour)
				{
					draw_color[0] = 1; draw_color[1] = 1; draw_color[2] = 1;
					line = new RECTANGLE();
					line->h = 3;
					line->w = 750;
					line->x = 0;
					line->y = y_coord - 5;
					DrawRectWithOpenGL(line);
					//DrawText_FTGL(current_hour,20,y_coord-15);
					t++;
					y_coord-=10;
					recycle.push_back(line);	
				}
				for(int j=1;j<read_csv.position_data[i].size();j++)
				{		
					int start = 0;
					for(int k=0;k<read_csv.histogram_data[i].size()-1;++k)
					{   
						if(read_csv.histogram_data[i][k+1]!=0)
						{
							pixels = read_csv.histogram_data[i][k+1];
							//draw_color[0] = PCA_result(k,0); draw_color[1] = PCA_result(k,1); draw_color[2] = PCA_result(k,2);
							draw_color[0] = read_csv.color_data[k][1]; draw_color[1] = read_csv.color_data[k][2]; draw_color[2] = read_csv.color_data[k][3];
							for(int u=start;u<start+pixels;++u)
							{
								rect = new RECTANGLE();
								rect->h = 3;
								rect->w = 0.1;
								rect->x = read_csv.position_data[i][j]/10.0 + (float)u*0.1;
								rect->y = y_coord;
								DrawRectWithOpenGL(rect);	
								recycle.push_back(rect);
							}
							start += pixels;
						}		
					}

					y_coord-=5;
				}
				last_hour = current_hour;
			}


			//glutSwapBuffers(); // Swap front and back buffers
		}


		System::Void Render(System::Void)
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear screen and depth buffer
			glLoadIdentity();									// Reset the current modelview matrix
			glTranslatef(-1.5f,0.0f,-6.0f);						// Move left 1.5 units and into the screen 6.0
			glRotatef(rtri,0.0f,1.0f,0.0f);						// Rotate the triangle on the y axis 
			glBegin(GL_TRIANGLES);								// Start drawing a triangle
				glColor3f(1.0f,0.0f,0.0f);						// Red
				glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of triangle (front)
				glColor3f(0.0f,1.0f,0.0f);						// Green
				glVertex3f(-1.0f,-1.0f, 1.0f);					// Left of triangle (front)
				glColor3f(0.0f,0.0f,1.0f);						// Blue
				glVertex3f( 1.0f,-1.0f, 1.0f);					// Right of triangle (front)
				glColor3f(1.0f,0.0f,0.0f);						// Red
				glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of triangle (right)
				glColor3f(0.0f,0.0f,1.0f);						// Blue
				glVertex3f( 1.0f,-1.0f, 1.0f);					// Left of triangle (right)
				glColor3f(0.0f,1.0f,0.0f);						// Green
				glVertex3f( 1.0f,-1.0f, -1.0f);					// Right of triangle (right)
				glColor3f(1.0f,0.0f,0.0f);						// Red
				glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of triangle (back)
				glColor3f(0.0f,1.0f,0.0f);						// Green
				glVertex3f( 1.0f,-1.0f, -1.0f);					// Left of triangle (back)
				glColor3f(0.0f,0.0f,1.0f);						// Blue
				glVertex3f(-1.0f,-1.0f, -1.0f);					// Right of triangle (back)
				glColor3f(1.0f,0.0f,0.0f);						// Red
				glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of triangle (left)
				glColor3f(0.0f,0.0f,1.0f);						// Blue
				glVertex3f(-1.0f,-1.0f,-1.0f);					// Left of triangle (left)
				glColor3f(0.0f,1.0f,0.0f);						// Green
				glVertex3f(-1.0f,-1.0f, 1.0f);					// Right of triangle (left)
			glEnd();											// Done drawing the pyramid

			glLoadIdentity();									// Reset the current modelview matrix
			glTranslatef(1.5f,0.0f,-7.0f);						// Move right 1.5 units and into the screen 7.0
			glRotatef(rquad,1.0f,1.0f,1.0f);					// Rotate the quad on the x axis 
			glBegin(GL_QUADS);									// Draw a quad
				glColor3f(0.0f,1.0f,0.0f);						// Set The color to green
				glVertex3f( 1.0f, 1.0f,-1.0f);					// Top Right of the quad (top)
				glVertex3f(-1.0f, 1.0f,-1.0f);					// Top Left of the quad (top)
				glVertex3f(-1.0f, 1.0f, 1.0f);					// Bottom left of the quad (top)
				glVertex3f( 1.0f, 1.0f, 1.0f);					// Bottom right of the quad (top)
				glColor3f(1.0f,0.5f,0.0f);						// Set The color to orange
				glVertex3f( 1.0f,-1.0f, 1.0f);					// Top Right of the quad (bottom)
				glVertex3f(-1.0f,-1.0f, 1.0f);					// Top Left of the quad (bottom)
				glVertex3f(-1.0f,-1.0f,-1.0f);					// Bottom left of the quad (bottom)
				glVertex3f( 1.0f,-1.0f,-1.0f);					// Bottom right of the quad (bottom)
				glColor3f(1.0f,0.0f,0.0f);						// Set The color to red
				glVertex3f( 1.0f, 1.0f, 1.0f);					// Top Right of the quad (front)
				glVertex3f(-1.0f, 1.0f, 1.0f);					// Top Left of the quad (front)
				glVertex3f(-1.0f,-1.0f, 1.0f);					// Bottom left of the quad (front)
				glVertex3f( 1.0f,-1.0f, 1.0f);					// Bottom right of the quad (front)
				glColor3f(1.0f,1.0f,0.0f);						// Set The color to yellow
				glVertex3f( 1.0f,-1.0f,-1.0f);					// Top Right of the quad (back)
				glVertex3f(-1.0f,-1.0f,-1.0f);					// Top Left of the quad (back)
				glVertex3f(-1.0f, 1.0f,-1.0f);					// Bottom left of the quad (back)
				glVertex3f( 1.0f, 1.0f,-1.0f);					// Bottom right of the quad (back)
				glColor3f(0.0f,0.0f,1.0f);						// Set The color to blue
				glVertex3f(-1.0f, 1.0f, 1.0f);					// Top Right of the quad (left)
				glVertex3f(-1.0f, 1.0f,-1.0f);					// Top Left of the quad (left)
				glVertex3f(-1.0f,-1.0f,-1.0f);					// Bottom left of the quad (left)
				glVertex3f(-1.0f,-1.0f, 1.0f);					// Bottom right of the quad (left)
				glColor3f(1.0f,0.0f,1.0f);						// Set The color to violet
				glVertex3f( 1.0f, 1.0f,-1.0f);					// Top Right of the quad (right)
				glVertex3f( 1.0f, 1.0f, 1.0f);					// Top Left of the quad (right)
				glVertex3f( 1.0f,-1.0f, 1.0f);					// Bottom left of the quad (right)
				glVertex3f( 1.0f,-1.0f,-1.0f);					// Bottom right of the quad (right)
			glEnd();											// Done drawing the quad

			//rtri+=0.2f;										// Increase the rotation variable for the triangle
			rquad-=0.15f;										// Decrease the rotation variable for the quad
		}

		System::Void Render2(System::Void)
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear screen and depth buffer
			glLoadIdentity();									// Reset the current modelview matrix
			glTranslatef(-1.5f,0.0f,-6.0f);						// Move left 1.5 units and into the screen 6.0
			glRotatef(rtri,0.0f,1.0f,0.0f);						// Rotate the triangle on the y axis 
			glBegin(GL_TRIANGLES);								// Start drawing a triangle
				glColor3f(1.0f,0.0f,0.0f);						// Red
				glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of triangle (front)
				glColor3f(0.0f,1.0f,0.0f);						// Green
				glVertex3f(-1.0f,-1.0f, 1.0f);					// Left of triangle (front)
				glColor3f(0.0f,0.0f,1.0f);						// Blue
				glVertex3f( 1.0f,-1.0f, 1.0f);					// Right of triangle (front)
				glColor3f(1.0f,0.0f,0.0f);						// Red
				glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of triangle (right)
				glColor3f(0.0f,0.0f,1.0f);						// Blue
				glVertex3f( 1.0f,-1.0f, 1.0f);					// Left of triangle (right)
				glColor3f(0.0f,1.0f,0.0f);						// Green
				glVertex3f( 1.0f,-1.0f, -1.0f);					// Right of triangle (right)
				glColor3f(1.0f,0.0f,0.0f);						// Red
				glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of triangle (back)
				glColor3f(0.0f,1.0f,0.0f);						// Green
				glVertex3f( 1.0f,-1.0f, -1.0f);					// Left of triangle (back)
				glColor3f(0.0f,0.0f,1.0f);						// Blue
				glVertex3f(-1.0f,-1.0f, -1.0f);					// Right of triangle (back)
				glColor3f(1.0f,0.0f,0.0f);						// Red
				glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of triangle (left)
				glColor3f(0.0f,0.0f,1.0f);						// Blue
				glVertex3f(-1.0f,-1.0f,-1.0f);					// Left of triangle (left)
				glColor3f(0.0f,1.0f,0.0f);						// Green
				glVertex3f(-1.0f,-1.0f, 1.0f);					// Right of triangle (left)
			glEnd();											// Done drawing the pyramid


			//rtri+=0.2f;										// Increase the rotation variable for the triangle
			rquad-=0.15f;										// Decrease the rotation variable for the quad
		}

		System::Void SwapOpenGLBuffers(System::Void)
		{
			SwapBuffers(m_hDC) ;
		}


        System::Void wglmakecur()
		{
			wglMakeCurrent(m_hDC,m_hglrc);
		}


		HDC m_hDC;
		HGLRC m_hglrc;
		GLfloat	rtri;				// Angle for the triangle
		GLfloat	rquad;				// Angle for the quad


		~COpenGL(System::Void)
		{
			this->DestroyHandle();
		}

		GLint MySetPixelFormat(HDC hdc)
		{
			static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
				{
					sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
					1,											// Version Number
					PFD_DRAW_TO_WINDOW |						// Format Must Support Window
					PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
					PFD_DOUBLEBUFFER,							// Must Support Double Buffering
					PFD_TYPE_RGBA,								// Request An RGBA Format
					16,										// Select Our Color Depth
					0, 0, 0, 0, 0, 0,							// Color Bits Ignored
					0,											// No Alpha Buffer
					0,											// Shift Bit Ignored
					0,											// No Accumulation Buffer
					0, 0, 0, 0,									// Accumulation Bits Ignored
					16,											// 16Bit Z-Buffer (Depth Buffer)  
					0,											// No Stencil Buffer
					0,											// No Auxiliary Buffer
					PFD_MAIN_PLANE,								// Main Drawing Layer
					0,											// Reserved
					0, 0, 0										// Layer Masks Ignored
				};
			
			GLint  iPixelFormat; 
		 
			// get the device context's best, available pixel format match 
			if((iPixelFormat = ChoosePixelFormat(hdc, &pfd)) == 0)
			{
				MessageBox::Show("ChoosePixelFormat Failed");
				return 0;
			}
			 
			// make that match the device context's current pixel format 
			if(SetPixelFormat(hdc, iPixelFormat, &pfd) == FALSE)
			{
				MessageBox::Show("SetPixelFormat Failed");
				return 0;
			}

			if((m_hglrc = wglCreateContext(m_hDC)) == NULL)
			{
				MessageBox::Show("wglCreateContext Failed");
				return 0;
			}

			if((wglMakeCurrent(m_hDC, m_hglrc)) == NULL)
			{
				MessageBox::Show("wglMakeCurrent Failed");
				return 0;
			}


			return 1;
		}

		bool InitGL(GLvoid)										// All setup for opengl goes here
		{
			glShadeModel(GL_SMOOTH);							// Enable smooth shading
			glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black background
			glClearDepth(1.0f);									// Depth buffer setup
			glEnable(GL_DEPTH_TEST);							// Enables depth testing
			glDepthFunc(GL_LEQUAL);								// The type of depth testing to do
			glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really nice perspective calculations
			return TRUE;										// Initialisation went ok
		}

		GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize and initialise the gl window
		{
			if (height==0)										// Prevent A Divide By Zero By
			{
				height=1;										// Making Height Equal One
			}

			glViewport(0,0,width,height);						// Reset The Current Viewport

			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glLoadIdentity();									// Reset The Projection Matrix

			// Calculate The Aspect Ratio Of The Window
			gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glLoadIdentity();									// Reset The Modelview Matrix
		}
	};
}