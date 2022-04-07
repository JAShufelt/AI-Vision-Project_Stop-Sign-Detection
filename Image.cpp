#include "Image.h"

Image::Image()
{
}

Image::~Image()
{
}

void Image::readImage(std::string path)
{
	Image newImage;
	std::fstream reader;
	
	reader.open(path, std::ios::in | std::ios::binary);

	if(!reader.is_open())
	{
		std::cout << "Could not open";
	}

	reader.read(reinterpret_cast<char *>(header), 54);
	//headerdata = newImage.header;
	//reinterpret_cast<char*>(headerdata);

	int fileSize = header[2] + (header[3] << 8) + (header[4] << 16) + (header[5] << 24);
	width = (header[18]) + (header[19] << 8) + (header[20] << 16) + (header[21] << 24);
	height = (header[22]) + (header[23] << 8) + (header[24] << 16) + (header[25] << 24);

	paddingAmount = ((4 - (width * 3) % 4) % 4);
	const int pixelDataSize = fileSize - 54;

	pixelArray.reserve(height);
	for (int i = 0; i < height; i++)
	{
		std::vector<Pixel> row;
		row.reserve(height);
		pixelArray.push_back(row);
	}
	
	for (int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			Pixel newPixel;
			unsigned char color[3];
			reader.read(reinterpret_cast<char*>(color), 3);
			
			newPixel.b = color[0];
			newPixel.g = color[1];
			newPixel.r = color[2];

			newPixel.x_coord = x + 1;
			newPixel.y_coord = y + 1;

			pixelArray[y].push_back(newPixel);
		}
		reader.ignore(paddingAmount);
	}
}

void Image::convertBWImage()
{
	unsigned char midgray = 127;
	unsigned char black = 0;
	unsigned char white = 255;
	
	for(int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if(pixelArray[y][x].r >= midgray)
			{
				pixelArray[y][x].r = white;
				pixelArray[y][x].b = white;
				pixelArray[y][x].g = white;
			}

			else
			{
				pixelArray[y][x].r = black;
				pixelArray[y][x].b = black;
				pixelArray[y][x].g = black;
			}
		}
	}
}

void Image::exportImage(std::string path)
{
	std::fstream writer;
	writer.open(path, std::ios::out | std::ios::binary);

	if(!writer.is_open())
	{
		std::cout << "There was an error with Image exporting" << std::endl;
	}

	unsigned char padding[3] = {0, 0, 0};
	
	writer.write(reinterpret_cast<char*>(header), 54);

	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			unsigned char color[3];
			color[0] = pixelArray[y][x].b;
			color[1] = pixelArray[y][x].g;
			color[2] = pixelArray[y][x].r;

			writer.write(reinterpret_cast<char*>(color), 3);
		}
		writer.write(reinterpret_cast<char*>(padding), paddingAmount);
	}
}

void Image::histogramEqualizeImage()
{
	int H[256];	//Histogram of values
	int value;
	
	for(int i = 0; i < 256; i++)	//Initialize
	{
		H[i] = 0;
	}

	for(int y = 0; y < height; y++)	//Collect values
	{
		for(int x = 0; x < width; x++)
		{
			value = int(pixelArray[y][x].r);
			H[value] = H[value] + 1;
		}
	}

	for(int i = 1; i < 256; i++)
	{
		H[i] = H[i] + H[i - 1];
	}

	for (int i = 0; i < 256; i++)
	{
		H[i] = int((H[i] * 255) / (width * height));
	}

	for (int y = 0; y < height; y++)	//Collect values
	{
		for (int x = 0; x < width; x++)
		{
			value = int(pixelArray[y][x].r);
			pixelArray[y][x].r = unsigned char(H[value]);
			pixelArray[y][x].g = unsigned char(H[value]);
			pixelArray[y][x].b = unsigned char(H[value]);
		}
	}
};

Image Image::convertEdgeMap(int threshold)
{
	Image newImage;				//Generate a new image of the same dimensions
	newImage.width = width;		//Copy width
	newImage.height = height;	//Copy height
	newImage.paddingAmount = paddingAmount;	//Copy paddingAmount
	
	for(int i = 0; i < 54; i++)	//Copy header
	{
		newImage.header[i] = header[i];
	}

	for(int i = 0; i < height; i++)	//Occupy 2D pixel array with empty rows of pixel arrays
	{
		std::vector<Pixel> new_row;
		newImage.pixelArray.push_back(new_row);
	}
	
	for (int y = 0; y < height; y++)	//Occupy all pixels with 255 white
	{
		for (int x = 0; x < width; x++)
		{
			newImage.pixelArray[y].push_back(pixelArray[y][x]);

			newImage.pixelArray[y][x].r = unsigned char(255);
			newImage.pixelArray[y][x].g = unsigned char(255);
			newImage.pixelArray[y][x].b = unsigned char(255);
		}
	}


	
	
	for(int y = 1; y < height -1; y++)	// 3x3 window edge detection, vertical, horizontal, and diagonals
	{
		for(int x = 1; x < width -1; x++)
		{
			//Horizontal Edge Detection
			int top_sum = 0;
			int bottom_sum = 0;
			
			top_sum = int(pixelArray[y + 1][x - 1].r) + int(pixelArray[y + 1][x].r) + int(pixelArray[y + 1][x + 1].r);
			bottom_sum = int(pixelArray[y - 1][x - 1].r) + int(pixelArray[y - 1][x].r) + int(pixelArray[y - 1][x + 1].r);

			if (int(abs(top_sum - bottom_sum)) >= threshold)
			{
				newImage.pixelArray[y][x].r = unsigned char(0);
				newImage.pixelArray[y][x].g = unsigned char(0);
				newImage.pixelArray[y][x].b = unsigned char(0);
			};
			
			//Vertical Edge Detection
			int left_sum = 0;
			int right_sum = 0;

			left_sum = int(pixelArray[y + 1][x - 1].r) + int(pixelArray[y][x - 1].r) + int(pixelArray[y - 1][x - 1].r);
			right_sum = int(pixelArray[y + 1][x + 1].r) + int(pixelArray[y][x + 1].r) + int(pixelArray[y - 1][x + 1].r);

			if (int(abs(left_sum - right_sum)) >= threshold)
			{
				newImage.pixelArray[y][x].r = unsigned char(0);
				newImage.pixelArray[y][x].g = unsigned char(0);
				newImage.pixelArray[y][x].b = unsigned char(0);
			};

			//Diagonal (positive slope) Edge Detection 
			int upper_corner = 0;
			int lower_corner = 0;

			upper_corner = int(pixelArray[y + 1][x].r) + int(pixelArray[y + 1][x + 1].r) + int(pixelArray[y][x + 1].r);
			lower_corner = int(pixelArray[y - 1][x].r) + int(pixelArray[y - 1][x - 1].r) + int(pixelArray[y][x - 1].r);

			if (int(abs(upper_corner - lower_corner)) >= threshold)
			{
				newImage.pixelArray[y][x].r = unsigned char(0);
				newImage.pixelArray[y][x].g = unsigned char(0);
				newImage.pixelArray[y][x].b = unsigned char(0);
			}
			
			//Diagonal (negative slope) Edge Detection
			upper_corner = 0;
			lower_corner = 0;

			upper_corner = int(pixelArray[y + 1][x].r) + int(pixelArray[y + 1][x - 1].r) + int(pixelArray[y][x - 1].r);
			lower_corner = int(pixelArray[y - 1][x].r) + int(pixelArray[y - 1][x + 1].r) + int(pixelArray[y][x + 1].r);

			if (int(abs(upper_corner - lower_corner)) >= threshold)
			{
				newImage.pixelArray[y][x].r = unsigned char(0);
				newImage.pixelArray[y][x].g = unsigned char(0);
				newImage.pixelArray[y][x].b = unsigned char(0);
			}
		}
	}

	return newImage;
}

