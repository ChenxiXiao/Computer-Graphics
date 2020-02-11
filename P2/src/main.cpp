/*
 * Program 2 base code - includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, prog1, prog2;

	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<Shape> campus, person, cube;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 gMin;

	//animation data
	float sTheta = 0;
	float gTrans = 0, Up = 0;

    
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

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			gTrans -= 0.2;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			gTrans += 0.2;
		}
        if (key == GLFW_KEY_W && action == GLFW_PRESS) {
            Up -= 0.2;
        }
        if (key == GLFW_KEY_S && action == GLFW_PRESS) {
            Up += 0.2;
        }
      
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
        // Initialize the GLSL program.
        prog = make_shared<Program>();
        prog->setVerbose(true);
        prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
        prog->init();
        prog->addUniform("P");
        prog->addUniform("V");
        prog->addUniform("M");
        prog->addAttribute("vertPos");
        prog->addAttribute("vertNor");
        //        //second shader - Red
        prog1 = make_shared<Program>();
        prog1->setVerbose(true);
        prog1->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/height_frag.glsl");
        prog1->init();
        prog1->addUniform("P");
        prog1->addUniform("V");
        prog1->addUniform("M");
        prog1 -> addAttribute("vertPos");
        prog1->addAttribute("vertNor");
        // third shader - Yellow
        prog2 = make_shared<Program>();
        prog2->setVerbose(true);
        prog2->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/yellow_frag.glsl");
        prog2->init();
        prog2->addUniform("P");
        prog2->addUniform("V");
        prog2->addUniform("M");
        prog2 ->addAttribute("vertPos");
        prog2->addAttribute("vertNor");
        
	}

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes, Pshape, Cshape;
 		vector<tinyobj::material_t> objMaterials,objMaterials1 , objMaterials2;
 		string errStr,errStr1, errStr2;
		//load in the mesh and make the shape(s)
 		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/Lib.obj").c_str());
        bool rc1 = tinyobj::LoadObj(Pshape, objMaterials1, errStr1, (resourceDirectory + "/SmoothSphere.obj").c_str());
        bool rc2 = tinyobj::LoadObj(Cshape, objMaterials2, errStr2, (resourceDirectory + "/cube.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			campus = make_shared<Shape>();
			campus->createShape(TOshapes[0]);
			campus->measure();
			campus->init();
            
		}
        if (!rc1) {
            cerr << errStr1 << endl;
        } else {
            person= make_shared<Shape>();
            resize_obj(Pshape);
            person->createShape(Pshape[0]);
            person->measure();
            person->init();
            
        }
         if (!rc2) {
             cerr << errStr2 << endl;
         } else {
             cube= make_shared<Shape>();
             resize_obj(Cshape);
             cube->createShape(Cshape[0]);
             cube->measure();
             cube->init();
             
     }
		//read out information stored in the shape about its size - something like this...
		//then do something with that information.....
		gMin.x = campus->min.x;
		gMin.y = campus->min.y;
	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   }

	void render() {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

		// View is global translation along negative z for now
		View->pushMatrix();
			View->loadIdentity();
			View->translate(vec3(0, -0.2, -5));
    
		// Draw the campus as the backgorund
		prog->bind();
        glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
        glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
        Model->pushMatrix();
            Model->loadIdentity();
            Model->translate(vec3(gTrans, 0, 0));
            Model->translate(vec3(0, Up, 0));
            Model->pushMatrix();
            Model->translate(vec3(2.1, 0.1, 1.5));
            Model->scale(vec3(0.009, 0.009, 0.009));
            Model->rotate(-1.21, vec3(1,0,0));
            setModel(prog, Model);
            campus->draw(prog);
        Model->popMatrix();
        prog->unbind();
        
       // draw the sun
       prog1->bind();
       glUniformMatrix4fv(prog1->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
       glUniformMatrix4fv(prog1->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
       Model->pushMatrix();
           Model->loadIdentity();
           Model->translate(vec3(gTrans, 0, 0));
           Model->translate(vec3(0, Up, 0));
            Model->pushMatrix();
                Model->translate(vec3(0.2, 1.96, 1));
                Model->scale(vec3(0.2, 0.2, 0.2));
                setModel(prog1, Model);
                person->draw(prog1);
            Model->popMatrix();
        Model->popMatrix();
        prog1->unbind();
        
        // draw the ray
        prog2->bind();
        glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
        glUniformMatrix4fv(prog2->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
        Model->pushMatrix();
            Model->loadIdentity();
            Model->translate(vec3(gTrans, 0, 0));
            Model->translate(vec3(0, Up, 0));
            Model->translate(vec3(0.2, 2.0, 1));
            Model->pushMatrix();
                Model->scale(vec3(0.06, 0.6+0.2*sTheta, 0));
                Model->translate(vec3(0.1, -0.1, -1));
                setModel(prog2, Model);
                cube->draw(prog2);
            Model->popMatrix();
            Model->pushMatrix();
                Model->scale(vec3(0.6+0.2*sTheta,0.06, 0));
                Model->translate(vec3(0.1, -0.1, -1));
                setModel(prog2, Model);
                cube->draw(prog2);
            Model->popMatrix();
        Model->popMatrix();
        prog2->unbind();
     
		//animation update example
		sTheta = sin(glfwGetTime());
        
		// Pop matrix stacks.
		Projection->popMatrix();
		View->popMatrix();

	}
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../../resources";
   //std::string resourceDir = " /Users/ccmac/Documents/GitHub/CPE471/OGLBase/resources";
   

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();
		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
