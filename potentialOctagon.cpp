#include "potentialOctagon.h"

std::vector<int> potentialOctagon::findIntersection(Line one, Line two)
{
	double degree_to_radian = 0.01745329251;
	std::vector<int> coord_pair;

	int x = ((((sin(one.theta * degree_to_radian)) * ((-1) * (two.r_value))) - ((sin(two.theta * degree_to_radian)) * ((-1) * (one.r_value)))) / ((cos(one.theta * degree_to_radian) * sin(two.theta * degree_to_radian)) - (cos(two.theta * degree_to_radian) * sin(one.theta * degree_to_radian))));
	int y = (((((-1) * one.r_value) * cos(two.theta * degree_to_radian)) - (((-1) * two.r_value) * cos(one.theta * degree_to_radian))) / ((cos(one.theta * degree_to_radian) * sin(two.theta * degree_to_radian)) - (cos(two.theta * degree_to_radian) * sin(one.theta * degree_to_radian))));

	coord_pair = { y, x };
	return coord_pair;
}

void potentialOctagon::calculateAllIntersections()
{

	int1_2 = findIntersection(line1, line2);
	int1_3 = findIntersection(line1, line3);
	int2_3 = findIntersection(line2, line3);
	int2_4 = findIntersection(line2, line4);
	int3_4 = findIntersection(line3, line4);
	int3_5 = findIntersection(line3, line5);
	int4_5 = findIntersection(line4, line5);
	int5_6 = findIntersection(line5, line6);
	int4_6 = findIntersection(line4, line6);
}

double  potentialOctagon::triangleArea(std::vector<int> point1, std::vector<int> point2, std::vector<int> point3)
{
	int x1, x2, x3, y1, y2, y3;
	x1 = point1[1];
	x2 = point2[1];
	x3 = point3[1];
	y1 = point1[0];
	y2 = point2[0];
	y3 = point3[0];

	double area;
	area = (x1 * y2) + (x2 * y3) + (x3 * y1);
	area = area - (x1 * y3) - (x2 * y1) - (x3 * y2);
	area = abs(area / 2);
	return area;
}