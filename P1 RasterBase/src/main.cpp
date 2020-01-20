/* Release code for program 1 CPE 471 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <assert.h>
#include <cmath> // abs() is ambiguous (Xcode warning)
#include "tiny_obj_loader.h"
#include "Image.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;
//global
int g_width, g_height, option;
float L, R, B, T, C, D, E, F;
float alpha, beta, gama;
float freq1 = 10.0, freq2 = 100.0;
/*
   Helper function you will want all quarter
   Given a vector of shapes which has already been read from an obj file
   resize all vertices to the range [-1, 1]
 */
void resize_obj(std::vector<tinyobj::shape_t> &shapes){
   float minX, minY, minZ;
   float maxX, maxY, maxZ;
   float scaleX, scaleY, scaleZ;
   float shiftX, shiftY, shiftZ;
   float epsilon = 0.001;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t i = 0; i < shapes.size(); i++) {
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         if(shapes[i].mesh.positions[3*v+0] < minX) minX = shapes[i].mesh.positions[3*v+0];
         if(shapes[i].mesh.positions[3*v+0] > maxX) maxX = shapes[i].mesh.positions[3*v+0];

         if(shapes[i].mesh.positions[3*v+1] < minY) minY = shapes[i].mesh.positions[3*v+1];
         if(shapes[i].mesh.positions[3*v+1] > maxY) maxY = shapes[i].mesh.positions[3*v+1];

         if(shapes[i].mesh.positions[3*v+2] < minZ) minZ = shapes[i].mesh.positions[3*v+2];
         if(shapes[i].mesh.positions[3*v+2] > maxZ) maxZ = shapes[i].mesh.positions[3*v+2];
      }
   }

	//From min and max compute necessary scale and shift for each dimension
   float maxExtent, xExtent, yExtent, zExtent;
   xExtent = maxX-minX;
   yExtent = maxY-minY;
   zExtent = maxZ-minZ;
   if (xExtent >= yExtent && xExtent >= zExtent) {
      maxExtent = xExtent;
   }
   if (yExtent >= xExtent && yExtent >= zExtent) {
      maxExtent = yExtent;
   }
   if (zExtent >= xExtent && zExtent >= yExtent) {
      maxExtent = zExtent;
   }
   scaleX = 2.0 /maxExtent;
   shiftX = minX + (xExtent/ 2.0);
   scaleY = 2.0 / maxExtent;
   shiftY = minY + (yExtent / 2.0);
   scaleZ = 2.0/ maxExtent;
   shiftZ = minZ + (zExtent)/2.0;

   //Go through all verticies shift and scale them
   for (size_t i = 0; i < shapes.size(); i++) {
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         shapes[i].mesh.positions[3*v+0] = (shapes[i].mesh.positions[3*v+0] - shiftX) * scaleX;
         assert(shapes[i].mesh.positions[3*v+0] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+0] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3*v+1] = (shapes[i].mesh.positions[3*v+1] - shiftY) * scaleY;
         assert(shapes[i].mesh.positions[3*v+1] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+1] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3*v+2] = (shapes[i].mesh.positions[3*v+2] - shiftZ) * scaleZ;
         assert(shapes[i].mesh.positions[3*v+2] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+2] <= 1.0 + epsilon);
      }
   }
}

//calculate area of a 2D triangle
float calcArea(vector<float> A, vector<float> B, vector<float> C){
    float area = (A[0] * (B[1] - C[1]) + B[0] * (C[1] - A[1]) + C[0] * (A[1] - B[1])) / 2;
    return abs(area);
}

//test if pt is inside of a triangle
int isinside(vector<float> A, vector<float> B, vector<float> C, vector<float> pt)
{
    float epsilon = 0.001;
    float total = calcArea(A, B, C);
    float a1 = calcArea(pt,B, C);  //Aa
    float a2 = calcArea(pt, C, A); //Ab
    float a3 = calcArea(pt, A, B); //Ac
    alpha = a1/total;
    beta = a2/total;
    gama = a3/total;

    //Add epsilon to account for floating point tomfoolery.
    if((abs(alpha+beta+gama) < 1.0+epsilon )&& (abs(alpha+beta+gama)> 1.0-epsilon)){
        return 1;
    }
    return 0;
}

//convert world points into pixel points
//X-axis: xp = C * xw + D; Y-axis: yp = E * yw + D;
vector<float> W2P(vector<float> V){
    float pixel_x = C * V[0] + D;
    float pixel_y = E * V[1] + F;
    return {pixel_x, pixel_y};
}

//set up z buffer and fill it with a fixed value to ensure depth testing isn't done with garbage data.
float* SetUpZbuffer(int g_width, int g_height){
    float* zbuffer;
    zbuffer = new float[g_width* g_height];
    for(int y = 0; y < g_height; ++y){
        for(int x = 0; x < g_width; ++x){
            zbuffer[y*g_width + x] = -200.0;
        }
    }
    return zbuffer;
}

