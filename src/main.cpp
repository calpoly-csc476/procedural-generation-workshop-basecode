/* Base code for inverse kinematics lab */
/* This code is incomplete - follow tasks listed in handout */

// C standard library
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// C++ standard library
#include <iostream>
#include <memory>
#include <random>
#include <cassert>
#include <cmath>

// External dependencies
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tiny_obj_loader/tiny_obj_loader.h>

// GLM math
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Engine
#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "Texture.h"
#include "WindowManager.h"
#include "Util.h"



using namespace std;
using namespace glm;



class Application : public EventCallbacks
{

public:

	const int NumLights = 100;
	const int NumTrees = 25;

	WindowManager * windowManager = nullptr;

	// Shaders
	shared_ptr<Program> TerrainProg;
	shared_ptr<Program> BlinnPhongProg;

	// Shapes
	shared_ptr<Shape> sphere;
	shared_ptr<Shape> cube;
	shared_ptr<Shape> plus;
	shared_ptr<Shape> cylinder;

	int g_width = -1;
	int g_height = -1;

	// Ground Plane vertex data
	GLuint GroundVertexArray;
	int GroundIndexCount;

	vec3 g_light = vec3(-2, 6, -4);


	/////////////////
	// Camera Data //
	/////////////////

	// Previous frame start time (for time-based movement)
	float t0 = 0;

	vec3 cameraLookAt;

	float cTheta = 3.14159f / 2.f;
	float cPhi = -1.2f;
	bool mouseDown = false;

	double lastX = 0;
	double lastY = 0;
	float cameraRotateSpeed = 0.005f;

	bool moveForward = false;
	bool moveBack = false;
	bool moveLeft = false;
	bool moveRight = false;
	vec3 cameraPos = vec3(0, 4, -2);
	float cameraMoveSpeed = 3.0f;


	/////////////////////
	// Event Callbacks //
	/////////////////////

	void mouseCallback(GLFWwindow* window, int but, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			mouseDown = true;
		}