void Image::thinEdgeMap()
{
	bool reduction_made = true;

	while(reduction_made)
	{
		reduction_made = false;

		std::vector<std::vector<int>> thinning_coordinates;	//Start a vector to hold the thinning coordinates
		//TOP SHAVE
		for(int y = 1; y < height-1; y++)	//Detecting edges which may be shaved from the top
		{
			for(int x = 1; x < width - 1; x++)
			{
				if(int(pixelArray[y][x].r) == 0)	//If we've found an edge pixel
				{
					if(int(pixelArray[y+1][x-1].r) + int(pixelArray[y + 1][x].r) + int(pixelArray[y + 1][x + 1].r) == 765)	//If all the pixels above are space
					{
						if (int(pixelArray[y - 1][x - 1].r) + int(pixelArray[y - 1][x].r) + int(pixelArray[y - 1][x + 1].r) == 0) //If all the pixels below are edges
						{
							std::vector<int> coordinate;
							coordinate.push_back(y);
							coordinate.push_back(x);

							thinning_coordinates.push_back(coordinate);
							reduction_made = true;
						}
					}
				}
			}
		} 
		
		for(int i = 0; i < thinning_coordinates.size(); i++)	//Shaving all of the known top shavable pixels
		{
			int y = thinning_coordinates[i][0];
			int x = thinning_coordinates[i][1];

			pixelArray[y][x].r = unsigned char(255);
			pixelArray[y][x].g = unsigned char(255);
			pixelArray[y][x].b = unsigned char(255);
		}

		//LEFT SHAVE
		thinning_coordinates.clear();	//Reset the coordinates of pixels to be shaved
		for (int y = 1; y < height - 1; y++)
		{
			for (int x = 1; x < width - 1; x++)
			{
				if (int(pixelArray[y][x].r) == 0)	//If we've found an edge pixel
				{
					if (int(pixelArray[y + 1][x - 1].r) + int(pixelArray[y][x - 1].r) + int(pixelArray[y - 1][x - 1].r) == 765)	//If all the pixels above are space
					{
						if (int(pixelArray[y + 1][x + 1].r) + int(pixelArray[y][x + 1].r) + int(pixelArray[y - 1][x + 1].r) == 0) //If all the pixels below are edges
						{
							std::vector<int> coordinate;
							coordinate.push_back(y);
							coordinate.push_back(x);

							thinning_coordinates.push_back(coordinate);
							reduction_made = true;
						}
					}
				}
			}
		}

		for (int i = 0; i < thinning_coordinates.size(); i++)
		{
			int y = thinning_coordinates[i][0];
			int x = thinning_coordinates[i][1];

			pixelArray[y][x].r = unsigned char(255);
			pixelArray[y][x].g = unsigned char(255);
			pixelArray[y][x].b = unsigned char(255);
		}

		//BOTTOM SHAVE
		thinning_coordinates.clear();	//Reset the coordinates of pixels to be shaved
		for (int y = 1; y < height - 1; y++)
		{
			for (int x = 1; x < width - 1; x++)
			{
				if (int(pixelArray[y][x].r) == 0)	//If we've found an edge pixel
				{
					if (int(pixelArray[y - 1][x - 1].r) + int(pixelArray[y - 1][x].r) + int(pixelArray[y - 1][x + 1].r) == 765)	//If all the pixels above are space
					{
						if (int(pixelArray[y + 1][x - 1].r) + int(pixelArray[y + 1][x].r) + int(pixelArray[y + 1][x + 1].r) == 0) //If all the pixels below are edges
						{
							std::vector<int> coordinate;
							coordinate.push_back(y);
							coordinate.push_back(x);

							thinning_coordinates.push_back(coordinate);
							reduction_made = true;
						}
					}
				}
			}
		}

		for (int i = 0; i < thinning_coordinates.size(); i++)
		{
			int y = thinning_coordinates[i][0];
			int x = thinning_coordinates[i][1];

			pixelArray[y][x].r = unsigned char(255);
			pixelArray[y][x].g = unsigned char(255);
			pixelArray[y][x].b = unsigned char(255);
		}

		//RIGHT SHAVE
		thinning_coordinates.clear();	//Reset the coordinates of pixels to be shaved
		for (int y = 1; y < height - 1; y++)
		{
			for (int x = 1; x < width - 1; x++)
			{
				if (int(pixelArray[y][x].r) == 0)	//If we've found an edge pixel
				{
					if (int(pixelArray[y + 1][x - 1].r) + int(pixelArray[y][x - 1].r) + int(pixelArray[y - 1][x - 1].r) == 0)	//If all the pixels above are space
					{
						if (int(pixelArray[y + 1][x + 1].r) + int(pixelArray[y][x + 1].r) + int(pixelArray[y - 1][x + 1].r) == 765) //If all the pixels below are edges
						{
							std::vector<int> coordinate;
							coordinate.push_back(y);
							coordinate.push_back(x);

							thinning_coordinates.push_back(coordinate);
							reduction_made = true;
						}
					}
				}
			}
		}

		for (int i = 0; i < thinning_coordinates.size(); i++)
		{
			int y = thinning_coordinates[i][0];
			int x = thinning_coordinates[i][1];

			pixelArray[y][x].r = unsigned char(255);
			pixelArray[y][x].g = unsigned char(255);
			pixelArray[y][x].b = unsigned char(255);
		}

		//UPPER RIGHT CORNER SHAVE
		thinning_coordinates.clear();	//Reset the coordinates of pixels to be shaved
		for (int y = 1; y < height - 1; y++)
		{
			for (int x = 1; x < width - 1; x++)
			{
				if (int(pixelArray[y][x].r) == 0)	//If we've found an edge pixel
				{
					if (int(pixelArray[y + 1][x].r) + int(pixelArray[y + 1][x + 1].r) + int(pixelArray[y][x + 1].r) == 765)	//If all the pixels above are space
					{
						if (int(pixelArray[y][x - 1].r) + int(pixelArray[y - 1][x].r) == 0) //If all the pixels below are edges
						{
							std::vector<int> coordinate;
							coordinate.push_back(y);
							coordinate.push_back(x);

							thinning_coordinates.push_back(coordinate);
							reduction_made = true;
						}
					}
				}
			}
		}

		for (int i = 0; i < thinning_coordinates.size(); i++)
		{
			int y = thinning_coordinates[i][0];
			int x = thinning_coordinates[i][1];

			pixelArray[y][x].r = unsigned char(255);
			pixelArray[y][x].g = unsigned char(255);
			pixelArray[y][x].b = unsigned char(255);
		}

		//LOWER RIGHT CORNER SHAVE
		thinning_coordinates.clear();	//Reset the coordinates of pixels to be shaved
		for (int y = 1; y < height - 1; y++)
		{
			for (int x = 1; x < width - 1; x++)
			{
				if (int(pixelArray[y][x].r) == 0)	//If we've found an edge pixel
				{
					if (int(pixelArray[y - 1][x].r) + int(pixelArray[y - 1][x + 1].r) + int(pixelArray[y][x + 1].r) == 765)	//If all the pixels above are space
					{
						if (int(pixelArray[y][x - 1].r) + int(pixelArray[y + 1][x].r) == 0) //If all the pixels below are edges
						{
							std::vector<int> coordinate;
							coordinate.push_back(y);
							coordinate.push_back(x);

							thinning_coordinates.push_back(coordinate);
							reduction_made = true;
						}
					}
				}
			}
		}

		for (int i = 0; i < thinning_coordinates.size(); i++)
		{
			int y = thinning_coordinates[i][0];
			int x = thinning_coordinates[i][1];

			pixelArray[y][x].r = unsigned char(255);
			pixelArray[y][x].g = unsigned char(255);
			pixelArray[y][x].b = unsigned char(255);
		}

		//LOWER LEFT CORNER SHAVE
		thinning_coordinates.clear();	//Reset the coordinates of pixels to be shaved
		for (int y = 1; y < height - 1; y++)
		{
			for (int x = 1; x < width - 1; x++)
			{
				if (int(pixelArray[y][x].r) == 0)	//If we've found an edge pixel
				{
					if (int(pixelArray[y - 1][x].r) + int(pixelArray[y - 1][x - 1].r) + int(pixelArray[y][x - 1].r) == 765)	//If all the pixels above are space
					{
						if (int(pixelArray[y + 1][x].r) + int(pixelArray[y][x + 1].r) == 0) //If all the pixels below are edges
						{
							std::vector<int> coordinate;
							coordinate.push_back(y);
							coordinate.push_back(x);

							thinning_coordinates.push_back(coordinate);
							reduction_made = true;
						}
					}
				}
			}
		}

		for (int i = 0; i < thinning_coordinates.size(); i++)
		{
			int y = thinning_coordinates[i][0];
			int x = thinning_coordinates[i][1];

			pixelArray[y][x].r = unsigned char(255);
			pixelArray[y][x].g = unsigned char(255);
			pixelArray[y][x].b = unsigned char(255);
		}

		//UPPER LEFT CORNER SHAVE
		thinning_coordinates.clear();	//Reset the coordinates of pixels to be shaved
		for (int y = 1; y < height - 1; y++)
		{
			for (int x = 1; x < width - 1; x++)
			{
				if (int(pixelArray[y][x].r) == 0)	//If we've found an edge pixel
				{
					if (int(pixelArray[y][x - 1].r) + int(pixelArray[y + 1][x - 1].r) + int(pixelArray[y + 1][x].r) == 765)	//If all the pixels above are space
					{
						if (int(pixelArray[y][x + 1].r) + int(pixelArray[y - 1][x].r) == 0) //If all the pixels below are edges
						{
							std::vector<int> coordinate;
							coordinate.push_back(y);
							coordinate.push_back(x);

							thinning_coordinates.push_back(coordinate);
							reduction_made = true;
						}
					}
				}
			}
		}

		for (int i = 0; i < thinning_coordinates.size(); i++)
		{
			int y = thinning_coordinates[i][0];
			int x = thinning_coordinates[i][1];

			pixelArray[y][x].r = unsigned char(255);
			pixelArray[y][x].g = unsigned char(255);
			pixelArray[y][x].b = unsigned char(255);
		}

		//reduction_made = false;
	}
}

