#ifndef DAY_H
#define DAY_H

#include <iostream>
#include <cstdio>
#include <cstdlib>

class day{
public:
	int year;
	int month;
	int date;
	//int red_traffic_volulme;
	//int orange_traffic_volulme;
	//int total_traffic_volume;
	float data[100];
	int data_dim;

	bool IsHoliday;
};

#endif