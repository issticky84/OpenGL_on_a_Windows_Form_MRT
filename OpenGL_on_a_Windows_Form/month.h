#ifndef MONTH_H
#define MONTH_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "day.h"

using namespace std;

class month{
public:
	vector<day> day_vec;
	int this_year;
	int this_month;
};

#endif