void Image::lineDetection()
{
	int param_space_theta = 360;
	int param_space_r;

	param_space_r = sqrt((width * width) + (height * height));

	param_space_r = (param_space_r * 2);	//Account for negatives -r to r;

	//Initialize Parameter Space Array
	std::vector<std::vector<int>> param_space;	
	for (int i = 0; i < param_space_r; i++)
	{
		std::vector<int> param_space_row;
		for(int i = 0; i < param_space_theta; i++)
		{
			param_space_row.push_back(0);
		}
		
		param_space.push_back(param_space_row);
	}

	//Collect all coordinates of edges
	std::vector<std::vector<int>> edge_coords;
	for (int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			if(pixelArray[y][x].r == 255)
			{
				std::vector<int> coord_pair;
				coord_pair.push_back(x + 1);
				coord_pair.push_back(y + 1);

				edge_coords.push_back(coord_pair);
			}
		}
	}

	const double to_radians = 0.017453292519943; // Pi/180
	
	for (int i = 0; i < edge_coords.size(); i++)
	{
		double r_value;
		int x_coord = edge_coords[i][0];
		int y_coord = edge_coords[i][1];

		for(double theta = 0; theta < 180; theta += 0.5)
		{
			r_value = double(x_coord * cos(double(theta * to_radians))) + double(y_coord * sin(double(theta * to_radians)));
			
			param_space[int(r_value) + (int(param_space_r / 2))][int(theta * 2)] += 1;
		}
	}

	int largest_accumulation = 0;
	int most_common_r;
	int most_common_theta;

	//Find the most prominent point
	for(int i = 0; i < param_space.size(); i++)
	{
		for(int j = 0; j < param_space[1].size(); j++)
		{
			if(param_space[i][j] > largest_accumulation)
			{
				most_common_r = i - (int(param_space_r / 2));
				most_common_theta = j;

				largest_accumulation = param_space[i][j];
			}
		}
	}

	/*int count = 0;
	for (int i = 0; i < param_space.size(); i++)
	{
		for (int j = 0; j < param_space[1].size(); j++)
		{
			if(param_space[i][j] >= 1)
			{
				Line newLine;
				newLine.line_id = 0;
				newLine.r_value = i - (int(param_space_r / 2));
				newLine.theta = double(j) / 2;
				newLine.votes = param_space[i][j];
				
				lineSet.push_back(newLine);
				count++;
			}
		}
	}*/

	int largest_vote_count = -1;
	int count = 0;
	for (int i = 40; i < param_space.size() - 1; i+= 81)
	{
		for (int j = 10; j < param_space[1].size() - 1; j+= 21)
		{
			
			int r_window_midpoint = i ;
			int theta_window_midpoint = j;
			int most_voted_r;
			int most_voted_theta;

			// Setting Y window
			for(int y = -40; y < 41; y++)
			{
				// Setting X window
				for(int x = -10; x < 11; x++)
				{
					//If the the point in the window is a valid point in the param_space
					if((-1 < (y + r_window_midpoint)) && ((y + r_window_midpoint) < (param_space.size())) && (-1 < x + (theta_window_midpoint)) && ((x + (theta_window_midpoint)) < (param_space[1].size())))
					{
						if(param_space[y + r_window_midpoint][x + theta_window_midpoint] > largest_vote_count)
						{
							largest_vote_count = param_space[y + r_window_midpoint][x + theta_window_midpoint];
							most_voted_r =(y + r_window_midpoint) - (int(param_space_r / 2));
							most_voted_theta = double(x + theta_window_midpoint) / 2;
						}
					}
				}
			}
			if(largest_vote_count > 0)
			{
				Line newLine;
				newLine.r_value = most_voted_r;
				newLine.theta = most_voted_theta;
				newLine.votes = largest_vote_count;
				newLine.line_id = count;

				lineSet.push_back(newLine);
			}

			count++;
			largest_vote_count = -1;
		}
	}

	

	std::vector<Line> finalLines;
	int largest_vote = 0;
	int element_number = 0;
	for(int j = 0; j < 100; j++)
	{
		for (int i = 0; i < lineSet.size(); i++)
		{
			if (lineSet[i].votes > largest_vote)
			{
				element_number = i;
				largest_vote = lineSet[i].votes;
			}
		}
		finalLines.push_back(lineSet[element_number]);
		lineSet[element_number].votes = -1;
		largest_vote = 0;
	}
	lineSet = finalLines;


	Image param_space_visual;
	param_space_visual.height = param_space_r;
	param_space_visual.width = param_space_theta;
	param_space_visual.paddingAmount = ((4 - (param_space_visual.width * 3) % 4) % 4);
	
	int fileSize = 54 + param_space_visual.width + (param_space_visual.height * 3) + (param_space_visual.paddingAmount * param_space_visual.height);
	param_space_visual.header[0] = 'B';
	param_space_visual.header[1] = 'M';
	
	param_space_visual.header[2] = fileSize;
	param_space_visual.header[3] = fileSize >> 8;
	param_space_visual.header[4] = fileSize >> 16;
	param_space_visual.header[5] = fileSize >> 24;

	param_space_visual.header[6] = 0;
	param_space_visual.header[7] = 0;
	param_space_visual.header[8] = 0;
	param_space_visual.header[9] = 0;
	// Pixel data offset
	param_space_visual.header[10] = 54;
	param_space_visual.header[11] = 0;
	param_space_visual.header[12] = 0;
	param_space_visual.header[13] = 0;

	param_space_visual.header[14] = 40;
	param_space_visual.header[15] = 0;
	param_space_visual.header[16] = 0;
	param_space_visual.header[17] = 0;
	// Image width
	param_space_visual.header[18] = param_space_visual.width;
	param_space_visual.header[19] = param_space_visual.width >> 8;
	param_space_visual.header[20] = param_space_visual.width >> 16;
	param_space_visual.header[21] = param_space_visual.width >> 24;
	// Image height
	param_space_visual.header[22] = param_space_visual.height;
	param_space_visual.header[23] = param_space_visual.height >> 8;
	param_space_visual.header[24] = param_space_visual.height >> 16;
	param_space_visual.header[25] = param_space_visual.height >> 24;
	// Planes
	param_space_visual.header[26] = 1;
	param_space_visual.header[27] = 2;
	// Bits per pixel (RGB)
	param_space_visual.header[28] = 24;
	param_space_visual.header[29] = 0;
	// Compression (No compression)
	param_space_visual.header[30] = 0;
	param_space_visual.header[31] = 0;
	param_space_visual.header[32] = 0;
	param_space_visual.header[33] = 0;
	// Image size (No compression)
	param_space_visual.header[34] = 0;
	param_space_visual.header[35] = 0;
	param_space_visual.header[36] = 0;
	param_space_visual.header[37] = 0;
	// X pixels per meter (Not specified)
	param_space_visual.header[38] = 0;
	param_space_visual.header[39] = 0;
	param_space_visual.header[40] = 0;
	param_space_visual.header[41] = 0;
	// Y pixels per meter (Not specified)
	param_space_visual.header[42] = 0;
	param_space_visual.header[43] = 0;
	param_space_visual.header[44] = 0;
	param_space_visual.header[45] = 0;
	// Total colors (Color palette not used)
	param_space_visual.header[46] = 0;
	param_space_visual.header[47] = 0;
	param_space_visual.header[48] = 0;
	param_space_visual.header[49] = 0;
	// Important colors (Generally ignored)
	param_space_visual.header[51] = 0;
	param_space_visual.header[52] = 0;
	param_space_visual.header[53] = 0;
	param_space_visual.header[54] = 0;

	
	for(int y = 0; y < param_space_visual.height; y++)
	{	
		std::vector<Pixel> newRow;
		param_space_visual.pixelArray.push_back(newRow);

		for(int x = 0; x < param_space_visual.width; x++)
		{
			Pixel newPixel;
			newPixel.r = int((param_space[y][x]) );
			newPixel.b = int((param_space[y][x]) );
			newPixel.g = int((param_space[y][x]) );

			param_space_visual.pixelArray[y].push_back(newPixel);
		}
		
	}

	param_space_visual.exportImage("Super.bmp");

	std::cout << std::endl << "Most Common R = " << most_common_r << std::endl << "Most Common Theta = " << int(most_common_theta / 2);

}

