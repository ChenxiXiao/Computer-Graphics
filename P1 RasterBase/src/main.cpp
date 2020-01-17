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
int g_width, g_height;
float L, R, B, T, C, D, E, F;

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
//----------MY HELPER FUNCTIONS----------------//
double calcArea(vector<float> A, vector<float> B, vector<float> C){
    double area = ((A[0] * (B[1] - C[1]) + B[0] * (C[1] - A[1]) + C[0] * (A[1] - B[1])) / 2);
    return abs(area);
}
int isinside(vector<float> A, vector<float> B, vector<float> C, vector<float> pt, double* alpha, double* beta, double* gamma)
{//get three vertices in pixel world <int> and return the area
    float epsilon = 0.001;
    double total = calcArea(A, B, C);
    double a1 = calcArea(A, B, pt);
    double a2 = calcArea(B, C, pt);
    double a3 = calcArea(C, A, pt);
    *alpha = a1/total;
    *beta = a2/total;
    *gamma = a3/total;
    //cout<< "ALPHA" << *alpha<< endl;
    if (*alpha > epsilon|| *alpha < (1+epsilon) || *beta > epsilon || *beta < (1+epsilon) || *gamma > epsilon || *gamma < (1+epsilon)){//α+β+γ=1 or 0<α/β/γ<1 inside. Add epsilon to account for floating point tomfoolery.
        return 1;
    }
    return 0;
}

vector<float> W2P(vector<float> V){ //X-axis: xp = C * xw + D //Y-axis: ; yp = E * yw + D;
    float pixel_x = C * V[0] + D;
    float pixel_y = E * V[1] + F;
    return {pixel_x, pixel_y, V[2]};
}

void Print(vector<float> V){
    cout<< "Vertices: "<< V[0] <<" "<< V[1] << " " << V[2]<< endl;
}

int main1(int argc, char **argv)
{
	if(argc < 3) {
      //cout << "Usage: raster meshfile imagefile" << endl;
      cout << "Usage: raster meshfile imagefile width height coloring_mode" << endl;
      return 0;
   }
    for(int i = 0; i < argc; i++){
        cout<< argv[i] <<" ";
    }
	// OBJ filename
	string meshName(argv[1]);
	string imgName(argv[2]);

	//set g_width and g_height appropriately!
    g_width = stoi(argv[3]);
    g_height = stoi(argv[4]);
   //create an image
	auto image = make_shared<Image>(g_width, g_height);

	// triangle buffer
	vector<unsigned int> triBuf;
	// position buffer
	vector<float> posBuf;
	// Some obj files contain material information.
	// We'll ignore them for this assignment.
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
	cout << "Number of vertices: " << posBuf.size()/3 << endl;
	cout << "Number of triangles: " << triBuf.size()/3 << endl;
    
    //get L R B T C D E F
    if(g_width > g_height){
        L = -1*(g_width/g_height);
        R = g_width/g_height;
        B = -1;
        T = 1;
    }
    else{
        L = -1;
        R = 1;
        B = -1*(g_height/g_width);
        T = g_height/g_width;
    }
    // 0 = C * L + D  ; width - 1 = C * R + D
    // (width - 1) - 0 = C(R-L) --> C = (width-1)/(R-L)
    //put C into 1st equation: D = (1-width)*L/(R-L)
    C = (g_width-1) / (R-L);
    D = (1-g_width) * L / (R-L);
    E = (g_height - 1) / (T-B);
    F = (1-g_height) * B/ (T-B);
    
    //convert world points xw <float> into pixel points xp <int>
    
    cout << "C D E F: "<< C << " " << D << " " << E << " "<< F << endl;
    cout << "triBuf size " << triBuf.size() << endl;
	//TODO add code to iterate through each triangle and rasterize it 
   //shapes[i].mesh.positions stores the vertices as {x0,y0,z0... x,y,z...}
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
        //         float xw1 = posBuf[3*V1+0] ;
        //         float yw1 = posBuf[3*V1+1] ;
        //         float xw2 = posBuf[3*V2+0];
        //         float yw2 = posBuf[3*V2+1];
        //         float xw3 =posBuf[3*V3+0];
        //         float yw3 =posBuf[3*V3+1];

        //convert {Xw, Yw} -> {Xp,Yp,Zw}   vector{x,y,z}
        //W2P updates only XY
        vector<float> V1p = W2P(V1);
        vector<float> V2p = W2P(V2);
        vector<float> V3p = W2P(V3);

        Xmax = max(max(V1p[0], V2p[0]),V3p[0]);
        Xmin = min(min(V1p[0], V2p[0]),V3p[0]);
        Ymax = max(max(V1p[1], V2p[1]),V3p[1]);
        Ymin = min(min(V1p[1], V2p[1]),V3p[1]);
        
        //compute bounding box
        for (int y = Ymin; y <= Ymax; y++) {
            for (int x = Xmin; x <= Xmax; x++) {//for each pixel in the bounding box
                vector<float>pt {(float)x, (float)y};
                double alpha = 0, beta = 0, gamma =0;
                if (isinside(V1p, V2p, V3p, pt, &alpha, &beta, &gamma)==1){//α+β+γ=1 --> inside
                    //update z
                    double z = alpha * V1p[2]+ beta * V2p[2] + gamma * V3p[2];
                    image->setPixel(x, y, 255, 255, 0);
                    //occlusion : some triangles might not be visible due to z order
                    //Start Coloring
                    //1: map the z-value to the range 0 to 255.
                    //2. Z- buffer
                    
                   
                }
            }
        }
    }
	//write out the image
   image->writeToFile(imgName);

	return 0;
}


int main(int argc, char *tf[]){
     tf[1] =  "/Users/ccmac/Documents/GitHub/CPE471/P1 RasterBase/resources/bunny.obj";
    tf[2] = "bunny_bbox.png";
     tf[3] = "500";
     tf[4] = "500";
    main1(6, tf);
     tf[1] =  "/Users/ccmac/Documents/GitHub/CPE471/P1 RasterBase/resources/sphere.obj";
     tf[2] = "sphere_bbox.png";
    main1(6, tf);
    tf[1] =  "/Users/ccmac/Documents/GitHub/CPE471/P1 RasterBase/resources/teapot.obj";
    tf[2] = "teapot__box.png";
    main1(6, tf);
    tf[1] =  "/Users/ccmac/Documents/GitHub/CPE471/P1 RasterBase/resources/tri.obj";
    tf[2] = "tri__box.png";
    main1(6, tf);
    tf[3] = "500";
    tf[4] = "100";
    tf[1] =  "/Users/ccmac/Documents/GitHub/CPE471/P1 RasterBase/resources/teapot.obj";
    tf[2] = "teapot__ratio.png";
    main1(6, tf);
    
}
