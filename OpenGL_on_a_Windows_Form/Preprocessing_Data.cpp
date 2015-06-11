#include "stdafx.h"
#include "Preprocessing_Data.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include "tapkee/tapkee.hpp"
#include "tapkee/callbacks/dummy_callbacks.hpp"
#include <opencv2/core/eigen.hpp> //cv2eigen
#include "mtxlib.h"//vector2
#include <vector>
#include <map>
#include <iostream>

//////boost
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random.hpp>
#include <boost/timer.hpp>
#include <boost/integer_traits.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/simple_point.hpp>
#include <boost/graph/metric_tsp_approx.hpp>
#include <boost/graph/graphviz.hpp>
//////
#include "tsp_brute.h"

using namespace tapkee;
using namespace Eigen;
using namespace boost;

extern void cuda_kmeans(Mat, int, Mat& , Mat&);

Preprocessing_Data::Preprocessing_Data()
{
	read_lab_csv();
}

void Preprocessing_Data::Initial_selection_flag(bool f1,bool f2,bool f3,bool f4)
{
	select_gravity = f1;
	select_linear_acc = f2;
	select_gyro = f3;
	select_distance = f4;	
}

void Preprocessing_Data::start2(vector<month> month_vec_read, int k)
{
	month_vec = month_vec_read;


	float max_data = -1000;
	day_amount = 0;
	for(int i=0;i<month_vec.size();i++)
	{
		//cout << "year " << month_vec[i].this_year << " " << " month " << month_vec[i].this_month << endl;
		for(int j=0;j<month_vec[i].day_vec.size();j++)
		{
			day_amount++;
			float data = month_vec[i].day_vec[j].data[0];
			if(data > max_data)
			{
				max_data = data;
			}
			//cout << month_vec[i].day_vec[j].month << " " << month_vec[i].day_vec[j].date << " ";
			//cout << month_vec[i].day_vec[j].data[0] << " " << month_vec[i].day_vec[j].data[1] << endl;
		}
		//cout << endl;
	}

	//cout << "max data " << max_data << endl;
	int digit = count_digit(max_data);
	//cout << "digit " << digit << endl;


	//int dim = month_vec[0].day_vec[0].data_dim;
	int dim = 1;
	Mat model = Mat::zeros(day_amount,dim,CV_32F);
	int t = 0;
	for(int i=0;i<month_vec.size();i++)
	{
		for(int j=0;j<month_vec[i].day_vec.size();j++)
		{
			for(int u=0;u<dim;u++)
			{
				if(month_vec[i].day_vec[j].data[u] < 0) month_vec[i].day_vec[j].data[u] = 0.0;
				model.at<float>(t,u) = month_vec[i].day_vec[j].data[u]/pow(10.0,digit-2);
				//model.at<float>(t,u) = month_vec[i].day_vec[j].data[u]/10.0;	
			}
			t++;
		}
	}

	output_mat_as_csv_file_float("model.csv",model);
	//============Setting matrix for K-means============//
	/*
	clock_t begin2 = clock();
	set_hour_data(time_title);
	Mat model = set_matrix(attribute_title,attribute_index.size()).clone();
	delete[] attribute_title;
	clock_t end2 = clock();
	printf("Setting matrix elapsed time: %f\n",double(end2 - begin2) / CLOCKS_PER_SEC);

	output_mat_as_csv_file("model.csv",model);
	*/
	//==============K means clustering with no speed up==================//
	
    //int k = 4; 
    Mat cluster_tag; //Tag:0~k-1
    int attempts = 2;//應該是執行次數
	Mat cluster_centers;
	//使用k means分群
	clock_t begin3 = clock();
	kmeans(model, k, cluster_tag,TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 100, 0.0001), attempts,KMEANS_PP_CENTERS,cluster_centers);
	clock_t end3 = clock();
    //TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 10, 1),  這裡有三個參數，決定k-means何時結束，第二個參數是指迭代最大次數，第三個參數是精確度多少，第一個參數是指依照前兩個參數的哪一個為準，以範例中就是兩者都參照，以 or 的方式決定
	//printf("Kmeans (K = %d) elapsed time: %f\n",k,double(end3 - begin3) / CLOCKS_PER_SEC);
	
	//================K means clustering with cuda=====================//
	//int k = Find_Cluster_By_Elbow_Method(model) + 12;
	//cout << "Find_Cluster_By_Elbow_Method " << Find_Cluster_By_Elbow_Method(model) << endl;
	/*
	int k = 8;
	Mat cluster_tag = Mat::zeros(model.rows,1,CV_32S);
	Mat cluster_centers = Mat::zeros(k,model.cols,CV_32F);

	clock_t begin3 = clock();
	cuda_kmeans(model, k, cluster_tag, cluster_centers);
	clock_t end3 = clock();
	printf("Kmeans (K = %d) elapsed time: %f\n",k,double(end3 - begin3) / CLOCKS_PER_SEC);
	*/
	output_mat_as_csv_file_float("cluster_centers_old.csv",cluster_centers);

	//=================LAB alignment====================//
	
	rgb_mat3 = Mat::zeros(k,3,CV_32F);
	//if(cluster_centers.cols>=3) rgb_mat3 = lab_alignment(cluster_centers);
	if(cluster_centers.cols>=3) rgb_mat3 = lab_alignment_new(cluster_centers,30).clone();
	else if(cluster_centers.cols==1) rgb_mat3 = lab_alignment_dim1(cluster_centers,30).clone();
	else if(cluster_centers.cols==2) rgb_mat3 = lab_alignment_dim2 (cluster_centers,30).clone();


	output_mat_as_csv_file_float("rgb_mat_old.csv",rgb_mat3);
	
	//=============TSP for lab color================//
	
	//TSP_for_lab_color(cluster_centers);
	Mat lab_color_sort_index = Mat::zeros(k,1,CV_32S);
	//TSP_boost_for_lab_color(lab,lab_color_sort_index);
	//TSP_boost_for_lab_color_coarse_to_fine(lab,lab_color_sort_index);
	//TSP_boost_for_lab_color_coarse_to_fine3(lab,lab_color_sort_index);
	//sort_pattern_by_color_by_TSP_coarse_to_fine(lab,lab_color_sort_index);
	tsp_brute tsp;
	tsp.tsp_path(lab,lab_color_sort_index);

	output_mat_as_csv_file_int("lab_color_sort_index.csv",lab_color_sort_index);

	//====================TSP Brute======================//
	//Mat TSP_brute_sort_index;
	//tsp_brute tsp;
	//tsp.start(lab,0,5,TSP_brute_sort_index);
	//tsp.start(lab,TSP_brute_sort_index);
	//====================sort pattern by color========================//
	
	//sort the cluster by color and generate new cluster tag and cluster center

	//sort_by_color_by_TSP(lab_color_sort_index,cluster_centers,cluster_tag,rgb_mat2);
	rearrange_mat_by_sort_color_index(lab_color_sort_index,cluster_centers,cluster_tag,rgb_mat3);



	//sort_by_color(k,rgb_mat2,cluster_centers,cluster_tag);
	output_mat_as_csv_file_float("rgb_mat_sort.csv",rgb_mat3);
	output_mat_as_csv_file_float("cluster_center_sort.csv",cluster_centers);

	
	voting_for_data(month_vec,k,cluster_tag);
	output_mat_as_csv_file_int("histogram.csv",histogram);
	//=======================voting=================//
	/*
	clock_t begin7 = clock();
	voting(k,cluster_tag,model.rows); // Type: int	
	output_mat_as_csv_file_int("histogram.csv",histogram);
	clock_t end7 = clock();
	printf("Histogram voting elapsed time: %f\n",double(end7 - begin7) / CLOCKS_PER_SEC);
	*/
	//===============Position (MDS)=====================//
	/*
	clock_t begin6 = clock();
	position = Position_by_MDS(cluster_centers,model,cluster_tag,k).clone(); //Type:double
	cluster_tag.release();
	output_mat_as_csv_file_double("position.csv",position);
	clock_t end6 = clock();
	printf("MDS Position elapsed time: %f\n",double(end6 - begin6) / CLOCKS_PER_SEC);
	*/
	//===============Position (neighbor distance)=====================//
	
	Mat histo_sort_index = Mat::zeros(histogram.rows,1,CV_32S); 
	//TSP_boost_for_histogram(cluster_centers,histo_sort_index);
	sort_histogram_by_Ev_by_TSP_coarse_to_fine(cluster_centers,histo_sort_index);
	output_mat_as_csv_file_int("histo_sort_index.csv",histo_sort_index);
	
	
	Mat histo_position = Mat::zeros(histogram.rows,1,CV_64F);
	Position_by_histogram_sort_index(histo_position,histo_sort_index);
	position = histo_position.clone();

	output_mat_as_csv_file_double("position.csv",position);
	
	/*
	clock_t begin9 = clock();
	Mat histo_position = Mat::zeros(histogram.rows,1,CV_64F);
	Position_by_histogram(histo_position,cluster_centers);
	position = histo_position.clone();
	clock_t end9 = clock();
	printf("Histogram neighbor distance elapsed time: %f\n",double(end9 - begin9) / CLOCKS_PER_SEC);	
	*/
	//======================raw data 3D color by lab alignment===================//
	
	if(model.cols>=3) raw_data_3D = lab_alignment(model,30).clone();
	else if(model.cols==2) raw_data_3D = lab_alignment_dim2(model,30).clone();
	else if(model.cols==1) raw_data_3D = lab_alignment_dim1(model,30).clone();	

	raw_data_3D_array = new Mat[month_vec.size()];
	for(int i=0;i<month_vec.size();i++)
	{
		raw_data_3D_array[i] = Mat::zeros(month_vec[i].day_vec.size(),3,CV_32F);
	}

	t = 0;
	for(int i=0;i<month_vec.size();i++)
	{
		for(int j=0;j<month_vec[i].day_vec.size();j++)
		{
			for(int u=0;u<3;u++)
			{
				raw_data_3D_array[i].at<float>(j,u) = raw_data_3D.at<float>(t,u); 
			}
			t++;
		}
	}

	cluster_centers.release();
	model.release();


}

void Preprocessing_Data::start(vector < vector<float> > raw_data,vector<int> attribute_index,int time_index,int k)
{
	//int attribute_title_size = 11;
	//int attribute_title[] = {4,5,6,7,8,9,10,11,12,22,23};//(gravity_x,gravity_y,gravity_z),(linear_acc_x,linear_acc_y,linear_acc_z),(gyro_x,gyro_y,gyro_z),(latitude,longitude)
	//int time_title[] = {29,30,31,32,33};//hour(30),minute(31)
	//=====================get Attribute title & time index from Read_CSV=====================//
	int* attribute_title = new int[attribute_index.size()];
	for(int i=0;i<attribute_index.size();i++)
	{
		attribute_title[i] = attribute_index[i] + 1;
	}
	
	//int time_title[] = {29,30,31,32,33};//hour(30),minute(31)
	int time_title[5];
	for(int i=0;i<5;i++)
	{
		time_title[i] = time_index + i + 1;
	}		
	//============Setting matrix for K-means============//
	set_hour_data(raw_data,time_title);
	Mat model = set_matrix(raw_data,attribute_title,attribute_index.size()).clone();
	delete[] attribute_title;
	output_mat_as_csv_file_float("model.csv",model);
	//==============K means clustering==================//
    //Mat cluster_tag; //Tag:0~k-1
	//Mat cluster_centers;
	int attempts = 2;//應該是執行次數
	//使用k means分群
	Mat cluster_tag = Mat::zeros(model.rows,1,CV_32S);
	Mat cluster_centers = Mat::zeros(k,model.cols,CV_32F);
	cuda_kmeans(model, k, cluster_tag, cluster_centers);
	//kmeans(model, k, cluster_tag,TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 100, 0.0001), attempts,KMEANS_PP_CENTERS,cluster_centers);
    //TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 10, 1),  這裡有三個參數，決定k-means何時結束，第二個參數是指迭代最大次數，第三個參數是精確度多少，第一個參數是指依照前兩個參數的哪一個為準，以範例中就是兩者都參照，以 or 的方式決定
	output_mat_as_csv_file_float("cluster_centers_old.csv",cluster_centers);
	//===============LAB alignment======================//
	rgb_mat3 = Mat::zeros(k,3,CV_32F);
	lab = Mat::zeros(k,3,CV_32F);
	if(model.cols>=3) rgb_mat3 = lab_alignment_new(cluster_centers,30).clone();
	else if(model.cols==2) rgb_mat3 = lab_alignment_dim2(cluster_centers,30).clone();
	else if(model.cols==1) rgb_mat3 = lab_alignment_dim1(cluster_centers,30).clone();

	output_mat_as_csv_file_float("rgb_mat_old.csv",rgb_mat3);
	//==================================================//
	//sort the cluster by color and generate new cluster tag and cluster center
	//sort_by_color(k,rgb_mat3,cluster_centers,cluster_tag);

	Mat lab_color_sort_index = Mat::zeros(k,1,CV_32S);
	sort_pattern_by_color_by_TSP_coarse_to_fine(lab,lab_color_sort_index);
	output_mat_as_csv_file_int("lab_color_sort_index.csv",lab_color_sort_index);

	rearrange_mat_by_sort_color_index(lab_color_sort_index,cluster_centers,cluster_tag,rgb_mat3);
	output_mat_as_csv_file_float("rgb_mat_sort.csv",rgb_mat3);
	output_mat_as_csv_file_float("cluster_center_sort.csv",cluster_centers);
	//output_mat_as_csv_file_int("cluster_tag.csv",cluster_tag);
	//===============Voting (Generate Histogram)========//
	voting(k,cluster_tag,model.rows); // Type: int
	output_mat_as_csv_file_int("histogram.csv",histogram);
	cluster_tag.release();
	//===============Position (TSP)=====================//
	Mat histo_sort_index = Mat::zeros(histogram.rows,1,CV_32S); 
	sort_histogram_by_Ev_by_TSP_coarse_to_fine(cluster_centers,histo_sort_index);
	output_mat_as_csv_file_int("histo_sort_index.csv",histo_sort_index);
	
	Mat histo_position = Mat::zeros(histogram.rows,1,CV_64F);
	Position_by_histogram_sort_index(histo_position,histo_sort_index);
	position = histo_position.clone();
	
	output_mat_as_csv_file_double("position.csv",position);
	//===============Position (MDS)=====================//
	//position = Position_by_MDS(cluster_centers,k,20).clone(); //Type:double
	
	//===================PCA raw data 3 dim=======================//
	if(model.cols>=3) raw_data_3D = lab_alignment(model,30).clone();
	else if(model.cols==2) raw_data_3D = lab_alignment_dim2(model,30).clone();
	else if(model.cols==1) raw_data_3D = lab_alignment_dim1(model,30).clone();
	
	cluster_centers.release();
	model.release();	
}
    
void Preprocessing_Data::voting_for_data(vector<month> month_vec, int k, Mat cluster_tag)
{
	histogram = Mat::zeros(month_vec.size(),k,CV_32S);
	int t = 0;
	for(int i=0;i<month_vec.size();i++)
	{
		for(int j=0;j<month_vec[i].day_vec.size();j++)
		{
			int tag = cluster_tag.at<int>(t++,0);
			histogram.at<int>(i,tag)++;
		}

	}
}

