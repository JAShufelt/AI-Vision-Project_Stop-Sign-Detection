#pragma once
class Pixel
{public:
	unsigned char r, g, b;
	double G;
	int G_theta;
	int G_theta_approx;
	int y_coord;
	int x_coord;

	Pixel();
	~Pixel();
};

