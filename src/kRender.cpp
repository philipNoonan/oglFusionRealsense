#include "kRender.h"

kRender::~kRender()
{
}

void kRender::GLFWCallbackWrapper::MousePositionCallback(GLFWwindow* window, double positionX, double positionY)
{
	s_application->MousePositionCallback(window, positionX, positionY);
}

void kRender::GLFWCallbackWrapper::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	s_application->MouseButtonCallback(window, button, action, mods);
}


void kRender::GLFWCallbackWrapper::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	s_application->KeyboardCallback(window, key, scancode, action, mods);
}

void kRender::GLFWCallbackWrapper::SetApplication(kRender* application)
{
	GLFWCallbackWrapper::s_application = application;
}

kRender* kRender::GLFWCallbackWrapper::s_application = nullptr;

void kRender::MousePositionCallback(GLFWwindow* window, double positionX, double positionY)
{
	//...
	//std::cout << "mouser" << std::endl;
	m_mouse_pos_x = positionX;
	m_mouse_pos_y = positionY;
}
void kRender::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	float zDist = 1500.0f;
	float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...

																		// the height of the screen at the distance of the image is 2 * halfheight
																		// to go from the middle to the top 

																		//m_model_depth = glm::translate(glm::mat4(1.0f), glm::vec3(-halfWidthAtDistance, halfHeightAtDist - m_depth_height, -zDist));

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && m_selectInitialPoseFlag == true)
	{
		//if (m_mouse_pos_x > m_display2DPos.x && m_mouse_pos_x < m_display2DPos.x + m_display2DSize.x && m_mouse_pos_y < m_display2DPos.y + m_display2DSize.y && m_mouse_pos_y > 0) // this is the hight and width of the render window, this is a bug
		//{
		//	m_center_pixX = m_mouse_pos_x - m_display2DPos.x;
		//	m_center_pixY = m_mouse_pos_y - m_display2DPos.y + m_display2DSize.y;

		///*	std::cout << std::endl;
		//	std::cout << "cente pix x: " << m_center_pixX  << " y: " << m_center_pixY << std::endl;
		//	std::cout << "mouse pos x: " << m_mouse_pos_x << " y: " << m_mouse_pos_y << std::endl;
		//	std::cout << "displ pos x: " << m_display2DPos.x << " y: " << m_display2DPos.y << std::endl;
		//	std::cout << "displ siz x: " << m_display2DSize.x << " y: " << m_display2DSize.y << std::endl;*/

		//	// get depth value, from texture buffer or float array???

		//	//// need to get depth pixel of this point
		//	//float x = (pixX - m_cameraParams.z) * (1.0f / m_cameraParams.x) * depth.x;
		//	//float y = (pixY - m_cameraParams.w) * (1.0f / m_cameraParams.y) * depth.x;
		//	//float z = depth.x;

		//	//m_cameraParams.x
		//}
	}


	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		// m_depthPixelPoints2D.push_back(std::make_pair(m_mouse_pos_x, m_mouse_pos_y));
		// get correct current offset and scakle for the window
		int depth_pos_x = m_mouse_pos_x / m_render_scale_width;
		int depth_pos_y = m_mouse_pos_y / m_render_scale_height;

		//std::cout <<" x: " << m_mouse_pos_x << " y: " << m_mouse_pos_y << " xS: " << m_render_scale_width << " yS: " << m_render_scale_height << std::endl;
		//std::cout << ((float)h / 424.0f) * m_mouse_pos_y << std::endl;


		if (depth_pos_x < m_depth_width && depth_pos_y < m_depth_height)
		{
			m_depthPixelPoints2D.push_back(std::make_pair(depth_pos_x, depth_pos_y));
			m_depthPointsFromBuffer.resize(m_depthPixelPoints2D.size() * 4); // for 4 floats per vertex (x,y,z, + padding)


		}
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		if (m_depthPixelPoints2D.size() > 0)
		{
			m_depthPixelPoints2D.pop_back();
			m_depthPointsFromBuffer.resize(m_depthPixelPoints2D.size() * 4); // for 4 floats per vertex (x,y,z, + padding)

		}
		// pop_back entry on vector
	}

	if (m_depthPixelPoints2D.size() > 0 && action == GLFW_PRESS)
	{
		//std::cout << m_depthPixelPoints2D.size();
		for (auto i : m_depthPixelPoints2D)
		{
			//std::cout << " x: " << i.first << " y: " << i.second << std::endl;
		}
	}
	else if (m_depthPixelPoints2D.size() == 0 && action == GLFW_PRESS)
	{
		//std::cout << "no entries yet, left click points on depth image" << std::endl;
	}
	//std::cout << "mouse button pressed: " << button << " " << action << " x: " <<  m_mouse_pos_x << " y: " << m_mouse_pos_y << std::endl;

}

void kRender::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//...
	//std::cout << "keyer" << std::endl;
	if (key == GLFW_KEY_H && action == GLFW_PRESS)
		m_show_imgui = !m_show_imgui;
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		m_registration_matrix[3][0] -= 5.0f;
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		m_registration_matrix[3][0] += 5.0f;
	if (key == GLFW_KEY_PAGE_UP && action == GLFW_PRESS)
		m_registration_matrix[3][1] -= 5.0f;
	if (key == GLFW_KEY_PAGE_DOWN && action == GLFW_PRESS)
		m_registration_matrix[3][1] += 5.0f;
	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		m_registration_matrix[3][2] -= 5.0f;
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		m_registration_matrix[3][2] += 5.0f;

	//std::cout << "manual x " << m_registration_matrix[3][0] << " manual y " << m_registration_matrix[3][1] << " manual z " << m_registration_matrix[3][2] << std::endl;
}

