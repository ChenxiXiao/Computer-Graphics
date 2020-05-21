/*
 * Program 2 base code - includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>
#include <unordered_map>
#include <string>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "Texture.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "stb_image.h"

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
    std::shared_ptr<Program> prog, prog2;
    std::shared_ptr<Program> texProg, cubeProg;;
	// Shape to be used (from  file) - modify to support multiple
	

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 player_pos = vec3(-0.7, -0.1 ,0.1);

	//animation data
	float sTheta = 0, animatedRot = 0;;
    
    bool turn_on_road = false;
    float x,y,z;
    float theta = 0;    // yaw control
    float phi = 0;        // pitch control
    float radius = 1;
    vec3 center, forward;
    float speed = 0.02;
    float gRot= 1;
    int prevX, prevY;
    bool restriction = true;
    //camera
    vec3 eye = vec3(-1.0, -0.1 ,0.1);
    vec3 up = vec3(0,1,0);
    bool obstacle1, obstacle2, obstacle3, has_mask;
    //terrain mapping
    float terrain[80];
    // Skybox
   vector<std::string> faces {"3_skybox.bmp", "1_skybox.bmp", "5_skybox.bmp", "4_skybox.bmp", "0_skybox.bmp", "2_skybox.bmp"};
    
    //store all the single meshes
    unordered_map<string, shared_ptr<Shape>> meshes; //contains shared_ptr<Shape> campus, person, cube;
    shared_ptr<Shape> student;
    vector<shared_ptr<Shape>> multi_obj;
    //texture
    shared_ptr<Texture> mask, world_t, virus, male, sun, sars;

     
    
    unsigned int createSky(string dir, vector<string> faces) {
        unsigned int textureID;
        glGenTextures(1, &textureID); glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(false);
        for(GLuint i = 0; i < faces.size(); i++) {
            unsigned char *data = stbi_load((dir+faces[i]).c_str(), &width, &height, &nrChannels, 0);
            if (data) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            }
            else
            {
                cout << "failed to load: " << (dir+faces[i]).c_str() << endl;
                
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
       // cout << " creating cube map any errors : " << glGetError() << endl;
      return textureID;
    }

    
	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
        if (key == GLFW_KEY_A && action == GLFW_PRESS) {
            eye += speed * cross(up, forward);
        }
        if (key == GLFW_KEY_D && action == GLFW_PRESS) {
            eye -= speed * cross(up, forward);
        }
        if (key == GLFW_KEY_W && action == GLFW_PRESS) {
            //forward
            eye += speed * forward;
            if (eye.y < 0)
                eye.y = 0;
        }
        if (key == GLFW_KEY_S && action == GLFW_PRESS) {
            eye -= speed * forward;
            if (eye.y < 0)
                eye.y = 0;
        
        }
        //faster speed
        if (key == GLFW_KEY_X && action == GLFW_PRESS) {
            speed = 0.1;
            gRot = 0.2;
            turn_on_road = true;
        }
        if (key == GLFW_KEY_X && action == GLFW_RELEASE) {
            speed = 0.02;
            gRot = 1;
            turn_on_road = false;
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

    void scrollCallback(GLFWwindow* window, double deltaX, double deltaY){
       
        theta += deltaX * speed; //yaw
        phi += deltaY * speed; //pitch
        //check in bound
        //Verticalz: 160 degrees (80 degrees up and 80 down). -- pitch
        //horizontal : spin their view 360 degrees around -- yaw
        if ( phi > 80.0f )
        {
           phi = 80.0f;
        }

        else if ( phi  < -80.0f )
        {
           phi = -80.0f;
        }
        if ( theta > 180.0f )
        {
            theta = 180.0f;
        }
        else if ( theta  < -180.0f )
        {
            theta = -180.0f;
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
        
        //campus
        prog = make_shared<Program>();
        prog->setVerbose(true);
        prog->setShaderNames(resourceDirectory + "/campus_vert.glsl", resourceDirectory + "/campus_frag.glsl");
        prog->init();
        prog->addUniform("P");
        prog->addUniform("V");
        prog->addUniform("M");
        prog->addAttribute("vertPos");
        prog->addAttribute("vertNor");
        prog->addAttribute("vertCol");
        //  Yellow ray
        prog2 = make_shared<Program>();
        prog2->setVerbose(true);
        prog2->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/yellow_frag.glsl");
        prog2->init();
        prog2->addUniform("P");
        prog2->addUniform("V");
        prog2->addUniform("M");
        prog2->addAttribute("vertPos");
        prog2->addAttribute("vertNor");
    
        //texture shader
        texProg = make_shared<Program>();
        texProg->setVerbose(true);
        texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl",resourceDirectory + "/tex_frag0.glsl");
        texProg->init();
        texProg->addUniform("P");
        texProg->addUniform("M");
        texProg->addUniform("V");
        texProg->addAttribute("vertPos");
        texProg->addAttribute("vertNor");
        texProg->addAttribute("vertTex");
        texProg->addUniform("Texture0");

        cubeProg = make_shared<Program>();
        cubeProg->setVerbose(true);
        cubeProg->setShaderNames(resourceDirectory + "/cube_vert.glsl",resourceDirectory + "/cube_frag.glsl");
        cubeProg->init();
        cubeProg->addUniform("P");
        cubeProg->addUniform("M");
        cubeProg->addUniform("V");
        cubeProg->addAttribute("vertPos");
        cubeProg->addAttribute("vertNor");
        cubeProg->addUniform("skybox");
    
	}
   
    void rotateX3D(float theta, tinyobj::shape_t &shape){
        
        float sin_t = sin(theta);
        float cos_t = cos(theta);
       for (size_t v = 0; v < shape.mesh.positions.size() / 3; v++) {
           float y = shape.mesh.positions[3*v+1];
           float z = shape.mesh.positions[3*v+2];
           shape.mesh.positions[3*v+1] = y * cos_t - z * sin_t;
           shape.mesh.positions[3*v+2] =z * cos_t + y * sin_t;
           float res =  terrain[int(20*(shape.mesh.positions[3*v+0]+1))+int(20*(shape.mesh.positions[3*v+2]+1))];
           if(res){
            res = (res+shape.mesh.positions[3*v+1])/2;
               }
           else{
               res =shape.mesh.positions[3*v+1];
               }
       }
    }

    float getHeight(float x, float z){
        return terrain[int(20*(x+1))+int(20*(z+1))]-0.02;
    }
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

    void loadMesh(const std::string name, const std::string& resourceDirectory)
    {
        vector<tinyobj::shape_t> shapes;
        vector<tinyobj::material_t> objMaterials;
        string errStr;
        shared_ptr<Shape> mesh;
        bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, (resourceDirectory+"/"+name).c_str());
        
        resize_obj(shapes);
        if(name == "road.obj"){
            rotateX3D(-1.6, shapes[0]);
        } // get terrain buffer
        if (! rc)
        {
            cerr << errStr << endl;
        }
        else
        {
            mesh = make_shared<Shape>();
            mesh->createShape(shapes[0]);
            mesh->measure();
            mesh->init();
        }
        const int pos = name.find('.');
        std::string name1 = name;
        if(pos) {
            name1.erase(pos);
        }

        meshes.insert(make_pair(name1,mesh));
    }

	void initGeom(const std::string& resourceDirectory)
	{

        
        vector<tinyobj::shape_t> TOshapes;
         vector<tinyobj::material_t> objMaterials;
         string errStr;
     //load in the mesh and make the shape(s)
         bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/cartoon_boy/male.obj").c_str());
         resize_obj(TOshapes);
         if (!rc) {
             cerr << errStr << endl;
         } else {
             for (size_t i = 0; i < TOshapes.size(); i++) {
                
                 student = make_shared<Shape>();
                 student->createShape(TOshapes[i]);
                 student->measure();
                 student->init();
                 multi_obj.push_back(student);
             }
         }
        
            loadMesh("Campus.obj", resourceDirectory);
            loadMesh("SmoothSphere.obj", resourceDirectory);
            loadMesh("cube.obj", resourceDirectory);
            loadMesh("virus1.obj", resourceDirectory);
            loadMesh("virus2.obj", resourceDirectory);
            loadMesh("road.obj", resourceDirectory);
            loadMesh("mask.obj", resourceDirectory);

	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   }
    void initTex(const std::string& resourceDirectory)
    {
       
        
         virus = make_shared<Texture>();
         virus->setFilename(resourceDirectory + "/virus.jpg");
         virus->init();
         virus->setUnit(1);
         virus->setWrapModes(GL_REPEAT, GL_REPEAT);
        
         sun = make_shared<Texture>();
         sun->setFilename(resourceDirectory + "/sun.jpg");
         sun->init();
         sun->setUnit(2);
         sun->setWrapModes(GL_REPEAT, GL_REPEAT);
        
        
        male = make_shared<Texture>();
        male->setFilename(resourceDirectory + "/cartoon_boy/male.jpg");
        male->init();
        male->setUnit(3);
        male->setWrapModes(GL_REPEAT, GL_REPEAT);
        
         sars = make_shared<Texture>();
         sars->setFilename(resourceDirectory + "/sars.jpg");
         sars->init();
         sars->setUnit(4);
         sars->setWrapModes(GL_REPEAT, GL_REPEAT);
        
         mask = make_shared<Texture>();
         mask->setFilename(resourceDirectory + "/mask.jpg");
         mask->init();
         mask->setUnit(1);
         mask->setWrapModes(GL_REPEAT, GL_REPEAT);
        
        
    }
    
      float getRot(float animatedRot, bool* restriction) {
        if (animatedRot < -0.5)
            *restriction = false;
        else if (animatedRot > 0.5)
            *restriction = true;
        if (*restriction)
            return -0.01;
        else
            return 0.01;
    }
    
    void draw_multi_obj(shared_ptr<MatrixStack> Model){
        animatedRot += getRot(animatedRot, &restriction) / gRot;
        int i = 0;
        bool matrix_on = false;
        Model->translate(player_pos);
        Model->scale(0.008);
        Model->rotate(1.4+0.5*sTheta, vec3(0,1,0));
                   
        for(shared_ptr<Shape> s : multi_obj) {

            if(i == 7 |i == 20 | i == 19){ //right leg
                Model->pushMatrix();
                    Model->rotate(animatedRot, vec3(1, 0, 0));
                    matrix_on = true;
                 }
            
            if( i == 21|i == 0 | i == 6){ //left leg
                Model->pushMatrix();
                    Model->rotate(-animatedRot, vec3(1, 0, 0));
                    matrix_on = true;
                 }
            if( i == 15|i == 16){ //arm
             Model->pushMatrix();
                 Model->rotate(-0.5*animatedRot, vec3(1, 0, 0));
                 matrix_on = true;
              }
            if( i == 17|i == 18){ //arm
                  Model->pushMatrix();
                      Model->rotate(0.5*animatedRot, vec3(1, 0, 0));
                      matrix_on = true;
                   }
            setModel(texProg, Model);
            s->draw(texProg);
            if(matrix_on){
                Model->popMatrix();
                matrix_on =false;
            }
             i += 1;
        }
     }

 
 
    bool collide(vec3 obj, vec4 other){ // x y z diameter
        bool collisionX =  obj.x  <= other.x+other.w &&obj.x >= other.x-other.w  ;
        bool collisionY = obj.z <= other.z+other.w &&obj.z >= other.z-other.w ;
        return collisionX && collisionY;
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

        //yaw pitch
       
        x = radius*cos(radians(phi))*cos(radians(theta));
        y = radius*sin(radians(phi));
        z = radius*cos(radians(phi))*sin(radians(theta));
       vec3 LA = vec3(x, y, z);
        forward = LA;
        center = eye + LA;
        unsigned int  cubeMapTexture = createSky("../../resources/Skybox/", faces);
        
        float x_dir = (eye + vec3(0.02) *LA).x;
        float z_dir =(eye +  vec3(0.02)*LA).z;
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(46.0f, aspect, 0.01f, 100.0f);

		// View is global translation along negative z for now
		View->pushMatrix();
			View->loadIdentity();
              View->lookAt(eye, center, up);
        
        //to draw the sky box bind the right shader
        cubeProg->bind();
            //set the projection matrix - can use the same one
            glUniformMatrix4fv(cubeProg->getUniform("P"), 1, GL_FALSE,value_ptr(Projection->topMatrix()));
            //set the depth function to always draw the box!
            glDepthFunc(GL_LEQUAL);
            //set up view matrix to include your view transforms
            //(your code likely will be different depending
            glUniformMatrix4fv(cubeProg->getUniform("V"), 1,GL_FALSE,value_ptr(View->topMatrix()) );
            //set and send model transforms - likely want a bigger cube
            Model->pushMatrix();
                Model->loadIdentity();
                Model->scale(vec3(2,2,2));
                setModel(cubeProg, Model);
                //bind the cube map texture
                glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
                //draw the actual cube
                meshes.at("cube")->draw(cubeProg);
                
            Model->popMatrix();
            //set the depth test back to normal!
            glDepthFunc(GL_LESS);
            //unbind the shader for the skybox
        cubeProg->unbind();
     
        
        //draw sun
        texProg->bind();
            glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
            glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
                
            Model->pushMatrix();
                Model->loadIdentity();
                 Model->pushMatrix();
                    Model->translate(vec3(-0.3, 0.4, 0.70));
                    Model->scale(0.05);
                    //Model->rotate(-0.4, vec3(1,0,0));
                    sun->bind(texProg->getUniform("Texture0"));
                    setModel(texProg, Model);
                    meshes.at("SmoothSphere")->draw(texProg);
                    sun->unbind();
                Model->popMatrix();
                vec3 Virus2_pos  = vec3(clamp(-0.1+0.2*sTheta, -0.9, 0.9), -0.08, clamp(0.2+ 0.05*sTheta, -0.2, -0.6));
                vec3 Virus1_pos = vec3(clamp(-0.3-0.05*sTheta, -0.5, 0.9),-0.08, -0.2);
                vec3 Virus3_pos = vec3(clamp(-0.3-0.0707*sTheta, 0.7, 1.0), -0.08,clamp( 0.9+0.0707*sTheta, 0.4, 1.0));
                vec3 mask_pos = vec3 (0.8, 0.01, 0.5);
                 player_pos = vec3(x_dir,getHeight(x_dir, z_dir),z_dir);
       
        //check collision
               if(collide(vec3(player_pos), vec4(Virus1_pos, 0.2))){
                     obstacle1 = true;
                 }
                if(collide(vec3(player_pos), vec4(Virus2_pos,0.2))){
                      obstacle2 = true;
                  }
                if(collide(vec3(player_pos), vec4(Virus3_pos,0.2))){
                    obstacle3 = true;
                }
            
                if(collide(vec3(player_pos), vec4(mask_pos , 0.2))){
                    has_mask = true;
                }
        
         //Draw obstacles
                Model->pushMatrix();
                    if(!obstacle1){
                        Model->pushMatrix();
                            Model->translate(Virus1_pos);
                            Model->scale(0.05);
                            virus->bind(texProg->getUniform("Texture0"));
                            setModel(texProg, Model);
                            meshes.at("virus1")->draw(texProg);
                            virus->unbind();
                        Model->popMatrix();
                    }
        //2nd obstacle
                    if(!obstacle2){
                        Model->pushMatrix();
                              Model->translate(Virus2_pos);
                                 Model->scale(0.1);
                                sars->bind(texProg->getUniform("Texture0"));
                              setModel(texProg, Model);
                              meshes.at("virus2")->draw(texProg);
                              sars->unbind();
                        Model->popMatrix();
                    }
        //3rd obstacle
                    if(!obstacle3){
                        Model->pushMatrix();
                              Model->translate(Virus3_pos);
                                 Model->scale(0.05);
                                virus->bind(texProg->getUniform("Texture0"));
                              setModel(texProg, Model);
                              meshes.at("virus2")->draw(texProg);
                              virus->unbind();
                        Model->popMatrix();
                    }
        //surgical mask
            if(!has_mask){
                Model->pushMatrix();
                      Model->translate(mask_pos);
                         Model->scale(0.05);
                        mask->bind(texProg->getUniform("Texture0"));
                      setModel(texProg, Model);
                      meshes.at("mask")->draw(texProg);
                      mask->unbind();
                Model->popMatrix();
                if(obstacle1|obstacle2|obstacle3){
                    Model->pushMatrix();
                        Model->translate(player_pos);
                        Model->translate(vec3(0.01,-0.01,0.01));
                        virus->bind(texProg->getUniform("Texture0"));
                        Model->scale(0.01);
                        Model->pushMatrix();
                            setModel(texProg, Model);
                            meshes.at("virus2")->draw(texProg);
                        Model->popMatrix();
                        Model->pushMatrix();
                            Model->translate(vec3(-0.04,0.01,0.02));
                            setModel(texProg, Model);
                            meshes.at("virus2")->draw(texProg);
                         Model->popMatrix();
                         virus->unbind();
                     Model->popMatrix();
                    Model->popMatrix();
                }
            }
        else{
            Model->pushMatrix();
                 Model->translate(player_pos);
                  Model->translate(vec3(0.0,0.005,0));
                 Model->scale(0.005);
                 mask->bind(texProg->getUniform("Texture0"));
                 setModel(texProg, Model);
                 meshes.at("mask")->draw(texProg);
                 mask->unbind();
            Model->popMatrix();
        }
        //draw the character
            Model->pushMatrix();
                male->bind(texProg->getUniform("Texture0"));
                draw_multi_obj(Model);
                male->unbind();
            Model->popMatrix();
        Model->popMatrix();
        texProg->unbind();
        
		// Draw the campus as the backgorund
		prog->bind();
        glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
        glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
         
        Model->pushMatrix();
            Model->loadIdentity();
        if(turn_on_road){
            Model->pushMatrix();
                Model->translate(vec3(0,-0.05, 0));
                setModel(prog, Model);
                meshes.at("road")->draw(prog);
            Model->popMatrix();
        }
            Model->pushMatrix();
                Model->rotate(-1.6, vec3(1,0,0));
                Model->scale(1.1);
                setModel(prog, Model);
                meshes.at("Campus")->draw(prog);
            Model->popMatrix();
         
        Model->popMatrix();
        prog->unbind();
        

        // draw the ray
        prog2->bind();
            glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
            glUniformMatrix4fv(prog2->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
            Model->pushMatrix();
                Model->loadIdentity();
                Model->translate(vec3(-0.3, 0.4, 0.705));
                Model->scale(0.2);
                Model->rotate(1.4, vec3(0,1,0));
                Model->pushMatrix();
                    Model->scale(vec3(0.05, 0.5+0.2*sTheta, 0.02));
                    setModel(prog2, Model);
                    meshes.at("cube")->draw(prog2);
                Model->popMatrix();
                Model->pushMatrix();
                    Model->scale(vec3(0.5+0.2*sTheta,0.05, 0.02));
                    setModel(prog2, Model);
                    meshes.at("cube")->draw(prog2);
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
	std::string resourceDir = "../../resources";

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
    application->initTex(resourceDir);
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