void Image::smoothImage(int window_size)
{
	int sum = 0;
	int average;
	
	std::vector<std::vector<int>> coord_and_values;
	if (window_size == 3)
	{
		for (int y = 1; y < height - 1; y++)
		{
			for (int x = 1; x < width - 1; x++)
			{
				sum += pixelArray[y + 1][x - 1].r + pixelArray[y + 1][x].r + pixelArray[y + 1][x + 1].r;
				sum += pixelArray[y][x - 1].r + pixelArray[y][x].r + pixelArray[y][x + 1].r;
				sum += pixelArray[y - 1][x - 1].r + pixelArray[y - 1][x].r + pixelArray[y - 1][x + 1].r;

				average = int(sum / 9);

				std::vector<int> coord_and_value_element;
				coord_and_value_element.push_back(x);
				coord_and_value_element.push_back(y);
				coord_and_value_element.push_back(average);

				coord_and_values.push_back(coord_and_value_element);
				sum = 0;
			}
		}
	}
	
	if (window_size == 5)
	{
		for (int y = 2; y < height - 2; y++)
		{
			for (int x = 2; x < width - 2; x++)
			{
				sum += pixelArray[y + 2][x - 2].r; + pixelArray[y + 2][x - 1].r + pixelArray[y + 2][x].r + pixelArray[y + 2][x + 1].r + pixelArray[y + 2][x + 2].r;
				sum += pixelArray[y + 1][x - 2].r; + pixelArray[y + 1][x - 1].r + pixelArray[y + 1][x].r + pixelArray[y + 1][x + 1].r + pixelArray[y + 1][x + 2].r;
				sum += pixelArray[y + 0][x - 2].r; + pixelArray[y + 0][x - 1].r + pixelArray[y + 0][x].r + pixelArray[y + 0][x + 1].r + pixelArray[y + 2][x + 2].r;
				sum += pixelArray[y - 1][x - 2].r; + pixelArray[y - 1][x - 1].r + pixelArray[y - 1][x].r + pixelArray[y - 1][x + 1].r + pixelArray[y - 1][x + 2].r;
				sum += pixelArray[y - 2][x - 2].r; + pixelArray[y - 2][x - 1].r + pixelArray[y - 2][x].r + pixelArray[y - 2][x + 1].r + pixelArray[y - 2][x + 2].r;

				average = int(sum / 25);

				std::vector<int> coord_and_value_element;
				coord_and_value_element.push_back(x);
				coord_and_value_element.push_back(y);
				coord_and_value_element.push_back(average);

				coord_and_values.push_back(coord_and_value_element);
				sum = 0;
			}
		}
	}
	

	for(int i = 0; i < coord_and_values.size(); i++)
	{
		int x = coord_and_values[i][0];
		int y = coord_and_values[i][1];
		int average = coord_and_values[i][2];

		pixelArray[y][x].r = average;
		pixelArray[y][x].g = average;
		pixelArray[y][x].b = average;
	}
}

void Image::drawLines()
{
	const double to_radians = 0.017453292519943;
	int y;
	int x;
	double theta;
	double r;

	for(int i = 0; i < lineSet.size(); i++)
	{
		theta = lineSet[i].theta;
		r = lineSet[i].r_value;

		for (int x = 0; x < width -1; x++)
		{
			y = int(r - (x * (cos(theta * to_radians)))) / (sin(theta * to_radians));

			if(0 <= y && y <= height - 2)
			{
				pixelArray[y + 1][x + 1].r = 255;
				pixelArray[y + 1][x + 1].g = 0;
				pixelArray[y + 1][x + 1].b = 0;
			}
		}
		
		for (int y = 0; y < height - 1; y++)
		{
			x = int(r - (y * (sin(theta * to_radians)))) / (cos(theta * to_radians));

			if (0 <= x && x <= width - 2)
			{
				pixelArray[y + 1][x + 1].r = 255;
				pixelArray[y + 1][x + 1].g = 0;
				pixelArray[y + 1][x + 1].b = 0;
			}
		}
		
	}
}

void Image::medianSmoothImage(int window_size)
{
	std::vector<std::vector<int>> coord_and_values;
	
	for (int y = 1; y < height - 1; y++)
	{
		for(int x = 1; x < width - 1; x++)
		{
			std::vector<int> window;

			window.push_back(pixelArray[y + 1][x - 1].r);
			window.push_back(pixelArray[y + 1][x].r);
			window.push_back(pixelArray[y + 1][x + 1].r);
			window.push_back(pixelArray[y][x - 1].r);
			window.push_back(pixelArray[y][x].r);
			window.push_back(pixelArray[y][x + 1].r);
			window.push_back(pixelArray[y - 1][x - 1].r);
			window.push_back(pixelArray[y - 1][x].r);
			window.push_back(pixelArray[y - 1][x + 1].r);

			window = mergeSort(window);

			std::vector<int> coord_and_value_element;
			coord_and_value_element.push_back(x);
			coord_and_value_element.push_back(y);
			coord_and_value_element.push_back(window[4]);

			coord_and_values.push_back(coord_and_value_element);
		}
	}

	for (int i = 0; i < coord_and_values.size(); i++)
	{
		int x = coord_and_values[i][0];
		int y = coord_and_values[i][1];
		int median = coord_and_values[i][2];

		pixelArray[y][x].r = median;
		pixelArray[y][x].g = median;
		pixelArray[y][x].b = median;
	}
}

std::vector<int> Image::mergeSort(std::vector<int> &numbers)
{
	int elements = numbers.size();
	
	if(elements == 1)
	{
		return numbers;
	}
	else
	{
		std::vector<int> vectorOne;
		std::vector<int> vectorTwo;

		for(int i = 0; i < floor(elements/2); i++)
		{
			vectorOne.push_back(numbers[i]);
		}
		
		for (int i = floor(elements/2); i < elements; i++)
		{
			vectorTwo.push_back(numbers[i]);
		}

		vectorOne = mergeSort(vectorOne);
		vectorTwo = mergeSort(vectorTwo);

		std::vector<int> vectorsMerged;

		int j = 0;
		int k = 0;
		while(j < vectorOne.size() && k < vectorTwo.size())
		{
			if(vectorOne[j] > vectorTwo[k])
			{
				vectorsMerged.push_back(vectorOne[j]);
				j++;
			}
			else
			{
				vectorsMerged.push_back(vectorTwo[k]);
				k++;
			}
		}
		
		if(j < k)
		{
			for(int i = j; i < vectorOne.size(); i++)
			{
				vectorsMerged.push_back(vectorOne[i]);
			}

		}
		else
		{
			for (int i = k; i < vectorTwo.size(); i++)
			{
				vectorsMerged.push_back(vectorTwo[i]);
			}
		}

		return vectorsMerged;
	}

}

