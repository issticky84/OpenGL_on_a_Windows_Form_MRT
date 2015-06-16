#include "stdafx.h"
#include <iostream>
#include <vector>
#include <map>
#include "mtxlib.h"

#include "month.h"
#include "holiday.h"

#define LENGTH 1000

using namespace std;

class ReadCSV
{
public:
	void read_raw_data(char file_name[]);
	void clear();

	vector < vector<float> > raw_data;
	int raw_data_size;
	vector<int> attribute_index;
	int time_index;

	vector<month> month_vec;
	vector<holiday> holiday_vec;

	void read_file_list(char*);
	void read_holiday_event(char*);
	//vector < vector<float> > position_data;
	//vector < vector<float> > histogram_data;
	//vector < vector<float> > color_data;
	//vector < vector<float> > hour_data;
	//vector < vector<float> > cluster_data;
	//vector < vector<float> > raw_data_3D;
	//vector < vector<float> > raw_data;
	//
	//int num_of_begin_hour;
	//int begin_hour;
	//int num_of_end_hour;
	//int end_hour;
	//vector<vector2> hour_range;
	//vector<int> hour_index;

	//int num_of_five_minutes;

	//char file_position[100];
	//char file_RGB[100];
	//char file_histo[100];
	//char file_hour[100];
	//char file_cluster[100];	
	//char file_raw_data_3D[100];
	//char file_raw_data[100];

	//void read_all_csv();

private:
	char file_csv_data[200];
	//void read_csv_position();
	//void read_csv_histo();
	//void read_csv_color();
	//void read_csv_time();
	//void read_csv_clustercenter();
	//void read_csv_raw_data_3D();
	//void read_csv_raw_data();
	//
	//map<int,int> hour_map;
	void read_single_file(char*);
	
	
	
};