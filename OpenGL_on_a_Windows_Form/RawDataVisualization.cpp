#include "stdafx.h"
#include "RawDataVisualization.h"

namespace OpenGLForm{

	RawDataVisualization::RawDataVisualization(Form ^ parentForm,Panel ^ parentPanel, GLsizei iWidth, GLsizei iHeight,ReadCSV read_csv_ref,Preprocessing_Data preprocessing_data_ref):VisualizationPanel(parentForm,parentPanel,iWidth,iHeight,read_csv_ref,preprocessing_data_ref){
			parentPanel->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &RawDataVisualization::RawDataMouseDown);
			parentPanel->MouseWheel += gcnew System::Windows::Forms::MouseEventHandler( this, &RawDataVisualization::RawDataMouseWheel );
			parentPanel->MouseMove += gcnew System::Windows::Forms::MouseEventHandler( this, &RawDataVisualization::RawDataMouseMove );	
			parentPanel->MouseUp += gcnew System::Windows::Forms::MouseEventHandler( this, &RawDataVisualization::RawDataMouseUp );	
			parentPanel->MouseDoubleClick += gcnew System::Windows::Forms::MouseEventHandler( this, &RawDataVisualization::RawDataMouseDoubleClick );	 
			time_string();
			//Initialize mouse handler variable
			scale_x[1] = 1.7; scale_y[1] = 0.2; scale_z[1] = 0.0;
			scale_size[1] = 0.05;
			move_x[1] = 0.0; move_y[1] = 0.0; move_z[1] = 0.0;
			scale_factor[1] = 0.6;
			//Initialize window size
			windowWidth[1] = iWidth; 
			windowHeight[1] = iHeight;
			raw_data_position_table.resize(100);
			for(int i=0;i<100;i++) raw_data_position_table[i].resize(31);
			//raw_data_position_table.resize( preprocessing_data.month_vec.size() );
			//for(int i=0;i<preprocessing_data.month_vec.size();i++) raw_data_position_table[i].resize( preprocessing_data.month_vec[i].day_vec.size() );
			select_histogram_flag = false;
	}

	System::Void RawDataVisualization::Render(int rawdata_width,int rawdata_height){
			wglmakecur();
				
			windowWidth[1] = rawdata_width;
			windowHeight[1] = rawdata_height;

			glClearColor(0.0, 0.0, 0.0, 0.0);  //Set the cleared screen colour to black
			glViewport(0, 0, windowWidth[1], windowHeight[1]);   //This sets up the viewport so that the coordinates (0, 0) are at the top left of the window		
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, windowWidth[1], windowHeight[1], 0, -10, 10);

			//Back to the modelview so we can draw stuff 
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the screen and depth buffer

			glTranslatef(0.0+move_x[1],0.0+move_y[1],0.0+move_z[1]);
			glScalef(scale_factor[1]+scale_x[1],scale_factor[1]+scale_y[1],scale_factor[1]+scale_z[1]);
			glGetDoublev(GL_MODELVIEW_MATRIX, ModelViewMatrix2);//////////////
			vector<float> color;
			color.resize(3);

			int y_position = 20;
			if(!histogram_index.empty())
			{
				for(int i=0;i<histogram_index.size();i++)
				{
					int p = 60;
					int index = 0;
					//DrawTime_FTGL(preprocessing_data.month_vec[i].this_year,10,y_position+5);
					//for(int j=histogram_index[i];j<histogram_index[i]+600;j++)
					int idx = histogram_index[i];
					DrawText_FTGL(preprocessing_data.month_vec[idx].this_year,10,y_position+7);
					DrawText_FTGL(preprocessing_data.month_vec[idx].this_month,30,y_position+7);
					for(int j=0;j<preprocessing_data.raw_data_3D_array[idx].rows;j++)
					{
						RECTANGLE *rect;
						rect = new RECTANGLE();
						rect->h = 15;
						rect->w = 10.0;
						rect->x = p;
						rect->y = y_position;
						color[0] = preprocessing_data.raw_data_3D_array[idx].at<float>(j,0);
						color[1] = preprocessing_data.raw_data_3D_array[idx].at<float>(j,1);
						color[2] = preprocessing_data.raw_data_3D_array[idx].at<float>(j,2);
						//color[0] = preprocessing_data.raw_data_3D.at<float>(j,0);
						//color[1] = preprocessing_data.raw_data_3D.at<float>(j,1);
						//color[2] = preprocessing_data.raw_data_3D.at<float>(j,2);
						DrawRectWithOpenGL(rect,color);
						delete(rect);

						raw_data_position_table[i][j].x = p;
						raw_data_position_table[i][j].y = y_position;
						raw_data_position_table[i][j].z = p + 10.0;
						raw_data_position_table[i][j].w = y_position + 15;
						raw_data_position_table[i][j].x *= (scale_factor[1] + scale_x[1]);
						raw_data_position_table[i][j].y *= (scale_factor[1] + scale_y[1]);
						raw_data_position_table[i][j].z *= (scale_factor[1] + scale_x[1]);
						raw_data_position_table[i][j].w *= (scale_factor[1] + scale_y[1]);
						raw_data_position_table[i][j].x += move_x[1];
						raw_data_position_table[i][j].y += move_y[1];
						raw_data_position_table[i][j].z += move_x[1];
						raw_data_position_table[i][j].w += move_y[1];

						p+=10.0;
						index++;
					}

					y_position += 30;
				}
			}

			SwapOpenGLBuffers();
			
	}
		 
	System::Void RawDataVisualization::FindPatternByTable2(int x,int y){
		vector2 pos_2D(x,y);
		vector3 pos_3D = Unprojection(pos_2D);//screen to 3D coordinate
		///////////bug///////////
		pos_3D.x *= (scale_factor[1] + scale_x[1]);
		pos_3D.y *= (scale_factor[1] + scale_y[1]);
		pos_3D.x += move_x[1];
		pos_3D.y += move_y[1];	
		//System::Windows::Forms::MessageBox::Show( pos_3D.x.ToString() + " " + pos_3D.y.ToString());	
		for(int i=0;i<raw_data_position_table.size();i++)
		{
			if(histogram_index[i] > read_csv.raw_data_size-1) break; ////////bug//////(index out of scrope)
			for(int j=0;j<raw_data_position_table[i].size();j++)
			{
				if(histogram_index[i]+j > read_csv.raw_data_size-1) break;
				if(pos_3D.x >= raw_data_position_table[i][j].x && pos_3D.x <= raw_data_position_table[i][j].z && pos_3D.y >= raw_data_position_table[i][j].y && pos_3D.y <= raw_data_position_table[i][j].w)
				{
					System::Windows::Forms::MessageBox::Show( (histogram_index[i]).ToString() + " " + (j+1).ToString());			
					//System::Windows::Forms::MessageBox::Show(histogram_index[i] + " " + j + " " + 
					//read_csv.raw_data[ histogram_index[i]+j][1].ToString() + " " + read_csv.raw_data[ histogram_index[i]+j][2].ToString() + " " + read_csv.raw_data[ histogram_index[i]+j][3].ToString());
					//raw_data_index.push_back(histogram_index[i]+j);
					vector<int> temp;
					temp.push_back(histogram_index[i]);
					temp.push_back(j);
					raw_data_index_2D.push_back(temp);
				}
			}
		}
	}

	System::Void RawDataVisualization::FindHistogram(int x,int y)
	{
		vector2 pos_2D(x,y);
		vector3 pos_3D = Unprojection(pos_2D);//screen to 3D coordinate
		///////////bug///////////
		pos_3D.x *= (scale_factor[1] + scale_x[1]);
		pos_3D.y *= (scale_factor[1] + scale_y[1]);
		pos_3D.x += move_x[1];
		pos_3D.y += move_y[1];		
		for(int i=0;i<raw_data_position_table.size();i++)
		{
			if(histogram_index[i] > read_csv.raw_data_size-1) break; 
			for(int j=0;j<raw_data_position_table[i].size();j++)
			{
				if(histogram_index[i]+j > read_csv.raw_data_size-1) break;
				if(pos_3D.x >= raw_data_position_table[i][j].x && pos_3D.x <= raw_data_position_table[i][j].z && pos_3D.y >= raw_data_position_table[i][j].y && pos_3D.y <= raw_data_position_table[i][j].w)
				{
					select_histogram_index = histogram_index[i]/600;
					select_histogram_flag = true;
				}
			}
		}		
	}

	System::Void RawDataVisualization::DrawText_FTGL(int n,int x, int y)
	{
		glPushMatrix();

		glTranslatef(x, y-30, 0);
		glScalef(1.0+scale_x[1],1.0+scale_y[1],1.0+scale_z[1]);	
	
		//float font_size = 10*(scale_factor[2]+0.4+scale_x[2]);	
		font.FaceSize(20);
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos2f(0 , 0 + font.LineHeight());
		stringstream ss;
		ss << n;
		string str = ss.str();
		char zero[] = "0";
		if(n<10)
		{
			strcat(zero,str.c_str());
			font.Render(zero);
		}
		else
		{
			char* text = (char*)str.c_str();
			font.Render(text);
		}

		glPopMatrix();
		
	}

	System::Void RawDataVisualization::DrawTime_FTGL(int index,int x, int y)
	{
		int hour;
		int five_minute_index;
		for(int i=0;i<preprocessing_data.hour_range.size();i++)
		{
			if(index>=preprocessing_data.hour_range[i].x && index<= preprocessing_data.hour_range[i].y)
			{
				hour = preprocessing_data.hour_index[i];
				if(hour == preprocessing_data.begin_hour && preprocessing_data.num_of_begin_hour<12)
					five_minute_index = index - preprocessing_data.hour_range[i].x + (12-preprocessing_data.num_of_begin_hour);
				else
					five_minute_index = index - preprocessing_data.hour_range[i].x;
				
				break;
			}
		}
		stringstream ss;
		ss << hour;
		string str = ss.str();
		char *hour_text = (char*)str.c_str();		

		strcat(hour_text,five_minutes[five_minute_index]);

		glPushMatrix();

		float font_size = 10*(scale_factor[1]+0.3+scale_x[1]);
		font.FaceSize(font_size);
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos2f(x , y-20.0 + font.LineHeight());
		font.Render(hour_text);

		glPopMatrix();		
	}

	vector3 RawDataVisualization::Unprojection(vector2 _2Dpos){
		float Depth;
		int viewport[4];
		//double ModelViewMatrix[16];				//Model_view matrix
		double ProjectionMatrix[16];			//Projection matrix

		glPushMatrix();

		glGetIntegerv(GL_VIEWPORT, viewport);
		//glGetDoublev(GL_MODELVIEW_MATRIX, ModelViewMatrix);
		glGetDoublev(GL_PROJECTION_MATRIX, ProjectionMatrix);

		glPopMatrix();

		glReadPixels((int)_2Dpos.x , viewport[3] - (int)_2Dpos.y , 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &Depth);

		double X = _2Dpos.x;
		double Y = _2Dpos.y;
		double wpos[3] = {0.0 , 0.0 , 0.0};

		gluUnProject(X , ((double)viewport[3] - Y) , (double)Depth , ModelViewMatrix2 , ProjectionMatrix , viewport, &wpos[0] , &wpos[1] , &wpos[2]);

		return vector3(wpos[0] , wpos[1] , wpos[2]);
	}

	System::Void RawDataVisualization::RawDataMouseDown( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e ){
			last_X[1] = e->X;
			last_Y[1] = e->Y;

			if (e->Button == System::Windows::Forms::MouseButtons::Right)
			{
				FindPatternByTable2(e->X,e->Y);
			}
	}
	System::Void RawDataVisualization::RawDataMouseWheel( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e ){
			
			if (e->Delta < 0){
				scale_x[1]+=scale_size[1];
				scale_y[1]+=scale_size[1];
				scale_z[1]+=scale_size[1];
			}
			else{
				scale_x[1]-=scale_size[1];
				scale_y[1]-=scale_size[1];
				scale_z[1]-=scale_size[1];		
			}
			
			
	}
	System::Void RawDataVisualization::RawDataMouseMove( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e ){
			if (e->Button == System::Windows::Forms::MouseButtons::Left)
			{
					vector2 Move(e->X - last_X[1] , e->Y - last_Y[1]);
					if (Move.length() < 500.0f)
					{
						move_x[1] = Move.x;
						move_y[1] = Move.y;
					}
			}		
	}
	System::Void RawDataVisualization::RawDataMouseUp( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e ){
			last_X[1] = e->X;
			last_Y[1] = e->Y;		
	}

	System::Void RawDataVisualization::RawDataMouseDoubleClick( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e ){
			if (e->Button == System::Windows::Forms::MouseButtons::Left)
			{			
				FindHistogram(e->X,e->Y);
				//System::Windows::Forms::MessageBox::Show("ha");	
			}
	}

	System::Void RawDataVisualization::time_string()
	{
		strcpy(five_minutes[0],":00");
		strcpy(five_minutes[1],":05");
		strcpy(five_minutes[2],":10");
		strcpy(five_minutes[3],":15");
		strcpy(five_minutes[4],":20");
		strcpy(five_minutes[5],":25");
		strcpy(five_minutes[6],":30");
		strcpy(five_minutes[7],":35");
		strcpy(five_minutes[8],":40");
		strcpy(five_minutes[9],":45");
		strcpy(five_minutes[10],":50");
		strcpy(five_minutes[11],":55");
	}

	System::Void RawDataVisualization::clear()
	{
		histogram_index.clear();
	}
}