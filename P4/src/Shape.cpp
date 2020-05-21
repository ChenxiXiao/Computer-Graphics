
#include "Shape.h"
#include <iostream>
#include <cassert>

#include "GLSL.h"
#include "Program.h"

using namespace std;


// copy the data from the shape to this object
void Shape::createShape(tinyobj::shape_t & shape)
{
	posBuf = shape.mesh.positions;
	norBuf = shape.mesh.normals;
	texBuf = shape.mesh.texcoords;
	eleBuf = shape.mesh.indices;
    colBuf = shape.mesh.colors;
    
    if(colBuf[0] == 0.0f){
        colBuf.clear();
    }
    if(norBuf.empty()) {
        createNormals();
    }
    /*
    for(int i = 0; i < (colBuf.size()/3); i++){
        cout <<" r b g "<< " : " << colBuf[i] << " " << colBuf[i+1]<<  " " << colBuf[i+2] << endl;
    }
*/
}
void Shape::turnOffVertTex(){
    texBuf.clear();
}

void Shape::createNormals() {
  int indices1, indices2, indices3 = 0;
  glm::vec3 v1, v2, v3, v3_v1, v2_v1, cross_res, norm;
  for (int i = 0; i < (posBuf.size()/ 3); i++)
  {
      norBuf.push_back(0);
      norBuf.push_back(0);
      norBuf.push_back(0);
  }
  for (int j = 0; j < (eleBuf.size() / 3); j++)
  {
      
      indices1 = eleBuf[3 * j + 0];
      indices2 = eleBuf[3 * j + 1];
      indices3 = eleBuf[3 * j + 2];

      v1 =  glm::vec3((posBuf[3 * indices1 + 0]), (posBuf[3 * indices1 + 1]), (posBuf[3 * indices1 + 2]));
      v2 =  glm::vec3((posBuf[3 * indices2 + 0]), (posBuf[3 * indices2 + 1]), (posBuf[3 * indices2 + 2]));
      v3 =  glm::vec3((posBuf[3 * indices3 + 0]), (posBuf[3 * indices3 + 1]), (posBuf[3 * indices3 + 2]));

      v3_v1 = v3 - v1;
      v2_v1 = v2 - v1;

      cross_res = cross(v2_v1, v3_v1);
      norm = normalize(cross_res);
      norBuf[3 * indices1 + 0] += norm.x;
      norBuf[3 * indices1 + 1] += norm.y;
      norBuf[3 * indices1 + 2] += norm.z;

      norBuf[3 * indices2 + 0] += norm.x;
      norBuf[3 * indices2 + 1] += norm.y;
      norBuf[3 * indices2 + 2] += norm.z;

      norBuf[3 * indices3 + 0] += norm.x;
      norBuf[3 * indices3 + 1] += norm.y;
      norBuf[3 * indices3 + 2] += norm.z;
  }
  for (int i = 0; i < norBuf.size() / 3; ++i) {
      glm::vec3 toNorm = glm::vec3(norBuf[3 * i + 0], norBuf[3 * i + 1], norBuf[3 * i + 2]);
      glm::vec3 norm = glm::normalize(toNorm);
      norBuf[3 * i + 0] = norm.x;
      norBuf[3 * i + 1] = norm.y;
      norBuf[3 * i + 2] = norm.z;
  }
}

void Shape::measure()
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	minX = minY = minZ = std::numeric_limits<float>::max();
	maxX = maxY = maxZ = -std::numeric_limits<float>::max();

	//Go through all vertices to determine min and max of each dimension
	for (size_t v = 0; v < posBuf.size() / 3; v++)
	{
		if (posBuf[3*v+0] < minX) minX = posBuf[3 * v + 0];
		if (posBuf[3*v+0] > maxX) maxX = posBuf[3 * v + 0];

		if (posBuf[3*v+1] < minY) minY = posBuf[3 * v + 1];
		if (posBuf[3*v+1] > maxY) maxY = posBuf[3 * v + 1];

		if (posBuf[3*v+2] < minZ) minZ = posBuf[3 * v + 2];
		if (posBuf[3*v+2] > maxZ) maxZ = posBuf[3 * v + 2];
	}

	min.x = minX;
	min.y = minY;
	min.z = minZ;
	max.x = maxX;
	max.y = maxY;
	max.z = maxZ;
}


void Shape::init()
{
	// Initialize the vertex array object
	CHECKED_GL_CALL(glGenVertexArrays(1, &vaoID));
	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Send the position array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &posBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW));

	// Send the normal array to the GPU
	if (norBuf.empty())
	{
		norBufID = 0;
	}
	else
	{
		CHECKED_GL_CALL(glGenBuffers(1, &norBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW));
       // std::cout << "Size of norBuf: " <<norBuf.size() << std::endl;
	}
	// Send the texture array to the GPU
	if (texBuf.empty())
	{
		texBufID = 0;
	}
	else
	{
		CHECKED_GL_CALL(glGenBuffers(1, &texBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW));
      
	}
    
    //send the color array to the GPU
     if (colBuf.empty())
     {
         colBufID = 0;
     }
     else{
         CHECKED_GL_CALL(glGenBuffers(1, &colBufID));
         CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colBufID));
         CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, colBuf.size()*sizeof(float), &colBuf[0], GL_STATIC_DRAW));
         //std::cout << "Size of ColBuf: " <<colBuf.size() << std::endl;
     }

	// Send the element array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &eleBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));
	CHECKED_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW));

	// Unbind the arrays
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}



void Shape::draw(const shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex, h_col;
	h_pos = h_nor = h_tex = h_col = -1;

	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));

	// Bind normal buffer
	
    h_nor = prog->getAttribute("vertNor");
    if (h_nor != -1 && norBufID != 0)
    {
        GLSL::enableVertexAttribArray(h_nor);
        CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
        CHECKED_GL_CALL(glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
    }
    
	if (texBufID != 0)
	{
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");

		if (h_tex != -1 && texBufID != 0)
        {
            GLSL::enableVertexAttribArray(h_tex);
            CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
            CHECKED_GL_CALL(glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }
	}
    
    if (colBufID != 0 )
    {
        // Bind colcoords buffer
       h_col = prog->getAttribute("vertCol");

        if (h_col != -1 && colBufID != 0 )
        {
            GLSL::enableVertexAttribArray(h_col);
            CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colBufID));
            CHECKED_GL_CALL(glVertexAttribPointer(h_col, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
            //r g b
        }
    }


	// Bind element buffer
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));

	// Draw
	CHECKED_GL_CALL(glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0));

	// Disable and unbind
	if (h_tex != -1)
	{
		GLSL::disableVertexAttribArray(h_tex);
	}
	if (h_nor != -1)
	{
		GLSL::disableVertexAttribArray(h_nor);
	}
    if (h_col != -1)
    {
        GLSL::disableVertexAttribArray(h_col);
    }
	GLSL::disableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
