/*
CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include <math.h>
#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
# define PI 3.14159265358979323846

using namespace std;
using namespace glm;
shared_ptr<Shape> shape, trench, space,turret,barrier,blast;
vector<vec3>posBuf_noInd;
vector<vec3>normals;
vector<vec2>texture_noInd;
static float angleY = 1.0f;
static float speed = 0.0f;
static float leanAngle = 0.0f;
static bool leanRight = false;
static bool leanLeft = false;
static bool noseUp = false;
static bool noseDown = false;
float equilibrium = 0;
float MaxLeft = PI * 30 / 180 + equilibrium;
float MaxRight = equilibrium - PI * 30 / 180;
float originX = 0.0f;
float originY = -0.5f;
float originZ = -4.0f;
vec2 BoundX = vec2(originX + 1.0f,originX -1.0f);
vec2 BoundY = vec2(originY + 1.8f,originY -.9f);
static vec3 XWingPos = vec3(originX, originY,originZ);
static float noseAngle = 0.0f;
static float noseEquilibrium = 0.0f;
float MaxUp = PI * 30 / 180 + noseEquilibrium;
float MaxDown = noseEquilibrium - MaxUp;
static float speedX = .05;
static float speedY = .05;
static float flightSpeed = .05;
static bool speeding = false;
static float speedingZoomIn = 0.0f;
static float trenchRender = 0.0f;
static int turretOffset = 0;
static int barrierOffset = 0;
static bool starting = true;
static std::vector<float> turretpos;
static std::vector<float> barrierpos;
static int reload = 0;
static vec2 expcoords = vec2(0, 0);
static float sumtime;
static int frame = 0;
static int exploded = 0;
static std::vector<vec4> boxes;//first box turret,second box barrier
static bool gameOver = false;
static bool crashed = false;
static bool restart = false;
static float blastrad = 1.0f;
float invincibleZ = -6.0f;
static bool reachedEnd = false;
float its_away = 30;
static int dstiles = 10;
static float startRender;

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		if (w == 1)
		{
			speed = 1*ftime;
		}
		else if (s == 1)
		{
			speed = -1*ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = -1*ftime;
		else if(d==1)
			yangle = 1*ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed,1);
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog,psky,pexplode;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID,VertexArrayID2;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexColorIDBox,NorBufferID,TexBufferID, IndexBufferIDBox;

	//texture data
	GLuint Texture,Texture2;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			noseUp = true;
			//mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			noseUp = false;
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			noseDown = true;
			//mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			noseDown = false;
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			leanLeft = true;
			//mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			leanLeft = false;
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			leanRight = true;
			//mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			leanRight = false;
			mycam.d = 0;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		{
			flightSpeed = flightSpeed * 2.0f;
			speeding = true;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
		{
			flightSpeed = flightSpeed / 2.0f;
			speeding = false;
		}

	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		
		string resourceDirectory = "../resources" ;
		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/XWing.obj", &resourceDirectory);
		shape->resize();
		shape->init();

		trench = make_shared<Shape>();
		trench->loadMesh(resourceDirectory + "/DStar.obj");
		trench->resize();
		trench->init();

		turret = make_shared<Shape>();
		turret->loadMesh(resourceDirectory + "/Turret/DSTurret.obj", &resourceDirectory);
		turret->resize();
		turret->init();

		space = make_shared<Shape>();
		space->loadMesh(resourceDirectory + "/sphere.obj");
		space->resize();
		space->init();

		barrier = make_shared<Shape>();
		barrier->loadMesh(resourceDirectory + "/DStarSurface.obj");
		barrier->resize();
		barrier->init();

		blast = make_shared<Shape>();
		blast->loadMesh(resourceDirectory + "/laserBolt.obj", &resourceDirectory);
		blast->resize();
		blast->init();

		int width, height, channels;
		char filepath[1000];

		//texture 1
		string str = resourceDirectory + "/space.png";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//texture 2
		str = resourceDirectory + "/explosion.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
																	 // Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		//generate the VAO1
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		/*for (int j = 0; j < shape->eleBuf[0].size();j++)
		{
			posBuf_noInd.push_back(glm::vec3(shape->posBuf[0][3 * shape->eleBuf[0][j] + 0], shape->posBuf[0][3 * shape->eleBuf[0][j] + 1], shape->posBuf[0][3 * shape->eleBuf[0][j] + 2]));
		}
		printf("size: %d\n", posBuf_noInd.size());

		glBufferData(GL_ARRAY_BUFFER, posBuf_noInd.size() * sizeof(vec3), posBuf_noInd.data(), GL_DYNAMIC_DRAW);
		*/

		GLfloat cube_vertices[] = {
			// front
			-1.0, -1.0,  1.0,//LD
			1.0, -1.0,  1.0,//RD
			1.0,  1.0,  1.0,//RU
			-1.0,  1.0,  1.0,//LU
		};
		//make it a bit smaller
		for (int i = 0; i < 12; i++)
			cube_vertices[i] *= 0.5;
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);


		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//color
		GLfloat cube_norm[] = {
			// front colors
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,

		};

		//generate normal buffer to hand off to OGL
		glGenBuffers(1, &NorBufferID);
		//set the current state to focus on our normal buffer
		glBindBuffer(GL_ARRAY_BUFFER, NorBufferID);
		/*
		for (int j = 0; j < shape->eleBuf[0].size()/3;j++)
		{
			//if (j == shape->eleBuf[0].size() - 1) { printf("%d\n", j); }
			//assign into ABC positions
			glm::vec3 pos1 = posBuf_noInd[3 * j + 0];
			glm::vec3 pos2 = posBuf_noInd[3 * j + 1];
			glm::vec3 pos3 = posBuf_noInd[3 * j + 2];
			glm::vec3 vec1_2 = pos2 - pos1;
			glm::vec3 vec1_3 = pos3 - pos1;
			//normalized normal
			glm::vec3 normal = glm::normalize(glm::cross(vec1_2, vec1_3));
			for (int rep = 0; rep < 3; rep++)
			{
				normals.push_back(normal);
			}
		}*/

		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_norm), cube_norm, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(1);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &TexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, TexBufferID);

		glm::vec2 cube_tex[] = {
			// front colors
			glm::vec2(0.0, 1.0),
			glm::vec2(1.0, 1.0),
			glm::vec2(1.0, 0.0),
			glm::vec2(0.0, 0.0),

		};
		/*for (int j = 0; j < shape->eleBuf[0].size();j++)
		{
			texture_noInd.push_back(glm::vec2(shape->texBuf[0][2 * shape->eleBuf[0][j] + 0], shape->texBuf[0][2 * shape->eleBuf[0][j] + 1]));
			//printf("%f,%f,%f\n", posBuf_noInd[j].x, posBuf_noInd[j].y, posBuf_noInd[j].z);
		}
		printf("size: %d\n", texture_noInd.size());*/

		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tex), cube_tex, GL_DYNAMIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(2);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort cube_elements[] = {

			// front
			0, 1, 2,
			2, 3, 0,
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);


		//generate the VAO2
		glGenVertexArrays(1, &VertexArrayID2);
		glBindVertexArray(VertexArrayID2);

		glGenBuffers(1, &VertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		posBuf_noInd.clear();

		for (int j = 0; j < trench->eleBuf[0].size();j++)
		{
			posBuf_noInd.push_back(glm::vec3(trench->posBuf[0][3 * trench->eleBuf[0][j] + 0], trench->posBuf[0][3 * trench->eleBuf[0][j] + 1], trench->posBuf[0][3 * trench->eleBuf[0][j] + 2]));
			//printf("%f,%f,%f\n", posBuf_noInd[j].x, posBuf_noInd[j].y, posBuf_noInd[j].z);
		}

		glBufferData(GL_ARRAY_BUFFER, posBuf_noInd.size() * sizeof(vec3), posBuf_noInd.data(), GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		
		//generate normal buffer to hand off to OGL
		glGenBuffers(1, &NorBufferID);
		//set the current state to focus on our normal buffer
		glBindBuffer(GL_ARRAY_BUFFER, NorBufferID);

		normals.clear();

		for (int j = 0; j < trench->eleBuf[0].size() / 3;j++)
		{
			if (j == trench->eleBuf[0].size() - 1) { printf("%d\n", j); }
			//assign into ABC positions
			glm::vec3 pos1 = posBuf_noInd[3 * j + 0];
			glm::vec3 pos2 = posBuf_noInd[3 * j + 1];
			glm::vec3 pos3 = posBuf_noInd[3 * j + 2];
			glm::vec3 vec1_2 = pos2 - pos1;
			glm::vec3 vec1_3 = pos3 - pos1;
			//normalized normal
			glm::vec3 normal = glm::normalize(glm::cross(vec1_2, vec1_3));
			for (int rep = 0; rep < 3; rep++)
			{
				normals.push_back(normal);
			}
		}

		/*vector<int>visited;

		for (int i = 0; i < trench->eleBuf[0].size();i++)
		{
			auto p =std::find(visited.begin(), visited.begin() + visited.size(), i);
			if(p != visited.begin() + visited.size())
			{
				visited.erase(p);
				continue;
			}
			vector<vec3*>norms;
			norms.push_back(&normals[i]);
			for(int j = i+1; j < shape->eleBuf[0].size();j++)
			{
				//printf("%d\n",j);
				if(posBuf_noInd[i] == posBuf_noInd[j])
				{
					visited.push_back(j);
					norms.push_back(&(normals[j]));
				}
			}
			vec3 normalsum = vec3(0, 0, 0);
			for (int j = 0;j < norms.size();j++)
			{
				normalsum += *norms[j];
			}
			normalsum = normalize(normalsum);
			for (int j = 0; j < norms.size();j++)
			{
				*norms[j] = normalsum;
			}
		}*/

		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), normals.data(), GL_DYNAMIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(1);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		//generate vertex buffer to hand off to OGL
		/*glGenBuffers(1, &TexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, TexBufferID);
		glBufferData(GL_ARRAY_BUFFER, texture_noInd.size() * sizeof(vec2), texture_noInd.data(), GL_DYNAMIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(2);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);*/
		
		glBindVertexArray(0);

		appendBox(turret);
		appendBox(barrier);
		appendBox(shape);
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.9f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");
		prog->addUniform("obj");

		psky = std::make_shared<Program>();
		psky->setVerbose(true);
		psky->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		if (!psky->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		psky->addUniform("P");
		psky->addUniform("V");
		psky->addUniform("M");
		psky->addUniform("campos");
		psky->addAttribute("vertPos");
		psky->addAttribute("vertNor");
		psky->addAttribute("vertTex");

		pexplode = std::make_shared<Program>();
		pexplode->setVerbose(true);
		pexplode->setShaderNames(resourceDirectory + "/explodevertex.glsl", resourceDirectory + "/explodefrag.glsl");
		if (!pexplode->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pexplode->addUniform("P");
		pexplode->addUniform("V");
		pexplode->addUniform("M");
		pexplode->addUniform("campos");
		pexplode->addUniform("exp_frame");
		pexplode->addAttribute("vertPos");
		pexplode->addAttribute("vertNor");
		pexplode->addAttribute("vertTex");

	}

	void appendBox(shared_ptr<Shape> shapeToBox)
	{
		float minX, minY, minZ;
		float maxX, maxY, maxZ;

		minX = minY = minZ = 1.1754E+38F;
		maxX = maxY = maxZ = -1.1754E+38F;

		// Go through all vertices to determine min and max of each dimension
		for (int i = 0; i < shapeToBox->obj_count; i++)
		{
			for (size_t v = 0; v < shapeToBox->posBuf[i].size() / 3; v++)
			{
				vec4 pos = vec4(shapeToBox->posBuf[i][3 * v + 0], shapeToBox->posBuf[i][3 * v + 1], shapeToBox->posBuf[i][3 * v + 2], 1.0f);
				if (pos.x < minX) minX = pos.x;
				if (pos.x > maxX) maxX = pos.x;

				if (pos.y < minY) minY = pos.y;
				if (pos.y > maxY) maxY = pos.y;

				if (pos.z < minZ) minZ = pos.z;
				if (pos.z > maxZ) maxZ = pos.z;
			}
		}
		//printf("%f,%f,%f,%f,%f,%f,%f\n",minX,maxX,minY,maxY,minZ,maxZ);
		boxes.push_back(vec4(minX, minY, minZ,1.0f)); 
		boxes.push_back(vec4(minX, minY, maxZ,1.0f));
		boxes.push_back(vec4(minX, maxY, minZ,1.0f));
		boxes.push_back(vec4(minX, maxY, maxZ,1.0f));
		boxes.push_back(vec4(maxX, minY, minZ,1.0f));
		boxes.push_back(vec4(maxX, minY, maxZ,1.0f));
		boxes.push_back(vec4(maxX, maxY, minZ,1.0f));
		boxes.push_back(vec4(maxX, maxY, maxZ,1.0f));
	}
	void restarting()
	{
		gameOver = false;
		starting = true;
		XWingPos = vec3(originX, originY, originZ);
		frame = 0;
		sumtime = 0;
		exploded = 0;
		crashed = false;
		barrierpos.clear();
		turretpos.clear();
		trenchRender = 0.0f;
		blastrad = 1.0f;
		expcoords = vec2(0, 0);
		barrierOffset = 0;
		turretOffset = 0;
		reload = 0;
		reachedEnd = false;
		dstiles = 10;
	}

	bool checkCollision(int offset,glm::mat4 M)
	{
		if (XWingPos.z > invincibleZ)
		{
			return false;
		}
		if (XWingPos.x > BoundX.x || XWingPos.x < BoundX.y || XWingPos.y < BoundY.y)
		{
			printf("walls\n");
			return true;
		}
		std::vector<vec4> adjpos;
		float minX, minY, minZ, maxX, maxY, maxZ;

		minX = minY = minZ = 1.1754E+38F;
		maxX = maxY = maxZ = -1.1754E+38F;

		if (offset == 2)
		{
			glm::mat4 ScaleCollision = glm::scale(mat4(1.0f), vec3(1.0, 1.0f,1.0f));
			M = M * ScaleCollision;
		}

		for (int i = 0;i < 8;i++)
		{
			vec4 adj = M * boxes[offset * 8 + i];
			if (adj.x < minX) minX = adj.x;
			if (adj.x > maxX) maxX = adj.x;
			if (adj.y < minY) minY = adj.y;
			if (adj.y > maxY) maxY = adj.y;
			if (adj.z < minZ) minZ = adj.z;
			if (adj.z > maxZ) maxZ = adj.z;

		}
		if (XWingPos.x < maxX && XWingPos.x > minX && XWingPos.y > minY && XWingPos.y < maxY && XWingPos.z < maxZ && XWingPos.z > minZ)
		{
			
			//printf("XWingPosz: %f", XWingPos.z);
			//printf("minZ: %f,maxZ %f", minZ, maxZ);
			//printf("true\n");
			return true;
		}
		return false;
	}

	void moveXWing()
	{
		if (leanRight == true)
		{
			if (leanAngle > MaxRight)
			{
				leanAngle -= 4 * PI / 180;
			}
			XWingPos.x += speedX;
		}
		if (leanLeft == true)
		{
			if (leanAngle < MaxLeft)
			{ 
				leanAngle += 4 * PI / 180;
			}
			XWingPos.x -= speedX;
		}
		if (leanRight == false && leanLeft == false)
		{
			float relativeToEquilibrium = 0;
			if (leanAngle - equilibrium < 0)
			{
				relativeToEquilibrium = -1;
			}
			else if (leanAngle - equilibrium > 0)
			{
				relativeToEquilibrium = 1;
			}
			float newDif = abs(leanAngle - equilibrium) - 4 * PI / 180;
			leanAngle = equilibrium + relativeToEquilibrium * newDif;
			if (newDif < 4 * PI / 180)
			{
				leanAngle = equilibrium;
			}
		}

		if (noseUp == true)
		{
			if (noseAngle < MaxUp)
				noseAngle += 4 * PI / 180;
			if (XWingPos.y < BoundY.x)
			{
				XWingPos.y += speedY;
			}
		}
		if (noseDown == true)
		{
			if (noseAngle > MaxDown)
				noseAngle -= 4 * PI / 180;
			XWingPos.y -= speedY;
			
		}
		if (noseUp == false && noseDown == false)
		{
			float relativeToEquilibrium = 0;
			if (noseAngle - noseEquilibrium < 0)
			{
				relativeToEquilibrium = -1;
			}
			else if (noseAngle - noseEquilibrium > 0)
			{
				relativeToEquilibrium = 1;
			}
			float newDif = abs(noseAngle - noseEquilibrium) - 4 * PI / 180;
			noseAngle = noseEquilibrium + relativeToEquilibrium * newDif;
			if (newDif < 4 * PI / 180)
			{
				noseAngle = noseEquilibrium;
			}
		}

		if (speeding == true && speedingZoomIn < 1.0f)
		{
			speedingZoomIn += .05f;
		}
		else if (speeding == false && speedingZoomIn > 0.0f)
		{
			speedingZoomIn -= .05f;
		}
	}

	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/


	void render()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		double frametime = get_last_elapsed_time();

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now

		float angleX = PI * -0.5f;
		angleY = PI;

		glm::mat4 V, M, P, RY, RX; //View, Model and Perspective matrix
		V = glm::mat4(1);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);
		if (width < height)
		{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect, -2.0f, 100.0f);
		}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones

		//animation with the model matrix:
		static float w = 0.0;

		//XWing fly
		if (crashed == false)
		{
			XWingPos.z -= flightSpeed;

			moveXWing();
		}

		mycam.pos = vec3(-XWingPos.x,-XWingPos.y,-XWingPos.z) + vec3(0.0f, -0.25f + .05 * speedingZoomIn, -2.0f + .25 * speedingZoomIn);

		float trans = 0;// sin(t) * 2;
		float aX = PI / 2;
		glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), angleY, glm::vec3(0.0f, 1.0f, 0.0f));
		float angle = -3.1415926 / 2.0;
		glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), noseAngle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), aX, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 Trans = glm::translate(glm::mat4(1.0f), glm::vec3(XWingPos.x, XWingPos.y, XWingPos.z));
		glm::mat4 CamTrans = glm::translate(glm::mat4(1.0f), glm::vec3(mycam.pos.x, mycam.pos.y, mycam.pos.z));
		glm::mat4 skyTrans = glm::translate(glm::mat4(1.0f), glm::vec3(-mycam.pos.x, -mycam.pos.y, -mycam.pos.z));
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f));
		glm::mat4 RotateZ = glm::rotate(glm::mat4(1.0f), leanAngle, vec3(0.0f, 0.0f, 1.0f));
		//glm::mat4 thirdPerson = glm::translate(mat4(1.0f), vec3(0.0f, -0.25f + .05 * speedingZoomIn, -2.0f + .25 * speedingZoomIn));

		V = CamTrans;

		float sangle = 3.1415926 / 2.;
		glm::mat4 RotateXSky = glm::rotate(glm::mat4(1.0f), sangle, glm::vec3(1.0f, 0.0f, 0.0f));

		S = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
		M = skyTrans * S * Rx;

		psky->bind();

		S = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f));
		//send the matrices to the shaders
		glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(psky->getUniform("campos"), 1, &mycam.pos[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glDisable(GL_DEPTH_TEST);
		space->draw(psky, FALSE);
		glEnable(GL_DEPTH_TEST);

		psky->unbind();

		M = Trans * RotateZ * RotateX * RotateY * S;

		float expAngle = PI;
		glm::mat4 RotateExp = glm::rotate(glm::mat4(1.0f), expAngle, vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 TransExp = glm::translate(glm::mat4(1.0f), vec3(0.0f, 0.0f, -0.75f));
		glm::mat4 ScaleExp = glm::scale(glm::mat4(1.0f), vec3(1.5f, 1.5f, 1.0f));

		glm::mat4 M2 = M * TransExp * RotateExp * ScaleExp;

		// Draw the box using GLSL.
		//prog->bind();

		//prog->unbind();

		prog->bind();

		//send the matrices to the shaders
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
		glUniform1i(prog->getUniform("obj"), 0);

		//actually draw from vertex 0, 3 vertices
		/*glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
		glDrawArrays(GL_TRIANGLES, 0, shape->eleBuf[0].size());
		glBindVertexArray(VertexArrayID);*/
		if (crashed == false)
		{
			shape->draw(prog, FALSE);
		}

		angleX = -PI / 2;
		float angleZ = PI / 2;
		float turretAngle = PI/8;
		float barrierangle = PI / 2;
		float blastAngle = - PI / 6.8;
		glm::mat4 TurretTrans;
		glm::mat4 BarrierTrans;
		glm::mat4 TurretScale = glm::scale(mat4(1.0f), vec3(0.3f,0.4f,0.3f));
		glm::mat4 BarrierScale = glm::scale(mat4(1.0f), vec3(1.3f, 0.5f, 1.0f));
		glm::mat4 BarrierRotate = glm::rotate(mat4(1.0f),barrierangle,vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 TurretRotate = glm::rotate(mat4(1.0f), turretAngle, vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 BT2 = glm::translate(mat4(1.0f), vec3(0.25f,0.0f,0.0f));
		glm::mat4 BlastScale = glm::scale(mat4(1.0f), vec3(1.0f, 1.0f, 0.3f));
		glm::mat4 BlastRadius = glm::translate(mat4(1.0f), vec3(0.0f,0.0f,blastrad));
		glm::mat4 BlastTrans = glm::translate(mat4(1.0f),vec3(-0.25f,0.5f,0.0f));
		glm::mat4 BlastRotate = glm::rotate(mat4(1.0f), blastAngle, vec3(1.0f, 0.0f, 0.0f));
		RotateX = glm::rotate(mat4(1.0f), angleX, vec3(1.0f, 0.0f, 0.0f));
		RotateZ = glm::rotate(mat4(1.0f), angleZ, vec3(0.0f, 0.0f, 1.0f));
		S = glm::scale(mat4(1.0f), vec3(1.5f, 1.5f, 1.0f));

		std::vector<float> turretposcpy = turretpos;
		std::vector<float> barrierposcpy = barrierpos;

		if (blastrad < 10.0f)
		{
			blastrad += .15f;
		}
		else
		{
			blastrad = 1.0f;
		}
		glUniform1i(prog->getUniform("obj"), 2);
		bool collided = false;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < dstiles; j++)
			{
				Trans = glm::translate(mat4(1.0f), vec3(-2.0f + i, -1.3f, -1.0f * j + startRender));

				M = Trans * RotateX * S;
				glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
				glBindVertexArray(VertexArrayID2);
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
				glDrawArrays(GL_TRIANGLES, 0, trench->eleBuf[0].size());

				if (i == 0 && j % 3 == turretOffset)
				{
					if (starting == true)
					{
						float r = 1.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.0 - 1.0)));
						turretpos.push_back(r);
						TurretTrans = glm::translate(mat4(1.0f), vec3(r, 0.5f, 0.0f));
						//BarrierTrans = glm::translate(mat4(1.0f), vec3(2.0f, r -1.0f, 0.0f));
					}
					else
					{
						TurretTrans = glm::translate(mat4(1.0f), vec3(turretposcpy[0], 0.5f, 0.0f));
						//BarrierTrans = glm::translate(mat4(1.0f), vec3(2.0f, turretposcpy[0] - 1.0f, 0.0f));
						turretposcpy.erase(turretposcpy.begin());
					}
					
					M = Trans * TurretTrans * TurretScale;
					M = M * BlastTrans * BlastRotate * BlastRadius * BlastScale;
					if (checkCollision(2, M)) // offset of 2 for boxes when checking blast collision
					{
						printf("blaster\n");
						collided = true;
					}
					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
					glUniform1i(prog->getUniform("obj"), 1);
					blast->draw(prog, FALSE);

					M = M * BT2;
					if (checkCollision(2, M)) // offset of 2 for boxes when checking blast collision
					{
						printf("blaster\n");
						collided = true;
					}
					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
					blast->draw(prog, FALSE);
					glUniform1i(prog->getUniform("obj"), 2);

					M = Trans * TurretTrans* TurretRotate*TurretScale;
					if (checkCollision(0, M)) // offset of 0 for boxes when checking turret collision
					{
						printf("Turret\n");
						collided = true;
					}

					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
					turret->draw(prog, FALSE);

					/*M = Trans * BarrierTrans * BarrierScale * BarrierRotate;
					if (checkCollision(1, M)) // offset of 1 for boxes when checking barrier collision
					{
						collided = true;
					}

					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
					barrier->draw(prog, FALSE);*/
					//trench->draw(prog, FALSE);
				}
				if (i == 0 && j % 2 == barrierOffset && !(reachedEnd == true && j > dstiles - 6))
				{
					if (starting == true)
					{
						float r = 0.0f;
						if (j < 4)
						{
							r = 10.0f;
						}
						else
						{
							r = 1.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.0 - 1.0)));
						}
						barrierpos.push_back(r);
						BarrierTrans = glm::translate(mat4(1.0f), vec3(2.0f, r - 1.0f, 0.0f));
					}
					else
					{
						BarrierTrans = glm::translate(mat4(1.0f), vec3(2.0f, barrierposcpy[0] - 1.0f, 0.0f));
						barrierposcpy.erase(barrierposcpy.begin());
					}

					M = Trans * BarrierTrans * BarrierScale * BarrierRotate;
					if (checkCollision(1, M)) // offset of 1 for boxes when checking barrier collision
					{
						printf("barrier\n");
						collided = true;
					}

					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
					if (XWingPos.z + .4 > (-1.0f * j + trenchRender))
					{
						barrier->draw(prog, FALSE);
					}
					//trench->draw(prog, FALSE);
				}

				RotateZ = glm::rotate(mat4(1.0f), angleZ, vec3(0.0f, 0.0f, 1.0f));
				M = RotateZ * Trans * RotateX * S;
				glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
				glBindVertexArray(VertexArrayID2);
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
				glDrawArrays(GL_TRIANGLES, 0, trench->eleBuf[0].size());


				//trench->draw(prog, FALSE);

				RotateZ = glm::rotate(mat4(1.0f), -angleZ, vec3(0.0f, 0.0f, 1.0f));
				M = RotateZ * Trans * RotateX * S;
				glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
				glBindVertexArray(VertexArrayID2);
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
				glDrawArrays(GL_TRIANGLES, 0, trench->eleBuf[0].size());
				//trench->draw(prog, FALSE);

				if (reachedEnd == true && j == dstiles -1)
				{
					BarrierTrans = glm::translate(mat4(1.0f), vec3(1.5f, 1.35f, 0.5f));
					BarrierScale = glm::scale(mat4(1.0f), vec3(1.0f, 1.3f, 0.4f));
					M = Trans * BarrierTrans * BarrierScale * BarrierRotate;
					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
					barrier->draw(prog, FALSE);
				}
			}
		}
		if (collided == true)
		{
			exploded++;
		}
		else
		{
			exploded = 0;
		}
		starting = false;

		if (trenchRender - 1.0f > XWingPos.z)
		{
			trenchRender -= 1.0f;
			reload++;
			if (reload >= its_away)
			{
				//dstiles = 10 - (reload - its_away);
				reachedEnd = true;
			}
			else
			{
				startRender = trenchRender;
			}
			if (turretOffset == 0 && reachedEnd == false)
			{
				turretpos.erase(turretpos.begin());
				float r = 1.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.0 - 1.0)));
				turretpos.push_back(r);
			}
			if (barrierOffset == 0 && reachedEnd == false)
			{
				barrierpos.erase(barrierpos.begin());
				float r = 1.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (3.0 - 1.0)));
				barrierpos.push_back(r);
			}
			if (reachedEnd == false)
			{
				turretOffset = 2 - ((reload + 2) % 3);
				barrierOffset = reload % 2;
			}
		}
		printf("exploded: %d\n", exploded);
		if (exploded >= 3)
		{
			crashed = true;
		}

		if (crashed == true && gameOver == false)
		{

			pexplode->bind();

			//glBindVertexArray(VertexArrayID);

			float expframeX = 0.0f;
			float expframeY = 0.0f;
			if (sumtime > 0.0625)
			{
				sumtime = 0;
				frame++;
			}
			if (frame == 15)
			{
				restarting();
			}
			sumtime += get_last_elapsed_time();
			//printf("frame:%d,%d,%d\n",frame, (frame % 4),(frame / 4));
			vec2 expframe = vec2((frame % 4), (frame / 4));
			glUniformMatrix4fv(pexplode->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(pexplode->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniformMatrix4fv(pexplode->getUniform("M"), 1, GL_FALSE, &M2[0][0]);
			glUniform2fv(pexplode->getUniform("exp_frame"), 1, &expframe[0]);
			glUniform3fv(pexplode->getUniform("campos"), 1, &mycam.pos[0]);

			glBindVertexArray(VertexArrayID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Texture2);
			//glUniformMatrix4fv(pexplode->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);

			pexplode->unbind();

		}

		/*M = RotateX * S;

		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		//actually draw from vertex 0, 3 vertices
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
		glDrawArrays(GL_TRIANGLES, 0, posBuf_noInd.size());
		
		M = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, -0.15f, -1.0f));
		S = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		M = M * RotateX * S;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayID2);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
		glDrawArrays(GL_TRIANGLES, 0, trench->eleBuf[0].size());*/
		
		/*
		M = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.1f, -3.0f));
		S = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 10.0f, 1.0f));
		M = M * RotateX * S;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, trench->eleBuf[0].size());*/

		/*Trans = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, -3 + trans));
		RotateY = glm::rotate(glm::mat4(1.0f), -angleY, glm::vec3(0.0f, 1.0f, 0.0f));
		M = Trans * RotateY * RotateX * S;

		glBindVertexArray(VertexArrayID2);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, posBuf_noInd.size());*/


	}

};
//******************************************************************************************
int main(int argc, char **argv)
{


	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}
	
	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
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