void kRender::SetCallbackFunctions()
{
	GLFWCallbackWrapper::SetApplication(this);
	glfwSetCursorPosCallback(m_window, GLFWCallbackWrapper::MousePositionCallback);
	glfwSetKeyCallback(m_window, GLFWCallbackWrapper::KeyboardCallback);
	glfwSetMouseButtonCallback(m_window, GLFWCallbackWrapper::MouseButtonCallback);
}

GLFWwindow * kRender::loadGLFWWindow()
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_REFRESH_RATE, 30);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	m_window = glfwCreateWindow(m_screen_width, m_screen_height, "oglfusion", nullptr, nullptr);

	if (m_window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		//return -1;
	}

	glfwMakeContextCurrent(m_window);
	//glfwSwapInterval(1); // Enable vsync
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		//return -1;
	}


	return m_window;
}

void kRender::requestShaderInfo()
{
	renderProg.printActiveUniforms();
}

void kRender::compileAndLinkShader()
{
	try {
		renderProg.compileShader("shaders/vertShader.vs");
		renderProg.compileShader("shaders/fragShader.fs");
		renderProg.link();


	}
	catch (GLSLProgramException &e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

void kRender::setLocations()
{
	m_ProjectionID = glGetUniformLocation(renderProg.getHandle(), "projection");
	m_MvpID = glGetUniformLocation(renderProg.getHandle(), "MVP");
	m_ModelID = glGetUniformLocation(renderProg.getHandle(), "model");
	m_ViewProjectionID = glGetUniformLocation(renderProg.getHandle(), "ViewProjection");
	m_sliceID = glGetUniformLocation(renderProg.getHandle(), "slice");
	m_imSizeID = glGetUniformLocation(renderProg.getHandle(), "imSize");
	m_depthScaleID = glGetUniformLocation(renderProg.getHandle(), "depthScale");
	m_depthRangeID = glGetUniformLocation(renderProg.getHandle(), "depthRange");
	m_cameraDeviceID = glGetUniformLocation(renderProg.getHandle(), "cameraDevice");

	m_renderOptionsID = glGetUniformLocation(renderProg.getHandle(), "renderOptions");

	m_getPositionSubroutineID = glGetSubroutineUniformLocation(renderProg.getHandle(), GL_VERTEX_SHADER, "getPositionSubroutine");
	m_fromTextureID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromTexture");
	m_fromPosition4DID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromPosition4D");
	m_fromPosition2DID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromPosition2D");
	m_fromPosePoints2DID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromPosePoints2D");

	m_fromStandardTextureID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromStandardTexture");
	m_fromMarkersVerticesID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromMarkersVertices");

	m_colorSelectionRoutineID = glGetSubroutineUniformLocation(renderProg.getHandle(), GL_FRAGMENT_SHADER, "getColorSelection");
	m_fromDepthID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromDepth");
	m_fromColorID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromColor");
	m_fromRayNormID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromRayNorm");
	m_fromRayVertID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromRayVert");
	m_fromPointsID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromPoints");
	m_fromVolumeID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromVolume");
	m_fromVolumeSDFID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromVolumeSDF");
	m_fromTrackID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromTrack");
	m_fromMarkersID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromMarkers");
	m_fromStandardFragmentID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromStandardFragment");



	//m_ambientID = glGetUniformLocation(renderProg.getHandle(), "ambient");
	//m_lightID = glGetUniformLocation(renderProg.getHandle(), "light");

	//m_irLowID = glGetUniformLocation(renderProg.getHandle(), "irLow");
	//m_irHighID = glGetUniformLocation(renderProg.getHandle(), "irHigh");

	//m_testTextureFragOut = GLHelper::createTexture(m_testTextureFragOut, GL_TEXTURE_3D, 1, 50, 50, 50, GL_RGBA32F);
}

void kRender::updateVerts(float w, float h)
{
//std::vector<float> vertices = {
//	// Positions		// Texture coords
//	w / 2.0f, h / 2.0f, 0.0f, 1.0f, 1.0f, // top right
//	w / 2.0f, -h / 2.0f, 0.0f, 1.0f, 0.0f, // bottom right
//	-w / 2.0f, -h / 2.0f, 0.0f, 0.0f, 0.0f, // bottom left
//	-w / 2.0f, h / 2.0f, 0.0f, 0.0f, 1.0f  // Top left
//};
//
//m_standard_verts = vertices;
//
//glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Standard);
//glBufferSubData(GL_ARRAY_BUFFER, 0, m_standard_verts.size() * sizeof(float), m_standard_verts.data());

}

void kRender::setVertPositions()
{
	std::vector<float> vertices = {
		// Positions				// Texture coords
		1.0f,	1.0f,	0.0f,		1.0f, 1.0f, // top right
		1.0f,	-1.0f,	0.0f,		1.0f, 0.0f, // bottom right
		-1.0f,	-1.0f,	0.0f,		0.0f, 0.0f, // bottom left
		-1.0f,	1.0f,	0.0f,		0.0f, 1.0f  // Top left
	};

	m_standard_verts = vertices;

	std::vector<float> verticesColor = {
		// Positions				// Texture coords
		1920,	1080,	0.0f,		1.0f, 1.0f, // top right
		1920,	0,		0.0f,		1.0f, 0.0f, // bottom right
		0,		0,		0.0f,		0.0f, 0.0f, // bottom left
		0,		1080.0,	0.0f,		0.0f, 1.0f  // Top left
	};
	m_color_vert = verticesColor;

	std::vector<float> verticesDepth = {
		// Positions					// Texture coords
		848.0f,  480.0f,	0.0f,		1.0f, 1.0f, // top right
		848.0f,	 0,			0.0f,		1.0f, 0.0f, // bottom right
		0,		 0,			0.0f,		0.0f, 0.0f, // bottom left
		0,		 480.0f,	0.0f,		0.0f, 1.0f  // Top left 
	};
	m_depth_vert = verticesDepth;

	m_vertices = vertices;

	std::vector<unsigned int>  indices = {  // Note that we start from 0!
		0, 1, 3, // First Triangle
		1, 2, 3  // Second Triangle
	};

	m_indices = indices;

	m_trackedPoints.resize(1000 * 1000 * 2);

	for (int i = 0; i < 2000; i += 2)
	{
		for (int j = 0; j < 1000; j++)
		{
			m_trackedPoints[j * 2000 + i] = (1920 >> 1) - 500 + (i / 2) * 10;
			m_trackedPoints[j * 2000 + i + 1] = (1080 >> 1) - 500 + j * 10;

		}
	}



}

void kRender::allocateTextures()
{
	int patch_size = 8;
	int numLevels = (int)(log((2 * m_color_width) / (4.0 * patch_size)) / log(2.0) + 0.5) + 1; ;// 1 + floor(std::log2(std::max(m_color_width, m_color_height)));
	m_textureColor = GLHelper::createTexture(m_textureColor, GL_TEXTURE_2D, numLevels, m_color_width, m_color_height, 0, GL_RGB8, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);
	m_textureInfra = GLHelper::createTexture(m_textureInfra, GL_TEXTURE_2D, numLevels, m_depth_width, m_depth_height, 0, GL_R8, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);

}

void kRender::setColorFrame(std::vector<uint16_t> imageArray)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureColor);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_color_width, m_color_height, GL_RGBA, GL_UNSIGNED_BYTE, imageArray.data());
	glGenerateMipmap(GL_TEXTURE_2D);
}