int Image::findMedian(std::vector<int> numbers)
{
	return 0;
}

Image Image::sobelEdgeMap()
{
	Image newImage;				//Generate a new image of the same dimensions
	newImage.width = width;		//Copy width
	newImage.height = height;	//Copy height
	newImage.paddingAmount = paddingAmount;	//Copy paddingAmount

	for (int i = 0; i < 54; i++)	//Copy header
	{
		newImage.header[i] = header[i];
	}

	for (int i = 0; i < height; i++)	//Occupy 2D pixel array with empty rows of pixel arrays
	{
		std::vector<Pixel> new_row;
		newImage.pixelArray.push_back(new_row);
	}

	for (int y = 0; y < height; y++)	//Occupy all pixels with 255 white
	{
		for (int x = 0; x < width; x++)
		{
			newImage.pixelArray[y].push_back(pixelArray[y][x]);

			newImage.pixelArray[y][x].r = unsigned char(0);
			newImage.pixelArray[y][x].g = unsigned char(0);
			newImage.pixelArray[y][x].b = unsigned char(0);
		}
	}

	double radian_to_degree = 180 / 3.1415926536;
	int Gx = 0;
	int Gy = 0;
	int G_theta;
	int G_theta_approx;
	double G;
	double max_G = 0;


	
	for(int y = 1; y < height - 1; y++)
	{
		for(int x = 1; x < width - 1; x++)
		{
			Gx += pixelArray[y + 1][x + 1].r + (2 * pixelArray[y][x + 1].r) + pixelArray[y - 1][x + 1].r;
			Gx += -pixelArray[y + 1][x - 1].r - (2 * pixelArray[y][x - 1].r) - pixelArray[y - 1][x - 1].r;

			Gy += pixelArray[y + 1][x - 1].r + (2 * pixelArray[y + 1][x].r) + pixelArray[y + 1][x + 1].r;
			Gy += -pixelArray[y - 1][x - 1].r - (2 * pixelArray[y - 1][x].r) - pixelArray[y - 1][x + 1].r;

			G = sqrt((Gx * Gx) + (Gy * Gy));
			G = G / 100;
			G_theta = atan2(Gy, Gx) * radian_to_degree;
			if(G_theta < 0)
			{
				G_theta += 180;
			}


			if ((0 <= G_theta && G_theta <= 22.5) || (157.5 < G_theta && G_theta <= 180))
			{
				G_theta_approx = 0;
			}
			else if (22.5 < G_theta  && G_theta <= 67.5)
			{
				G_theta_approx = 45;
			}
			else if (67.5 < G_theta && G_theta <= 112.5)
			{
				G_theta_approx = 90;
			}
			else if (112.5 < G_theta && G_theta <= 157.5)
			{
				G_theta_approx = 135;
			}
			
			if(G > max_G)
			{
				max_G = G;
			}
			
			newImage.pixelArray[y][x].G = G;
			newImage.pixelArray[y][x].G_theta = G_theta;
			newImage.pixelArray[y][x].G_theta_approx = G_theta_approx;

			Gy = 0;
			Gx = 0;
		}
	}

	double pixel_scaling = 255 / max_G;

	for(int y = 0; y < newImage.height; y++)
	{
		for(int x = 0; x < newImage.width; x++)
		{
			newImage.pixelArray[y][x].r = (newImage.pixelArray[y][x].G) * pixel_scaling;
			newImage.pixelArray[y][x].g = newImage.pixelArray[y][x].r;
			newImage.pixelArray[y][x].b = newImage.pixelArray[y][x].r;
		}
	}

	return newImage;
}

void Image::nonMaxSuppression()
{
	std::vector<std::vector<int>> coord_vector;

	for(int y = 1; y < height - 1; y++)
	{
		for(int x = 1; x < width - 1; x++)
		{
			switch(pixelArray[y][x].G_theta_approx)
			{
			case 0:
				if( !(pixelArray[y][x + 1].r < pixelArray[y][x].r && pixelArray[y][x - 1].r < pixelArray[y][x].r) )
				{
					std::vector<int> coord_pair = { y, x };
					coord_vector.push_back(coord_pair);
				}
				break;
			case 45:
				if (pixelArray[y + 1][x + 1].r > pixelArray[y][x].r || pixelArray[y - 1][x - 1].r > pixelArray[y][x].r)
				{
					std::vector<int> coord_pair = { y, x };
					coord_vector.push_back(coord_pair);
				}
				break;
			case 90:
				if (pixelArray[y + 1][x].r > pixelArray[y][x].r || pixelArray[y - 1][x].r > pixelArray[y][x].r)
				{
					std::vector<int> coord_pair = { y, x };
					coord_vector.push_back(coord_pair);
				}
				break;
			case 135:
				if (pixelArray[y + 1][x - 1].r > pixelArray[y][x].r || pixelArray[y - 1][x + 1].r > pixelArray[y][x].r)
				{
					std::vector<int> coord_pair = { y, x };
					coord_vector.push_back(coord_pair);
				}
				break;
			default:
				std::cout << "Houston, we've got a problem here... non-maximum suppression performed on a non-edgemap type image.";
				break;
			}
		}
	}

	for(int i = 0; i < coord_vector.size(); i++)
	{
		int y = coord_vector[i][0];
		int x = coord_vector[i][1];

		pixelArray[y][x].r = 0;
		pixelArray[y][x].g = 0;
		pixelArray[y][x].b = 0;
		
		pixelArray[y][x].G = 0;
	}
}

void Image::gaussianSmooth()
{
	int sum = 0;
	std::vector<std::vector<int>> coord_vector;

	for(int y = 1; y < height - 1; y++)
	{
		for(int x = 1; x < width - 1; x++)
		{
			sum += pixelArray[y + 1][x - 1].r + (2 *pixelArray[y + 1][x].r) + pixelArray[y + 1][x + 1].r;
			sum += (2 *pixelArray[y][x - 1].r) + (4 * pixelArray[y][x].r) + (2 * pixelArray[y][x + 1].r);
			sum += pixelArray[y - 1][x - 1].r + (2 * pixelArray[y - 1][x].r) + pixelArray[y - 1][x + 1].r;
			sum = sum / 16;

			std::vector<int> coord_pair;
			coord_pair = { y, x, sum };
			
			coord_vector.push_back(coord_pair);
			sum = 0;
		}
	}

	for(int i = 0; i < coord_vector.size(); i++)
	{
		int y = coord_vector[i][0];
		int x = coord_vector[i][1];
		int value = coord_vector[i][2];

		pixelArray[y][x].r = value;
		pixelArray[y][x].g = value;
		pixelArray[y][x].b = value;
	}
}

void Image::edgeColorize()
{
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			switch(pixelArray[y][x].G_theta_approx)
			{
			case 0:
				pixelArray[y][x].b = 0;
				pixelArray[y][x].g = 0;
				break;
			case 45:
				pixelArray[y][x].r = 0;
				pixelArray[y][x].g = 0;
				break;
			case 90:
				pixelArray[y][x].r = 0;
				pixelArray[y][x].b = 0;
				break;
			case 135:
				pixelArray[y][x].b = 0;
				break;

			}
		}
	}
}

void Image::edgeThreshold(int threshold)
{
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			if (pixelArray[y][x].r < threshold)
			{
				pixelArray[y][x].r = 0;
				pixelArray[y][x].g = 0;
				pixelArray[y][x].b = 0;
			}
			
		}
	}
}