int Preprocessing_Data::count_digit(float num)
{
	int digit = 0;
	int num_int = num;
	while(num_int>0)
	{
		num_int/=10;
		digit++;
	}

	return digit;
}

void Preprocessing_Data::sort_by_color(int k,Mat& rgb_mat2,Mat& cluster_centers, Mat& cluster_tag)
{
	vector< vector<float> > rgb_vector;
	for(int i=0;i<k;i++)
	{
		vector<float> rgb;
		for(int j=0;j<3;j++)
		{
			rgb.push_back(rgb_mat2.at<float>(i,j));
		}
		rgb_vector.push_back(rgb);
	}

	class cluster_info{
	public:
		int key;
		vector<float> rgb_vec;
		
		cluster_info(int k,vector<float> rgb){
			key = k;
			rgb_vec = rgb;
		} 
	};
	class sort_by_rgb{
	public:
		inline bool operator() (cluster_info& c1, cluster_info& c2)
		{
			float R1 = c1.rgb_vec[0];
			float G1 = c1.rgb_vec[1];
			float B1 = c1.rgb_vec[2];
			float R2 = c2.rgb_vec[0];
			float G2 = c2.rgb_vec[1];
			float B2 = c2.rgb_vec[2];
			Mat rgb_color1(1, 1, CV_32FC3);
			Mat rgb_color2(1, 1, CV_32FC3);
			Mat hsv_color1(1, 1, CV_32FC3);
			Mat hsv_color2(1, 1, CV_32FC3);
			rgb_color1.at<Vec3f>(0,0) = Vec3f(R1,G1,B1);
			rgb_color2.at<Vec3f>(0,0) = Vec3f(R2,G2,B2);
			cvtColor(rgb_color1,hsv_color1,CV_RGB2HLS);
			cvtColor(rgb_color2,hsv_color2,CV_RGB2HLS);
			//float intensity1 = 0.2126*R1 + 0.7152*G1 + 0.0722*B1;
			//float intensity2 = 0.2126*R2 + 0.7152*G2 + 0.0722*B2;
			//return (intensity1 < intensity2);
			
			//return ( R1*256*256 + G1*256 + B1 < R2*256*256 + G2*256 + B2 );
			float H1 = hsv_color1.at<Vec3f>(0,0).val[0];
			float H2 = hsv_color2.at<Vec3f>(0,0).val[0];
			float L1 = hsv_color1.at<Vec3f>(0,0).val[1];
			float L2 = hsv_color2.at<Vec3f>(0,0).val[1];
			float S1 = hsv_color1.at<Vec3f>(0,0).val[2];
			float S2 = hsv_color2.at<Vec3f>(0,0).val[2];
			return (H1<H2 || (H1==H2 && L1>L2) || (H1==H2 && L1==L2 && S1>S2) );

			//return ( R1 > R2 || (R1 == R2 && G1 > G2) || (R1 == R2 && G1 == G2 && B1 > B2) );
		}
	};

	vector< cluster_info > cluster_vec;
	for(int i=0;i<k;i++)
	{
		cluster_vec.push_back( cluster_info(i,rgb_vector[i]) );
	}

	sort(cluster_vec.begin(), cluster_vec.end(), sort_by_rgb());

	//for(int i=0;i<cluster_vec.size();i++)
	//{
	//	cout << cluster_vec[i].key << " ";
	//}
	
	Mat cluster_centers_old = cluster_centers.clone();
	Mat rgb_mat2_old = rgb_mat2.clone();
	Mat cluster_tag_old = cluster_tag.clone();
	for(int i=0;i<k;i++)
	{
		int new_tag = cluster_vec[i].key;
		cluster_centers_old.row(new_tag).copyTo(cluster_centers.row(i));
		rgb_mat2_old.row(new_tag).copyTo(rgb_mat2.row(i));
		//注意:row的複製不能用rgb_mat2.row(i) = rgb_mat2_old.row(new_tag).clone();!!!!!!!
	}
	for(int i=0;i<cluster_tag_old.rows;i++)
	{
		int find;
		for(int j=0;j<k;j++)
		{
			if(cluster_vec[j].key == cluster_tag_old.at<int>(i,0))
			{
				find = j;
				break;
			}
		}
		cluster_tag.at<int>(i,0) = find;	
	}

	rgb_vector.clear();
}

void Preprocessing_Data::interpolate_distance(Mat& first_order_distance_mat,int interval)
{
	//int interval = 10;
	for(int i=0;i<first_order_distance_mat.cols-1;i++)
	{
		int start = i;
		int cur_index = i;
		int count = 0;
		if(first_order_distance_mat.at<float>(0,cur_index) != 0.0)
		{
			//cout << "i " << i << endl;
			cur_index++;
			while( first_order_distance_mat.at<float>(0,cur_index) == 0.0)
			{
				count++;
				cur_index++;
				//cout << "count " << count << " " << "current index " << cur_index << endl;
				if(count>interval) break;
			}
			if(count>0 && count<=interval)
			{
				//cout << "start " << first_order_distance_mat.at<float>(0,start) << " end " << first_order_distance_mat.at<float>(0,cur_index) << endl;
				float interpolation = ( first_order_distance_mat.at<float>(0,start) + first_order_distance_mat.at<float>(0,cur_index) ) / (count+2);
				//cout << "interpolation " << interpolation << endl;
				for(int j=0;j<count+2;j++)
				{
					first_order_distance_mat.at<float>(0,start+j) = interpolation;
				}
				//system("pause");
			}
		}
		//i = cur_index;
	}
}

/**
 * 從data計算平均與covariance matrix
 * http://en.wikipedia.org/wiki/Covariance_matrix#Definition
 *
 */
void Preprocessing_Data::calcCovMat(Mat& data, Mat& mean, Mat& cov){
    // 初始化
	cov = Mat::zeros(data.cols, data.cols, CV_32F);

	// 計算資料點的重心(平均)
	mean = Mat::zeros(1, data.cols, CV_32F);
	for (int i = 0; i < data.rows; i++){
		mean += data.row(i);
	}
	mean /= double(data.rows);

	// 計算covariance matrix
	for (int i = 0; i < data.rows; i++){
		cov += (data.row(i) - mean).t() * (data.row(i) - mean);
	}
	cov /= double(data.rows);
}



/**
 * 用Principal Component Analysis (PCA) 做降維
 *
 */
void Preprocessing_Data::reduceDimPCA(Mat& data, int rDim, Mat& components, Mat& result){
	// 計算covariance matrix
	Mat cov, mean;
	calcCovMat(data, mean, cov);

	// 從covariance matrix計算eigenvectors
	// http://docs.opencv.org/modules/core/doc/operations_on_arrays.html?highlight=pca#eigen
	Mat eigenVal, eigenVec;
	eigen(cov, eigenVal, eigenVec);

	// 記錄前rDim個principal components
	components = Mat(rDim, data.cols, CV_32F);
	for (int i = 0; i < rDim; i++){
		// http://docs.opencv.org/modules/core/doc/basic_structures.html?highlight=mat%20row#mat-row
		eigenVec.row(i).copyTo(components.row(i));

		// http://docs.opencv.org/modules/core/doc/operations_on_arrays.html?highlight=normalize#normalize
		normalize(components.row(i), components.row(i));
	}

	// 計算結果
	result = Mat(data.rows, rDim, CV_32F);
	for (int i = 0; i < data.rows; i++){
		for (int j = 0; j < rDim; j++){
			// 內積(投影到principal component上)
			// http://docs.opencv.org/modules/core/doc/basic_structures.html?highlight=dot#mat-dot
			result.at<float>(i, j) = (data.row(i) - mean).dot(components.row(j));
		}
	}
}


float Preprocessing_Data::degtorad(float deg)
{
	float rad = deg *3.14159265 / 180;
	return rad;
}

float Preprocessing_Data::norm_value(float v1,float v2,float v3)
{
	return sqrt(v1*v1 + v2*v2 + v3*v3);
}

float Preprocessing_Data::DistanceOfLontitudeAndLatitude(float lat1,float lat2,float lon1,float lon2)
{
	float R = 6371; //km
	float theta1 = degtorad(lat1);
	float theta2 = degtorad(lat2);
	float delta_theta = degtorad(lat2-lat1);
	float delta_lumda = degtorad(lon2-lon1);
	float a = sin(delta_theta/2) * sin(delta_theta/2) + cos(theta1) * cos(theta2) * sin(delta_lumda/2) * sin(delta_lumda/2);
	float c = 2 * atan2((double)sqrt(a),(double)sqrt(1.0-a));
	float d = R * c;

	return d;
}

void Preprocessing_Data::set_hour_data(vector < vector<float> > raw_data,int time_title[])
{
	int hour_id = time_title[1];
	int time_step_amount = floor(raw_data.size()/600.0);
	num_of_five_minutes = time_step_amount;
	hour_data.resize(time_step_amount);
	hour_index.clear();
	hour_range.clear();
	hour_map.clear();
	int t = 0;
	for(int i=0;i<time_step_amount;i++)
	{
		hour_data[i] = raw_data[t][hour_id]; 
		t += 600;
	}
	//////////////////////////////////////////////////////////////////
	//num_of_five_minutes = hour_data.size()-1;
	int num_of_end_hour,end_hour;
	for(int i=0;i<hour_data.size();i++)
	{
		hour_map[hour_data[i]]++;
	}

	map<int,int>::iterator it;
	int start = 0;
	int hour_num;
	for(it = hour_map.begin(); it!=hour_map.end(); ++it)
	{
		hour_num = 11;
		if(it == hour_map.begin())
		{
			begin_hour = it->first;
			num_of_begin_hour = it->second;
			hour_num = num_of_begin_hour - 1;
		}
		else if(next(it,1)==hour_map.end())
		{
			end_hour = it->first;
			num_of_end_hour = it->second;
			hour_num = num_of_end_hour - 1 - 1;
		}

		vector2 temp;
		temp.x = start;
		temp.y = start + hour_num;
		hour_range.push_back(temp);
		hour_index.push_back(it->first);
		
		start += (hour_num+1);
		
	}
}

Mat Preprocessing_Data::Gaussian_filter(vector < vector<float> > raw_data,int attribute_title[],int KERNEL_LENGTH)
{
	Mat Gaussian_filter_mat(raw_data.size(),9, CV_32F);

	//Apply Gaussian filter to raw data(0~8)
	for(int i=0;i<raw_data.size();i++)
	{
		for(int j=0;j<9;j++)
		{
			Gaussian_filter_mat.at<float>(i,j) = raw_data[i][attribute_title[j]];
		}
	}

	//int MAX_KERNEL_LENGTH = 20;
    
	//for(int i=1;i<MAX_KERNEL_LENGTH;i=i+2)
    //{ 
	//	GaussianBlur( Gaussian_filter_mat, Gaussian_filter_mat, Size( i, i ), 0.5, 0.5 );
	//}
	

	int MAX_KERNEL_LENGTH = 5;
	for(int j=0;j<Gaussian_filter_mat.cols;j++)
	{
		GaussianBlur( Gaussian_filter_mat.col(j), Gaussian_filter_mat.col(j), Size(MAX_KERNEL_LENGTH, 1), 0.1, 0.1);
	}

	//GaussianBlur( Gaussian_filter_mat, Gaussian_filter_mat, Size( MAX_KERNEL_LENGTH, MAX_KERNEL_LENGTH ), 0, 0 );
	
	/*
	for(int j=0;j<Gaussian_filter_mat.cols;j++)
	{
		for(int i=1;i<MAX_KERNEL_LENGTH;i=i+2)
		{ 
			GaussianBlur( Gaussian_filter_mat.col(j), Gaussian_filter_mat.col(j), Size( i, i ), 0, 0);
		}
	}
	*/
	return Gaussian_filter_mat;
}

Mat Preprocessing_Data::set_matrix(vector < vector<float> > raw_data,int attribute_title[],int attribute_title_size)
{
	Mat handle_mat;
	Mat handle_mat_raw;

	Mat Gaussian_filter_mat = Gaussian_filter(raw_data,attribute_title,3).clone();

	Mat norm_gravity(1, raw_data.size(), CV_32F);
	Mat norm_linear_acc(1, raw_data.size(), CV_32F);
	Mat norm_gyro(1, raw_data.size(), CV_32F);
	//Compute norm
	for(int i=0;i<raw_data.size();i++)
	{
		norm_gravity.at<float>(0,i) = norm_value(Gaussian_filter_mat.at<float>(i,0),Gaussian_filter_mat.at<float>(i,1),Gaussian_filter_mat.at<float>(i,2));
		norm_linear_acc.at<float>(0,i) = norm_value(Gaussian_filter_mat.at<float>(i,3),Gaussian_filter_mat.at<float>(i,4),Gaussian_filter_mat.at<float>(i,5));
		norm_gyro.at<float>(0,i) = norm_value(Gaussian_filter_mat.at<float>(i,6),Gaussian_filter_mat.at<float>(i,7),Gaussian_filter_mat.at<float>(i,8));
	}
	//for(int i=0;i<raw_data.size();i++)
	//{
	//	norm_gravity.at<float>(0,i) = norm_value(raw_data[i][attribute_title[0]],raw_data[i][attribute_title[1]],raw_data[i][attribute_title[2]]);
	//	norm_linear_acc.at<float>(0,i) = norm_value(raw_data[i][attribute_title[3]],raw_data[i][attribute_title[4]],raw_data[i][attribute_title[5]]);
	//	norm_gyro.at<float>(0,i) = norm_value(raw_data[i][attribute_title[6]],raw_data[i][attribute_title[7]],raw_data[i][attribute_title[8]]);
	//}
	
	handle_mat_raw.push_back(norm_gravity);
	if(select_gravity)
	{
		handle_mat.push_back(norm_gravity);	
	}
	handle_mat_raw.push_back(norm_linear_acc);
	if(select_linear_acc)
	{
		handle_mat.push_back(norm_linear_acc);
	}
	handle_mat_raw.push_back(norm_gyro);
	if(select_gyro)
	{
		handle_mat.push_back(norm_gyro);	
	}
	//Compute latitude & longitude
	int lat_index = attribute_title[9];
	int lon_index = attribute_title[10];
	Mat first_order_distance_mat(1, raw_data.size(), CV_32F);
	for(int i=0;i<raw_data.size();i++)
	{
		if(i==0)
			first_order_distance_mat.at<float>(0,i) = 0.0;
		else
		{
			float dist = DistanceOfLontitudeAndLatitude(raw_data[i-1][lat_index],raw_data[i][lat_index],raw_data[i-1][lon_index],raw_data[i][lon_index]);
			first_order_distance_mat.at<float>(0,i) = dist;
		}
	}

	output_mat_as_csv_file_float("first_order_distance_mat.csv",first_order_distance_mat.t());
	interpolate_distance(first_order_distance_mat,3000);
	output_mat_as_csv_file_float("first_order_distance_mat_new.csv",first_order_distance_mat.t());

	Mat first_order_distance_adjust_mat(1, raw_data.size(), CV_32F);
	for(int i=0;i<raw_data.size();i++)
	{
		float d = first_order_distance_mat.at<float>(0,i);
		float d1;
		if(d==0.0 || d<1.0e-4) 
			d1 = 0.0;
		else if(d>0.05)//threshold:0.044km/sample=315km/hr
			d1 = 0.05;
		else if(d!=0.0)	
		{
			d1 = log(d);
		}
		first_order_distance_adjust_mat.at<float>(0,i) = d1;
	}
	
	//interpolate_distance(first_order_distance_adjust_mat,100);

	double min, max;
	minMaxLoc(first_order_distance_adjust_mat, &min, &max);
	for(int i=0;i<first_order_distance_adjust_mat.cols;i++)
	{
		if(first_order_distance_adjust_mat.at<float>(0,i) != 0)
			first_order_distance_adjust_mat.at<float>(0,i) -= min;
	}

	handle_mat_raw.push_back(first_order_distance_adjust_mat);/////////////////////////////
	if(select_distance)
		handle_mat.push_back(first_order_distance_adjust_mat);	
	
	Mat handle_mat_transpose = handle_mat.t();
	Mat handle_mat_raw_transpose = handle_mat_raw.t();
	handle_mat.release();
	handle_mat_raw.release();

	raw_data_mat = handle_mat_raw_transpose.clone();//////////////////////////


	Mat normalize_mat = handle_mat_transpose;
	for(int i=0;i<handle_mat_transpose.cols;i++)
		//normalize_mat.col(i) = normalize_column(handle_mat_transpose.col(i)).clone();
		normalize(handle_mat_transpose.col(i),normalize_mat.col(i),0,1,NORM_MINMAX);

	//output_mat_as_csv_file("normalize_mat.csv",normalize_mat);

	//raw_data_mat = normalize_mat.clone();

	return normalize_mat;

}

