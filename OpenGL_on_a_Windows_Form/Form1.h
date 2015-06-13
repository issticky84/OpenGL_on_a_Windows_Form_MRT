#pragma once

//#include "OpenGL.h"
#include "VisualizationPanel.h"
#include "RawDataVisualization.h"
#include "DetailVisualization.h"
#include "HistogramVisualization.h"
namespace OpenGL_on_a_Windows_Form 
{

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace OpenGLForm;
	public ref class Form1 : public System::Windows::Forms::Form 
	{
	public:
		Form1(void)
		{
			InitializeComponent();
			//system("matlab -nodesktop -nosplash -r draw_rect_wear2");

			ReadCSV read_csv;
			char file_name[200] = "file_list_MRT_rain.txt";
			//char file_name[200] = "file_list_MRT.txt";
			read_csv.read_file_list(file_name);;

			Preprocessing_Data preprocessing_data;
			preprocessing_data.Initial_selection_flag(this->Gravity_Norm->Checked,this->Linear_Acceleration_Norm->Checked,
													  this->Gyroscope_Norm->Checked,this->First_Order_of_Distance->Checked);
			//preprocessing_data.start(read_csv.raw_data,read_csv.attribute_index,read_csv.time_index,trackBar1->Value);
			preprocessing_data.start2(read_csv.month_vec,trackBar1->Value);

			//waiting_flag = true;
			//start_flag = false;

			histogram = gcnew HistogramVisualization(this,this->panel3,panel3->Width,panel3->Height,read_csv,preprocessing_data);
			rawData = gcnew RawDataVisualization(this,this->panel1,panel1->Width,panel1->Height,read_csv,preprocessing_data);
			detail = gcnew DetailVisualization(this,this->panel2,panel2->Width,panel2->Height,read_csv,preprocessing_data);
	
			/*OpenGL = gcnew COpenGL(this,this->panel1,panel1->Width,panel1->Height,read_csv);
			OpenGL_2 = gcnew COpenGL(this,this->panel2,panel2->Width,panel2->Height,read_csv);
			OpenGL_3 = gcnew COpenGL(this,this->panel3,panel3->Width,panel3->Height,read_csv);
			*/

			
		}
	protected:

		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::ComponentModel::IContainer^  components;
	protected: 
	private: System::Windows::Forms::Timer^  timer1;
	private: System::Windows::Forms::Panel^  panel1;
	private: System::Windows::Forms::Panel^  panel2;
	private: System::Windows::Forms::Panel^  panel3;
	private: System::Windows::Forms::Button^  start;


	private: System::Windows::Forms::Button^  button2;

	private:

		RawDataVisualization ^ rawData;
		DetailVisualization ^ detail;
		HistogramVisualization ^ histogram;
		/*OpenGLForm2::COpenGL ^ OpenGL;
		OpenGLForm2::COpenGL ^ OpenGL_2;
		OpenGLForm2::COpenGL ^ OpenGL_3;
		OpenGLForm2::COpenGL ^ OpenGL_4;*/
	private: System::Windows::Forms::TrackBar^  trackBar1;
	
	private: System::Boolean Move_3_21_flag,Down_3_21_flag;
	private: System::Boolean Move_12_horiz_flag,Down_12_horiz_flag;
	private: System::Boolean Move_12_vert_flag,Down_12_vert_flag;
	private: System::Int32 last_x,last_y;
	private: System::Windows::Forms::Button^  Detail_Clear;
	private: System::Windows::Forms::TextBox^  textBox1;
	private: System::Boolean waiting_flag;
	private: System::Windows::Forms::CheckBox^  Gravity_Norm;
	private: System::Windows::Forms::CheckBox^  Linear_Acceleration_Norm;

	private: System::Windows::Forms::Label^  feature_selection_label;
	private: System::Windows::Forms::CheckBox^  Gyroscope_Norm;
	private: System::Windows::Forms::CheckBox^  First_Order_of_Distance;
	private: System::Windows::Forms::Label^  cluster_label;
	private: System::Boolean start_flag;
	private: System::Windows::Forms::ProgressBar^  progressBar1;
	private: System::Windows::Forms::Button^  load_csv;
	private: System::Windows::Forms::Label^  file_directory;
	private: System::ComponentModel::BackgroundWorker^  backgroundWorker1;

	public:  System::Windows::Forms::OpenFileDialog ofdOpen;
	//private: System::String^ file_string;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->panel1 = (gcnew System::Windows::Forms::Panel());
			this->panel2 = (gcnew System::Windows::Forms::Panel());
			this->panel3 = (gcnew System::Windows::Forms::Panel());
			this->start = (gcnew System::Windows::Forms::Button());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->trackBar1 = (gcnew System::Windows::Forms::TrackBar());
			this->Detail_Clear = (gcnew System::Windows::Forms::Button());
			this->textBox1 = (gcnew System::Windows::Forms::TextBox());
			this->Gravity_Norm = (gcnew System::Windows::Forms::CheckBox());
			this->Linear_Acceleration_Norm = (gcnew System::Windows::Forms::CheckBox());
			this->feature_selection_label = (gcnew System::Windows::Forms::Label());
			this->Gyroscope_Norm = (gcnew System::Windows::Forms::CheckBox());
			this->First_Order_of_Distance = (gcnew System::Windows::Forms::CheckBox());
			this->cluster_label = (gcnew System::Windows::Forms::Label());
			this->progressBar1 = (gcnew System::Windows::Forms::ProgressBar());
			this->load_csv = (gcnew System::Windows::Forms::Button());
			this->file_directory = (gcnew System::Windows::Forms::Label());
			this->backgroundWorker1 = (gcnew System::ComponentModel::BackgroundWorker());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->trackBar1))->BeginInit();
			this->SuspendLayout();
			// 
			// timer1
			// 
			this->timer1->Enabled = true;
			this->timer1->Interval = 10;
			this->timer1->Tick += gcnew System::EventHandler(this, &Form1::timer1_Tick);
			// 
			// panel1
			// 
			this->panel1->Location = System::Drawing::Point(12, 12);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(1547, 363);
			this->panel1->TabIndex = 0;
			this->panel1->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::panel1_Paint);
			this->panel1->MouseEnter += gcnew System::EventHandler(this, &Form1::panel1_MouseEnter);
			this->panel1->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::panel1_MouseMove);
			// 
			// panel2
			// 
			this->panel2->Location = System::Drawing::Point(671, 381);
			this->panel2->Name = L"panel2";
			this->panel2->Size = System::Drawing::Size(888, 661);
			this->panel2->TabIndex = 1;
			this->panel2->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::panel2_Paint);
			this->panel2->MouseEnter += gcnew System::EventHandler(this, &Form1::panel2_MouseEnter);
			this->panel2->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::panel2_MouseMove);
			// 
			// panel3
			// 
			this->panel3->Location = System::Drawing::Point(12, 381);
			this->panel3->Name = L"panel3";
			this->panel3->Size = System::Drawing::Size(653, 661);
			this->panel3->TabIndex = 2;
			this->panel3->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::panel3_Paint);
			this->panel3->MouseEnter += gcnew System::EventHandler(this, &Form1::panel3_MouseEnter);
			this->panel3->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::panel3_MouseMove);
			// 
			// start
			// 
			this->start->Location = System::Drawing::Point(1716, 309);
			this->start->Name = L"start";
			this->start->Size = System::Drawing::Size(134, 67);
			this->start->TabIndex = 4;
			this->start->Text = L"Start";
			this->start->UseVisualStyleBackColor = true;
			this->start->Click += gcnew System::EventHandler(this, &Form1::start_Click);
			// 
			// button2
			// 
			this->button2->Location = System::Drawing::Point(1565, 17);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(63, 32);
			this->button2->TabIndex = 5;
			this->button2->Text = L"Clear";
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &Form1::button2_Click);
			// 
			// trackBar1
			// 
			this->trackBar1->LargeChange = 1;
			this->trackBar1->Location = System::Drawing::Point(1682, 249);
			this->trackBar1->Maximum = 50;
			this->trackBar1->Name = L"trackBar1";
			this->trackBar1->Size = System::Drawing::Size(196, 45);
			this->trackBar1->TabIndex = 6;
			this->trackBar1->Value = 4;
			this->trackBar1->Scroll += gcnew System::EventHandler(this, &Form1::trackBar1_Scroll);
			// 
			// Detail_Clear
			// 
			this->Detail_Clear->Location = System::Drawing::Point(1565, 317);
			this->Detail_Clear->Name = L"Detail_Clear";
			this->Detail_Clear->Size = System::Drawing::Size(63, 36);
			this->Detail_Clear->TabIndex = 7;
			this->Detail_Clear->Text = L"Clear";
			this->Detail_Clear->UseVisualStyleBackColor = true;
			this->Detail_Clear->Click += gcnew System::EventHandler(this, &Form1::Detail_Clear_Click);
			// 
			// textBox1
			// 
			this->textBox1->Location = System::Drawing::Point(1873, 249);
			this->textBox1->Name = L"textBox1";
			this->textBox1->Size = System::Drawing::Size(24, 22);
			this->textBox1->TabIndex = 8;
			this->textBox1->Text = L"4";
			// 
			// Gravity_Norm
			// 
			this->Gravity_Norm->AutoSize = true;
			this->Gravity_Norm->Location = System::Drawing::Point(1691, 61);
			this->Gravity_Norm->Name = L"Gravity_Norm";
			this->Gravity_Norm->Size = System::Drawing::Size(89, 16);
			this->Gravity_Norm->TabIndex = 9;
			this->Gravity_Norm->Text = L"Gravity Norm";
			this->Gravity_Norm->UseVisualStyleBackColor = true;
			this->Gravity_Norm->CheckedChanged += gcnew System::EventHandler(this, &Form1::Gravity_Norm_CheckedChanged);
			// 
			// Linear_Acceleration_Norm
			// 
			this->Linear_Acceleration_Norm->AutoSize = true;
			this->Linear_Acceleration_Norm->Checked = true;
			this->Linear_Acceleration_Norm->CheckState = System::Windows::Forms::CheckState::Checked;
			this->Linear_Acceleration_Norm->Location = System::Drawing::Point(1691, 100);
			this->Linear_Acceleration_Norm->Name = L"Linear_Acceleration_Norm";
			this->Linear_Acceleration_Norm->Size = System::Drawing::Size(145, 16);
			this->Linear_Acceleration_Norm->TabIndex = 10;
			this->Linear_Acceleration_Norm->Text = L"Linear Acceleration Norm";
			this->Linear_Acceleration_Norm->UseVisualStyleBackColor = true;
			this->Linear_Acceleration_Norm->CheckedChanged += gcnew System::EventHandler(this, &Form1::Linear_Acceleration_Norm_CheckedChanged);
			// 
			// feature_selection_label
			// 
			this->feature_selection_label->AutoSize = true;
			this->feature_selection_label->Location = System::Drawing::Point(1689, 27);
			this->feature_selection_label->Name = L"feature_selection_label";
			this->feature_selection_label->Size = System::Drawing::Size(84, 12);
			this->feature_selection_label->TabIndex = 11;
			this->feature_selection_label->Text = L"Feature Selection";
			// 
			// Gyroscope_Norm
			// 
			this->Gyroscope_Norm->AutoSize = true;
			this->Gyroscope_Norm->Checked = true;
			this->Gyroscope_Norm->CheckState = System::Windows::Forms::CheckState::Checked;
			this->Gyroscope_Norm->Location = System::Drawing::Point(1691, 138);
			this->Gyroscope_Norm->Name = L"Gyroscope_Norm";
			this->Gyroscope_Norm->Size = System::Drawing::Size(104, 16);
			this->Gyroscope_Norm->TabIndex = 12;
			this->Gyroscope_Norm->Text = L"Gyroscope Norm";
			this->Gyroscope_Norm->UseVisualStyleBackColor = true;
			this->Gyroscope_Norm->CheckedChanged += gcnew System::EventHandler(this, &Form1::Gyroscope_Norm_CheckedChanged);
			// 
			// First_Order_of_Distance
			// 
			this->First_Order_of_Distance->AutoSize = true;
			this->First_Order_of_Distance->Checked = true;
			this->First_Order_of_Distance->CheckState = System::Windows::Forms::CheckState::Checked;
			this->First_Order_of_Distance->Location = System::Drawing::Point(1691, 179);
			this->First_Order_of_Distance->Name = L"First_Order_of_Distance";
			this->First_Order_of_Distance->Size = System::Drawing::Size(129, 16);
			this->First_Order_of_Distance->TabIndex = 13;
			this->First_Order_of_Distance->Text = L"First Order of Distance";
			this->First_Order_of_Distance->UseVisualStyleBackColor = true;
			this->First_Order_of_Distance->CheckedChanged += gcnew System::EventHandler(this, &Form1::First_Order_of_Distance_CheckedChanged);
			// 
			// cluster_label
			// 
			this->cluster_label->AutoSize = true;
			this->cluster_label->Location = System::Drawing::Point(1689, 224);
			this->cluster_label->Name = L"cluster_label";
			this->cluster_label->Size = System::Drawing::Size(129, 12);
			this->cluster_label->TabIndex = 14;
			this->cluster_label->Text = L"Cluster Number (k-means)";
			// 
			// progressBar1
			// 
			this->progressBar1->Location = System::Drawing::Point(1682, 436);
			this->progressBar1->Name = L"progressBar1";
			this->progressBar1->Size = System::Drawing::Size(196, 23);
			this->progressBar1->TabIndex = 15;
			// 
			// load_csv
			// 
			this->load_csv->Location = System::Drawing::Point(1721, 538);
			this->load_csv->Name = L"load_csv";
			this->load_csv->Size = System::Drawing::Size(115, 32);
			this->load_csv->TabIndex = 16;
			this->load_csv->Text = L"Load csv File";
			this->load_csv->UseVisualStyleBackColor = true;
			this->load_csv->Click += gcnew System::EventHandler(this, &Form1::load_csv_Click);
			// 
			// file_directory
			// 
			this->file_directory->AutoSize = true;
			this->file_directory->Location = System::Drawing::Point(1669, 600);
			this->file_directory->Name = L"file_directory";
			this->file_directory->Size = System::Drawing::Size(8, 12);
			this->file_directory->TabIndex = 17;
			this->file_directory->Text = L" ";
			// 
			// backgroundWorker1
			// 
			this->backgroundWorker1->WorkerReportsProgress = true;
			this->backgroundWorker1->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &Form1::backgroundWorker1_DoWork);
			this->backgroundWorker1->ProgressChanged += gcnew System::ComponentModel::ProgressChangedEventHandler(this, &Form1::backgroundWorker1_ProgressChanged);
			this->backgroundWorker1->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &Form1::backgroundWorker1_RunWorkerCompleted);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1916, 1054);
			this->Controls->Add(this->file_directory);
			this->Controls->Add(this->load_csv);
			this->Controls->Add(this->progressBar1);
			this->Controls->Add(this->cluster_label);
			this->Controls->Add(this->First_Order_of_Distance);
			this->Controls->Add(this->Gyroscope_Norm);
			this->Controls->Add(this->feature_selection_label);
			this->Controls->Add(this->Linear_Acceleration_Norm);
			this->Controls->Add(this->Gravity_Norm);
			this->Controls->Add(this->textBox1);
			this->Controls->Add(this->Detail_Clear);
			this->Controls->Add(this->trackBar1);
			this->Controls->Add(this->button2);
			this->Controls->Add(this->start);
			this->Controls->Add(this->panel3);
			this->Controls->Add(this->panel2);
			this->Controls->Add(this->panel1);
			this->Name = L"Form1";
			this->Text = L"OpenGL on a Windows Form using Managed C++";
			this->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::Form1_MouseDown);
			this->MouseEnter += gcnew System::EventHandler(this, &Form1::Form1_MouseEnter);
			this->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::Form1_MouseMove_1);
			this->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::Form1_MouseUp);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->trackBar1))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
		void MarshalString ( System::String ^ s, string& os ) {
		   using namespace Runtime::InteropServices;
		   const char* chars = 
			  (const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();
		   os = chars;
		   Marshal::FreeHGlobal(IntPtr((void*)chars));
		}
#pragma endregion
	private: System::Void timer1_Tick(System::Object^  sender, System::EventArgs^  e)
			 {
				 if(rawData)
					 rawData->Render(panel1->Width,panel1->Height);
				 if(detail)
					 detail->Render(panel2->Width,panel2->Height);
				 if(histogram)
					 histogram->Render(panel3->Width,panel3->Height);

				 if(Move_3_21_flag==false && Move_12_horiz_flag==false && Move_12_vert_flag==false) 
					 this->Cursor = System::Windows::Forms::Cursors::Default;

				 //this->progressBar1->Value = preprocessing_data.progress_value;
				 //if(start_flag) progressBar1->Value += 1;
				 //if(progressBar1->Value==progressBar1->Maximum)
				 //{
					//start_flag = false;
					//progressBar1->Value = 0;
				 //}
			 }

	private: System::Void start_Click(System::Object^  sender, System::EventArgs^  e) 
			 {
				 backgroundWorker1->RunWorkerAsync();

				 histogram->clear();
				 rawData->clear();
				 detail->clear();
				 //progressBar1->Style = ProgressBarStyle::Continuous;
				 //progressBar1->MarqueeAnimationSpeed = 30;

				 preprocessing_data.Initial_selection_flag(this->Gravity_Norm->Checked,this->Linear_Acceleration_Norm->Checked,
															this->Gyroscope_Norm->Checked,this->First_Order_of_Distance->Checked);
				 
				 preprocessing_data.start2(read_csv.month_vec,trackBar1->Value);
				 //preprocessing_data.start(read_csv.raw_data,read_csv.attribute_index,read_csv.time_index,trackBar1->Value);
				 //System::Windows::Forms::MessageBox::Show(trackBar1->Value.ToString());
				 //progressBar1->Style = ProgressBarStyle::Marquee;
				 //progressBar1->MarqueeAnimationSpeed = 0;
			 }

    private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) 
			 {
				 rawData->clear();
			 }

	private: System::Void Detail_Clear_Click(System::Object^  sender, System::EventArgs^  e)
			 {
				 detail->clear();
			 }

	private: System::Void load_csv_Click(System::Object^  sender, System::EventArgs^  e) {
				 System::Windows::Forms::OpenFileDialog^ ofdOpen=gcnew System::Windows::Forms::OpenFileDialog();


				 if ((ofdOpen->InitialDirectory == NULL.ToString())) 
				 {
					ofdOpen->InitialDirectory = "E:Github/csv_data/"; // 語音檔預設開啟目錄
				 }

				 ofdOpen->Filter = 
						//"Audio files (*.wav; *.mpa; *.mp2; *.mp3; *.au; *.aif; *.aiff; *.snd; *.wma)|*.wav; *.mpa; *.mp2; *.mp3; *.au; *.aif; *.aiff; *.snd; *.wma|" +
						//"Video Files (*.avi; *.qt; *.mov; *.mpg; *.mpeg; *.m1v; *.wmv)|*.avi; *.qt; *.mov; *.mpg; *.mpeg; *.m1v; *.wmv|" +
						//"MIDI Files (*.mid, *.midi, *.rmKKTimeri)|*.mid; *.midi; *.rmi|" +
						//"Image Files (*.jpg, *.bmp, *.gif, *.tga)|*.jpg; *.bmp; *.gif; *.tga|" +
						"CSV Files (*.csv)|*.csv|" +
						"All Files (*.*)|*.*";
						ofdOpen->Title = "Open CSV file";
						ofdOpen->Multiselect = true; // 允許選取多檔案

				 if (ofdOpen->ShowDialog(this) == System::Windows::Forms::DialogResult::Cancel) 
				 {
				 		// 使用者沒有選檔案
						return;
				 }
		 
				 System::String^ Filename = ofdOpen->FileName; // 選擇的完整路徑

				 file_directory->Text = "" + Filename;
				 //System::Windows::Forms::MessageBox::Show(Filename);

				 read_csv.clear();
				 //System::String to string, string to char*
				 string file;
				 MarshalString(Filename, file);
				 char file_char[200];
				 strcpy(file_char,file.c_str());
				 read_csv.read_raw_data(file_char);

				 //clear the mat & reset the flag & start
				 detail->clear();
				 rawData->clear();
				 histogram->clear();
				 histogram_position_table.clear();
				 preprocessing_data.Initial_selection_flag(this->Gravity_Norm->Checked,this->Linear_Acceleration_Norm->Checked,
															this->Gyroscope_Norm->Checked,this->First_Order_of_Distance->Checked);
				 //Gravity_Norm->Checked = this->Gravity_Norm->Checked;
				 //Linear_Acceleration_Norm->Checked = this->Linear_Acceleration_Norm->Checked;
				 //Gyroscope_Norm->Checked = this->Gyroscope_Norm->Checked;
				 //First_Order_of_Distance->Checked = this->First_Order_of_Distance->Checked;
				 
				 preprocessing_data.start(read_csv.raw_data,read_csv.attribute_index,read_csv.time_index,trackBar1->Value);
				 //System::Windows::Forms::MessageBox::Show(preprocessing_data.num_of_five_minutes.ToString() + " " + histogram_position_table.size());
				 histogram->resize();
				 //System::Windows::Forms::MessageBox::Show(preprocessing_data.num_of_five_minutes.ToString() + " " + histogram_position_table.size());
			 }

	private: System::Void trackBar1_Scroll(System::Object^  sender, System::EventArgs^  e) 
			 {
				 textBox1->Text = "" + trackBar1->Value;
				 //System::Windows::Forms::MessageBox::Show(trackBar1->Value.ToString() + " " + trackBar1->TickFrequency.ToString() );		
			 }

	private: System::Void panel1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
			 {
			 }

	private: System::Void panel3_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
			 }

	private: System::Void panel3_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
				 this->panel3->Focus();
			 }
	private: System::Void panel1_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
				 this->panel1->Focus();
			 }
	private: System::Void panel2_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
				 this->panel2->Focus();
			 }

	private: System::Void panel3_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
				 Move_3_21_flag = false;
				 Move_12_horiz_flag = false;
				 Move_12_vert_flag = false;
			}
	private: System::Void panel1_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
				 Move_3_21_flag = false;
				 Move_12_horiz_flag = false;
				 Move_12_vert_flag = false;
			 }
	private: System::Void panel2_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
				 Move_3_21_flag = false;
				 Move_12_horiz_flag = false;
				 Move_12_vert_flag = false;
				
			 }

	private: System::Void Form1_MouseMove_1(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
					 ///////////////move panel3 vs panel12 horizontally///////////////
					 int side_3_21_x1 = panel3->Location.X + panel3->Width;
					 int side_3_21_x2 = panel1->Location.X;
					 if(e->X > side_3_21_x1 && e->X < side_3_21_x2)
					 {
						 Move_3_21_flag = true;
						 this->Cursor = System::Windows::Forms::Cursors::NoMoveHoriz;
					 }

					 int move_3_21_x = e->X - last_x;
					 if(Down_3_21_flag)
					 {
						panel3->Width += move_3_21_x;
						panel1->Location = System::Drawing::Point(panel1->Location.X+move_3_21_x, panel1->Location.Y);
						panel1->Width -= move_3_21_x;
						panel2->Location = System::Drawing::Point(panel2->Location.X+move_3_21_x, panel2->Location.Y);
						panel2->Width -= move_3_21_x;	
					 }
					 ///////////////////move pane1 & panel2 horizontally//////////////////////
					 int side_12_x1 = panel2->Location.X + panel2->Width;
					 int side_12_x2 = side_12_x1 + 5;
					 int side_12_y1 = panel1->Location.Y;
					 int side_12_y2 = panel2->Location.Y + panel2->Height;
					 if(e->X > side_12_x1 && e->X < side_12_x2 && e->Y > side_12_y1 && e->Y < side_12_y2)
					 {
						 Move_12_horiz_flag = true;
						 this->Cursor = System::Windows::Forms::Cursors::NoMoveHoriz;							
					 }
					 else
					 {
						Move_12_horiz_flag = false;
					 }

					 int move_12_x = e->X - last_x;
					 if(Down_12_horiz_flag)
					 {
						 //panel2->Location = System::Drawing::Point(panel2->Location.X,panel2->Location.Y+move_12_y);
						 panel1->Width += move_12_x;
						 panel2->Width += move_12_x;
					 }
					 ///////////////move panel1 & panel2 vertically///////////////
					 side_12_x1 = panel1->Location.X;
					 side_12_x2 = panel1->Location.X + panel1->Width;
					 side_12_y1 = panel1->Location.Y;
					 side_12_y2 = panel2->Location.Y;
					 if(e->X > side_12_x1 && e->X < side_12_x2 && e->Y > side_12_y1 && e->Y < side_12_y2)
					 {
						 Move_12_vert_flag = true;
						 this->Cursor = System::Windows::Forms::Cursors::NoMoveVert;							
					 }
					 else
					 {
						Move_12_vert_flag = false;
					 }

					 int move_y = e->Y - last_y;
					 if(Down_12_vert_flag)
					 {
						 panel2->Location = System::Drawing::Point(panel2->Location.X,panel2->Location.Y+move_y);
						 panel1->Height += move_y;
						 panel2->Height -= move_y;
					 }
					 ////////////////////////////////////////////////////
					 last_x = e->X;
					 last_y = e->Y;
			 }


	private: System::Void Form1_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
					last_x = e->X;
					last_y = e->Y;

					if(Move_3_21_flag)
					{
						Down_3_21_flag = true;
					}

					if(Move_12_horiz_flag)
					{
						Down_12_horiz_flag = true;
					}

					if(Move_12_vert_flag)
					{
						Down_12_vert_flag = true;
					}
			 }
	private: System::Void Form1_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
					Down_3_21_flag = false;
					Down_12_horiz_flag = false;
					Down_12_vert_flag = false;
			 }

	private: System::Void Form1_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
					Form::Focus();
			 }

	private: System::Void Gravity_Norm_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if(Gravity_Norm->Checked)
					 preprocessing_data.select_gravity = true;
				 else
					 preprocessing_data.select_gravity = false;
			 }
	private: System::Void Linear_Acceleration_Norm_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if(Linear_Acceleration_Norm->Checked)
					 preprocessing_data.select_linear_acc = true;
				 else
					 preprocessing_data.select_linear_acc = false;
			 }
	private: System::Void Gyroscope_Norm_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if(Gyroscope_Norm->Checked)
					 preprocessing_data.select_gyro = true;
				 else
					 preprocessing_data.select_gyro = false;
			 }
	private: System::Void First_Order_of_Distance_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if(First_Order_of_Distance->Checked)
					 preprocessing_data.select_distance = true;
				 else
					 preprocessing_data.select_distance = false;
			 }

	private: System::Void backgroundWorker1_ProgressChanged(System::Object^  sender, System::ComponentModel::ProgressChangedEventArgs^  e) {
				 progressBar1->Value = e->ProgressPercentage;
			 }

	private: System::Void backgroundWorker1_RunWorkerCompleted(System::Object^  sender, System::ComponentModel::RunWorkerCompletedEventArgs^  e) {
				 //System::Windows::Forms::MessageBox::Show("Processing was completed");
			 }
	private: System::Void backgroundWorker1_DoWork(System::Object^  sender, System::ComponentModel::DoWorkEventArgs^  e) {
				for (int i = 0; i < 100; i++)
				{
					//System::Threading::Thread::Sleep(10);
					backgroundWorker1->ReportProgress(i);
				}    
			 }
private: System::Void panel2_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
		 }
};
}