void Image::doubleThreshold(int upper, int lower)
{
	std::vector<std::vector<int>> strong_edges;
	
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			if(pixelArray[y][x].r >= upper)
			{
				pixelArray[y][x].r = 255;
				pixelArray[y][x].g = 255;
				pixelArray[y][x].b = 255;

				std::vector<int> coord_pair = { x, y };
				strong_edges.push_back(coord_pair);
			}
		}
	}

	while(strong_edges.size() > 0)
	{
		int x = strong_edges[strong_edges.size() - 1][0];
		int y = strong_edges[strong_edges.size() - 1][1];
		strong_edges.pop_back();

		if(y < height - 1 && x < width - 1)
		{
			if (lower < pixelArray[y + 1][x + 1].r && pixelArray[y + 1][x + 1].r <= upper)
			{
				pixelArray[y + 1][x + 1].r = 255;
				pixelArray[y + 1][x + 1].g = 255;
				pixelArray[y + 1][x + 1].b = 255;


				std::vector<int> coord_pair = { x + 1, y + 1};
				strong_edges.push_back(coord_pair);
			}
		}

		if (y < height - 1)
		{
			if (lower < pixelArray[y + 1][x].r && pixelArray[y + 1][x].r <= upper)
			{
				pixelArray[y + 1][x].r = 255;
				pixelArray[y + 1][x].g = 255;
				pixelArray[y + 1][x].b = 255;


				std::vector<int> coord_pair = { x, y + 1 };
				strong_edges.push_back(coord_pair);
			}
		}

		if (y < height - 1 && x > 0)
		{
			if (lower < pixelArray[y + 1][x - 1].r && pixelArray[y + 1][x - 1].r <= upper)
			{
				pixelArray[y + 1][x - 1].r = 255;
				pixelArray[y + 1][x - 1].g = 255;
				pixelArray[y + 1][x - 1].b = 255;

				std::vector<int> coord_pair = { x - 1, y + 1 };
				strong_edges.push_back(coord_pair);
			}
		}

		if ( x < width - 1)
		{
			if (lower < pixelArray[y][x + 1].r && pixelArray[y][x + 1].r <= upper)
			{
				pixelArray[y][x + 1].r = 255;
				pixelArray[y][x + 1].g = 255;
				pixelArray[y][x + 1].b = 255;

				std::vector<int> coord_pair = { x + 1, y };
				strong_edges.push_back(coord_pair);
			}
		}

		if (x > 0)
		{
			if (lower < pixelArray[y][x - 1].r && pixelArray[y][x - 1].r <= upper)
			{
				pixelArray[y][x - 1].r = 255;
				pixelArray[y][x - 1].g = 255;
				pixelArray[y][x - 1].b = 255;

				std::vector<int> coord_pair = { x - 1, y };
				strong_edges.push_back(coord_pair);
			}
		}
		
		if (x > 0 && y > 0)
		{
			if (lower < pixelArray[y - 1][x - 1].r && pixelArray[y - 1][x - 1].r <= upper)
			{
				pixelArray[y - 1][x - 1].r = 255;
				pixelArray[y - 1][x - 1].g = 255;
				pixelArray[y - 1][x - 1].b = 255;

				std::vector<int> coord_pair = { x - 1, y - 1 };
				strong_edges.push_back(coord_pair);
			}
		}

		if ( y > 0)
		{
			if (lower < pixelArray[y - 1][x].r && pixelArray[y - 1][x].r <= upper)
			{
				pixelArray[y - 1][x].r = 255;
				pixelArray[y - 1][x].g = 255;
				pixelArray[y - 1][x].b = 255;

				std::vector<int> coord_pair = { x, y - 1 };
				strong_edges.push_back(coord_pair);
			}
		}

		if (y > 0 && x < width - 1)
		{
			if (lower < pixelArray[y - 1][x + 1].r && pixelArray[y - 1][x + 1].r <= upper)
			{
				pixelArray[y - 1][x + 1].r = 255;
				pixelArray[y - 1][x + 1].g = 255;
				pixelArray[y - 1][x + 1].b = 255;

				std::vector<int> coord_pair = { x + 1, y - 1 };
				strong_edges.push_back(coord_pair);
			}
		}
	}

	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			if(pixelArray[y][x].r != 255)
			{
				pixelArray[y][x].r = 0;
				pixelArray[y][x].b = 0;
				pixelArray[y][x].g = 0;
			}
		}
	}
}

void Image::weightLineContinuitity()
{
	const double to_radians = 0.017453292519943;
	int y;
	int x;
	double theta;
	double r;

	for (int i = 0; i < lineSet.size(); i++)
	{
		theta = lineSet[i].theta;
		r = lineSet[i].r_value;
		int current_count = 0;
		int x_continuity = 0;
		int y_continuity = 0;
		int best_continuity;

		for (int x = 0; x < width - 1; x++)
		{
			y = int(r - (x * (cos(theta * to_radians)))) / (sin(theta * to_radians));

			if (0 <= y && y <= height - 2)
			{
				if (pixelArray[y + 1][x + 1].r = 255)
				{
					current_count++;
					if (current_count > x_continuity)
					{
						x_continuity = current_count;
					}
					else
					{
						current_count = 0;
					}
				}
			}
		}
		current_count = 0;

		for (int y = 0; y < height - 1; y++)
		{
			x = int(r - (y * (sin(theta * to_radians)))) / (cos(theta * to_radians));

			if (0 <= x && x <= width - 2)
			{
				if (pixelArray[y + 1][x + 1].r = 255)
				{
					current_count++;
					if (current_count > x_continuity)
					{
						y_continuity = current_count;
					}
					else
					{
						current_count = 0;
					}
				}
			}

		}

		if (x_continuity > y_continuity)
		{
			best_continuity = x_continuity;
		}
		else
		{
			best_continuity = y_continuity;
		}

		lineSet[i].best_Continuity = best_continuity;
		best_continuity = 0;
		y_continuity = 0;
		x_continuity = 0;
	}
}

