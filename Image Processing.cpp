// Image Processing.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Image.h"
#include <iostream>

int main()
{
    Image newImage;
    Image newImage2;

    std::string filename = "image-9.bmp";
    
    std::string edgemap_name = filename;
    std::string output_filename = filename;
    output_filename.erase(output_filename.length() - 4, 4);;
    edgemap_name.erase(edgemap_name.length()-4,4);
    edgemap_name += "_EdgeMap.bmp";
    output_filename += "_Stop_Sign_Detected.bmp";

    std::cout << "Reading in .bmp file " << filename << "...";
    newImage.readImage(filename);
    std::cout << "COMPLETE" << std::endl;

    std::cout << "Applying 3x3 window Gaussian smoothing...";
    newImage.gaussianSmooth();
    std::cout << "COMPLETE" << std::endl;

    std::cout << "Generating Sobel edge map....";
    newImage2 = newImage.sobelEdgeMap();
    std::cout << "COMPLETE" << std::endl;

    std::cout << "Thinning Edge map...";
    newImage2.nonMaxSuppression();
    std::cout << "COMPLETE" << std::endl;

    std::cout << "Canny Edge Detection: Double thresholding edge map...";
    newImage2.doubleThreshold(100, 60);
    std::cout << "COMPLETE" << std::endl;

    newImage2.exportImage(edgemap_name);

    std::cout << "Assigning edge groups and thresholding..." << std::endl;
    std::cout << "Theta Min: " << 3 << " Pixel Count Min: " << 10 << "...";
    newImage2.thetaEdgeThreshold(3, 10);
    std::cout << "COMPLETE" << std::endl;

    std::cout << "Generating regression lines...";
    newImage2.regressionLineDetection();
    std::cout << "COMPLETE" << std::endl;

    std::cout << "Searching for stop sign...";
    newImage2.generateOctagons();
    //newImage2.detectStopSign();
    std::cout << "COMPLETE" << std::endl;

    newImage2.exportImage(output_filename);
    std::cout << std::endl <<  "Stop sign detection complete." << std::endl;
    std::cout << "Output file created: " << output_filename << std::endl;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