void kRender::setColorFrame(std::vector<rs2::frame_queue> colorQ, int devNumber, cv::Mat &colorMat)
{
	rs2::frame colorFrame;

	if (colorQ[devNumber].poll_for_frame(&colorFrame))
	{

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureColor);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_color_width, m_color_height, GL_RGB, GL_UNSIGNED_BYTE, colorFrame.get_data());
		glGenerateMipmap(GL_TEXTURE_2D);

		if (colorFrame != NULL)
		{
			colorMat = cv::Mat(m_color_height, m_color_width, CV_8UC3, (void*)colorFrame.get_data());

			//cv::imshow("col", colorMat);
			//cv::waitKey(1);


		}

	}


	
}

void kRender::setInfraFrame(std::vector<rs2::frame_queue> infraQ, int devNumber, cv::Mat &infraMat)
{
	rs2::frame infraFrame;

	if (infraQ[devNumber].poll_for_frame(&infraFrame))
	{

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureInfra);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_depth_width, m_depth_height, GL_RED, GL_UNSIGNED_BYTE, infraFrame.get_data());
		glGenerateMipmap(GL_TEXTURE_2D);

		if (infraFrame != NULL)
		{
			infraMat = cv::Mat(m_depth_height, m_depth_width, CV_8UC1, (void*)infraFrame.get_data());

			//cv::imshow("ir1", infraMat);
			//cv::waitKey(1);


		}

	}



}

void kRender::allocateBuffers()
{
	//glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO_Standard);
	glGenBuffers(1, &m_VBO_Color);
	glGenBuffers(1, &m_VBO_Depth);
	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);
	//glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Color);
	//glBufferData(GL_ARRAY_BUFFER, m_color_vert.size() * sizeof(float), &m_color_vert[0], GL_STATIC_DRAW);
	//// EBO
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);
	//// Position attribute for Color
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
	//glEnableVertexAttribArray(0);
	//// TexCoord attribute for Color
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);

	//// now go for depth
	//glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Depth);
	//glBufferData(GL_ARRAY_BUFFER, m_depth_vert.size() * sizeof(float), &m_depth_vert[0], GL_STATIC_DRAW);
	//// EBO
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);
	//// Position attribute for Depth
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
	//glEnableVertexAttribArray(2);
	////// TexCoord attribute for Depth
	//glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(3);

	// standard verts
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Standard);
	glBufferData(GL_ARRAY_BUFFER, m_standard_verts.size() * sizeof(float), &m_standard_verts[0], GL_DYNAMIC_DRAW);
	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_DYNAMIC_DRAW);
	// Position attribute for Depth
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	//// TexCoord attribute for Depth
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);


	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_posBufMC);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, 128*128*128*4 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &m_VAO_MC);
	glGenBuffers(1, &m_bufferTrackedPoints);

	glBindVertexArray(m_VAO_MC);

	glBindBuffer(GL_ARRAY_BUFFER, m_bufferTrackedPoints);
	glBufferData(GL_ARRAY_BUFFER, 100 * 100 * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, 0, 0); // 2  floats per vertex, x,y
	glEnableVertexAttribArray(7);

	glBindVertexArray(0);

	//glBindBuffer(GL_ARRAY_BUFFER, m_posBufMC);
	//glBufferData(GL_ARRAY_BUFFER, 128 * 128 * 128 * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	//glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 0, 0); // 4  floats per vertex, x,y,z and 1 for padding? this is annoying...
	//glEnableVertexAttribArray(6);

	//glBindVertexArray(0);


	//glGenBuffers(1, &m_bufferTrackedPoints);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferTrackedPoints);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_bufferTrackedPoints);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, m_trackedPoints.size() * sizeof(float), m_trackedPoints.data(), GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &m_VAO_Markers);
	glBindVertexArray(m_VAO_Markers);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Standard);
	glBufferData(GL_ARRAY_BUFFER, m_standard_verts.size() * sizeof(float), &m_standard_verts[0], GL_DYNAMIC_DRAW);
	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_DYNAMIC_DRAW);
	// Position attribute for Depth
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	//// TexCoord attribute for Depth
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


		// ALLOCATE buffer for 256 markers, which is probably enough
	//m_tMat.resize(256, glm::mat4(1.0f));
	glGenBuffers(1, &m_VBO_Markers);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Markers);
	glBufferData(GL_ARRAY_BUFFER, 256 * sizeof(glm::mat4), glm::value_ptr(m_tMat[0]), GL_DYNAMIC_DRAW);

	// We're no longer using a model uniform variable, but instead declare a mat4 as a vertex attribute so we can store an 
	// instanced array of transformation matrices. However, when we declare a datatype as a vertex attribute that is greater
	// than a vec4 things work a bit differently. The maximum amount of data allowed as a vertex attribute is equal to a vec4. 
	// Because a mat4 is basically 4 vec4s, we have to reserve 4 vertex attributes for this specific matrix. Because we 
	// assigned it a location of 3, the columns of the matrix will have vertex attribute locations of 3, 4, 5 and 6. 

	GLsizei vec4Size = sizeof(glm::vec4);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);

	glBindVertexArray(0);





	std::vector<float> bodyPosePoints(21 * 3, 0.75);

	std::vector<uint32_t> bodyPosePairs = { 1, 8, 1, 2, 1,
								5, 2, 3, 3, 4,
								5, 6, 6, 7, 8,
								9, 9, 10, 10, 11,
								8, 12, 12, 13, 13,
								14, 1, 0, 0, 15,
								15, 17, 0, 16, 16,
								18, 1, 19, 19, 20,
								2, 9, 5, 12 };

	glGenVertexArrays(1, &m_poseVAO);
	glGenBuffers(1, &m_poseVBO);
	glGenBuffers(1, &m_poseEBO);

	glBindVertexArray(m_poseVAO);

	//// standard verts
	glBindBuffer(GL_ARRAY_BUFFER, m_poseVBO);
	glBufferData(GL_ARRAY_BUFFER, 21 * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	//// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_poseEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bodyPosePairs.size() * sizeof(unsigned int), &bodyPosePairs[0], GL_DYNAMIC_DRAW);
	//
	glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(9);

	glBindVertexArray(0);

}