void Image::thetaEdgeThreshold(int threshold, int min)
{
	int count = 0;
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			if(pixelArray[y][x].r == 255)
			{
				count++;
			}
		}
	}

	std::vector<std::vector<std::vector<int>>> edgeGroups;	//3D vector containing vectors which hold int vectors (coordinates)
	
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			if(pixelArray[y][x].r == 255)
			{
				pixelArray[y][x].r = 0;
				pixelArray[y][x].g = 0;
				pixelArray[y][x].b = 0;	//Set the edge to 0 so its no longer regrouped

				std::vector<std::vector<int>> newEdgeGroup;	//Create a new edge group
				std::vector<int> edgeCoord = { y, x };		//Load this coordinate into an edge coord vector
				newEdgeGroup.push_back(edgeCoord);			//Push the edge coord vector into the new edge group

				std::vector<std::vector<int>> coord_stack; //Stack of coordinate pairs that need to be examined
				coord_stack.push_back(edgeCoord);

				while(coord_stack.size() > 0)	//While there is still coordinates to be examined...
				{
					int Y = coord_stack[coord_stack.size() - 1][0];
					int X = coord_stack[coord_stack.size() - 1][1];
					coord_stack.pop_back();

					if(Y < height - 1 && X > 0)
					{
						if(pixelArray[Y + 1][X - 1].r == 255 && abs(pixelArray[Y + 1][X - 1].G_theta - pixelArray[Y][X].G_theta) <= threshold)
						{
							pixelArray[Y + 1][X - 1].r = 0;
							pixelArray[Y + 1][X - 1].g = 0;
							pixelArray[Y + 1][X - 1].b = 0;

							edgeCoord = { Y + 1, X - 1 };
							newEdgeGroup.push_back(edgeCoord);
							coord_stack.push_back(edgeCoord);
						}
					}
					if(Y < height - 1)
					{
						if (pixelArray[Y + 1][X].r == 255 && abs(pixelArray[Y + 1][X].G_theta - pixelArray[Y][X].G_theta) <= threshold)
						{
							pixelArray[Y + 1][X].r = 0;
							pixelArray[Y + 1][X].g = 0;
							pixelArray[Y + 1][X].b = 0;

							edgeCoord = { Y + 1, X};
							newEdgeGroup.push_back(edgeCoord);
							coord_stack.push_back(edgeCoord);
						}
					}
					if(Y < height - 1 && X < width - 1)
					{
						if (pixelArray[Y + 1][X + 1].r == 255 && abs(pixelArray[Y + 1][X + 1].G_theta - pixelArray[Y][X].G_theta) <= threshold)
						{
							pixelArray[Y + 1][X + 1].r = 0;
							pixelArray[Y + 1][X + 1].g = 0;
							pixelArray[Y + 1][X + 1].b = 0;

							edgeCoord = { Y + 1, X + 1 };
							newEdgeGroup.push_back(edgeCoord);
							coord_stack.push_back(edgeCoord);
						}
					}
					if(X > 0)
					{
						if (pixelArray[Y][X - 1].r == 255 && abs(pixelArray[Y][X - 1].G_theta - pixelArray[Y][X].G_theta) <= threshold)
						{
							pixelArray[Y][X - 1].r = 0;
							pixelArray[Y][X - 1].g = 0;
							pixelArray[Y][X - 1].b = 0;

							edgeCoord = { Y, X - 1 };
							newEdgeGroup.push_back(edgeCoord);
							coord_stack.push_back(edgeCoord);
						}
					}
					if(X < width - 1)
					{
						if (pixelArray[Y][X + 1].r == 255 && abs(pixelArray[Y][X + 1].G_theta - pixelArray[Y][X].G_theta) <= threshold)
						{
							pixelArray[Y][X + 1].r = 0;
							pixelArray[Y][X + 1].g = 0;
							pixelArray[Y][X + 1].b = 0;

							edgeCoord = { Y, X + 1 };
							newEdgeGroup.push_back(edgeCoord);
							coord_stack.push_back(edgeCoord);
						}
					}
					if(Y > 0 && X > 0)
					{
						if (pixelArray[Y - 1][X - 1].r == 255 && abs(pixelArray[Y - 1][X - 1].G_theta - pixelArray[Y][X].G_theta) <= threshold)
						{
							pixelArray[Y - 1][X - 1].r = 0;
							pixelArray[Y - 1][X - 1].g = 0;
							pixelArray[Y - 1][X - 1].b = 0;

							edgeCoord = { Y - 1, X - 1 };
							newEdgeGroup.push_back(edgeCoord);
							coord_stack.push_back(edgeCoord);
						}
					}
					if(Y > 0)
					{
						if (pixelArray[Y - 1][X].r == 255 && abs(pixelArray[Y - 1][X].G_theta - pixelArray[Y][X].G_theta) <= threshold)
						{
							pixelArray[Y - 1][X].r = 0;
							pixelArray[Y - 1][X].g = 0;
							pixelArray[Y - 1][X].b = 0;

							edgeCoord = { Y - 1, X };
							newEdgeGroup.push_back(edgeCoord);
							coord_stack.push_back(edgeCoord);
						}
					}
					if(Y > 0 && X < width - 1)
					{
						if (pixelArray[Y - 1][X + 1].r == 255 && abs(pixelArray[Y - 1][X + 1].G_theta - pixelArray[Y][X].G_theta) <= threshold)
						{
							pixelArray[Y - 1][X + 1].r = 0;
							pixelArray[Y - 1][X + 1].g = 0;
							pixelArray[Y - 1][X + 1].b = 0;

							edgeCoord = { Y - 1, X };
							newEdgeGroup.push_back(edgeCoord);
							coord_stack.push_back(edgeCoord);
						}
					}
				}
				edgeGroups.push_back(newEdgeGroup);
			}
		}
	}

	for(int i = 0; i < edgeGroups.size(); i++)
	{
		if(edgeGroups[i].size() >= min)
		{
			finalEdgeGroups.push_back(edgeGroups[i]);	//occupy 3D vector representing final edge groups in the image
			
			for(int j = 0; j < edgeGroups[i].size(); j++)
			{
				int y = edgeGroups[i][j][0];
				int x = edgeGroups[i][j][1];

				pixelArray[y][x].r = 255;
				pixelArray[y][x].g = 255;
				pixelArray[y][x].b = 255;

			}
		}
	}
}

void Image::regressionLineDetection()
{
	std::vector<Line> allLines;
	const double to_radians = 0.017453292519943; // Pi/180
	int count = 0;

	for(int i = 0; i < finalEdgeGroups.size(); i++)
	{
		int x_avg;
		int y_avg;
		int theta_avg;
		int x_sum = 0;
		int y_sum = 0;
		int theta_sum = 0;

		for(int j = 0; j < finalEdgeGroups[i].size(); j++)
		{
			int y = finalEdgeGroups[i][j][0];
			int x = finalEdgeGroups[i][j][1];

			x_sum = x_sum + (x);
			y_sum = y_sum + (y);
			theta_sum = theta_sum + pixelArray[y][x].G_theta;
		}
		x_avg = int(x_sum / finalEdgeGroups[i].size());
		y_avg = int(y_sum / finalEdgeGroups[i].size());
		theta_avg = int(theta_sum / finalEdgeGroups[i].size());
		int r = int((x_avg * cos(theta_avg * to_radians)) + (y_avg * sin(theta_avg * to_radians)));
		
		Line newLine;
		newLine.r_value = r;
		newLine.theta = theta_avg;
		newLine.line_id = count;

		allLines.push_back(newLine);
		count++;
	}

	for(int i = 0; i < allLines.size(); i++)
	{
		for(int j = i + 1; j < allLines.size(); j++)
		{
			if(abs(allLines[i].r_value - allLines[j].r_value) < 40 && abs(allLines[i].theta - allLines[j].theta) < 10)
			{
				if(finalEdgeGroups[i].size() > finalEdgeGroups[j].size())
				{
					allLines[j].votes = -1;
				}
				else
				{
					allLines[i].votes = -1;
				}
			}
		}
	}

	for(int i = 0; i < allLines.size(); i++)
	{
		if(allLines[i].votes != -1)
		{
			lineSet.push_back(allLines[i]);
		}
	}
}

void Image::findParallelLines()
{
	std::vector<std::vector<Line>> parallelLines;

	int theta_upper = 4;
	int theta_lower = 0;

	while( theta_lower < 180)
	{
		std::vector<Line> parallelGroup;
		for (int j = 0; j < lineSet.size(); j++)
		{
			if (lineSet[j].theta >= theta_lower && lineSet[j].theta < theta_upper)
			{
				lineSet[j].parallelGroup = (theta_lower + 2);
				parallelGroup.push_back(lineSet[j]);
			}
		}
		
		if (parallelGroup.size() > 0)
		{
			parallelLines.push_back(parallelGroup);
		}

		theta_upper += 4;
		theta_lower += 4;
	}
	parallelLineSet = (parallelLines);

	
	const double to_radians = 0.017453292519943;
	int y;
	int x;
	double theta;
	double r;
	for(int j = 0; j < parallelLines.size(); j++)
	{
		int redRandom = rand() % 255;
		int blueRandom = rand() % 255;
		int greenRandom = rand() % 255;

		for (int i = 0; i < parallelLines[j].size(); i++)
		{
			theta = parallelLines[j][i].theta;
			r = parallelLines[j][i].r_value;

			for (int x = 0; x < width - 1; x++)
			{
				y = int(r - (x * (cos(theta * to_radians)))) / (sin(theta * to_radians));

				if (0 <= y && y <= height - 2)
				{
					pixelArray[y + 1][x + 1].r = redRandom;
					pixelArray[y + 1][x + 1].g = greenRandom;
					pixelArray[y + 1][x + 1].b = blueRandom;
				}
			}

			for (int y = 0; y < height - 1; y++)
			{
				x = int(r - (y * (sin(theta * to_radians)))) / (cos(theta * to_radians));

				if (0 <= x && x <= width - 2)
					{
						pixelArray[y + 1][x + 1].r = redRandom;
						pixelArray[y + 1][x + 1].g = greenRandom;
						pixelArray[y + 1][x + 1].b = blueRandom;
					}
			}
		}
	}
	
	std::cout << "Pause here";
}

void Image::drawIntersections()
{
	for(int i = 0; i < lineSet.size(); i++)
	{
		for(int j = i + 1; j < lineSet.size(); j++)
		{
			std::vector<int> coord_pair = findIntersection(lineSet[i], lineSet[j]);
			int y = coord_pair[0];
			int x = coord_pair[1];

			if(0 < x && x < width && 0 < y && y < height)
			{
				pixelArray[y][x].r = 0;
				pixelArray[y][x].g = 255;
				pixelArray[y][x].b = 0;
			}
		}
	}
}

