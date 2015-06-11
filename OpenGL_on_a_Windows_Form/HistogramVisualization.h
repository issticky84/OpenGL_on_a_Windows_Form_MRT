#pragma once

#include "VisualizationPanel.h"



namespace OpenGLForm{
	public ref class HistogramVisualization: public OpenGLForm::VisualizationPanel{
		public:
			HistogramVisualization(Form ^ parentForm,Panel ^ parentPanel, GLsizei iWidth, GLsizei iHeight,ReadCSV read_csv_ref,Preprocessing_Data preprocessing_data_ref);
			virtual System::Void Render(int,int) override;
			void  HistogramMouseDown( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e);
			void  HistogramMouseWheel( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e);
			void  HistogramMouseMove( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e);
			void  HistogramMouseUp( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e);
			System::Void clear();
			System::Void resize();
		private:
			System::Void DrawHistogramVisualization(System::Void);
			System::Void DrawTime_FTGL(int,int,int);
			System::Void DrawText_FTGL(int,int,int);
			System::Void time_string(System::Void);
			System::Void FindPatternByTable(int x,int y);
			vector3 Unprojection(vector2 _2Dpos);
	};
}