void kRender::setWindowLayout()
{
	m_anchorMW = std::make_pair<int, int>(50, 50);


}

//void kRender::setComputeWindowPosition(int x, int y, int w, int h)
//{
//	//glViewport(m_anchorMW.first + m_depth_width * m_render_scale_width, m_anchorMW.second, m_depth_width * m_render_scale_width, m_depth_height * m_render_scale_height);
//	glViewport(x, y, w, h);
//}

//
//void kRender::setColorDepthMapping(int* colorDepthMap)
//{
//	// 2d array index is given by
//	// p.x = idx / size.x
//	// p.y = idx % size.x
//
//	//for m_colorDepthMapping[j + 1] = y color image axis, 1 at top
//	// m_colorDepthMapping[j] = x axis, 0 on the left, 
//
//	// MAP ME���
//	int j = 0;
//	for (int i = 0; i < (m_depth_width * m_depth_height); i++, j+=2)
//	{
//		int yCoord = colorDepthMap[i] / m_color_width;
//		int xCoord = colorDepthMap[i] % m_color_width;
//		m_colorDepthMapping[j] = ((float)xCoord) / (float)m_color_width;
//		m_colorDepthMapping[j + 1] = (1080.0f - (float)yCoord) / (float)m_color_height;
//
//
//
//	}
//
//
//
//	glBindBuffer(GL_ARRAY_BUFFER, m_buf_color_depth_map);
//	glBufferSubData(GL_ARRAY_BUFFER, 0, m_colorDepthMapping.size() * sizeof(float), m_colorDepthMapping.data());
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//	//// Other way to copy data to buffer, taken from https://learnopengl.com/#!Advanced-OpenGL/Advanced-Data
//	//glBindBuffer(GL_ARRAY_BUFFER, m_buf_color_depth_map);
//	//void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
//	//memcpy_s(ptr, m_colorDepthMapping.size() * sizeof(float), m_colorDepthMapping.data(), m_colorDepthMapping.size() * sizeof(float));
//	//glUnmapBuffer(GL_ARRAY_BUFFER);
//
//}


void kRender::setRenderingOptions(bool showDepthFlag, bool showBigDepthFlag, bool showInfraFlag, bool showColorFlag, bool showLightFlag, bool showPointFlag, bool showFlowFlag, bool showEdgesFlag, bool showNormalFlag, bool showVolumeFlag, bool showTrackFlag, bool showSDFlag, bool showMarkerFlag, bool showDepthSplatFlag, bool showNormalSplatFlag)
{
	m_showDepthFlag = showDepthFlag;
	m_showBigDepthFlag = showBigDepthFlag;
	m_showInfraFlag = showInfraFlag;
	m_showColorFlag = showColorFlag;
	m_showLightFlag = showLightFlag;
	m_showPointFlag = showPointFlag;
	m_showFlowFlag = showFlowFlag; 
	m_showEdgesFlag = showEdgesFlag;
	m_showNormalFlag = showNormalFlag;
	m_showVolumeFlag = showVolumeFlag; 
	m_showTrackFlag = showTrackFlag;
	m_showVolumeSDFFlag = showSDFlag;
	m_showMarkersFlag = showMarkerFlag;
	m_showSplatterDepth = showDepthSplatFlag;
	m_showSplatterNormal = showNormalSplatFlag;


					  
}

void kRender::setTextures(GLuint depthTex, GLuint colorTex, GLuint vertexTex, GLuint normalTex, GLuint volumeTex, GLuint trackTex, GLuint pvpNormTex, GLuint pvdNormTex, GLuint splatterDepthTex, GLuint splatterNormalTex)
{

	m_textureDepth = depthTex;
	//m_textureColor = colorTex;
	m_textureVertex = vertexTex; 
	m_textureNormal = normalTex;
	m_textureVolume = volumeTex;
	m_textureTrack = trackTex;
	m_texturePVPNormal = pvpNormTex;
	m_texturePVDNormal = pvdNormTex;
	m_textureSplatterDepth = splatterDepthTex;
	m_textureSplatterNormal = splatterNormalTex;

}
void kRender::setFlowTexture(GLuint flowTex)
{
	m_textureFlow = flowTex;
}

