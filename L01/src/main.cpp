#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <cmath>
#include "Image.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;



double calculateArea(vector<double> A, vector<double> B, vector<double> C)
    {
        double area = ((A[0] * (B[1] - C[1]) + B[0] * (C[1] - A[1]) + C[0] * (A[1] - B[1])) / 2);
        return abs(area);
    }

int main(int argc, char* argv[])
{
	if (argc < 15) {
		cout << "Usage: Lab1 <out_image_name>.png width height x0 y0 r0 g0 b0 x1 y1 r1 g1 b1 x2 y2 r2 g2 b2" << endl;
		return 0;
	}

	// Output filename
	string filename(argv[1]);
	// Width of image
	int width = atoi(argv[2]);
	// Height of image
	int height = atoi(argv[3]);
	int x1 = atoi(argv[4]);
	int y1 = atoi(argv[5]);
	int x2 = atoi(argv[9]);
	int y2 = atoi(argv[10]);
	int x3 = atoi(argv[14]);
	int y3 = atoi(argv[15]);
	int xmax, ymax, xmin, ymin;
	
	// Create the image. We're using a `shared_ptr`, a C++11 feature.
	auto image = make_shared<Image>(width, height);
	
	
	// COMPUTE BOUNDING BOX
	xmax = max(x1, x2);
	xmax = max(xmax, x3);
	xmin = min(x1, x2);
	xmin = min(xmin, x3);
	ymax = max(y1, y2);
	ymax = max(ymax, y3);
	ymin = min(y1, y2);
	ymin = min(ymin, y3);
	
	//cout << "xmin: 310 "<<xmin << "ymin: 225 " << ymin << " xmax: 339  " << xmax << "ymax: 240" << ymax << " "  << endl;

	//FOR EVERY PIXEL IN THE BOUNDING BOX, COMPUTE BARYCENTRIC COORDINATES FOR EACH PIXEL
	double centerpt_x = 1 / 3 * (x1 + x2 + x3);
	double centerpt_y = 1 / 3 * (y1 + y2 + y3);
  vector<double> center{centerpt_x, centerpt_y};
  vector<double> A{(double) x1, (double)y1};
  vector<double> B{(double) x2, (double)y2};
  vector<double> C{(double) x3, (double)y3};

 
 
   int r,g,b;
    
	for (int y = ymin; y <= ymax; y++) {
		for (int x = xmin; x <= xmax; x++) {
            vector<double>pt {(double)x,(double)y};
            double total = calculateArea(A, B, C);
            double a1 = calculateArea(pt, A, B);
            double a2 = calculateArea(pt, B, C);
            double a3 = calculateArea(pt, A, C);

			if ((x == x1 && y == y1) || (x == x2 && y == y2) || (x == x3 && y == y3)) {
				r = 255;
				g = 255;
				b = 255;
                image->setPixel(x, y, r, g, b);
			}
            else if (a1+a2+a3 == total){
                //α+β+γ=1
                image->setPixel(x, y, 255, 0, 0);
                image->setPixel(x, y, 255 * (a2 / total), 255 * (a1 / total), 255 * (a3 / total));
            }
		}
	}

	// Write image to file
	image->writeToFile(filename);
	return 0;
}

//Test  640 480 10 10 320 470 630 20
// 640 480 320 240 310 230 330 225