void Preprocessing_Data::voting(int k,Mat cluster_tag,int row_size)
{
	int time_step_amount = floor(row_size/600.0);
	histogram = Mat::zeros(time_step_amount,k,CV_32S);
	int t = 0;
	for(int i=0;i<time_step_amount;i++)
	{
		for(int j=0;j<600;j++)
		{
			int index = cluster_tag.at<int>(t,0);
			histogram.at<int>(i,index)++;
			t++;
		}
	}

}

Mat Preprocessing_Data::MDS(Mat target_mat, int dim)
{
	Matrix<double,Dynamic,Dynamic> target_mat_EigenType;//The type pass to Tapkee must be "double" not "float"
	cv2eigen(target_mat,target_mat_EigenType);
	TapkeeOutput output = tapkee::initialize() 
						   .withParameters((method=MultidimensionalScaling,target_dimension=1))
						   .embedUsing(target_mat_EigenType);
	
	Mat MDS_mat; //Type:double  
	eigen2cv(output.embedding,MDS_mat);
	normalize(MDS_mat.col(0),MDS_mat.col(0),1,5000,NORM_MINMAX);//normalize to 1~1000	
	//MDS_mat = MDS_mat.mul(50.0);

	return MDS_mat;
}

void Preprocessing_Data::distance_by_GMM(Mat& histo_coeff, Mat& Ev, Mat cluster_centers, int k)
{
	//GMM(Gaussian Mixutre Model)
	int time_step_amount = Ev.rows;
	for(int i=0;i<time_step_amount;i++)
	{
		for(int j=0;j<k;j++)
		{
			//Ev.row(i) += adjust_weight[i][j]*cluster_centers.row(j);
			Ev.row(i) += (histogram.at<int>(i,j)/600.0)*cluster_centers.row(j);
		}
	}
	output_mat_as_csv_file_float("Ev.csv",Ev);	

	for(int i=0;i<time_step_amount;i++)
	{
		for(int j=0;j<time_step_amount;j++)
		{
			if(i==j) continue;
			else if(i>j)
			{
				histo_coeff.at<double>(i,j) = histo_coeff.at<double>(j,i);
			}
			else
			{
				for(int t=0;t<cluster_centers.cols;t++)
				{
					histo_coeff.at<double>(i,j) += abs(Ev.at<float>(i,t)-Ev.at<float>(j,t));
					//histo_coeff.at<double>(i,j) += sqrt( Ev.at<float>(i,t) * Ev.at<float>(j,t) );
				}
				//histo_coeff.at<double>(i,j) = sqrt( 1.0 - histo_coeff.at<double>(i,j) );
			}
		}
	}
}

Mat Preprocessing_Data::Position_by_MDS(Mat cluster_centers,int k,float larger_weighting)
{
	int time_step_amount = histogram.rows;
	Mat histo_coeff = Mat::zeros(time_step_amount,time_step_amount,CV_64F);

	//==================GMM(Gaussian Mixture Model)======================//
	Mat Ev = Mat::zeros(time_step_amount,cluster_centers.cols,CV_32F);
	distance_by_GMM(histo_coeff,Ev,cluster_centers,k);

	output_mat_as_csv_file_double("histo_coeff.csv",histo_coeff);

	
	Mat position_mat = MDS(histo_coeff,1).clone();

	return position_mat;
}

Mat Preprocessing_Data::lab_alignment(Mat cluster_center,int luminance_threshold)
{   
	int vTotal = lab_vertices.size();
	//Turn LAB vector into LAB mat
	Mat lab_mat = Mat::zeros(vTotal,3,CV_32F);
	for(int i=0;i<vTotal;i++)
	{
		for(int j=0;j<3;j++)
		{
			lab_mat.at<float>(i,j) = lab_vertices[i][j+1];
		}
	}

	//Compute centroid of cluster center
	Mat cluster_center_centroid = compute_centroid(cluster_center).clone();

	//Align the centroid to the origin by subtract all points to centroid
	Mat cluster_center_alignment_mat = Mat::zeros(cluster_center.rows,cluster_center.cols,CV_32F);
	for(int i=0;i<cluster_center.rows;i++)
	{
		for(int j=0;j<cluster_center.cols;j++)
		{
			cluster_center_alignment_mat.at<float>(i,j) = cluster_center.at<float>(i,j) - cluster_center_centroid.at<float>(0,j);
		}
	}
	Mat cluster_center_component,cluster_center_PCA;
	Mat garbage;
	int rDim = 3;
	reduceDimPCA(cluster_center_alignment_mat, rDim, cluster_center_component, cluster_center_PCA); //PCA 4dim->3dim (for dimension reduction)
	reduceDimPCA(cluster_center_PCA, rDim, cluster_center_component, garbage); //PCA 3dim->3dim (for principal component)
	//cluster center 3 axis of PCA
	Mat cluster_center_axis = Mat::zeros(3,3,CV_32F);
	cluster_center_axis = cluster_center_component;

	//compute centroid of LAB
	Mat lab_centroid = compute_centroid(lab_mat);
	
	//lab vertices 3 axis of PCA
	Mat lab_components,lab_PCA;
	rDim = 3;
	reduceDimPCA(lab_mat, rDim, lab_components, lab_PCA); //PCA 3dim->3dim (for principal component)
	Mat lab_axis = Mat::zeros(3,3,CV_32F);
	lab_axis = lab_components;
	
	//////////////////////////////////////////////////////////////////
	Mat cluster_center_PCA_const = cluster_center_PCA;
	vector<float> move_vector;
	for(float k=-0.5;k<=0.5;k+=0.1)
		move_vector.push_back(k);

	float max_move = 0.0;
	float max_scale = 0.0;
	Mat align_mat;
	Mat max_align_mat = cluster_center_PCA;
	int start = 1;
	//int luminance_threshold = 30;
	vector<int> scale_vector;

	//output_mat_as_csv_file_float("cluster_center.csv",cluster_center_PCA);
	//binary search the best scale & convell hull for speed up
	for(int t=0;t<move_vector.size();t++)
	{	
		for(int i=start;i<=150;i++)
			scale_vector.push_back(i);
		
		int low = start;
		int high = scale_vector.size();
		while(low <= high)
		{
			int mid = (low + high)/2; 
			Mat cluster_center_PCA_temp,cluster_center_PCA_weight,cluster_center_axis_invert;
			add(cluster_center_PCA_const,move_vector[t],cluster_center_PCA_temp); //move
			cluster_center_PCA_temp = cluster_center_PCA_temp.mul(mid); //scale
			cluster_center_axis_invert = cluster_center_axis.inv();
			cluster_center_PCA_weight = cluster_center_PCA_temp * cluster_center_axis_invert;

			align_mat = cluster_center_PCA_weight*lab_axis;

			//把重心平移回去
			for(int i=0;i<align_mat.rows;i++)
			{
				for(int j=0;j<3;j++)
				{
					align_mat.at<float>(i,j) += lab_centroid.at<float>(0,j);
				}
			}
	
			bool flag = true;
			for(int i=0;i<align_mat.rows;i++)
			{
				if(align_mat.at<float>(i,0)<luminance_threshold)
				{
					flag = false;
					break;
				}
			}

			if(flag)
			{
				for(int i=0;i<align_mat.rows;i++)
				{
					if(lab_boundary_test(align_mat.at<float>(i,0),align_mat.at<float>(i,1),align_mat.at<float>(i,2))==false)
					{
						flag = false;
						break;
					}
				}
			}

			if(high<=low)
				break;
			else if(flag)
				low = mid + 1;
			else 
				high = mid - 1;

		}

		if(low>max_scale)
		{
			max_scale = low;
			max_move = move_vector[t];
			max_align_mat = align_mat.clone();
			start = max_scale;
		}

		scale_vector.clear();
	}

	if(max_scale==0)
	{
		for(int i=0;i<max_align_mat.rows;i++)
		{
			for(int j=0;j<max_align_mat.cols;j++)
			{
				max_align_mat.at<float>(i,j) += lab_centroid.at<float>(0,j);
			}
		}
	}

	//printf("max_move : %f max_scale : %f\n",max_move,max_scale);
	//System::Windows::Forms::MessageBox::Show(max_move + " " + max_scale);

	//lab = max_align_mat.clone();

	Mat rgb_mat2 = LAB2RGB(max_align_mat).clone();

	move_vector.clear();
	scale_vector.clear();

	return rgb_mat2;
}

Mat Preprocessing_Data::lab_alignment_new(Mat cluster_center, int luminance_threshold)
{
	int vTotal = lab_vertices.size();
	//Turn LAB vector into LAB mat
	Mat lab_mat = Mat::zeros(vTotal,3,CV_32F);
	for(int i=0;i<vTotal;i++)
	{
		for(int j=0;j<3;j++)
		{
			lab_mat.at<float>(i,j) = lab_vertices[i][j+1];
		}
	}
	//Compute centroid of cluster center
	Mat cluster_center_centroid = compute_centroid(cluster_center).clone();

	//Align the centroid to the origin by subtract all points to centroid
	Mat cluster_center_alignment_mat = Mat::zeros(cluster_center.rows,cluster_center.cols,CV_32F);
	for(int i=0;i<cluster_center.rows;i++)
	{
		for(int j=0;j<cluster_center.cols;j++)
		{
			cluster_center_alignment_mat.at<float>(i,j) = cluster_center.at<float>(i,j) - cluster_center_centroid.at<float>(0,j);
		}
	}
	Mat component,cluster_center_coeff;
	Mat cluster_center_component = Mat::zeros(3,3,CV_32F);
	int rDim = 3;
	reduceDimPCA(cluster_center_alignment_mat, rDim, component, cluster_center_coeff);//PCA 4dim->3dim (for dimension reduction)
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
		{
			cluster_center_component.at<float>(i,j) = component.at<float>(i,j);
		}
	}
	cout << "cluster_center_component " << endl << cluster_center_component << endl;
	//cluster center 3 axis of PCA


	//output_mat_as_csv_file("cluster_center_coeff.csv",cluster_center_coeff);
	//compute centroid of LAB
	Mat lab_centroid = compute_centroid(lab_mat).clone();
	//Align the centroid to the origin by subtract all points to centroid
	Mat lab_alignment_mat = Mat::zeros(lab_mat.rows,lab_mat.cols,CV_32F);
	for(int i=0;i<lab_mat.rows;i++)
	{
		for(int j=0;j<lab_mat.cols;j++)
		{
			lab_alignment_mat.at<float>(i,j) = lab_mat.at<float>(i,j) - lab_centroid.at<float>(0,j);
		}
	}
	//lab vertices 3 axis of PCA
	Mat lab_components,lab_coeff;
	rDim = 3;
	reduceDimPCA(lab_alignment_mat, rDim, lab_components, lab_coeff); //PCA 3dim->3dim (for principal component)
	cout << "lab_components " << endl << lab_components << endl;

	//////////////////////////////////////////////////////////////////////////////////////////////
	vector<float> move_vector;
	for(float k=-5.0;k<=5.0;k+=1.0)
		move_vector.push_back(k);

	//vector<float> scale_vector;
	//for(float k=1.0;k<=100.0;k+=1.0)
	//	scale_vector.push_back(k);

	Mat align_mat = Mat::zeros(cluster_center.rows,3,CV_32F);
	Mat max_align_mat = Mat::zeros(cluster_center.rows,3,CV_32F);
	int start = 1;
	//int luminance_threshold = 30;

	Mat cluster_center_coeff_const = cluster_center_coeff.clone();
	float scale = 1.0;
	float max_scale = 1.0;
	float max_move_x = 0.0;
	float max_move_y = 0.0;
	float max_move_z = 0.0;
	bool flag = true;

	for(int u=0;u<move_vector.size();u++)
	{
		for(int v=0;v<move_vector.size();v++)
		{
			for(int w=0;w<move_vector.size();w++)
			{
				scale = max_scale;
				flag = true;
				while(flag)
				{			
						//if(scale>=150) break;
						cluster_center_coeff = cluster_center_coeff_const.mul(scale);
						//for(int i=0;i<cluster_center_coeff_const.rows;i++)
						//{
						//	cluster_center_coeff.at<float>(i,0) = cluster_center_coeff_const.at<float>(i,0) * scale_vector[u];
						//	cluster_center_coeff.at<float>(i,1) = cluster_center_coeff_const.at<float>(i,1) * scale_vector[v];
						//	cluster_center_coeff.at<float>(i,2) = cluster_center_coeff_const.at<float>(i,2) * scale_vector[w];
						//}

						for(int i=0;i<cluster_center.rows;i++)
						{
							for(int j=0;j<3;j++)
							{
								Mat result = cluster_center_coeff.row(i)*lab_components.col(j);
								align_mat.at<float>(i,j) = result.at<float>(0,0);
							}
						}
						add(align_mat.col(0),move_vector[u],align_mat.col(0)); //move
						add(align_mat.col(1),move_vector[v],align_mat.col(1)); //move
						add(align_mat.col(2),move_vector[w],align_mat.col(2)); //move
				
						//把重心平移回去
						for(int i=0;i<align_mat.rows;i++)
						{
							for(int j=0;j<3;j++)
							{
								align_mat.at<float>(i,j) += lab_centroid.at<float>(0,j);
							}
						}

						//cout << align_mat << endl;
						//system("pause");

						for(int i=0;i<align_mat.rows;i++)
						{
							if( (align_mat.at<float>(i,0)<luminance_threshold) || (align_mat.at<float>(i,0)>85.0) )
							{
								flag = false;
								break;
							}
						}

						if(flag)
						{
							for(int i=0;i<align_mat.rows;i++)
							{
								if(lab_boundary_test(align_mat.at<float>(i,0),align_mat.at<float>(i,1),align_mat.at<float>(i,2))==false)
								{
									flag = false;
									break;
								}
							}	
						}

						if(flag)
						{
							if(scale>max_scale)
							{
								max_scale = scale;
								max_move_x = move_vector[u];
								max_move_y = move_vector[v];
								max_move_z = move_vector[w];
								max_align_mat = align_mat.clone();
							}	
						}

						scale+=5.0;
				}
			}
		}
	}

	move_vector.clear();

	cout << "max scale " << max_scale << endl;
	cout << "max move x " << max_move_x << endl;
	cout << "max move y " << max_move_y << endl;
	cout << "max move z " << max_move_z << endl;

	//output_mat_as_csv_file("lab_raw_data.csv",max_align_mat);
	//printf("max_move : %f max_scale : %f\n",max_move,max_scale);
	
	lab = max_align_mat.clone();
	Mat rgb_mat2 = LAB2RGB(max_align_mat).clone();

	return rgb_mat2;
}

