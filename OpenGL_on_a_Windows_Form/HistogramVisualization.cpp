#include "stdafx.h"
#include "HistogramVisualization.h"

namespace OpenGLForm{

	HistogramVisualization::HistogramVisualization(Form ^ parentForm,Panel ^ parentPanel, GLsizei iWidth, GLsizei iHeight,ReadCSV read_csv_ref,Preprocessing_Data preprocessing_data_ref):VisualizationPanel(parentForm,parentPanel,iWidth,iHeight,read_csv_ref,preprocessing_data_ref){
			parentPanel->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &HistogramVisualization::HistogramMouseDown);
			parentPanel->MouseWheel += gcnew System::Windows::Forms::MouseEventHandler( this, &HistogramVisualization::HistogramMouseWheel );
			parentPanel->MouseMove += gcnew System::Windows::Forms::MouseEventHandler( this, &HistogramVisualization::HistogramMouseMove );
			parentPanel->MouseUp += gcnew System::Windows::Forms::MouseEventHandler( this, &HistogramVisualization::HistogramMouseUp );
			time_string();
			histogram_position_table.resize(preprocessing_data.num_of_five_minutes);
			//Initialize mouse handler variable
			scale_x[0] = 0.0; scale_y[0] = 0.0; scale_z[0] = 0.0;
			scale_size[0] = 0.05;
			move_x[0] = 0.0; move_y[0] = 80.0; move_z[0] = 0.0;
			scale_factor[0] = 0.7;
			//Initialize window size
			windowWidth[0] = iWidth; 
			windowHeight[0] = iHeight;
			counter = 0;
	}

	System::Void HistogramVisualization::FindPatternByTable(int x,int y)
	{  
		vector2 pos_2D(x,y);
		vector3 pos_3D = Unprojection(pos_2D);//screen to 3D coordinate
		pos_3D.x *= (scale_factor[0] + scale_x[0]);
		pos_3D.y *= (scale_factor[0] + scale_y[0]);
		pos_3D.x += move_x[0];
		pos_3D.y += move_y[0];

		for(int i=0;i<histogram_position_table.size();i++)
		{
			if(pos_3D.x >= histogram_position_table[i].x && pos_3D.x <= histogram_position_table[i].z && pos_3D.y >= histogram_position_table[i].y && pos_3D.y <= histogram_position_table[i].w)
			{
				System::Windows::Forms::MessageBox::Show( (i+1).ToString());
				//System::Windows::Forms::MessageBox::Show( move_x[0].ToString() + " " + move_x[1].ToString());
				//histogram_index.push_back(i*600);
				histogram_index.push_back(i);
				///////////////////////////////////////////////////
			}
		}
	}

	System::Void HistogramVisualization::Render(int histogram_width,int histogram_height){
				wglmakecur();

				windowWidth[0] = histogram_width;
				windowHeight[0] = histogram_height;
				 
				DrawHistogramVisualization();
				//let the histogram be flickering
				counter++;
				counter%=5;
				if(counter==0) select_histogram_flag = false;

				SwapOpenGLBuffers();		
	}
	
	System::Void HistogramVisualization::DrawTime_FTGL(int l,int x, int y)
	{
		glPushMatrix();

		float font_size = 20*(scale_factor[0]+0.5+scale_x[0]);
		font.FaceSize(font_size);
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos2f(x , y-30.0 + font.LineHeight());
		font.Render(quote[l]);

		glPopMatrix();
		
	}

	System::Void HistogramVisualization::DrawText_FTGL(int n,int x, int y)
	{
		glPushMatrix();

		glTranslatef(x, y-30, 0);
		glScalef(1.0+scale_x[0],1.0+scale_y[0],1.0+scale_z[0]);	
	
		//float font_size = 10*(scale_factor[2]+0.4+scale_x[2]);	
		font.FaceSize(20);
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos2f(0 , 0 + font.LineHeight());
		stringstream ss;
		ss << n;
		string str = ss.str();
		char* text = (char*)str.c_str();
		font.Render(text);

		glPopMatrix();
		
	}

	vector3 HistogramVisualization::Unprojection(vector2 _2Dpos){
		float Depth;
		int viewport[4];
		double ModelViewMatrix[16];				//Model_view matrix
		double ProjectionMatrix[16];			//Projection matrix

		glPushMatrix();


		glGetIntegerv(GL_VIEWPORT, viewport);
		glGetDoublev(GL_MODELVIEW_MATRIX, ModelViewMatrix);
		glGetDoublev(GL_PROJECTION_MATRIX, ProjectionMatrix);

		glPopMatrix();

		glReadPixels((int)_2Dpos.x , viewport[3] - (int)_2Dpos.y , 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &Depth);

		double X = _2Dpos.x;
		double Y = _2Dpos.y;
		double wpos[3] = {0.0 , 0.0 , 0.0};

		gluUnProject(X , ((double)viewport[3] - Y) , (double)Depth , ModelViewMatrix , ProjectionMatrix , viewport, &wpos[0] , &wpos[1] , &wpos[2]);

		return vector3(wpos[0] , wpos[1] , wpos[2]);
	}

	System::Void HistogramVisualization::DrawHistogramVisualization(System::Void){
			vector<float>  draw_color;	
			draw_color.resize(3);


			glClearColor(0.0, 0.0, 0.0, 0.0);  //Set the cleared screen colour to black
			glViewport(0, 0, windowWidth[0], windowHeight[0]);   //This sets up the viewport so that the coordinates (0, 0) are at the top left of the window		
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, windowWidth[0], windowHeight[0], 0, -10, 10);

			//Back to the modelview so we can draw stuff 
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the screen and depth buffer

			glTranslatef(0.0+move_x[0],0.0+move_y[0],0.0+move_z[0]);
			glScalef(scale_factor[0]+scale_x[0],scale_factor[0]+scale_y[0],scale_factor[0]+scale_z[0]);


			if(!preprocessing_data.histogram.empty())
			{
				int y_coord = 1000;
				int pixels;
				int current_hour;
				int last_hour = -1;
				int t=0;
				for(int i=0;i<preprocessing_data.histogram.rows;++i)
				{
					float end_position;
					current_hour = preprocessing_data.month_vec[i].this_year;
					if(current_hour!=last_hour)
					{
						draw_color[0] = 1; 
						draw_color[1] = 1; 
						draw_color[2] = 1;
						RECTANGLE *line;
						line = new RECTANGLE();
						line->h = 3;
						line->w = 900;
						line->x = 0;
						line->y = y_coord - 4;
						DrawRectWithOpenGL(line,draw_color);
						DrawText_FTGL(current_hour,20,y_coord-10);
						//DrawTime_FTGL(current_hour,20,y_coord-10);
						t++;
						y_coord-=10;
						delete(line);
					}
	
						int start = 0;
						for(int k=0;k<preprocessing_data.histogram.cols;++k)
						{   
							if(preprocessing_data.histogram.at<int>(i,k)!=0)
							{
								pixels = preprocessing_data.histogram.at<int>(i,k);
								draw_color[0] = preprocessing_data.rgb_mat3.at<float>(k,0); 
								draw_color[1] = preprocessing_data.rgb_mat3.at<float>(k,1); 
								draw_color[2] = preprocessing_data.rgb_mat3.at<float>(k,2);

								if((counter==2 || counter==4) && select_histogram_flag)
								{
									if(i==select_histogram_index)
									{
										draw_color[0] = 0; draw_color[1] = 0; draw_color[2] = 0;
									}
								}

								for(int u=start;u<start+pixels;++u)
								{
									RECTANGLE *rect;
									rect = new RECTANGLE();
									rect->h = 6.0;
									rect->w = 2.0;
									rect->x = 110 + preprocessing_data.position.at<double>(i,0)/10.0 + (double)u*2.0;
									rect->y = y_coord;
									DrawRectWithOpenGL(rect,draw_color);	
									end_position = rect->x;
									delete(rect);
								}
								start += pixels;
							}		
						}

						//table record
						histogram_position_table[i].x = 110 + preprocessing_data.position.at<double>(i,0)/10.0;
						histogram_position_table[i].y = y_coord;
						histogram_position_table[i].z = end_position;
						histogram_position_table[i].w = y_coord + 6.0;
								
						histogram_position_table[i].x *= (scale_factor[0] + scale_x[0]);
						histogram_position_table[i].y *= (scale_factor[0] + scale_y[0]);
						histogram_position_table[i].z *= (scale_factor[0] + scale_x[0]);
						histogram_position_table[i].w *= (scale_factor[0] + scale_y[0]);
						histogram_position_table[i].x += move_x[0];
						histogram_position_table[i].y += move_y[0];
						histogram_position_table[i].z += move_x[0];
						histogram_position_table[i].w += move_y[0];

						y_coord-=6;
						last_hour = current_hour;
				}
			}

	}

	System::Void HistogramVisualization::time_string(System::Void)
	{
		font.Error();
		strcpy(quote[0],"00:00");
		strcpy(quote[1],"01:00");
		strcpy(quote[2],"02:00");
		strcpy(quote[3],"03:00");
		strcpy(quote[4],"04:00");
		strcpy(quote[5],"05:00");
		strcpy(quote[6],"06:00");
		strcpy(quote[7],"07:00");
		strcpy(quote[8],"08:00");
		strcpy(quote[9],"09:00");
		strcpy(quote[10],"10:00");
		strcpy(quote[11],"11:00");
		strcpy(quote[12],"12:00");
		strcpy(quote[13],"13:00");
		strcpy(quote[14],"14:00");
		strcpy(quote[15],"15:00");
		strcpy(quote[16],"16:00");
		strcpy(quote[17],"17:00");
		strcpy(quote[18],"18:00");
		strcpy(quote[19],"19:00");
		strcpy(quote[20],"20:00");
		strcpy(quote[21],"21:00");
		strcpy(quote[22],"22:00");
		strcpy(quote[23],"23:00");
	}		 
	System::Void   HistogramVisualization::HistogramMouseDown( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e ){
			 last_X[0] = e->X;
			 last_Y[0] = e->Y;
		

			if (e->Button == System::Windows::Forms::MouseButtons::Right)
			{
				FindPatternByTable(e->Location.X,e->Location.Y);
			}
	}
	System::Void  HistogramVisualization::HistogramMouseWheel( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e ){
			//System::Windows::Forms::MessageBox::Show(e->Location.ToString());
			if (e->Delta < 0){
				scale_x[0]+=scale_size[0];
				scale_y[0]+=scale_size[0];
				scale_z[0]+=scale_size[0];
			}
			else{
				scale_x[0]-=scale_size[0];
				scale_y[0]-=scale_size[0];
				scale_z[0]-=scale_size[0];		
			}
			
	}

	System::Void HistogramVisualization::HistogramMouseMove( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e ){
			if (e->Button == System::Windows::Forms::MouseButtons::Left)
			{
				 vector2 Move(e->X - last_X[0] , e->Y - last_Y[0]);
				//move_x[0] = Move.x * (1 + scale_x[0]);
				//move_y[0] = Move.y * (1 + scale_y[0]);
				move_x[0] = Move.x;
				move_y[0] = Move.y;

			}
			
	}

	System::Void HistogramVisualization::HistogramMouseUp( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e ){
				 last_X[0] = e->X;
				 last_Y[0] = e->Y;	
	}

	System::Void HistogramVisualization::clear()
	{
		//histogram_position_table.clear();
		preprocessing_data.histogram.release();
	}

	System::Void HistogramVisualization::resize()
	{
		histogram_position_table.resize(preprocessing_data.num_of_five_minutes);
	}

}