#pragma once

#include "VisualizationPanel.h"
namespace OpenGLForm{
	public ref class RawDataVisualization: public OpenGLForm::VisualizationPanel{
		public:
			RawDataVisualization(Form ^ parentForm,Panel ^ parentPanel, GLsizei iWidth, GLsizei iHeight,ReadCSV read_csv_ref,Preprocessing_Data preprocessing_data_ref);
			virtual System::Void Render(int,int) override;
			void  RawDataMouseDown( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e);
			void  RawDataMouseWheel( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e);
			void  RawDataMouseMove( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e);
			void  RawDataMouseUp( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e);
			void  RawDataMouseDoubleClick( Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e);
			System::Void clear();
	private:
			System::Void FindPatternByTable2(int x,int y);
			vector3 Unprojection(vector2 _2Dpos);
			System::Void FindHistogram(int x,int y);
			System::Void DrawTime_FTGL(int index,int x, int y);
			System::Void DrawText_FTGL(int,int,int);
			System::Void time_string();
	};
}