Mat Preprocessing_Data::lab_alignment_new_dim1(Mat cluster_center)
{
	int vTotal = lab_vertices.size();
	//Turn LAB vector into LAB mat
	Mat lab_mat = Mat::zeros(vTotal,3,CV_32F);
	for(int i=0;i<vTotal;i++)
	{
		for(int j=0;j<3;j++)
		{
			lab_mat.at<float>(i,j) = lab_vertices[i][j+1];
		}
	}
	//Compute centroid of cluster center
	Mat cluster_center_centroid = compute_centroid(cluster_center).clone();

	//Align the centroid to the origin by subtract all points to centroid
	Mat cluster_center_alignment_mat = Mat::zeros(cluster_center.rows,cluster_center.cols,CV_32F);
	for(int i=0;i<cluster_center.rows;i++)
	{
		for(int j=0;j<cluster_center.cols;j++)
		{
			cluster_center_alignment_mat.at<float>(i,j) = cluster_center.at<float>(i,j) - cluster_center_centroid.at<float>(0,j);
		}
	}
	Mat component,cluster_center_coeff;
	Mat cluster_center_component = Mat::zeros(1,3,CV_32F);
	int rDim = 1;
	reduceDimPCA(cluster_center_alignment_mat, rDim, component, cluster_center_coeff);//PCA 4dim->3dim (for dimension reduction)

	for(int j=0;j<cluster_center.cols;j++)
	{
		cluster_center_component.at<float>(0,j) = component.at<float>(0,j);
	}

	cout << "cluster_center_component " << endl << cluster_center_component << endl;
	//cluster center 3 axis of PCA


	//output_mat_as_csv_file("cluster_center_coeff.csv",cluster_center_coeff);
	//compute centroid of LAB
	Mat lab_centroid = compute_centroid(lab_mat).clone();
	//Align the centroid to the origin by subtract all points to centroid
	Mat lab_alignment_mat = Mat::zeros(lab_mat.rows,lab_mat.cols,CV_32F);
	for(int i=0;i<lab_mat.rows;i++)
	{
		for(int j=0;j<lab_mat.cols;j++)
		{
			lab_alignment_mat.at<float>(i,j) = lab_mat.at<float>(i,j) - lab_centroid.at<float>(0,j);
		}
	}
	//lab vertices 3 axis of PCA
	Mat lab_components,lab_coeff;
	rDim = 1;
	reduceDimPCA(lab_alignment_mat, rDim, lab_components, lab_coeff); //PCA 3dim->3dim (for principal component)
	cout << "lab_components " << endl << lab_components << endl;

	//////////////////////////////////////////////////////////////////////////////////////////////
	vector<float> move_vector;
	for(float k=-5.0;k<=5.0;k+=1.0)
		move_vector.push_back(k);

	//vector<float> scale_vector;
	//for(float k=1.0;k<=100.0;k+=1.0)
	//	scale_vector.push_back(k);

	Mat align_mat = Mat::zeros(cluster_center.rows,3,CV_32F);
	Mat max_align_mat = Mat::zeros(cluster_center.rows,3,CV_32F);
	int start = 1;
	int luminance_threshold = 20;

	Mat cluster_center_coeff_const = cluster_center_coeff.clone();
	float scale = 1.0;
	float max_scale = 1.0;
	float max_move_x = 0.0;
	float max_move_y = 0.0;
	float max_move_z = 0.0;
	bool flag = true;

	for(int u=0;u<move_vector.size();u++)
	{
		for(int v=0;v<move_vector.size();v++)
		{
			for(int w=0;w<move_vector.size();w++)
			{
				scale = max_scale;
				flag = true;
				while(flag)
				{			
						cluster_center_coeff = cluster_center_coeff_const.mul(scale);
						//for(int i=0;i<cluster_center_coeff_const.rows;i++)
						//{
						//	cluster_center_coeff.at<float>(i,0) = cluster_center_coeff_const.at<float>(i,0) * scale_vector[u];
						//	cluster_center_coeff.at<float>(i,1) = cluster_center_coeff_const.at<float>(i,1) * scale_vector[v];
						//	cluster_center_coeff.at<float>(i,2) = cluster_center_coeff_const.at<float>(i,2) * scale_vector[w];
						//}

						for(int i=0;i<cluster_center.rows;i++)
						{
							for(int j=0;j<3;j++)
							{
								Mat result = cluster_center_coeff.row(i)*lab_components.col(j);
								align_mat.at<float>(i,j) = result.at<float>(0,0);
							}
						}
						add(align_mat.col(0),move_vector[u],align_mat.col(0)); //move
						add(align_mat.col(1),move_vector[v],align_mat.col(1)); //move
						add(align_mat.col(2),move_vector[w],align_mat.col(2)); //move
				
						//把重心平移回去
						for(int i=0;i<align_mat.rows;i++)
						{
							for(int j=0;j<3;j++)
							{
								align_mat.at<float>(i,j) += lab_centroid.at<float>(0,j);
							}
						}

						//cout << align_mat << endl;
						//system("pause");

						for(int i=0;i<align_mat.rows;i++)
						{
							if( (align_mat.at<float>(i,0)<luminance_threshold) || (align_mat.at<float>(i,0)>75.0) )
							{
								flag = false;
								break;
							}
						}

						if(flag)
						{
							for(int i=0;i<align_mat.rows;i++)
							{
								if(lab_boundary_test(align_mat.at<float>(i,0),align_mat.at<float>(i,1),align_mat.at<float>(i,2))==false)
								{
									flag = false;
									break;
								}
							}	
						}

						if(flag)
						{
							if(scale>max_scale)
							{
								max_scale = scale;
								max_move_x = move_vector[u];
								max_move_y = move_vector[v];
								max_move_z = move_vector[w];
								max_align_mat = align_mat.clone();
							}	
						}

						scale+=1.0;
				}
			}
		}
	}

	move_vector.clear();

	cout << "max scale " << max_scale << endl;
	cout << "max move x " << max_move_x << endl;
	cout << "max move y " << max_move_y << endl;
	cout << "max move z " << max_move_z << endl;

	//output_mat_as_csv_file("lab_raw_data.csv",max_align_mat);
	//printf("max_move : %f max_scale : %f\n",max_move,max_scale);
	
	lab = max_align_mat.clone();
	Mat rgb_mat = LAB2RGB(max_align_mat).clone();

	return rgb_mat;
}


Mat Preprocessing_Data::lab_alignment_new_fast(Mat cluster_center, int luminance_threshold)
{
	int vTotal = lab_vertices.size();
	//Turn LAB vector into LAB mat
	Mat lab_mat = Mat::zeros(vTotal,3,CV_32F);
	for(int i=0;i<vTotal;i++)
	{
		for(int j=0;j<3;j++)
		{
			lab_mat.at<float>(i,j) = lab_vertices[i][j+1];
		}
	}
	//Compute centroid of cluster center
	Mat cluster_center_centroid = compute_centroid(cluster_center).clone();

	//Align the centroid to the origin by subtract all points to centroid
	Mat cluster_center_alignment_mat = Mat::zeros(cluster_center.rows,cluster_center.cols,CV_32F);
	for(int i=0;i<cluster_center.rows;i++)
	{
		for(int j=0;j<cluster_center.cols;j++)
		{
			cluster_center_alignment_mat.at<float>(i,j) = cluster_center.at<float>(i,j) - cluster_center_centroid.at<float>(0,j);
		}
	}
	Mat component,cluster_center_coeff;
	Mat cluster_center_component = Mat::zeros(3,3,CV_32F);
	int rDim = 3;
	reduceDimPCA(cluster_center_alignment_mat, rDim, component, cluster_center_coeff);//PCA 4dim->3dim (for dimension reduction)
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
		{
			cluster_center_component.at<float>(i,j) = component.at<float>(i,j);
		}
	}
	cout << "cluster_center_component " << endl << cluster_center_component << endl;
	//cluster center 3 axis of PCA


	//output_mat_as_csv_file("cluster_center_coeff.csv",cluster_center_coeff);
	//compute centroid of LAB
	Mat lab_centroid = compute_centroid(lab_mat).clone();
	//Align the centroid to the origin by subtract all points to centroid
	Mat lab_alignment_mat = Mat::zeros(lab_mat.rows,lab_mat.cols,CV_32F);
	for(int i=0;i<lab_mat.rows;i++)
	{
		for(int j=0;j<lab_mat.cols;j++)
		{
			lab_alignment_mat.at<float>(i,j) = lab_mat.at<float>(i,j) - lab_centroid.at<float>(0,j);
		}
	}
	//lab vertices 3 axis of PCA
	Mat lab_components,lab_coeff;
	rDim = 3;
	reduceDimPCA(lab_alignment_mat, rDim, lab_components, lab_coeff); //PCA 3dim->3dim (for principal component)
	cout << "lab_components " << endl << lab_components << endl;

	//////////////////////////////////////////////////////////////////////////////////////////////
	vector<float> move_vector;
	for(float k=-5.0;k<=5.0;k+=1.0)
		move_vector.push_back(k);

	//vector<float> scale_vector;
	//for(float k=1.0;k<=100.0;k+=1.0)
	//	scale_vector.push_back(k);

	Mat align_mat = Mat::zeros(cluster_center.rows,3,CV_32F);
	Mat max_align_mat = Mat::zeros(cluster_center.rows,3,CV_32F);
	int start = 1;
	//int luminance_threshold = 30;

	Mat cluster_center_coeff_const = cluster_center_coeff.clone();
	float scale = 1.0;
	float max_scale = 1.0;
	float max_move_x = 0.0;
	float max_move_y = 0.0;
	float max_move_z = 0.0;
	bool flag = true;

	//for(int u=0;u<move_vector.size();u++)
	//{
	//	for(int v=0;v<move_vector.size();v++)
	//	{
	//		for(int w=0;w<move_vector.size();w++)
	//		{
				scale = max_scale;
				flag = true;
				while(flag)
				{			
						//if(scale>=150) break;
						cluster_center_coeff = cluster_center_coeff_const.mul(scale);
						//for(int i=0;i<cluster_center_coeff_const.rows;i++)
						//{
						//	cluster_center_coeff.at<float>(i,0) = cluster_center_coeff_const.at<float>(i,0) * scale_vector[u];
						//	cluster_center_coeff.at<float>(i,1) = cluster_center_coeff_const.at<float>(i,1) * scale_vector[v];
						//	cluster_center_coeff.at<float>(i,2) = cluster_center_coeff_const.at<float>(i,2) * scale_vector[w];
						//}

						for(int i=0;i<cluster_center.rows;i++)
						{
							for(int j=0;j<3;j++)
							{
								Mat result = cluster_center_coeff.row(i)*lab_components.col(j);
								align_mat.at<float>(i,j) = result.at<float>(0,0);
							}
						}
						//add(align_mat.col(0),move_vector[u],align_mat.col(0)); //move
						//add(align_mat.col(1),move_vector[v],align_mat.col(1)); //move
						//add(align_mat.col(2),move_vector[w],align_mat.col(2)); //move
				
						//把重心平移回去
						for(int i=0;i<align_mat.rows;i++)
						{
							for(int j=0;j<3;j++)
							{
								align_mat.at<float>(i,j) += lab_centroid.at<float>(0,j);
							}
						}

						//cout << align_mat << endl;
						//system("pause");

						for(int i=0;i<align_mat.rows;i++)
						{
							if( (align_mat.at<float>(i,0)<luminance_threshold) || (align_mat.at<float>(i,0)>85.0) )
							{
								flag = false;
								break;
							}
						}

						if(flag)
						{
							for(int i=0;i<align_mat.rows;i++)
							{
								if(lab_boundary_test(align_mat.at<float>(i,0),align_mat.at<float>(i,1),align_mat.at<float>(i,2))==false)
								{
									flag = false;
									break;
								}
							}	
						}

						if(flag)
						{
							if(scale>max_scale)
							{
								max_scale = scale;
								//max_move_x = move_vector[u];
								//max_move_y = move_vector[v];
								//max_move_z = move_vector[w];
								max_align_mat = align_mat.clone();
							}	
						}

						scale+=5.0;
				}
	//		}
	//	}
	//}

	move_vector.clear();

	cout << "max scale " << max_scale << endl;
	cout << "max move x " << max_move_x << endl;
	cout << "max move y " << max_move_y << endl;
	cout << "max move z " << max_move_z << endl;

	//output_mat_as_csv_file("lab_raw_data.csv",max_align_mat);
	//printf("max_move : %f max_scale : %f\n",max_move,max_scale);
	
	lab = max_align_mat.clone();
	Mat rgb_mat2 = LAB2RGB(max_align_mat).clone();

	return rgb_mat2;
}


Mat Preprocessing_Data::compute_centroid(Mat input_mat)
{
	Mat input_mat_centroid = Mat::zeros(1,input_mat.cols,CV_32F);
	for(int i=0;i<input_mat.rows;i++)
	{
		for(int j=0;j<input_mat.cols;j++)
		{
			input_mat_centroid.at<float>(0,j) += input_mat.at<float>(i,j);
		}
	}

	for(int j=0;j<input_mat.cols;j++) input_mat_centroid.at<float>(0,j)/=input_mat.rows;
	
	return input_mat_centroid;
}

bool Preprocessing_Data::lab_boundary_test(float p1,float p2,float p3)
{
	bool test = true;
	Mat lab_color(1, 1, CV_32FC3);
	Mat rgb_color(1, 1, CV_32FC3);
	lab_color.at<Vec3f>(0, 0) = Vec3f(p1, p2, p3);
	cvtColor(lab_color, rgb_color, CV_Lab2BGR);
	cvtColor(rgb_color, lab_color, CV_BGR2Lab);
	if(abs(lab_color.at<Vec3f>(0,0).val[0] - p1) > 1.0 || abs(lab_color.at<Vec3f>(0,0).val[1] - p2) > 1.0 || abs(lab_color.at<Vec3f>(0,0).val[2] - p3) > 1.0)
		test = false;
	return test;
}