		if (action == GLFW_RELEASE)
		{
			mouseDown = false;
		}
	}

	void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		if (mouseDown)
		{
			cTheta += (float) (xpos - lastX) * cameraRotateSpeed;
			cPhi -= (float) (ypos - lastY) * cameraRotateSpeed;
			cPhi = glm::max(cPhi, -3.1415f / 2.f);
			cPhi = glm::min(cPhi, 3.1415f / 2.f);
		}

		lastX = xpos;
		lastY = ypos;
	}

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		switch (key)
		{
		case GLFW_KEY_W:
			moveForward = (action != GLFW_RELEASE);
			break;
		case GLFW_KEY_S:
			moveBack = (action != GLFW_RELEASE);
			break;
		case GLFW_KEY_A:
			moveLeft = (action != GLFW_RELEASE);
			break;
		case GLFW_KEY_D:
			moveRight = (action != GLFW_RELEASE);
			break;
		};

		static const float OffsetSpeed = 0.06f;

		if (action == GLFW_PRESS)
		{
			switch (key)
			{
				// Camera speed modifiers
			case GLFW_KEY_1:
				cameraMoveSpeed = 1.f;
				break;
			case GLFW_KEY_2:
				cameraMoveSpeed = 3.f;
				break;
			case GLFW_KEY_3:
				cameraMoveSpeed = 6.f;
				break;
			case GLFW_KEY_4:
				cameraMoveSpeed = 12.f;
				break;
			case GLFW_KEY_5:
				cameraMoveSpeed = 24.f;
				break;
			}
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{
	}

	void resizeCallback(GLFWwindow* window, int w, int h)
	{
		CHECKED_GL_CALL(glViewport(0, 0, g_width = w, g_height = h));
	}



	///////////////////////
	// Geometry Creation //
	///////////////////////

	// Create Geometry
	void initGeom()
	{
		// Create the ground plane
		const int groundSize = 128;

		vector<float> vertexData;
		vector<unsigned int> indexData;

		for (int i = 0; i <= groundSize; ++ i)
		{
			for (int j = 0; j <= groundSize; ++ j)
			{
				vertexData.push_back((float) i);
				vertexData.push_back(frand());
				vertexData.push_back((float) j);
			}
		}

		const int vertRowSize = groundSize + 1;
		for (int i = 0; i < groundSize; ++ i)
		{
			for (int j = 0; j < groundSize; ++ j)
			{
				indexData.push_back(i + j * vertRowSize + 0);
				indexData.push_back(i + j * vertRowSize + 1);
				indexData.push_back(i + j * vertRowSize + vertRowSize);

				indexData.push_back(i + j * vertRowSize + 1);
				indexData.push_back(i + j * vertRowSize + 1 + vertRowSize);
				indexData.push_back(i + j * vertRowSize + vertRowSize);
			}
		}

		CHECKED_GL_CALL(glGenVertexArrays(1, &GroundVertexArray));
		CHECKED_GL_CALL(glBindVertexArray(GroundVertexArray));

		GLuint GroundPositionBuffer, GroundIndexBuffer;

		CHECKED_GL_CALL(glGenBuffers(1, &GroundPositionBuffer));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, GroundPositionBuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexData.size(), vertexData.data(), GL_STATIC_DRAW));
		CHECKED_GL_CALL(glEnableVertexAttribArray(0));
		CHECKED_GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));

		GroundIndexCount = (int) indexData.size();
		CHECKED_GL_CALL(glGenBuffers(1, &GroundIndexBuffer));
		CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GroundIndexBuffer));
		CHECKED_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexData.size(), indexData.data(), GL_STATIC_DRAW));

		CHECKED_GL_CALL(glBindVertexArray(0));
	}



	///////////
	// Setup //
	///////////

	void init(string const & RESOURCE_DIR)
	{
		GLSL::checkVersion();

		glfwGetFramebufferSize(windowManager->getHandle(), &g_width, &g_height);
		CHECKED_GL_CALL(glViewport(0, 0, g_width, g_height));

		// Set background color
		CHECKED_GL_CALL(glClearColor(0.2f, 0.2f, 0.3f, 1.0f));
		// Enable z-buffer test
		CHECKED_GL_CALL(glEnable(GL_DEPTH_TEST));

		// Initialize mesh
		cube = make_shared<Shape>();
		cube->loadMesh(RESOURCE_DIR + "cube.obj");
		cube->resize();
		cube->init();

		sphere = make_shared<Shape>();
		sphere->loadMesh(RESOURCE_DIR + "sphere.obj");
		sphere->resize();
		sphere->init();

		plus = make_shared<Shape>();
		plus->loadMesh(RESOURCE_DIR + "plus.obj");
		plus->resize();
		plus->init();

		cylinder = make_shared<Shape>();
		cylinder->loadMesh(RESOURCE_DIR + "cylinder.obj");
		cylinder->resize();
		cylinder->init();

		// Initialize the GLSL programs

		BlinnPhongProg = make_shared<Program>();
		BlinnPhongProg->setVerbose(true);
		BlinnPhongProg->setShaderNames(RESOURCE_DIR + "blinnphong_vert.glsl", RESOURCE_DIR + "blinnphong_frag.glsl");
		if (! BlinnPhongProg->init())
		{
			exit(1);
		}

		BlinnPhongProg->addUniform("P");
		BlinnPhongProg->addUniform("V");
		BlinnPhongProg->addUniform("M");
		BlinnPhongProg->addUniform("uColor");
		BlinnPhongProg->addUniform("uLightPos");
		BlinnPhongProg->addUniform("uCameraPos");
		BlinnPhongProg->addAttribute("vertPos");
		BlinnPhongProg->addAttribute("vertNor");

		TerrainProg = make_shared<Program>();
		TerrainProg->setVerbose(true);
		TerrainProg->setShaderNames(RESOURCE_DIR + "terrain_vert.glsl", RESOURCE_DIR + "terrain_geom.glsl", RESOURCE_DIR + "terrain_frag.glsl");
		if (! TerrainProg->init())
		{
			exit(1);
		}

		TerrainProg->addUniform("P");
		TerrainProg->addUniform("V");
		TerrainProg->addUniform("M");
		TerrainProg->addUniform("uColor");
		TerrainProg->addUniform("uLightPos");
		TerrainProg->addUniform("uCameraPos");
		TerrainProg->addAttribute("vertPos");
	}



	////////////////
	// Transforms //
	////////////////

	mat4 SetProjectionMatrix(shared_ptr<Program> curShade)
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float) height;

		mat4 Projection = perspective(radians(50.0f), aspect, 0.1f, 200.0f);
		
		if (curShade)
		{
			CHECKED_GL_CALL(glUniformMatrix4fv(curShade->getUniform("P"), 1, GL_FALSE, value_ptr(Projection)));
		}
		return Projection;
	}

	mat4 SetView(shared_ptr<Program> curShade)
	{
		mat4 Cam = glm::lookAt(cameraPos, cameraLookAt, vec3(0, 1, 0));
		if (curShade)
		{
			CHECKED_GL_CALL(glUniformMatrix4fv(curShade->getUniform("V"), 1, GL_FALSE, value_ptr(Cam)));
		}
		return Cam;
	}

	void SetModel(vec3 const & trans, float rotY, float sc, shared_ptr<Program> curS)
	{
		mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
		mat4 Rot = glm::rotate(glm::mat4(1.0f), rotY, vec3(0, 1, 0));
		mat4 Scale = glm::scale(glm::mat4(1.0f), vec3(sc));
		mat4 ctm = Trans * Rot * Scale;
		CHECKED_GL_CALL(glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm)));
	}

	void SetModel(glm::mat4 const & transform, shared_ptr<Program> curS)
	{
		CHECKED_GL_CALL(glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(transform)));
	}



	////////////
	// Render //
	////////////

	// Draw the dog, sphere, dragon, and stairs and ground plane
	void DrawScene()
	{
		BlinnPhongProg->bind();

		SetProjectionMatrix(BlinnPhongProg);
		SetView(BlinnPhongProg);

		CHECKED_GL_CALL(glUniform3f(BlinnPhongProg->getUniform("uCameraPos"), cameraPos.x, cameraPos.y, cameraPos.z));
		CHECKED_GL_CALL(glUniform3f(BlinnPhongProg->getUniform("uLightPos"), g_light.x, g_light.y, g_light.z));

		// draw the cube mesh
		SetModel(vec3(-3, 0, -6), 0, 1, BlinnPhongProg);
		CHECKED_GL_CALL(glUniform3f(BlinnPhongProg->getUniform("uColor"), 0.8f, 0.2f, 0.2f));
		cube->draw(BlinnPhongProg);

		// draw the sphere mesh
		SetModel(vec3(-6, 0, -6), 0, 1, BlinnPhongProg);
		CHECKED_GL_CALL(glUniform3f(BlinnPhongProg->getUniform("uColor"), 0.2f, 0.2f, 0.8f));
		sphere->draw(BlinnPhongProg);

		// origin
		SetModel(vec3(0, 0, 0), glm::radians(45.f), 0.125f, BlinnPhongProg);
		CHECKED_GL_CALL(glUniform3f(BlinnPhongProg->getUniform("uColor"), 0.8f, 0.8f, 0.2f));
		plus->draw(BlinnPhongProg);

		BlinnPhongProg->unbind();


		TerrainProg->bind();

		SetProjectionMatrix(TerrainProg);
		SetView(TerrainProg);
		CHECKED_GL_CALL(glUniform3f(TerrainProg->getUniform("uCameraPos"), cameraPos.x, cameraPos.y, cameraPos.z));
		CHECKED_GL_CALL(glUniform3f(TerrainProg->getUniform("uLightPos"), g_light.x, g_light.y, g_light.z));

		// draw the ground plane
		SetModel(vec3(0, 0, 0), 0, 0.5f, TerrainProg);
		CHECKED_GL_CALL(glUniform3f(TerrainProg->getUniform("uColor"), 0.8f, 0.8f, 0.8f));
		CHECKED_GL_CALL(glBindVertexArray(GroundVertexArray));
		CHECKED_GL_CALL(glDrawElements(GL_TRIANGLES, GroundIndexCount, GL_UNSIGNED_INT, 0));
		CHECKED_GL_CALL(glBindVertexArray(0));

		TerrainProg->unbind();
	}

	void UpdateCamera(float const dT)
	{
		glm::vec3 up = glm::vec3(0, 1, 0);
		glm::vec3 forward = glm::vec3(cos(cTheta) * cos(cPhi), sin(cPhi), sin(cTheta) * cos(cPhi));
		glm::vec3 right = glm::normalize(glm::cross(forward, up));

		if (moveForward)
			cameraPos += forward * cameraMoveSpeed * dT;
		if (moveBack)
			cameraPos -= forward * cameraMoveSpeed * dT;
		if (moveLeft)
			cameraPos -= right * cameraMoveSpeed * dT;
		if (moveRight)
			cameraPos += right * cameraMoveSpeed * dT;

		cameraLookAt = cameraPos + forward;
	}
	
	void render()
	{
		float t1 = (float) glfwGetTime();

		float const dT = (t1 - t0);
		t0 = t1;

		UpdateCamera(dT);

		CHECKED_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		DrawScene();
	}

};

int main(int argc, char **argv)
{
	std::string resourceDir = "../resources/";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	WindowManager *windowManager = new WindowManager();
	windowManager->init(1024, 768);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	application->init(resourceDir);
	application->initGeom();

	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		application->render();
		glfwSwapBuffers(windowManager->getHandle());
		glfwPollEvents();
	}

	windowManager->shutdown();
	return 0;
}