int main(int argc, char **argv)
{
	if(argc < 6) {
      cout << "Usage: raster meshfile imagefile width height coloring_mode" << endl;
      return 0;
   }
	// OBJ filename
	string meshName(argv[1]);
	string imgName(argv[2]);
    
    g_width = stoi(argv[3]);
    g_height = stoi(argv[4]);
    option = stoi(argv[5]);
    
   //create an image
	auto image = make_shared<Image>(g_width, g_height);
    //create a 1D array Z buffer for depth testing
    float* zbuffer = SetUpZbuffer(g_width, g_height);
	// triangle buffer
	vector<unsigned int> triBuf;
	// position buffer
	vector<float> posBuf;
	vector<tinyobj::shape_t> shapes; // geometry
	vector<tinyobj::material_t> objMaterials; // material
	string errStr;
	
   bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());
	/* error checking on read */
	if(!rc) {
		cerr << errStr << endl;
	} else {
 		//keep this code to resize your object to be within -1 -> 1
        resize_obj(shapes);
		posBuf = shapes[0].mesh.positions;
		triBuf = shapes[0].mesh.indices;
	}
    
    //get L R B T C D E F
    if(g_width > g_height){
        R = (float)g_width/(float)g_height;
        L = -1.0*R;
        B = -1.0;
        T = 1.0;
    }else{
        T = (float)g_height/(float)g_width;
        B = -1.0*T;
        L = -1.0;
        R = 1.0;
    }
    // 0 = C * L + D  ; width - 1 = C * R + D ; (width - 1) - 0 = C(R-L)
    C = (g_width-1.0) / (R-L);
    D = -1.0*C*L;
    E = (g_height - 1.0) / (T-B);
    F = -1.0*E*B;

    //convert world points xw <float> into pixel points xp <int>
    for (size_t i = 0; i < triBuf.size()/3; i++) {
        int Xmax, Xmin, Ymax, Ymin;
        // get vertices
        int i1 = triBuf[3*i+0];
        int i2 = triBuf[3*i+1];
        int i3 = triBuf[3*i+2];
        //store XYZ world points into vector
        vector<float> V1 {posBuf[3*i1+0], posBuf[3*i1+1], posBuf[3*i1+2]};
        vector<float> V2 {posBuf[3*i2+0], posBuf[3*i2+1], posBuf[3*i2+2]};
        vector<float> V3 {posBuf[3*i3+0], posBuf[3*i3+1], posBuf[3*i3+2]};
        
        //convert {Xw, Yw} -> {Xp,Yp}
        vector<float> V1p = W2P(V1);
        vector<float> V2p = W2P(V2);
        vector<float> V3p = W2P(V3);

        Xmax = (int)max(max(V1p[0], V2p[0]),V3p[0]);
        Xmin = (int)min(min(V1p[0], V2p[0]),V3p[0]);
        Ymax = (int)max(max(V1p[1], V2p[1]),V3p[1]);
        Ymin = (int)min(min(V1p[1], V2p[1]),V3p[1]);
        
        //compute bounding box
        for (int y = Ymin; y <= Ymax; y++) {
            for (int x = Xmin; x <= Xmax; x++) {//for each pixel in the bounding box
                vector<float>pt {(float)x, (float)y};
                if (isinside(V1p, V2p, V3p, pt)){
                    /*use alpha beta gamma to get Z */
                    /* -1 <=  z  <= 1
                     0 <= z+1 <= 2
                    0 <= 127.5 * (z+1) <= 255*/
                    float z = alpha * (V1[2]) + beta * (V2[2]) + gama * (V3[2]) +1;
        //--------Start Coloring 1: map the z-value to the range 0 to 255. 2. Z- buffer-----
        // If its more (closer) cur z is smaller, do not overwrite the pixel or z value.closer vertices are brighter
                    int r,b,g;
                    
                    //standard shading
                    if(option == 1){
                       if(zbuffer[y*g_width + x] < z){
                           zbuffer[y*g_width + x] = z;
                           r = zbuffer[y*g_width + x] * (255/2);
                           b = 0;
                           g = 0;
                           image->setPixel(x, y, r, g, b);
                        }
                    }
                    //use sin to mix colors
                    else if (option == 2){
                        if(zbuffer[y*g_width + x] < z){
                            zbuffer[y*g_width + x] = z;
                            r =255-abs(sin(freq1* zbuffer[y*g_width + x])) * 255;
                            b =abs(cos(freq2* zbuffer[y*g_width + x])) * 255;
                            g = 255-abs(sin(freq1* zbuffer[y*g_width + x])) * 255;
                            image->setPixel(x, y, r, g, b);
                        }
                    }
                    else {
                        /* error checking on coloring mode */
                        cout << "Coloring mode " << option << " is not valid" << endl;
                        cout << "mode 1: shading | mode 2: distance coloring" << endl;
                        return 0;
                    }
                }
            }
        }
    }
	//write out the image
   image->writeToFile(imgName);

	return 0;
}