Mat Preprocessing_Data::LAB2RGB(Mat lab_mat)
{
	Mat rgb_mat2 = lab_mat;
	for(int i=0;i<lab_mat.rows;i++)
	{
		Mat color(1, 1, CV_32FC3);
		color.at<Vec3f>(0, 0) = Vec3f(lab_mat.at<float>(i,0),lab_mat.at<float>(i,1),lab_mat.at<float>(i,2));		
		cvtColor(color, color, CV_Lab2BGR);
		rgb_mat2.at<float>(i,0) = color.at<Vec3f>(0,0).val[2];//R
		rgb_mat2.at<float>(i,1) = color.at<Vec3f>(0,0).val[1];//G
		rgb_mat2.at<float>(i,2) = color.at<Vec3f>(0,0).val[0];//B
	}

	return rgb_mat2;
}

void Preprocessing_Data::read_lab_csv()
{
	FILE *csv_file;
	csv_file = fopen("LAB_vertices.csv","r");
	if(!csv_file) 
	{
		cout << "Can't open config file!" << endl;
		exit(1);
	}

	char line[LENGTH];
	char *token;
	int i,j;
	i = j = 0;
	//fgets(line,LENGTH,csv_file); //ignore sep=
	//fgets(line,LENGTH,csv_file); //ignore title

	while(!feof(csv_file))
	{
		fgets(line,LENGTH,csv_file);
		//token = strtok(line,";");
		token = strtok(line,",");
		lab_vertices.push_back(vector<float> (1));
		//printf("%s ",token);
		while(token!=NULL)
		{
			lab_vertices.back().push_back(atof(token));
			//token = strtok(NULL," ;:");
			token = strtok(NULL," ,");
		}
	}
	
	//cout << lab_vertices.size() << " " << lab_vertices[0].size() << endl; //6146 x 4
	//cout << lab_vertices[3][1] << " " << lab_vertices[3][2] << lab_vertices[3][3] << endl;

	fclose(csv_file);
}

Mat Preprocessing_Data::lab_alignment_dim1(Mat cluster_center,int luminance_threshold)
{
	//read_lab_csv();
	int vTotal = lab_vertices.size();
	//Turn LAB vector into LAB mat
	Mat lab_mat = Mat::zeros(vTotal,3,CV_32F);
	for(int i=0;i<vTotal;i++)
	{
		for(int j=0;j<3;j++)
		{
			lab_mat.at<float>(i,j) = lab_vertices[i][j+1];
		}
	}
	//Compute centroid of cluster center
	Mat cluster_center_centroid = compute_centroid(cluster_center);

	//Align the centroid to the origin by subtract all points to centroid
	Mat cluster_center_alignment_mat = Mat::zeros(cluster_center.rows,cluster_center.cols,CV_32F);
	for(int i=0;i<cluster_center.rows;i++)
	{
		for(int j=0;j<cluster_center.cols;j++)
		{
			cluster_center_alignment_mat.at<float>(i,j) = cluster_center.at<float>(i,j) - cluster_center_centroid.at<float>(0,j);
		}
	}
	Mat cluster_center_component,cluster_center_PCA;
	int rDim = 1;
	reduceDimPCA(cluster_center_alignment_mat, rDim, cluster_center_component, cluster_center_PCA);
	//cluster center 3 axis of PCA
	Mat cluster_center_axis = Mat::zeros(1,1,CV_32F);
	cluster_center_axis.at<float>(0,0) = cluster_center_component.at<float>(0,0);

	//compute centroid of LAB
	Mat lab_centroid = compute_centroid(lab_mat);
	
	//lab vertices 3 axis of PCA
	Mat lab_components,lab_PCA;
	rDim = 1;
	reduceDimPCA(lab_mat, rDim, lab_components, lab_PCA);
	Mat lab_axis = Mat::zeros(1,3,CV_32F);
	cout << lab_components << endl;
	for(int j=0;j<3;j++)
	{
		lab_axis.at<float>(0,j) = lab_components.at<float>(0,j);
	}
	
	//////////////////////////////////////////////////////////////////
	Mat cluster_center_PCA_const = cluster_center_PCA;
	vector<float> move_vector;
	for(float k=-0.5;k<=0.5;k+=0.1)
		move_vector.push_back(k);

	float max_move = 0.0;
	float max_scale = 0.0;
	Mat align_mat;
	Mat max_align_mat = cluster_center_PCA;
	int start = 1;
	//int luminance_threshold = 30;
	vector<int> scale_vector;
	//binary search the best scale & convell hull for speed up
	for(int t=0;t<move_vector.size();t++)
	{	
		for(int i=start;i<=250;i++)
			scale_vector.push_back(i);
		
		int low = start;
		int high = scale_vector.size();
		while(low <= high)
		{
			int mid = (low + high)/2; 
			Mat cluster_center_PCA_temp,cluster_center_PCA_weight,cluster_center_axis_invert;
			add(cluster_center_PCA_const,move_vector[t],cluster_center_PCA_temp); //move
			cluster_center_PCA_temp = cluster_center_PCA_temp.mul(mid); //scale
			cluster_center_axis_invert = cluster_center_axis.inv();
			cluster_center_PCA_weight = cluster_center_PCA_temp * cluster_center_axis_invert;
			align_mat = cluster_center_PCA_weight*lab_axis;

			//把重心平移回去
			for(int i=0;i<align_mat.rows;i++)
			{
				for(int j=0;j<3;j++)
				{
					align_mat.at<float>(i,j) += lab_centroid.at<float>(0,j);
				}
			}

			//cout << "align_mat " << align_mat << endl;
	
			bool flag = true;
			for(int i=0;i<align_mat.rows;i++)
			{
				if(align_mat.at<float>(i,0)<luminance_threshold)
				{
					flag = false;
					break;
				}
			}

			if(flag)
			{
				for(int i=0;i<align_mat.rows;i++)
				{
					if(lab_boundary_test(align_mat.at<float>(i,0),align_mat.at<float>(i,1),align_mat.at<float>(i,2))==false)
					{
						flag = false;
						break;
					}
				}
			}

			if(high<=low)
				break;
			else if(flag)
				low = mid + 1;
			else 
				high = mid - 1;

		}

		if(low>max_scale)
		{
			max_scale = low;
			max_move = move_vector[t];
			max_align_mat = align_mat.clone();
			start = max_scale;
		}

		scale_vector.clear();
	}

	if(max_scale==0)
	{
		for(int i=0;i<max_align_mat.rows;i++)
		{
			for(int j=0;j<max_align_mat.cols;j++)
			{
				max_align_mat.at<float>(i,j) += lab_centroid.at<float>(0,j);
			}
		}
	}

	lab = max_align_mat.clone();
	//printf("max_move : %f max_scale : %f\n",max_move,max_scale);
	Mat rgb_mat2 = LAB2RGB(max_align_mat).clone();

	move_vector.clear();
	scale_vector.clear();

	return rgb_mat2;
}

Mat Preprocessing_Data::lab_alignment_dim2(Mat cluster_center,int luminance_threshold)
{
	//read_lab_csv();
	int vTotal = lab_vertices.size();
	//Turn LAB vector into LAB mat
	Mat lab_mat = Mat::zeros(vTotal,3,CV_32F);
	for(int i=0;i<vTotal;i++)
	{
		for(int j=0;j<3;j++)
		{
			lab_mat.at<float>(i,j) = lab_vertices[i][j+1];
		}
	}
	//Compute centroid of cluster center
	Mat cluster_center_centroid = compute_centroid(cluster_center);

	//Align the centroid to the origin by subtract all points to centroid
	Mat cluster_center_alignment_mat = Mat::zeros(cluster_center.rows,cluster_center.cols,CV_32F);
	for(int i=0;i<cluster_center.rows;i++)
	{
		for(int j=0;j<cluster_center.cols;j++)
		{
			cluster_center_alignment_mat.at<float>(i,j) = cluster_center.at<float>(i,j) - cluster_center_centroid.at<float>(0,j);
		}
	}
	Mat cluster_center_component,cluster_center_PCA;
	int rDim = 2;
	reduceDimPCA(cluster_center_alignment_mat, rDim, cluster_center_component, cluster_center_PCA);
	//cluster center 3 axis of PCA
	Mat cluster_center_axis = Mat::zeros(2,2,CV_32F);
	for(int i=0;i<2;i++)
	{
		for(int j=0;j<2;j++)
		{
			cluster_center_axis.at<float>(i,j) = cluster_center_component.at<float>(i,j);
		}
	}

	//compute centroid of LAB
	Mat lab_centroid = compute_centroid(lab_mat);
	
	//lab vertices 3 axis of PCA
	Mat lab_components,lab_PCA;
	rDim = 2;
	reduceDimPCA(lab_mat, rDim, lab_components, lab_PCA);
	Mat lab_axis = Mat::zeros(2,3,CV_32F);
	for(int i=0;i<2;i++)
	{
		for(int j=0;j<3;j++)
		{
			lab_axis.at<float>(i,j) = lab_components.at<float>(i,j);
		}
	}
	
	//////////////////////////////////////////////////////////////////
	Mat cluster_center_PCA_const = cluster_center_PCA;
	vector<float> move_vector;
	for(float k=-0.5;k<=0.5;k+=0.1)
		move_vector.push_back(k);

	float max_move = 0.0;
	float max_scale = 0.0;
	Mat align_mat;
	Mat max_align_mat = cluster_center_PCA;
	int start = 1;
	//int luminance_threshold = 30;
	vector<int> scale_vector;
	//binary search the best scale & convell hull for speed up
	for(int t=0;t<move_vector.size();t++)
	{	
		for(int i=start;i<=150;i++)
			scale_vector.push_back(i);
		
		int low = start;
		int high = scale_vector.size();
		while(low <= high)
		{
			int mid = (low + high)/2; 
			Mat cluster_center_PCA_temp,cluster_center_PCA_weight,cluster_center_axis_invert;
			add(cluster_center_PCA_const,move_vector[t],cluster_center_PCA_temp); //move
			cluster_center_PCA_temp = cluster_center_PCA_temp.mul(mid); //scale
			cluster_center_axis_invert = cluster_center_axis.inv();
			cluster_center_PCA_weight = cluster_center_PCA_temp * cluster_center_axis_invert;

			align_mat = cluster_center_PCA_weight*lab_axis;

			//把重心平移回去
			for(int i=0;i<align_mat.rows;i++)
			{
				for(int j=0;j<3;j++)
				{
					align_mat.at<float>(i,j) += lab_centroid.at<float>(0,j);
				}
			}

	
			bool flag = true;
			for(int i=0;i<align_mat.rows;i++)
			{
				if(align_mat.at<float>(i,0)<luminance_threshold)
				{
					flag = false;
					break;
				}
			}

			if(flag)
			{
				for(int i=0;i<align_mat.rows;i++)
				{
					if(lab_boundary_test(align_mat.at<float>(i,0),align_mat.at<float>(i,1),align_mat.at<float>(i,2))==false)
					{
						flag = false;
						break;
					}
				}
			}

			if(high<=low)
				break;
			else if(flag)
				low = mid + 1;
			else 
				high = mid - 1;

		}

		if(low>max_scale)
		{
			max_scale = low;
			max_move = move_vector[t];
			max_align_mat = align_mat.clone();
			start = max_scale;
		}

		scale_vector.clear();
	}

	if(max_scale==0)
	{
		for(int i=0;i<max_align_mat.rows;i++)
		{
			for(int j=0;j<max_align_mat.cols;j++)
			{
				max_align_mat.at<float>(i,j) += lab_centroid.at<float>(0,j);
			}
		}
	}

	lab = max_align_mat.clone();

	//printf("max_move : %f max_scale : %f\n",max_move,max_scale);
	Mat rgb_mat2 = LAB2RGB(max_align_mat).clone();

	move_vector.clear();
	scale_vector.clear();

	return rgb_mat2;
}

void Preprocessing_Data::gray2rgb(float gray,float& r,float& g,float& b)
{
	r = g = b = 0.0;
	if(gray>1.0)
	{
		r = 1.0;
		g = 0.0;
		b = 0.0;
	}
	if(gray<0.0)
	{
		r = 0.0;
		g = 0.0;
		b = 1.0;
	}

	if(gray<0.33333)
	{
		b = 1.0 - gray*3.0;
		g = gray*3.0;
	}
	else if(gray<0.66666)
	{
		r = (gray-0.33333)*3.0;
		g = 1.0;
	}
	else
	{
		r = 1.0;
		g = 1.0 - (gray-0.66666)*3.0;
	}
}

Mat Preprocessing_Data::normalize_column(Mat col_mat)
{
	Mat output_mat = col_mat;
	double min,max;
	minMaxLoc(output_mat, &min, &max);
	for(int i=0;i<col_mat.rows;i++)
	{
		//output_mat.at<float>(i,0) = ( col_mat.at<float>(i,0) - min ) / (max - min);
		//OpenCV的這個normalize(handle_mat_transpose.col(i),normalize_mat.col(i),0,1,NORM_MINMAX);
		//底層的運作跟這個一樣
		//但會造成gravity normalize晃動太大(因為gravity彼此之間的值都太接近)
		output_mat.at<float>(i,0) = col_mat.at<float>(i,0) / max;
	}

	return output_mat;
}

void Preprocessing_Data::output_mat_as_csv_file_float(char file_name[],Mat mat)
{
	ofstream fout(file_name); 
	for(int i=0;i<mat.rows;i++)
	{
		for(int j=0;j<mat.cols;j++)
		{
			if(j!=0) fout << ",";
			fout << mat.at<float>(i,j) ;
		}
		fout << endl;
	}

	fout.close();//沒關檔會當掉
}   

void Preprocessing_Data::output_mat_as_csv_file_double(char file_name[],Mat mat)
{
	ofstream fout(file_name); 
	for(int i=0;i<mat.rows;i++)
	{
		for(int j=0;j<mat.cols;j++)
		{
			if(j!=0) fout << ",";
			fout << mat.at<double>(i,j) ;
		}
		fout << endl;
	}
	fout.close();
}        


void Preprocessing_Data::output_mat_as_csv_file_int(char file_name[],Mat mat)
{
	ofstream fout(file_name); 
	for(int i=0;i<mat.rows;i++)
	{
		for(int j=0;j<mat.cols;j++)
		{
			if(j!=0) fout << ",";
			fout << mat.at<int>(i,j) ;
		}
		fout << endl;
	}

	fout.close();//沒關檔會當掉
}   

