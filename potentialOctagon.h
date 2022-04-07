#pragma once
#include "Line.h"
#include <vector>

class potentialOctagon
{
public:
	Line line1, line2, line3, line4, line5, line6;
	std::vector<int> int1_2, int1_3, int2_3, int2_4, int3_4, int3_5, int4_5, int5_6, int4_6;

	std::vector<int> findIntersection(Line one, Line two);

	void calculateAllIntersections();

	double triangleArea(std::vector<int> point1, std::vector<int> point2, std::vector<int> point3);

};