void kRender::bindTexturesForRendering()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	if (m_showDepthFlag)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureDepth);
	}

	if (m_showNormalFlag)
	{
		if (m_usePVP)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_texturePVPNormal);
		}
		else if (m_usePVD)
		{
			glActiveTexture(GL_TEXTURE1);
			//glBindTexture(GL_TEXTURE_2D_ARRAY, m_texturePVPNormal);

			glBindTexture(GL_TEXTURE_2D_ARRAY, m_texturePVDNormal);
		}
		else
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureNormal);
		}




	}
	


	if (m_showTrackFlag)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureTrack);

	}

	if (m_showColorFlag)
	{
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, m_textureColor);
	}
	if (m_showInfraFlag)
	{
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, m_textureInfra);
	}


	if (m_showFlowFlag)
	{
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_textureFlow);


	}


	if (m_showVolumeFlag)
	{
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_3D, m_textureVolume);
	}

	if (m_showVolumeSDFFlag)
	{

		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_3D, m_textureFlood);

	}

	if (m_showSplatterDepth)
	{
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, m_textureSplatterDepth);
	}

	if (m_showSplatterNormal)
	{
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, m_textureSplatterNormal);


	}

}

void kRender::setMarkerData(std::vector<glm::mat4> tMat)
{
	m_numMarkers = tMat.size();
	for (int i = 0; i < m_numMarkers; i++)
	{
		//glm::mat4 flipZ(1.0f);
		//flipZ[2][2] = -1.0f;
		//m_tMat[i] = tMat[i];
		//m_tDMat[i] = m_colorToDepth * tMat[i];
		m_tDMat[i] = tMat[i];
		m_tMat[i] = m_depthToColor * tMat[i];

	}
}

void kRender::setOtherMarkerData(std::vector<glm::mat4> tMat)
{
	m_numMarkersOther = tMat.size();
	for (int i = 0; i < m_numMarkers; i++)
	{
		m_tOtherDMat[i] =  glm::inverse(m_depthToDepth) * tMat[i];
	}
}

void kRender::bindBuffersForRendering()
{
	glBindVertexArray(m_VAO_MC);

	glBindBuffer(GL_ARRAY_BUFFER, m_bufferTrackedPoints);
	glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(7);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_posBufMC);
	//glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, 0); // 4  floats per vertex, x,y,z and 1 for padding? this is annoying...
	//glEnableVertexAttribArray(4);


}



void kRender::Render(bool useInfrared, int devNumber)
{
	// set positions
	// set uniforms
	// render textures
	bindTexturesForRendering();
	bindBuffersForRendering();
	//setDepthImageRenderPosition();
	setNormalImageRenderPosition();

	renderLiveVideoWindow(useInfrared, devNumber);





}




// set up the data buffers for rendering
void kRender::setBuffersForRendering(float * depthArray, float * bigDepthArray, float * infraArray, float * colorArray, unsigned char * flowPtr)
{
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_DEPTH_TEST);

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//if (depthArray != NULL)
	//{
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, m_textureDepth);
	//	glTexSubImage2D(GL_TEXTURE_2D, /*level*/0, /*xoffset*/0, /*yoffset*/0, m_depth_width, m_depth_height, GL_RED, GL_FLOAT, depthArray);
	//}


	//if (infraArray != NULL)
	//{
	//	glActiveTexture(GL_TEXTURE1);
	//	glBindTexture(GL_TEXTURE_2D, m_textureInfra);
	//	glTexSubImage2D(GL_TEXTURE_2D, /*level*/0, /*xoffset*/0, /*yoffset*/0, m_depth_width, m_depth_height, GL_RED, GL_FLOAT, infraArray);
	//}


	//if (m_showColorFlag || colorArray != NULL)
	//{
	//	glActiveTexture(GL_TEXTURE2);
	//	glBindTexture(GL_TEXTURE_2D, m_textureColor);
	//	if (colorArray != NULL)
	//	{
	//		glTexSubImage2D(GL_TEXTURE_2D, /*level*/0, /*xoffset*/0, /*yoffset*/0, m_color_width, m_color_height, GL_BGRA, GL_UNSIGNED_BYTE, colorArray);
	//	}
	//}


	//if (flowPtr != NULL)
	//{
	//	glActiveTexture(GL_TEXTURE3);
	//	glBindTexture(GL_TEXTURE_2D, m_textureFlow);
	//	glTexSubImage2D(GL_TEXTURE_2D, /*level*/0, /*xoffset*/0, /*yoffset*/0, m_depth_width, m_depth_height, GL_RG, GL_FLOAT, flowPtr);
	//}


	//if (flowPtr != NULL)
	//{
	//	glActiveTexture(GL_TEXTURE3); // what number should this be FIX ME
	//	glBindTexture(GL_TEXTURE_2D, m_textureFlow);
	//	glTexSubImage2D(GL_TEXTURE_2D, /*level*/0, /*xoffset*/0, /*yoffset*/0, m_depth_width, m_depth_height, GL_RG, GL_FLOAT, flowPtr);
	//}


	//if (bigDepthArray != NULL)
	//{
	//	glActiveTexture(GL_TEXTURE6);
	//	glBindTexture(GL_TEXTURE_2D, m_textureBigDepth);
	//	glTexSubImage2D(GL_TEXTURE_2D, /*level*/0, /*xoffset*/0, /*yoffset*/0, m_color_width, m_color_height + 2, GL_RED, GL_FLOAT, bigDepthArray);
	//}

	//if (m_showEdgesFlag)
	//{
	//	glActiveTexture(GL_TEXTURE7);
	//	glBindTexture(GL_TEXTURE_2D, m_textureEdges);
	//}


}