double Preprocessing_Data::TSP_boost_path_by_EdgeWeight(Mat input_mat, Mat& sort_index)
{
	int row = input_mat.rows;

	//typedef vector<simple_point<double> > PositionVec;
	typedef adjacency_matrix<undirectedS, no_property,
	property <edge_weight_t, double> > Graph;
	typedef graph_traits<Graph>::vertex_descriptor Vertex;
	typedef graph_traits<Graph>::edge_descriptor edge_descriptor;
	typedef vector<Vertex> Container;
	typedef property_map<Graph, edge_weight_t>::type WeightMap;
	typedef property_map<Graph, vertex_index_t>::type VertexMap;

	/*	
	PositionVec position_vec;
	for(int i=0;i<row;i++)
	{
		simple_point<double> vertex;
		vertex.x = input_mat.at<float>(i,0);
		vertex.y = input_mat.at<float>(i,1);
		position_vec.push_back(vertex);
	}	*/	

	//cout << "position_vec size " << position_vec.size() << endl;

    int num_nodes = row;
    int num_arcs = row * (row-1) / 2;
	typedef std::pair<int, int> Edge;
	//Edge edge_array[] = { Edge(A, C), Edge(B, B), Edge(B, D), Edge(B, E),Edge(C, B), Edge(C, D), Edge(D, E), Edge(E, A), Edge(E, B) };
	//int weights[] = { 1, 2, 1, 2, 7, 3, 1, 1, 1 };
	Edge* edge_array = new Edge[num_arcs];
	float* weights = new float[num_arcs];
	int t = 0;
	for(int i=0;i<row;i++)
	{
		for(int j=0;j<row;j++)
		{
			if(i==j) 
				continue;
			else if(i>j) 
				continue;
			else
			{
				edge_array[t] = Edge(i,j);
				weights[t] = 0.0;
				for(int u=0;u<input_mat.cols;u++)
				{
					weights[t] += abs( input_mat.at<float>(i,u) - input_mat.at<float>(j,u) ); 
				}
				t++;
			}
		}
	}

	Graph g(num_nodes);
	WeightMap weight_map(get(edge_weight, g));
	VertexMap v_map = get(vertex_index, g);
	edge_descriptor e; 
	for (size_t j = 0; j < num_arcs; ++j) {
		bool inserted;
		boost::tie(e, inserted) = add_edge(edge_array[j].first, edge_array[j].second, g);
		weight_map[e] = weights[j];
		//cout << "weight[(" << edge_array[j].first << "," << edge_array[j].second << ")] = " << get(weight_map, e) << endl;
		//if(edge_array[j].second == num_nodes-1) cout << endl;
	}


	//Graph g(edge_array, edge_array + num_arcs, weights, num_nodes);

	Container c;

	//connectAllEuclidean(g, position_vec, weight_map, v_map);

	metric_tsp_approx_tour(g, back_inserter(c));

	int i = 0;
	for (vector<Vertex>::iterator itr = c.begin(); itr != c.end(); ++itr)
	{
		//cout << *itr << " ";
		sort_index.at<int>(i,0) = *itr;
		i++;
		if(i==row) break;
	}

	//cout << "i " << i << endl;
	cout << endl << endl;

	c.clear();
	
	double max = -1000;
	double max_index = 0;
	for(int i=0;i<sort_index.rows-1;i++)
	{
		Mat m1 = input_mat.row( sort_index.at<int>(i,0) );
		Mat m2 = input_mat.row( sort_index.at<int>(i+1,0) );
		double dist = compute_dist(m1,m2,input_mat.cols);
		if(dist >= max)
		{
			max = dist;
			max_index = i;
		}
	}

	Mat temp = sort_index.clone();
	t = 0;
	if(max_index!=0)
	{
		for(int i=max_index+1;i<sort_index.rows;i++)
		{
			sort_index.row(i).copyTo( temp.row(t++) );
		}
		for(int i=0;i<=max_index;i++)
		{
			sort_index.row(i).copyTo( temp.row(t++) );
		}
	}

	sort_index = temp.clone();

	//checkAdjList(position_vec);

	//metric_tsp_approx_from_vertex(g, *vertices(g).first,
	//	get(edge_weight, g), get(vertex_index, g),
	//	tsp_tour_visitor<back_insert_iterator<vector<Vertex> > >
	//	(back_inserter(c)));

	//for (vector<Vertex>::iterator itr = c.begin(); itr != c.end(); ++itr)
	//{
	//	cout << *itr << " ";
	//}
	//cout << endl << endl;

	//c.clear();
   
	double len(0.0);
	try {
		metric_tsp_approx(g, make_tsp_tour_len_visitor(g, back_inserter(c), len, weight_map));
	}
	catch (const bad_graph& e) {
		cerr << "bad_graph: " << e.what() << endl;
		//return;
	}

	cout << "Number of points: " << num_vertices(g) << endl;
	cout << "Number of edges: " << num_edges(g) << endl;
	cout << "Length of Tour: " << len << endl;

	return len;
}

double Preprocessing_Data::compute_dist(Mat m1,Mat m2,int dim)
{
	double dist = 0.0;
	for(int i=0;i<m1.rows;i++)
	{
		for(int j=0;j<dim;j++)
		{
			dist += sqrt( (m1.at<float>(i,j) - m2.at<float>(i,j) )*(m1.at<float>(i,j) - m2.at<float>(i,j) ) );
		}
	}

	return dist;
}

void Preprocessing_Data::sort_pattern_by_color_by_TSP_coarse_to_fine(Mat lab_data, Mat& lab_color_sort_index)
{
	int k = lab_data.rows;//k是4的倍數
	int dim = lab_data.cols;
	int group_num = 4;
	Mat cluster_tag = Mat::zeros(k,1,CV_32S);
	Mat cluster_centers = Mat::zeros(group_num,dim,CV_32F);
	cuda_kmeans(lab_data, group_num, cluster_tag, cluster_centers);
	//kmeans(lab_data, group_num, cluster_tag, TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 100, 0.0001), 2, KMEANS_PP_CENTERS,cluster_centers);
	class group{
	public:
		vector<int> index;
		vector<int> index2;
		vector<int> sort_index;
		vector<int> adj_index;
	};
	vector<group> groups(group_num);

	for(int i=0;i<k;i++)
	{
		int tag = cluster_tag.at<int>(i,0);
		groups[tag].index.push_back(i);
	}

	///////////////////////////////
	ofstream fout("color_test.txt"); 

	//output_mat_as_csv_file_float("lab_data.csv",lab_data);

	//cout << "index :" << endl;
	fout << "index :" << endl;
 	for(int i=0;i<group_num;i++)
	{
		for(int j=0;j<groups[i].index.size();j++)
		{
			//cout << groups[i].index[j] << " ";
			fout << groups[i].index[j] + 1 << " ";
		}
		//cout << endl;
		fout << endl;
	}

	Mat component, coeff;
	int rDim = 2;
	reduceDimPCA(cluster_centers, rDim, component, coeff);
	Mat lab_cluster_center2D_ndim = coeff * component;
	Mat lab_cluster_center2D = Mat::zeros(group_num,2,CV_32F);
	for(int i=0;i<rDim;i++)
	{
		lab_cluster_center2D_ndim.col(i).copyTo( lab_cluster_center2D.col(i) );
	}
	Mat color_sort_index = Mat::zeros(group_num,1,CV_32S);
	//TSP_boost(lab_cluster_center2D, color_sort_index);
	//TSP_path(lab_cluster_center2D, color_sort_index);
	TSP_boost_path_by_EdgeWeight(cluster_centers, color_sort_index);

	for(int i=0;i<group_num;i++)
	{
		int index = color_sort_index.at<int>(i,0);
		for(int j=0;j<groups[index].index.size();j++)
		{
			groups[i].index2.push_back( groups[index].index[j] );
		}
	}

	//cout << "index2 :" << endl;
	fout << "index2 :" << endl;
 	for(int i=0;i<group_num;i++)
	{
		for(int j=0;j<groups[i].index2.size();j++)
		{
			//cout << groups[i].index2[j] << " ";
			fout << groups[i].index2[j] + 1 << " ";
		}
		//cout << endl;
		fout << endl;
	}

	rDim = 2;
	reduceDimPCA(lab_data, rDim, component, coeff);
	Mat lab_PCA2D = coeff * component;
	////////////////////////////////////////////////////////////////////////////////////
	Mat lab_sub = Mat::zeros(groups[0].index2.size()+1,lab_data.cols,CV_32F);
	Mat lab_color_sort_index_sub = Mat::zeros(groups[0].index2.size()+1,1,CV_32S);
	Mat optimal_sort_index;
	for(int i=0;i<groups[0].index2.size();i++)
	{
		int index = groups[0].index2[i];
		lab_data.row(index).copyTo( lab_sub.row(i) );
	}
	double min_len = 10000;
	double min_target = 0;
	for(int j=0;j<groups[1].index2.size();j++)
	{
		int index = groups[1].index2[j];
		lab_data.row(index).copyTo( lab_sub.row(groups[0].index2.size()) );	
		//double len = TSP_path(lab_PCA2D_sub, lab_color_sort_index_sub);
		double len = TSP_boost_path_by_EdgeWeight(lab_sub, lab_color_sort_index_sub);
		if(len < min_len) 
		{
			min_len = len;
			min_target = j;
			optimal_sort_index = lab_color_sort_index_sub.clone();
		}
	}
	
	cout << "optimal_sort_index " << optimal_sort_index << endl;
	for(int i=0;i<optimal_sort_index.rows;i++)
	{
		int index = optimal_sort_index.at<int>(i,0);
		if( index!=groups[0].index2.size() ) //以防start點不是出現在最後面
		{
			groups[0].sort_index.push_back( groups[0].index2[index] );
		}
	}

	groups[0].sort_index.push_back( groups[1].index2[min_target] );
	int start = groups[1].index2[min_target];
	cout << "start " << start << endl;

	cout << "temp: ";
	vector<int> temp = groups[1].index2;
	groups[1].index2.clear();
	for(int i=0;i<temp.size();i++)
	{
		if(temp[i]!=start)
		{
			groups[1].index2.push_back( temp[i] );
			cout << temp[i] << " ";
		}
	}
	cout << endl;
	////////////////////////////////////////////////////////////////////////////////////
	lab_sub = Mat::zeros(groups[0].sort_index.size()+1,lab_data.cols,CV_32F);
	lab_color_sort_index_sub = Mat::zeros(groups[0].sort_index.size()+1,1,CV_32S);	
	for(int i=0;i<groups[0].sort_index.size();i++)
	{
		int index = groups[0].sort_index[i];
		lab_data.row(index).copyTo( lab_sub.row(i) );	
	}
	min_len = 10000;
	min_target = 0;
	for(int j=0;j<groups[2].index2.size();j++)
	{
		int index = groups[2].index2[j];
		lab_data.row(index).copyTo( lab_sub.row(groups[0].sort_index.size()) );	
		//double len = TSP_path(lab_PCA2D_sub, lab_color_sort_index_sub);
		double len = TSP_boost_path_by_EdgeWeight(lab_sub, lab_color_sort_index_sub);
		if(len < min_len) 
		{
			min_len = len;
			min_target = j;
			optimal_sort_index = lab_color_sort_index_sub.clone();
		}
	}

	int end = groups[2].index2[min_target];
	cout << "end " << end << endl;

	Mat TSP_brute_sort_index = Mat::zeros(groups[1].index2.size()+2,1,CV_32S);
	lab_sub = Mat::zeros(groups[1].index2.size()+2,lab_data.cols,CV_32F);
	int t=0;
	lab_data.row( start ).copyTo( lab_sub.row(t++) );
	for(int i=0;i<groups[1].index2.size();i++)
	{
		lab_data.row( groups[1].index2[i] ).copyTo( lab_sub.row(t++) );
	}
	lab_data.row( end ).copyTo( lab_sub.row(t++) );

	//tsp.start(lab_sub,0,t-1,TSP_brute_sort_index);
	TSP_boost_by_EdgeWeight(lab_sub,TSP_brute_sort_index,0,t-1);
	cout << "TSP_brute_sort_index " << TSP_brute_sort_index << endl;
	for(int i=1;i<TSP_brute_sort_index.rows-1;i++)
	{
		int index = TSP_brute_sort_index.at<int>(i,0) - 1;
		groups[1].sort_index.push_back( groups[1].index2[ index ] );
	}
	groups[1].sort_index.push_back(end);

	cout << "temp: ";
	temp.clear();
	temp = groups[2].index2;
	groups[2].index2.clear();
	for(int i=0;i<temp.size();i++)
	{
		if(temp[i]!=end)
		{
			groups[2].index2.push_back( temp[i] );
			cout << temp[i] << " ";
		}
	}
	cout << endl;

	start = end;
	cout << "start " << start << endl;
	///////////////////////////////////////////////////////////////////////
	lab_sub = Mat::zeros(groups[2].index2.size()+1,lab_data.cols,CV_32F);
	//lab_PCA2D_sub = Mat::zeros(groups[2].index2.size()+1,2,CV_32F);
	lab_color_sort_index_sub = Mat::zeros(groups[2].index2.size()+1,1,CV_32S);	
	for(int i=0;i<groups[2].index2.size();i++)
	{
		int index = groups[2].index2[i];
		lab_data.row(index).copyTo( lab_sub.row(i) ); 
		//lab_PCA2D_sub.at<float>(i,0) = lab_PCA2D.at<float>(index,0);
		//lab_PCA2D_sub.at<float>(i,1) = lab_PCA2D.at<float>(index,1);		
	}
	min_len = 10000;
	min_target = 0;
	for(int j=0;j<groups[3].index2.size();j++)
	{
		int index = groups[3].index2[j];
		lab_data.row(index).copyTo( lab_sub.row(groups[2].index2.size()) ); 
		//lab_PCA2D_sub.at<float>(groups[2].index2.size(),0) = lab_PCA2D.at<float>(index,0);
		//lab_PCA2D_sub.at<float>(groups[2].index2.size(),1) = lab_PCA2D.at<float>(index,1);		
		//double len = TSP_path(lab_PCA2D_sub, lab_color_sort_index_sub);
		double len = TSP_boost_path_by_EdgeWeight(lab_sub, lab_color_sort_index_sub);
		if(len < min_len) 
		{
			min_len = len;
			min_target = j;
			optimal_sort_index = lab_color_sort_index_sub.clone();
		}
	}

	end = groups[3].index2[min_target];
	cout << "end " << end << endl;

	TSP_brute_sort_index = Mat::zeros(groups[2].index2.size()+2,1,CV_32S);
	lab_sub = Mat::zeros(groups[2].index2.size()+2,lab_data.cols,CV_32F);
	t=0;
	lab_data.row( start ).copyTo( lab_sub.row(t++) );
	for(int i=0;i<groups[2].index2.size();i++)
	{
		lab_data.row( groups[2].index2[i] ).copyTo( lab_sub.row(t++) );
	}
	lab_data.row( end ).copyTo( lab_sub.row(t++) );

	//tsp_brute tsp;
	//tsp.start(lab_sub,0,t-1,TSP_brute_sort_index);
	TSP_boost_by_EdgeWeight(lab_sub,TSP_brute_sort_index,0,t-1);

	cout << "TSP_brute_sort_index " << TSP_brute_sort_index << endl;
	for(int i=1;i<TSP_brute_sort_index.rows-1;i++)
	{
		int index = TSP_brute_sort_index.at<int>(i,0) - 1;
		groups[2].sort_index.push_back( groups[2].index2[ index ] );
	}
	groups[2].sort_index.push_back(end);
	
	cout << "temp: ";
	temp.clear();
	temp = groups[3].index2;
	groups[3].index2.clear();
	for(int i=0;i<temp.size();i++)
	{
		if(temp[i]!=end)
		{
			groups[3].index2.push_back( temp[i] );
			cout << temp[i] << " ";
		}
	}
	cout << endl;	

	start = end;
	cout << "start " << start << endl;
	/////////////////////////////////////////////////////////
	lab_sub = Mat::zeros(groups[3].index2.size()+1,lab_data.cols,CV_32F);
	//lab_PCA2D_sub = Mat::zeros(groups[3].index2.size()+1,2,CV_32F);
	lab_color_sort_index_sub = Mat::zeros(groups[3].index2.size()+1,1,CV_32S);	
	lab_data.row(start).copyTo( lab_sub.row(0) );
	//lab_PCA2D_sub.at<float>(0,0) = lab_PCA2D.at<float>(start,0);
	//lab_PCA2D_sub.at<float>(0,1) = lab_PCA2D.at<float>(start,1);
	for(int i=0;i<groups[3].index2.size();i++)
	{
		int index = groups[3].index2[i];
		lab_data.row(index).copyTo( lab_sub.row(i+1) );
		//lab_PCA2D_sub.at<float>(i+1,0) = lab_PCA2D.at<float>(index,0);
		//lab_PCA2D_sub.at<float>(i+1,1) = lab_PCA2D.at<float>(index,1);		
	}
	//TSP_path(lab_PCA2D_sub, lab_color_sort_index_sub);
	TSP_boost_path_by_EdgeWeight(lab_sub, lab_color_sort_index_sub);

	cout << "lab_color_sort_index_sub " << lab_color_sort_index_sub << endl;

	for(int i=0;i<lab_color_sort_index_sub.rows;i++)
	{
		int index = lab_color_sort_index_sub.at<int>(i,0);
		if( index==0 )
		{
			continue;
		}
		else
		{
			groups[3].sort_index.push_back( groups[3].index2[index-1] );///////////////////
		}
	}

	t = 0;
	//cout << "sort index :" << endl;
	fout << "sort index :" << endl;
 	for(int i=0;i<4;i++)
	{
		for(int j=0;j<groups[i].sort_index.size();j++)
		{
			//cout << groups[i].sort_index[j] << " ";
			fout << groups[i].sort_index[j] + 1 << " ";
			lab_color_sort_index.at<int>(t,0) = groups[i].sort_index[j];
			t++;
		}
		//cout << endl;
		fout << endl;
	}

	fout.close();

	groups.clear();

}