void Image::generateOctagons()
{
	std::vector<std::vector<std::vector<Line>>> parallelLineSetPermutations;

	//Use parallel groups to make groups of parallel permutations organized by angle (so long as atleast one pair of parallel lines can be paired)
	for(int i = 0; i < parallelLineSet.size(); i++)
	{
		if(parallelLineSet[i].size() > 1)
		{
			std::vector<std::vector<Line>> parallelLinePairs;
			for (int j = 0; j < parallelLineSet[i].size(); j++)
			{
				for (int k = j + 1; k < parallelLineSet[i].size(); k++)
				{
					std::vector<Line> parallelLinePair1 = { parallelLineSet[i][j], parallelLineSet[i][k] };
					std::vector<Line> parallelLinePair2 = { parallelLineSet[i][k], parallelLineSet[i][j] };

					parallelLinePairs.push_back(parallelLinePair1);
					parallelLinePairs.push_back(parallelLinePair2);

				}
			}
			parallelLineSetPermutations.push_back(parallelLinePairs);
		}
	}

for(int i = 0; i < parallelLineSetPermutations.size(); i++)
{
	for(int j = 0; j < parallelLineSetPermutations.size(); j++)
	{
		if(i != j)
		{
			int angle = abs(parallelLineSetPermutations[i][0][0].parallelGroup - parallelLineSetPermutations[j][0][0].parallelGroup);
			if(angle < 90)
			{
				angle = 180 - angle;
			}
			if(130 <= angle && 140 <= angle)
			{
				//Determine possible Line 3 and 4
				int line3index = -1;
				int line4index = -1;
				for(int p = 0; p < parallelLineSet.size(); p++)
				{
					int angle2 = abs(parallelLineSetPermutations[i][0][0].parallelGroup - parallelLineSet[p][0].parallelGroup);
					int angle3 = abs(parallelLineSetPermutations[j][0][0].parallelGroup - parallelLineSet[p][0].parallelGroup);
					if(angle2 < 90)
					{
						angle2 = 180 - 90;
					}
					if (angle3 < 90)
					{
						angle3 = 180 - 90;
					}

					if(85 < angle2 && angle3 < 95)
					{
						line3index = p;
					}
					if(85 < angle3 && angle3 < 95)
					{
						line4index = p;
					}

				}

				if(line3index != - 1 && line4index != -1)
				{
					for(int k = 0; k < parallelLineSetPermutations[i].size(); k++)
					{
						for(int l = 0; l < parallelLineSetPermutations[j].size(); l++)
						{
							for(int m = 0; m < parallelLineSet[line3index].size(); m++)
							{
								for(int n = 0; n < parallelLineSet[line4index].size(); n++)
								{
									potentialOctagon newOctagon;
									newOctagon.line1 = parallelLineSetPermutations[i][k][0];
									newOctagon.line5 = parallelLineSetPermutations[i][k][1];
									newOctagon.line6 = parallelLineSetPermutations[j][l][0];
									newOctagon.line2 = parallelLineSetPermutations[j][l][1];
									newOctagon.line3 = parallelLineSet[line3index][m];
									newOctagon.line4 = parallelLineSet[line4index][n];

									potentialOctagonSet.push_back(newOctagon);
								}
							}
						}
					}
				}
				
			}
		}
	}
}

}

void Image::detectStopSign()
{
	int best_index = -1;
	double min = 99999;
	for(int i = 0; i < potentialOctagonSet.size(); i++)
	{
		bool flag = true;
		potentialOctagonSet[i].calculateAllIntersections();

		if ( 0 > potentialOctagonSet[i].int1_2[0] || potentialOctagonSet[i].int1_2[0] > height || 0 > potentialOctagonSet[i].int1_2[1] || potentialOctagonSet[i].int1_2[1] > width)
			flag = false;
		if ( 0 > potentialOctagonSet[i].int1_3[0] || potentialOctagonSet[i].int1_3[0] > height || 0 > potentialOctagonSet[i].int1_3[1] || potentialOctagonSet[i].int1_3[1] > width)
			flag = false;
		if ( 0 > potentialOctagonSet[i].int2_3[0] || potentialOctagonSet[i].int2_3[0] > height || 0 > potentialOctagonSet[i].int2_3[1] || potentialOctagonSet[i].int2_3[1] > width)
			flag = false;
		if ( 0 > potentialOctagonSet[i].int2_4[0] || potentialOctagonSet[i].int2_4[0] > height || 0 > potentialOctagonSet[i].int2_4[1] || potentialOctagonSet[i].int2_4[1] > width)
			flag = false;
		if ( 0 > potentialOctagonSet[i].int3_4[0] || potentialOctagonSet[i].int3_4[0] > height || 0 > potentialOctagonSet[i].int3_4[1] || potentialOctagonSet[i].int3_4[1] > width)
			flag = false;
		if ( 0 > potentialOctagonSet[i].int3_5[0] || potentialOctagonSet[i].int3_5[0] > height || 0 > potentialOctagonSet[i].int3_5[1] || potentialOctagonSet[i].int3_5[1] > width)
			flag = false;
		if ( 0 > potentialOctagonSet[i].int4_5[0] || potentialOctagonSet[i].int4_5[0] > height || 0 > potentialOctagonSet[i].int4_5[1] || potentialOctagonSet[i].int4_5[1] > width)
			flag = false;
		if ( 0 > potentialOctagonSet[i].int4_6[0] || potentialOctagonSet[i].int4_6[0] > height || 0 > potentialOctagonSet[i].int4_6[1] || potentialOctagonSet[i].int4_6[1] > width)
			flag = false;
		if ( 0 > potentialOctagonSet[i].int5_6[0] || potentialOctagonSet[i].int5_6[0] > height || 0 > potentialOctagonSet[i].int5_6[1] || potentialOctagonSet[i].int5_6[1] > width)
			flag = false;
		
		if(flag)
		{
			potentialOctagon& current = potentialOctagonSet[i];

			double Tri1 = current.triangleArea(current.int2_3, current.int3_4, current.int2_4);
			double Tri2 = current.triangleArea(current.int1_2, current.int2_3, current.int3_4);
			double Tri3 = current.triangleArea(current.int3_4, current.int4_5, current.int3_5);
			double Tri4 = current.triangleArea(current.int4_5, current.int5_6, current.int4_6);

			int Tri_avg = (Tri1 + Tri2 + Tri3 + Tri4) / 4;
			int Tri_score = abs(Tri1 - Tri_avg) + abs(Tri2 - Tri_avg) + abs(Tri3 - Tri_avg) + abs(Tri4 - Tri_avg);

			if (Tri_score < min)
			{
				best_index = i;
				min = Tri_score;
			}
		}
	}

	stopSign = potentialOctagonSet[best_index];
}

double  Image::triangleArea(std::vector<int> point1, std::vector<int> point2, std::vector<int> point3)
{
	int x1, x2, x3, y1, y2, y3;
	x1 = point1[1];
	x2 = point2[1];
	x3 = point3[1];
	y1 = point1[0];
	y2 = point2[0];
	y3 = point3[0];
	
	double area;
	area = (1 / 2) * ((x1 * y2) + (x2 * y3) + (x3 * y1) - (x1 * y3) - (x2 * y1) - (x3 * y2));
	return area;
}

std::vector<int> Image::findIntersection(Line one, Line two)
{
	double degree_to_radian = 0.01745329251;
	std::vector<int> coord_pair;

	int x = ((((sin(one.theta * degree_to_radian)) * ((-1)*(two.r_value))) - ((sin(two.theta * degree_to_radian)) * ((-1) * (one.r_value)))) / ((cos(one.theta * degree_to_radian) * sin(two.theta * degree_to_radian)) - (cos(two.theta * degree_to_radian) * sin(one.theta * degree_to_radian))));
	int y = (((((-1) * one.r_value) * cos(two.theta * degree_to_radian)) - (((-1) * two.r_value) * cos(one.theta * degree_to_radian))) / ((cos(one.theta * degree_to_radian) * sin(two.theta * degree_to_radian)) - (cos(two.theta * degree_to_radian) * sin(one.theta * degree_to_radian))));

	coord_pair = { y, x };
	return coord_pair;
}