void kRender::setMarchingCubesRenderPosition(float modelZ)
{

	//float zDist = 1500.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	float zDist = 0000.0f + modelZ;
	float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...

	m_model_MC = glm::translate(glm::mat4(1.0f), glm::vec3(-500, -500, -zDist)); // not sure why we have to offset by 0.5m xy

	glm::mat4 flipYZ = glm::mat4(1);
	//flipYZ[0][0] = -1.0f;
	flipYZ[1][1] = -1.0f;
	flipYZ[2][2] = -1.0f;

	m_model_MC = flipYZ * m_model_MC;
}

void kRender::setVolumeSDFRenderPosition(float slice)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//float zDist = 1500.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	float zDist = m_volume_size.z;
	//zDist = ((float)128 * 6) / tan(20 * M_PI / 180.0f);
	//float halfHeightAtDist = (float)h * 4;
	//float halfWidthAtDistance = (float)w * 4;

	glm::vec3 scaleVec = glm::vec3(1.f, 1.f, 1.0f);

	m_model_volume = glm::scale(glm::mat4(1.0f), scaleVec);

	m_model_volume = glm::translate(m_model_volume, glm::vec3(-m_volume_size.x/2.0f, -m_volume_size.y/2.0f, -m_volume_size.z));

	m_volumeSDFRenderSlice = slice / m_volume_size.z;

}

void kRender::setDepthImageRenderPosition(float vertFov)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//float zDist = 1500.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	float zDist;
	zDist = ((float)m_depth_height * 6) / tan(vertFov * M_PI / 180.0f);
	//float halfHeightAtDist = (float)h * 4;
	//float halfWidthAtDistance = (float)w * 4;

	glm::vec3 scaleVec = glm::vec3(6.f, 6.f, 1.0f);

	m_model_depth = glm::scale(glm::mat4(1.0f), scaleVec);

	m_model_depth = glm::translate(m_model_depth, glm::vec3(-m_depth_width / 2, -m_depth_height / 2, -zDist));


}

void kRender::setRayNormImageRenderPosition(float vertFov)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//float zDist = 1500.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	float zDist;
	zDist = ((float)m_depth_height * 6) / tan(vertFov * M_PI / 180.0f);
	//float halfHeightAtDist = (float)h * 4;
	//float halfWidthAtDistance = (float)w * 4;

	glm::vec3 scaleVec = glm::vec3(6.f, 6.f, 1.0f);

	m_model_raynorm = glm::scale(glm::mat4(1.0f), scaleVec);

	m_model_raynorm = glm::translate(m_model_raynorm, glm::vec3(-m_depth_width / 2, -m_depth_height / 2, -zDist + 2));


}

void kRender::setTrackImageRenderPosition(float vertFov)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//float zDist = 1500.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	float zDist;
	zDist = ((float)m_depth_height * 6) / tan(vertFov * M_PI / 180.0f);
	//float halfHeightAtDist = (float)h * 4;
	//float halfWidthAtDistance = (float)w * 4;

	glm::vec3 scaleVec = glm::vec3(6.f, 6.f, 1.0f);

	m_model_track = glm::scale(glm::mat4(1.0f), scaleVec);

	m_model_track = glm::translate(m_model_track, glm::vec3(-m_depth_width / 2, -m_depth_height / 2, -zDist));
}

void kRender::setNormalImageRenderPosition()
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//float zDist = 1500.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	float zDist;
	zDist = ((float)h / 2) / tan(22.5 * M_PI / 180.0f);
	float halfHeightAtDist = (float)h / 2;
	float halfWidthAtDistance = (float)w / 2;
	m_model_normal = glm::translate(glm::mat4(1.0f), glm::vec3(-m_depth_width / 2, -m_depth_height / 2, -zDist + 5));


}

void kRender::setColorImageRenderPosition(float vertFov)
{


	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);

	float zDist;
	zDist = ((float)m_color_height * 6) / tan(vertFov * M_PI / 180.0f);
	float halfHeightAtDist = (float)h * 4;
	float halfWidthAtDistance = (float)w * 4;

	glm::vec3 scaleVec = glm::vec3(6.f, 6.f, 1.0f);

	m_model_color = glm::scale(glm::mat4(1.0f), scaleVec);
	m_model_color = glm::translate(m_model_color, glm::vec3(-m_color_width / 2.0f, -m_color_height / 2.0f, -zDist));


}

void kRender::setInfraImageRenderPosition()
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	float zDist = 1500.0f;
	float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	m_model_infra = glm::translate(glm::mat4(1.0f), glm::vec3(-halfWidthAtDistance, -halfHeightAtDist, -zDist));


}

void kRender::setFlowImageRenderPosition(float vertFov)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//float zDist = 1500.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	float zDist;
	zDist = ((float)m_color_height * 6) / tan(vertFov * M_PI / 180.0f);
	//float halfHeightAtDist = (float)h * 4;
	//float halfWidthAtDistance = (float)w * 4;

	glm::vec3 scaleVec = glm::vec3(6.f, 6.f, 1.0f);

	m_model_flow = glm::scale(glm::mat4(1.0f), scaleVec);

	m_model_flow = glm::translate(m_model_flow, glm::vec3(-m_color_width / 2, -m_color_height / 2, -zDist + 7));



	}

void kRender::setPointCloudRenderPosition(float modelZ)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	float zDist = 0000.0f + modelZ;
	float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	m_model_pointcloud = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, zDist));

	glm::mat4 flipYZ = glm::mat4(1);
	//flipYZ[0][0] = -1.0f;
	flipYZ[1][1] = -1.0f;
	flipYZ[2][2] = -1.0f;

	m_model_pointcloud = m_registration_matrix * flipYZ * m_model_pointcloud;

}