double Preprocessing_Data::TSP_boost_by_EdgeWeight(Mat input_mat, Mat& sort_index)
{
	int row = input_mat.rows;

	typedef vector<simple_point<double> > PositionVec;
	typedef adjacency_matrix<undirectedS, no_property,
	property <edge_weight_t, double> > Graph;
	typedef graph_traits<Graph>::vertex_descriptor Vertex;
	typedef graph_traits<Graph>::edge_descriptor edge_descriptor;
	typedef vector<Vertex> Container;
	typedef property_map<Graph, edge_weight_t>::type WeightMap;
	typedef property_map<Graph, vertex_index_t>::type VertexMap;

	/*	
	PositionVec position_vec;
	for(int i=0;i<row;i++)
	{
		simple_point<double> vertex;
		vertex.x = input_mat.at<float>(i,0);
		vertex.y = input_mat.at<float>(i,1);
		position_vec.push_back(vertex);
	}	*/	

	//cout << "position_vec size " << position_vec.size() << endl;

    int num_nodes = row;
    int num_arcs = row * (row-1) / 2;
	typedef std::pair<int, int> Edge;
	//Edge edge_array[] = { Edge(A, C), Edge(B, B), Edge(B, D), Edge(B, E),Edge(C, B), Edge(C, D), Edge(D, E), Edge(E, A), Edge(E, B) };
	//int weights[] = { 1, 2, 1, 2, 7, 3, 1, 1, 1 };
	Edge* edge_array = new Edge[num_arcs];
	float* weights = new float[num_arcs];
	int t = 0;
	for(int i=0;i<row;i++)
	{
		for(int j=0;j<row;j++)
		{
			if(i==j) 
				continue;
			else if(i>j) 
				continue;
			else
			{
				edge_array[t] = Edge(i,j);
				weights[t] = 0.0;
				for(int u=0;u<input_mat.cols;u++)
				{
					weights[t] += abs( input_mat.at<float>(i,u) - input_mat.at<float>(j,u) ); 
				}
				t++;
			}
		}
	}

	Graph g(num_nodes);
	WeightMap weight_map(get(edge_weight, g));
	VertexMap v_map = get(vertex_index, g);
	edge_descriptor e; 
	for (size_t j = 0; j < num_arcs; ++j) {
		bool inserted;
		boost::tie(e, inserted) = add_edge(edge_array[j].first, edge_array[j].second, g);
		weight_map[e] = weights[j];
		//cout << "weight[(" << edge_array[j].first << "," << edge_array[j].second << ")] = " << get(weight_map, e) << endl;
		//if(edge_array[j].second == num_nodes-1) cout << endl;
	}


	//Graph g(edge_array, edge_array + num_arcs, weights, num_nodes);

	Container c;

	//connectAllEuclidean(g, position_vec, weight_map, v_map);

	metric_tsp_approx_tour(g, back_inserter(c));

	int i = 0;
	for (vector<Vertex>::iterator itr = c.begin(); itr != c.end(); ++itr)
	{
		//cout << *itr << " ";
		sort_index.at<int>(i,0) = *itr;
		i++;
		if(i==row) break;
	}

	//cout << "i " << i << endl;
	cout << endl << endl;

	c.clear();

	//checkAdjList(position_vec);

	//metric_tsp_approx_from_vertex(g, *vertices(g).first,
	//	get(edge_weight, g), get(vertex_index, g),
	//	tsp_tour_visitor<back_insert_iterator<vector<Vertex> > >
	//	(back_inserter(c)));

	//for (vector<Vertex>::iterator itr = c.begin(); itr != c.end(); ++itr)
	//{
	//	cout << *itr << " ";
	//}
	//cout << endl << endl;

	//c.clear();
   
	double len(0.0);
	try {
		metric_tsp_approx(g, make_tsp_tour_len_visitor(g, back_inserter(c), len, weight_map));
	}
	catch (const bad_graph& e) {
		cerr << "bad_graph: " << e.what() << endl;
		//return;
	}

	cout << "Number of points: " << num_vertices(g) << endl;
	cout << "Number of edges: " << num_edges(g) << endl;
	cout << "Length of Tour: " << len << endl;

	return len;
}


double Preprocessing_Data::TSP_boost_by_EdgeWeight(Mat input_mat, Mat& sort_index, int start_index, int end_index)
{
	int row = input_mat.rows;

	typedef vector<simple_point<double> > PositionVec;
	typedef adjacency_matrix<undirectedS, no_property,
	property <edge_weight_t, double> > Graph;
	typedef graph_traits<Graph>::vertex_descriptor Vertex;
	typedef graph_traits<Graph>::edge_descriptor edge_descriptor;
	typedef vector<Vertex> Container;
	typedef property_map<Graph, edge_weight_t>::type WeightMap;
	typedef property_map<Graph, vertex_index_t>::type VertexMap;

    int num_nodes = row + 1; //dummy node
    int num_arcs = num_nodes * (num_nodes-1) / 2;
	//int num_arcs = num_nodes * (num_nodes-1) / 2 - (num_nodes-1-2);
	typedef std::pair<int, int> Edge;
	//Edge edge_array[] = { Edge(A, C), Edge(B, B), Edge(B, D), Edge(B, E),Edge(C, B), Edge(C, D), Edge(D, E), Edge(E, A), Edge(E, B) };
	//int weights[] = { 1, 2, 1, 2, 7, 3, 1, 1, 1 };
	Edge* edge_array = new Edge[num_arcs];
	float* weights = new float[num_arcs];
	//float check_weight[50][50];
	int t = 0;

	Mat input_mat_add_dim = Mat::zeros(num_nodes,input_mat.cols+1,CV_32F);
	for(int i=0;i<num_nodes;i++)
	{
		if(i==num_nodes-1) //dummy_node
		{
			for(int j=0;j<input_mat.cols;j++)
			{
				input_mat_add_dim.at<float>(i,j) = ( input_mat.at<float>(start_index,j) + input_mat.at<float>(end_index,j) ) / 2.0;
			}
		    input_mat_add_dim.at<float>(i,input_mat.cols) = 1000.0;
		}
		else
		{
			for(int j=0;j<input_mat.cols;j++)
			{
				input_mat_add_dim.at<float>(i,j) = input_mat.at<float>(i,j);
			}
			//if(i==start_index || i==end_index)
			//{
			//	input_mat_add_dim.at<float>(i,input_mat.cols) = 499.0;
			//}
			if(i==start_index)
			{
				input_mat_add_dim.at<float>(i,input_mat.cols) = 999.9;
			}
			else if(i==end_index)
			{
				input_mat_add_dim.at<float>(i,input_mat.cols) = 999.9;
			}
			else
			{
				input_mat_add_dim.at<float>(i,input_mat.cols) = 0.0;
			}
		}
	}

	Mat input_mat_add_dim_rearrange = Mat::zeros(num_nodes,input_mat.cols+1,CV_32F);
	input_mat_add_dim.row(num_nodes-1).copyTo( input_mat_add_dim_rearrange.row(0) );
	for(int i=0;i<num_nodes-1;i++)
	{
		input_mat_add_dim.row(i).copyTo( input_mat_add_dim_rearrange.row(i+1) );
	}

	//output_mat_as_csv_file("input_mat_add_dim.csv",input_mat_add_dim);
	//output_mat_as_csv_file("input_mat_add_dim_rearrange.csv",input_mat_add_dim_rearrange);

	for(int i=0;i<num_nodes;i++)
	{
		for(int j=0;j<num_nodes;j++)
		{
			if(i==j) 
				continue;
			else if(i>j) 
				continue;
			else
			{	
				//if(i==start_index && j==num_nodes-1)
				//{
				//	weights[t] = 0.1;
				//}
				//else if(i==end_index && j==num_nodes-1)
				//{
				//	weights[t] =  0.1;
				//}
				//else if(j==num_nodes-1)
				//{
				//	//continue;
				//	if(i!=num_nodes-1) 
				//		weights[t] = big_num;
				//}
				//else
				//{
					weights[t] = 0.0;
					for(int u=0;u<input_mat_add_dim_rearrange.cols;u++)
					{
						weights[t] += abs( input_mat_add_dim_rearrange.at<float>(i,u) - input_mat_add_dim_rearrange.at<float>(j,u) ); 
					}

				//}
				edge_array[t] = Edge(i,j);
				//check_weight[i][j] = weights[t];
				t++;
			}
		}
	}

	/*
	Graph g(edge_array, edge_array + num_arcs, weights, num_nodes);
	WeightMap weight_map(get(edge_weight, g));
	VertexMap v_map = get(vertex_index, g);
	*/
	Graph g(num_nodes);
	WeightMap weight_map(get(edge_weight, g));
	VertexMap v_map = get(vertex_index, g);
	for (size_t j = 0; j < num_arcs; ++j) {
		edge_descriptor e; 
		bool inserted;
		boost::tie(e, inserted) = add_edge(edge_array[j].first, edge_array[j].second, g);
		weight_map[e] = weights[j];
		//cout << "weight[(" << edge_array[j].first << "," << edge_array[j].second << ")] = " << get(weight_map, e) << endl;
	}

	Container c;
	//connectAllEuclidean(g, position_vec, weight_map, v_map);

	metric_tsp_approx_tour(g, back_inserter(c));


	//save
	int i = 0;
	for (vector<Vertex>::iterator itr = c.begin(); itr != c.end(); ++itr)
	{
		cout << *itr << " ";
		if(*itr==0) 
			; //sort_index.at<int>(i,0) = num_nodes - 1;
		else
		{
			sort_index.at<int>(i,0) = *itr - 1;
			i++;
		}
		if(i==num_nodes) break;
	}
	cout << endl;

	//print
	for (vector<Vertex>::iterator itr = c.begin(); itr != c.end(); ++itr)
	{
		//cout << *itr << " ";
		if(*itr==0) 
			cout << num_nodes - 1 << " ";
		else
			cout << *itr - 1 << " ";;
	}

	cout << endl << endl;

	vector<int> sort_index_triple;
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<sort_index.rows;j++)
		{
			sort_index_triple.push_back( sort_index.at<int>(j,0) );
		}
	}

	//int second_start_index = num_nodes-1;
	int second_start_index;
	int start_index_count = 0;
	for(int i=0;i<sort_index_triple.size();i++)
	{
		if(sort_index_triple[i]==start_index)
		{
			start_index_count++;
			if(start_index_count==2)
			{
				second_start_index = i;
				break;
			}
		}
	}
	
	t = 0;
	sort_index.at<int>(t++,0) = sort_index_triple[second_start_index];

	if(sort_index_triple[second_start_index+1] == end_index)
	{
		int current = second_start_index-1;
		while(sort_index_triple[current] != start_index)
		{
			sort_index.at<int>(t++,0) = sort_index_triple[current];
			current--;
		}
	}
	else if(sort_index_triple[second_start_index-1] == end_index)
	{
		int current = second_start_index+1;
		while(sort_index_triple[current] != start_index)
		{
			sort_index.at<int>(t++,0) = sort_index_triple[current];
			current++;
		}	
	}

	sort_index_triple.clear();

	c.clear();

	//checkAdjList(position_vec);

	//metric_tsp_approx_from_vertex(g, *vertices(g).first,
	//	get(edge_weight, g), get(vertex_index, g),
	//	tsp_tour_visitor<back_insert_iterator<vector<Vertex> > >
	//	(back_inserter(c)));

	//for (vector<Vertex>::iterator itr = c.begin(); itr != c.end(); ++itr)
	//{
	//	cout << *itr << " ";
	//}
	//cout << endl << endl;

	//c.clear();
   
	double len(0.0);
	try {
		metric_tsp_approx(g, make_tsp_tour_len_visitor(g, back_inserter(c), len, weight_map));
	}
	catch (const bad_graph& e) {
		cerr << "bad_graph: " << e.what() << endl;
		//return;
	}

	//for (vector<Vertex>::iterator itr = c.begin(); itr != c.end(); ++itr)
	//{
	//	cout << *itr << " ";
	//}
	cout << "Number of points: " << num_vertices(g) << endl;
	cout << "Number of edges: " << num_edges(g) << endl;
	cout << "Length of Tour: " << len << endl;

	return len;
}

