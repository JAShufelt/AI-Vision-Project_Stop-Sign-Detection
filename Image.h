#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
#include "Pixel.h"
#include "Line.h"
#include "time.h"
#include "potentialOctagon.h"

class Image
{
public:
	int width;			//Width of Picture 
	int height;			//Height of Picture	
	int paddingAmount;	//Amount of padding in bytes at the end of each row of the image
	unsigned char header[54];
	std::vector<std::vector<Pixel>> pixelArray;
	std::vector<Line> lineSet;
	std::vector<std::vector<std::vector<int>>> finalEdgeGroups;
	std::vector<std::vector<Line>> parallelLineSet;
	std::vector<potentialOctagon> potentialOctagonSet;
	potentialOctagon stopSign;

	
	Image();

	void readImage(std::string path);

	void convertBWImage();

	void exportImage(std::string path);

	void histogramEqualizeImage();

	Image convertEdgeMap(int threshold);

	void thinEdgeMap();

	void lineDetection();

	void smoothImage(int window_size);

	void medianSmoothImage(int window_size);

	void drawLines();

	std::vector<int> mergeSort(std::vector<int>& numbers);

	int findMedian(std::vector<int> numbers);

	Image sobelEdgeMap();

	void nonMaxSuppression();

	void gaussianSmooth();

	void edgeColorize();

	void edgeThreshold(int threshold);

	void doubleThreshold(int upper, int lower);

	void weightLineContinuitity();

	void thetaEdgeThreshold(int threshold, int min);

	void regressionLineDetection();

	void findParallelLines();

	std::vector<int> findIntersection(Line one, Line two);

	void drawIntersections();

	void generateOctagons();

	void detectStopSign();

	double triangleArea(std::vector<int> point1, std::vector<int> point2, std::vector<int> point3);

	~Image();
};