void kRender::setLightModelRenderPosition()
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//float zDist = 3000.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...

	float zDist;
	zDist = ((float)h * 4) / tan(30.5f * M_PI / 180.0f);
	float halfHeightAtDist = (float)h * 4;
	float halfWidthAtDistance = (float)w * 4;
	//m_model_color = glm::translate(glm::mat4(1.0f), glm::vec3(-m_color_width / 2.0f, -halfHeightAtDist, -zDist));

	if (m_showBigDepthFlag)
	{
		glm::vec3 scaleVec = glm::vec3(4.f, 4.f, 1.0f);

		m_model_lightmodel = glm::scale(glm::mat4(1.0f), scaleVec);
		m_model_lightmodel = glm::translate(m_model_lightmodel, glm::vec3(-m_color_width / 2.0f, -m_color_height / 2.0f, -zDist + 10.0f));

	}
	else
	{
		m_model_lightmodel = glm::translate(glm::mat4(1.0f), glm::vec3(halfWidthAtDistance - m_depth_width, -halfHeightAtDist, -zDist));

	}

}


void kRender::setViewMatrix(float xRot, float yRot, float zRot, float xTran, float yTran, float zTran)
{
	glm::mat4 t0, t1, r0;
	m_view = glm::mat4(1.0f);

	t0 = glm::translate(glm::mat4(1.0), glm::vec3(xTran, yTran, zTran));
	t1 = glm::translate(glm::mat4(1.0), glm::vec3(-xTran, -yTran, -zTran));

	r0 = glm::eulerAngleXYZ(glm::radians(xRot), glm::radians(yRot), glm::radians(zRot));


m_view = t1 * r0 * t0;
//m_view = glm::translate(m_view, glm::vec3(0.0f, 0.0f, 0.0f));

}

void kRender::setProjectionMatrix(int camDevice)
{



	float skew = 1.0f;
	if (m_cameraParams.size() > 0)
	{
		GLHelper::projectionFromIntrinsics(m_projection, m_cameraParams[camDevice].x, m_cameraParams[camDevice].y, skew, m_cameraParams[camDevice].z, m_cameraParams[camDevice].w, m_depth_width, m_depth_height, 0.01, 100.0);

	}

	//glm::mat4 testProj = glm::perspective(glm::radians(45.0f), 848.0f / 480.0f, 0.01f, 100.0f);

	if (m_cameraParams_color.size() > 0)
	{
		GLHelper::projectionFromIntrinsics(m_projectionColor, m_cameraParams_color[camDevice].x, m_cameraParams_color[camDevice].y, skew, m_cameraParams_color[camDevice].z, m_cameraParams_color[camDevice].w, m_color_width, m_color_height, 0.01, 100.0);
	}
}

void kRender::setDepthTextureProjectionMatrix()
{

	// use the actual cameras vertical fov not a guess!
	//m_projection = glm::perspective(glm::radians(59.1741f), (float)m_depth_width / (float)m_depth_height, 0.1f, 10.0f); // scaling the texture to the current window size seems to work
	//m_projectionColor = glm::perspective(glm::radians(42.7669f), (float)m_color_width / (float)m_color_height, 0.1f, 10.0f); // scaling the texture to the current window size seems to work
	
	// use actual camera intrinsics not just glm::perspective

}

void kRender::setViewport(int x, int y, int w, int h)
{
	glViewport(x, y, w, h);
}

int kRender::getRenderOptions(bool depth, bool infra, bool color, bool norms, bool track, bool flood, bool volume, bool splatterDepth, bool splatterNormal)
{
	int opts = depth << 0 |
		       infra << 1 |
			   color << 2 |
		       norms << 3 |
		       track << 4 |
		       flood << 5 |
			  volume << 6 |
	   splatterDepth << 7 |
	   splatterNormal << 8;

	return opts;
}