void Preprocessing_Data::rearrange_mat_by_sort_color_index(Mat lab_color_sort_index, Mat& cluster_center, Mat& cluster_tag, Mat& rgb_mat3)
{
	Mat cluster_center_old = cluster_center.clone();
	Mat cluster_tag_old = cluster_tag.clone();
	Mat rgb_mat3_old = rgb_mat3.clone();
	int k = cluster_center.rows;

	//注意:row的複製不能用rgb_mat2.row(i) = rgb_mat2_old.row(new_tag).clone();!!!!!!!
	for(int i=0;i<k;i++)
	{
		int key = lab_color_sort_index.at<int>(i,0);
		cluster_center_old.row(key).copyTo(cluster_center.row(i));
		rgb_mat3_old.row(key).copyTo(rgb_mat3.row(i));
	}

	//更新cluster的tag
	for(int i=0;i<cluster_tag_old.rows;i++)
	{
		int find;
		for(int j=0;j<k;j++)
		{
			if(lab_color_sort_index.at<int>(j,0) == cluster_tag_old.at<int>(i,0))
			{
				find = j;
				break;
			}
		}
		cluster_tag.at<int>(i,0) = find;	
	}

}

void Preprocessing_Data::sort_histogram_by_Ev_by_TSP_coarse_to_fine(Mat cluster_center, Mat& histo_sort_index)
{
	int k = cluster_center.rows;//k是4的倍數
	int dim = cluster_center.cols;
	int five_minutes = histogram.rows; 
	int group_num = 4;
	Mat Ev = Mat::zeros(five_minutes,cluster_center.cols,CV_32F);
	for(int i=0;i<five_minutes;i++)
	{
		int votes = 0;
		for(int j=0;j<k;j++)
		{
			Ev.row(i) += histogram.at<int>(i,j)*cluster_center.row(j);
			votes +=  histogram.at<int>(i,j);
		}
		Ev.row(i) /= votes;

	}
	output_mat_as_csv_file_float("Ev.csv",Ev);	

	Mat cluster_tag = Mat::zeros(five_minutes,1,CV_32S);
	Mat group_cluster_centers  = Mat::zeros(group_num,dim,CV_32F);
	cuda_kmeans(Ev, group_num, cluster_tag, group_cluster_centers);
	//kmeans(Ev, group_num, cluster_tag, TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 100, 0.0001), 2, KMEANS_PP_CENTERS,group_cluster_centers );
	class group{
	public:
		vector<int> index;
		vector<int> index2;
		vector<int> sort_index;
		vector<int> adj_index;
	};
	vector<group> groups(group_num);

	for(int i=0;i<five_minutes;i++)
	{
		int tag = cluster_tag.at<int>(i,0);
		groups[tag].index.push_back(i);
	}

	cout << "group_cluster_centers " << group_cluster_centers << endl;

	ofstream fout("histo_test.txt"); 

	//cout << "index :" << endl;
	fout << "index :" << endl;
 	for(int i=0;i<group_num;i++)
	{
		for(int j=0;j<groups[i].index.size();j++)
		{
			//cout << groups[i].index[j] << " ";
			fout << groups[i].index[j] + 1 << " ";
		}
		//cout << endl;
		fout << endl;
	}

	/*
	Mat component, coeff;
	int rDim = 2;
	reduceDimPCA(group_cluster_centers , rDim, component, coeff);
	Mat group_cluster_center_dr = coeff * component;
	Mat group_cluster_center_2D = Mat::zeros(group_num,2,CV_32F);
	
	for(int i=0;i<rDim;i++)
	{
		group_cluster_center_dr.col(i).copyTo( group_cluster_center_2D.col(i) );
	}
	*/
	Mat group_sort_index = Mat::zeros(group_num,1,CV_32S);
	//TSP_boost(group_cluster_centers, color_sort_index);
	//TSP_boost_by_EdgeWeight(group_cluster_centers, group_sort_index);
	//TSP_boost_path_by_EdgeWeight(group_cluster_centers, group_sort_index);

	tsp_brute tsp;
	tsp.tsp_path(group_cluster_centers,group_sort_index);

	for(int i=0;i<group_num;i++)
	{
		int index = group_sort_index.at<int>(i,0);
		for(int j=0;j<groups[index].index.size();j++)
		{
			groups[i].index2.push_back( groups[index].index[j] );
		}
	}

	//cout << "index2 :" << endl;
	fout << "index2 :" << endl;
 	for(int i=0;i<group_num;i++)
	{
		for(int j=0;j<groups[i].index2.size();j++)
		{
			//cout << groups[i].index2[j] << " ";
			fout << groups[i].index2[j] + 1 << " ";
		}
		//cout << endl;
		fout << endl;
	}

	/*
	rDim = 2;
	reduceDimPCA(Ev, rDim, component, coeff);
	Mat Ev_PCA2D = coeff * component;
	*/

	////////////////////////////////////////////////////////////////////////////////////
	Mat Ev_sub = Mat::zeros(groups[0].index2.size()+1,Ev.cols,CV_32F);
	Mat histo_sort_index_sub = Mat::zeros(groups[0].index2.size()+1,1,CV_32S);
	Mat optimal_sort_index;
	for(int i=0;i<groups[0].index2.size();i++)
	{
		int index = groups[0].index2[i];
		Ev.row(index).copyTo( Ev_sub.row(i) );
	}
	double min_len = 10000;
	double min_target = 0;
	for(int j=0;j<groups[1].index2.size();j++)
	{
		int index = groups[1].index2[j];
		Ev.row(index).copyTo( Ev_sub.row(groups[0].index2.size()) );
		//double len = TSP_boost(Ev_PCA2D_sub, histo_sort_index_sub);
		//double len = TSP_boost_by_EdgeWeight(Ev_sub, histo_sort_index_sub);
		double len = TSP_boost_path_by_EdgeWeight(Ev_sub, histo_sort_index_sub);
		if(len < min_len) 
		{
			min_len = len;
			min_target = j;
			optimal_sort_index = histo_sort_index_sub.clone();
		}
	}
	
	//cout << "optimal_sort_index " << optimal_sort_index << endl;
	for(int i=0;i<optimal_sort_index.rows;i++)
	{
		int index = optimal_sort_index.at<int>(i,0);
		if( index!=groups[0].index2.size() ) //以防start點不是出現在最後面
		{
			groups[0].sort_index.push_back( groups[0].index2[index] );
		}
	}

	groups[0].sort_index.push_back( groups[1].index2[min_target] );
	int start = groups[1].index2[min_target];
	cout << "start " << start << endl;

	cout << "temp: ";
	vector<int> temp = groups[1].index2;
	groups[1].index2.clear();
	for(int i=0;i<temp.size();i++)
	{
		if(temp[i]!=start)
		{
			groups[1].index2.push_back( temp[i] );
			cout << temp[i] << " ";
		}
	}
	cout << endl;
	////////////////////////////////////////////////////////////////////////////////////
	Ev_sub = Mat::zeros(groups[0].sort_index.size()+1,Ev.cols,CV_32F);
	histo_sort_index_sub = Mat::zeros(groups[0].sort_index.size()+1,1,CV_32S);	
	for(int i=0;i<groups[0].sort_index.size();i++)
	{
		int index = groups[0].sort_index[i];
		Ev.row(index).copyTo( Ev_sub.row(i) );
	}
	min_len = 10000;
	min_target = 0;
	for(int j=0;j<groups[2].index2.size();j++)
	{
		int index = groups[2].index2[j];	
		Ev.row(index).copyTo( Ev_sub.row(groups[0].sort_index.size()) );
		//double len = TSP_boost_by_EdgeWeight(Ev_sub, histo_sort_index_sub);
		double len = TSP_boost_path_by_EdgeWeight(Ev_sub, histo_sort_index_sub);
		if(len < min_len) 
		{
			min_len = len;
			min_target = j;
			optimal_sort_index = histo_sort_index_sub.clone();
		}
	}

	int end = groups[2].index2[min_target];
	cout << "end " << end << endl;

	Mat TSP_brute_sort_index = Mat::zeros(groups[1].index2.size()+2,1,CV_32S);
	Ev_sub = Mat::zeros(groups[1].index2.size()+2,Ev.cols,CV_32F);
	int t=0;
	Ev.row( start ).copyTo( Ev_sub.row(t++) );
	for(int i=0;i<groups[1].index2.size();i++)
	{
		Ev.row( groups[1].index2[i] ).copyTo( Ev_sub.row(t++) );
	}
	Ev.row( end ).copyTo( Ev_sub.row(t++) );

	//tsp_brute tsp;
	//tsp.start(Ev_sub,0,t-1,TSP_brute_sort_index);
	TSP_boost_by_EdgeWeight(Ev_sub,TSP_brute_sort_index,0,t-1);
	//cout << "TSP_brute_sort_index " << TSP_brute_sort_index << endl;
	for(int i=1;i<TSP_brute_sort_index.rows-1;i++)
	{
		int index = TSP_brute_sort_index.at<int>(i,0) - 1;
		groups[1].sort_index.push_back( groups[1].index2[ index ] );
	}
	groups[1].sort_index.push_back(end);

	cout << "temp: ";
	temp.clear();
	temp = groups[2].index2;
	groups[2].index2.clear();
	for(int i=0;i<temp.size();i++)
	{
		if(temp[i]!=end)
		{
			groups[2].index2.push_back( temp[i] );
			cout << temp[i] << " ";
		}
	}
	cout << endl;

	start = end;
	cout << "start " << start << endl;
	///////////////////////////////////////////////////////////////////////
	Ev_sub = Mat::zeros(groups[2].index2.size()+1,Ev.cols,CV_32F);
	histo_sort_index_sub = Mat::zeros(groups[2].index2.size()+1,1,CV_32S);	
	for(int i=0;i<groups[2].index2.size();i++)
	{
		int index = groups[2].index2[i];		
		Ev.row(index).copyTo( Ev_sub.row(i) );
	}
	min_len = 10000;
	min_target = 0;
	for(int j=0;j<groups[3].index2.size();j++)
	{
		int index = groups[3].index2[j];
		Ev.row(index).copyTo( Ev_sub.row(groups[2].index2.size()) );
		double len = TSP_boost_path_by_EdgeWeight(Ev_sub, histo_sort_index_sub);
		//double len = TSP_path(Ev_PCA2D_sub, histo_sort_index_sub);
		if(len < min_len) 
		{
			min_len = len;
			min_target = j;
			optimal_sort_index = histo_sort_index_sub.clone();
		}
	}

	end = groups[3].index2[min_target];
	cout << "end " << end << endl;

	TSP_brute_sort_index = Mat::zeros(groups[2].index2.size()+2,1,CV_32S);
	Ev_sub = Mat::zeros(groups[2].index2.size()+2,Ev.cols,CV_32F);
	t = 0;
	Ev.row( start ).copyTo( Ev_sub.row(t++) );
	for(int i=0;i<groups[2].index2.size();i++)
	{
		Ev.row( groups[2].index2[i] ).copyTo( Ev_sub.row(t++) );
	}
	Ev.row( end ).copyTo( Ev_sub.row(t++) );

	//tsp_brute tsp;
	//tsp.start(Ev_sub,0,t-1,TSP_brute_sort_index);
	TSP_boost_by_EdgeWeight(Ev_sub,TSP_brute_sort_index,0,t-1);

	//cout << "TSP_brute_sort_index " << TSP_brute_sort_index << endl;
	for(int i=1;i<TSP_brute_sort_index.rows-1;i++)
	{
		int index = TSP_brute_sort_index.at<int>(i,0) - 1;
		groups[2].sort_index.push_back( groups[2].index2[ index ] );
	}
	groups[2].sort_index.push_back(end);
	
	cout << "temp: ";
	temp.clear();
	temp = groups[3].index2;
	groups[3].index2.clear();
	for(int i=0;i<temp.size();i++)
	{
		if(temp[i]!=end)
		{
			groups[3].index2.push_back( temp[i] );
			cout << temp[i] << " ";
		}
	}
	cout << endl;	

	start = end;
	cout << "start " << start << endl;
	/////////////////////////////////////////////////////////
	Ev_sub = Mat::zeros(groups[3].index2.size()+1,Ev.cols,CV_32F);
	histo_sort_index_sub = Mat::zeros(groups[3].index2.size()+1,1,CV_32S);	
	Ev.row(start).copyTo( Ev_sub.row(0) );
	for(int i=0;i<groups[3].index2.size();i++)
	{
		int index = groups[3].index2[i];
		//Ev_sub.at<float>(i+1,0) = Ev.at<float>(index,0);
		//Ev_sub.at<float>(i+1,1) = Ev.at<float>(index,1);		
		Ev.row(index).copyTo( Ev_sub.row(i+1) );
	}
	//TSP_boost(Ev_sub, histo_sort_index_sub);
	TSP_boost_by_EdgeWeight(Ev_sub,histo_sort_index_sub);
	//TSP_boost_path_by_EdgeWeight(Ev_sub, histo_sort_index_sub);
	cout << "histo_sort_index_sub " << histo_sort_index_sub << endl;

	for(int i=0;i<histo_sort_index_sub.rows;i++)
	{
		int index = histo_sort_index_sub.at<int>(i,0);
		if( index==0 )
		{
			continue;
		}
		else
		{
			groups[3].sort_index.push_back( groups[3].index2[index-1] ); //////////////////
		}
	}

	t = 0;
	cout << "sort index :" << endl;
	fout << "sort index :" << endl;
 	for(int i=0;i<4;i++)
	{
		for(int j=0;j<groups[i].sort_index.size();j++)
		{
			cout << groups[i].sort_index[j] << " ";
			fout << groups[i].sort_index[j] + 1 << " ";
			histo_sort_index.at<int>(t,0) = groups[i].sort_index[j];
			t++;
		}
		cout << endl;
		fout << endl;
	}


	groups.clear();
	fout.close();
	
}

void Preprocessing_Data::Position_by_histogram_sort_index(Mat& histo_position,Mat histo_sort_index)
{
	int k = histogram.cols;
	int five_minutes = histogram.rows;
	Mat histogram_copy = histogram.clone();
	Mat dist = Mat::zeros(1,five_minutes-1,CV_64F);

	for(int i=0;i<five_minutes-1;i++)
	{
		int index = histo_sort_index.at<int>(i,0);
		int index2 = histo_sort_index.at<int>(i+1,0);
		double vote_dist = 0.0;
		for(int j=0;j<k;j++)
		{
			vote_dist += abs( histogram_copy.at<int>(index,j) - histogram_copy.at<int>(index2,j) );
		}

		dist.at<double>(0,i) = pow(vote_dist,5);
	}
	//output_mat_as_csv_file_double("vote_dist.csv",dist.t());
	
	Mat histo_position_sort = Mat::zeros(five_minutes,1,CV_64F);
	histo_position_sort.at<double>(0,0) = 0.0;
	double accumulate_dist = 0.0;
	for(int i=0;i<five_minutes-1;i++)
	{
		accumulate_dist += dist.at<double>(0,i);
		histo_position_sort.at<double>(i+1,0) = accumulate_dist;
	}
	//output_mat_as_csv_file_double("histo_position_sort.csv",histo_position_sort);

	normalize(histo_position_sort.col(0),histo_position_sort.col(0),0,7000,NORM_MINMAX); 

	for(int i=0;i<five_minutes;i++)
	{
		int index = histo_sort_index.at<int>(i,0);
		histo_position.at<double>(index,0) = histo_position_sort.at<double>(i,0);
	}
}