void kRender::renderLiveVideoWindow(bool useInfrared, int devNumber)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	renderProg.use();
	glm::mat4 MVP;




	//// RENDER LEFT WINDOW
	int leftRenderOptions = getRenderOptions(m_showDepthFlag, m_showInfraFlag, 0, m_showNormalFlag, m_showTrackFlag, 0, 0, m_showSplatterDepth, m_showSplatterNormal);


	glm::vec2 imageSize(848.0f, 480.0f);
	setViewport(m_display2DPos.x, m_display2DPos.y, m_display2DSize.x, m_display2DSize.y);
	glBindVertexArray(m_VAO);
	//MVP = m_projection * m_view * m_model_depth;
	glm::vec2 depthRange(m_depthMin, m_depthMax);
	glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));
	glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
	glUniform1ui(m_renderOptionsID, leftRenderOptions);
	glUniform1i(m_cameraDeviceID, devNumber);
	glUniform2fv(m_depthRangeID, 1, glm::value_ptr(depthRange));
	glUniform1f(m_depthScaleID, 100.0f / 1000000.0f); // 1000 == each depth unit == 1 mm
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromStandardFragmentID);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	//// RENDER RIGHT WINDOW
	int rightRenderOptions = getRenderOptions(0, 0, m_showColorFlag, 0, 0, m_showVolumeSDFFlag, m_showVolumeFlag, 0, 0);


	setViewport(m_display3DPos.x, m_display3DPos.y, m_display3DSize.x, m_display3DSize.y);
	glBindVertexArray(m_VAO);
	imageSize = glm::vec2(1920.0f, 1080.0f);
	glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

	//MVP = m_projection * m_view * m_model_depth;
	glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
	glUniform1ui(m_renderOptionsID, rightRenderOptions);
	glUniform1f(m_sliceID, m_volumeSDFRenderSlice);
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromStandardFragmentID);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);








	if (m_showMarkersFlag)
	{


		{


			glBindVertexArray(m_VAO_Markers);
			setViewport(m_display3DPos.x, m_display3DPos.y, m_display3DSize.x, m_display3DSize.y);

			glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Markers);
			glBufferSubData(GL_ARRAY_BUFFER, 0, m_numMarkers * sizeof(glm::mat4), glm::value_ptr(m_tMat[0]));

			glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromMarkersVerticesID);
			glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromMarkersID);

			glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projectionColor));

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, m_numMarkers);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glBindVertexArray(0);

		}

		{
			glBindVertexArray(m_VAO_Markers);
			setViewport(m_display2DPos.x, m_display2DPos.y, m_display2DSize.x, m_display2DSize.y);

			glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Markers);
			glBufferSubData(GL_ARRAY_BUFFER, 0, m_numMarkers * sizeof(glm::mat4), glm::value_ptr(m_tDMat[0]));


			glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromMarkersVerticesID);
			glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromMarkersID);

			glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));

			// USEFUL SANITY CHECKING DO NOT DELETE
			//glm::vec4 prospace = m_projection * m_tDMat[0] * glm::vec4(0.0, 0.0, 0.0, 1.0);
			//std::cout << glm::to_string(m_tDMat[0] * glm::vec4(0.0, 0.0, 0.0, 1.0)) << " : : " << glm::to_string(prospace) << " : : : " << glm::to_string(glm::vec4(prospace.x / prospace.w, prospace.y / prospace.w, prospace.z / prospace.w, prospace.w / prospace.w)) << std::endl;

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, m_numMarkers); // each marker is 2 triangles

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			//glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
			//glDrawArrays(GL_POINTS, 0, m_numMarkers);
			glBindVertexArray(0);
		}

		{
			glBindVertexArray(m_VAO_Markers);
			setViewport(m_display3DPos.x, m_display3DPos.y, m_display3DSize.x, m_display3DSize.y);

			glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Markers);
			glBufferSubData(GL_ARRAY_BUFFER, 0, m_numMarkersOther * sizeof(glm::mat4), glm::value_ptr(m_tOtherDMat[0]));


			glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromMarkersVerticesID);
			glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromMarkersID);

			glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projectionColor));

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, m_numMarkersOther); // each marker is 2 triangles
			//glDrawArrays(GL_POINTS, 0, m_numMarkers);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glBindVertexArray(0);
		}



	}

	if (m_showPointFlag)
	{
		glBindVertexArray(m_poseVAO);
		//glBindVertexArray(m_VAO);
		glm::vec2 imageSize(848.0f, 480.0f);
		setViewport(m_display2DPos.x, m_display2DPos.y, m_display2DSize.x, m_display2DSize.y);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_poseEBO);
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromPosePoints2DID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromPointsID);

		//imageSize = glm::vec2(m_color_width, m_color_height);
		for (int person = 0; person < m_bodyPosePoints.size(); person++)
		{
			glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

			glBindBuffer(GL_ARRAY_BUFFER, m_poseVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, m_bodyPosePoints[person].size() * sizeof(float), m_bodyPosePoints[person].data());
			glDrawElements(GL_LINES, 44, GL_UNSIGNED_INT, 0);
			glDrawArrays(GL_POINTS, 0, m_bodyPosePoints[person].size() / 3);

		}



	}
	//if (m_showVolumeSDFFlag)
	//{
	//	// change verts for size of texture
	//	setViewport(m_display3DPos.x, m_display3DPos.y, m_display3DSize.x, m_display3DSize.y);

	//	//updateVerts(m_volume_size.x, m_volume_size.y);
	//	glm::vec2 imageSize(m_depth_width, m_depth_height);

	//	//glBindImageTexture(5, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16I);

	//	glBindVertexArray(m_VAO);
	//	MVP = m_projection * m_view * m_model_track;
	//	glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
	//	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromVolumeSDFID);
	//	glUniform1f(m_sliceID, m_volumeSDFRenderSlice);

	//	//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
	//	glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
	//	glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

	//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	//}

	//if (m_showVolumeFlag)
	//{
	//	// change verts for size of texture
	//	setViewport(m_display3DPos.x, m_display3DPos.y, m_display3DSize.x, m_display3DSize.y);

	//	//updateVerts(m_volume_size.x, m_volume_size.y);
	//	glm::vec2 imageSize(m_depth_width, m_depth_height);

	//	//glBindImageTexture(5, m_testTextureFragOut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	//	glBindVertexArray(m_VAO);
	//	MVP = m_projection * m_view * m_model_track;
	//	glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
	//	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromVolumeID);
	//	glUniform1f(m_sliceID, m_volumeSDFRenderSlice);

	//	//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
	//	glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
	//	glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

	//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	//
	//}

	////if (m_showFloodFlag)
	////{
	////	glm::vec2 imageSize(m_volume_size.x, m_volume_size.y);

	////	glBindImageTexture(5, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16I);

	////	glBindVertexArray(m_VAO);
	////	MVP = m_projection * m_view * m_model_volume;
	////	glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
	////	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromVolumeID);
	////	glUniform1f(m_sliceID, m_volumeSDFRenderSlice);

	////	//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
	////	glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
	////	glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

	////	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	////}



	//if (m_showFlowFlag)
	//{
	//	setViewport(m_display2DPos.x, m_display2DPos.y, m_display2DSize.x, m_display2DSize.y);

	//	glm::vec2 imageSize;
	//	if (useInfrared)
	//	{
	//		imageSize = glm::vec2(m_depth_width, m_depth_height);
	//		MVP = m_projection * m_view * m_model_flow;
	//	}
	//	else
	//	{
	//		imageSize = glm::vec2(m_color_width, m_color_height);
	//		MVP = m_projection * m_view * m_model_flow;
	//	}
	//	glBindVertexArray(m_VAO);


	//	glBindVertexArray(m_VAO);
	//	//MVP = glm::translate(MVP, glm::vec3(0.0f, 0.0f, 5.0f));
	//	glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
	//	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromFlowID);
	//	//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
	//	glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
	//	glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

	//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	//}







	//}



	glBindVertexArray(0);


}


