#include "flood.h"


#include "opencv2/core/utility.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

void gFlood::compileAndLinkShader()
{
	try { 

	jumpFloodProg.compileShader("shaders/jumpFlood.cs");
	jumpFloodProg.link();

	//edgeDetectProg.compileShader("shaders/edgeDetect.cs");
	//edgeDetectProg.link();
	 
	}            
	catch (GLSLProgramException &e) {        
		std::cerr << e.what() << std::endl;           
		exit(EXIT_FAILURE);          
	}                                                 
}                                                                          
void gFlood::setLocations()
{            


	// jump flood algorithm
	m_subroutine_jumpFloodID = glGetSubroutineUniformLocation(jumpFloodProg.getHandle(), GL_COMPUTE_SHADER, "jumpFloodSubroutine");
	m_jfaInitID = glGetSubroutineIndex(jumpFloodProg.getHandle(), GL_COMPUTE_SHADER, "jumpFloodAlgorithmInit");
	m_jfaInitFromDepthID = glGetSubroutineIndex(jumpFloodProg.getHandle(), GL_COMPUTE_SHADER, "jfaInitFromDepth");

	m_jfaUpdateID = glGetSubroutineIndex(jumpFloodProg.getHandle(), GL_COMPUTE_SHADER, "jumpFloodAlgorithmUpdate");
	m_jfaUpscaleID = glGetSubroutineIndex(jumpFloodProg.getHandle(), GL_COMPUTE_SHADER, "jfaUpscale");

	m_getColorID = glGetSubroutineIndex(jumpFloodProg.getHandle(), GL_COMPUTE_SHADER, "getColorFromRGB");

	m_jumpID = glGetUniformLocation(jumpFloodProg.getHandle(), "jump");
	
	m_trackID = glGetUniformLocation(jumpFloodProg.getHandle(), "trackMat");
	m_scaleFactorID = glGetUniformLocation(jumpFloodProg.getHandle(), "scaleFactor");


	//m_subroutine_edgeDetectID = glGetSubroutineUniformLocation(edgeDetectProg.getHandle(), GL_COMPUTE_SHADER, "edgeDetectSubroutine");
	//m_applyFilterID = glGetSubroutineIndex(edgeDetectProg.getHandle(), GL_COMPUTE_SHADER, "applyFilter");
	//m_edgeThresholdID = glGetUniformLocation(edgeDetectProg.getHandle(), "edgeThreshold");
	
	glGenQueries(1, timeQuery);


} 
 
   
GLuint gFlood::createTexture(GLuint ID, GLenum target, int levels, int w, int h, int d, GLuint internalformat)
{
	GLuint texid;

	if (ID == 0)
	{
		glGenTextures(1, &texid);
	}
	else
	{
		glDeleteTextures(1, &ID);
		texid = ID;
		glGenTextures(1, &texid);
	}
	glBindTexture(target, texid);


	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// https://stackoverflow.com/questions/15405869/is-gltexstorage2d-imperative-when-auto-generating-mipmaps
	//glTexImage2D(target, 0, internalformat, w, h, 0, format, type, 0); // cretes mutable storage that requires glTexImage2D

	if (target == GL_TEXTURE_1D) 
	{   
		glTexStorage1D(target, levels, internalformat, w); 
	}  
	else if (target == GL_TEXTURE_2D)  
	{
		glTexStorage2D(target, levels, internalformat, w, h); // creates immutable storage and requires glTexSubImage2D

	}  
	else if (target == GL_TEXTURE_3D || d > 0) 
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexStorage3D(target, levels, internalformat, w, h, d);  
	} 
	return texid;      
}            
     
void gFlood::allocateBuffers()
{ 
	//m_trackedPoints.resize(2, 0.0f);
	/*glGenBuffers(1, &m_bufferClickedPoints);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferClickedPoints);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bufferClickedPoints);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_trackedPoints.size() * sizeof(float), m_trackedPoints.data(), GL_DYNAMIC_DRAW);*/
}  
    
          
void gFlood::allocateTextures()
{
	//GLenum theError;
	GLenum theError;
	theError = glGetError();
	// JFA stuff
	//std::vector<float> zeroVals(m_texture_width * m_texture_height * 2, 0);
	m_texture_initial = createTexture(m_texture_initial, GL_TEXTURE_3D, 2, m_volSize, m_volSize, m_volSize, GL_RGBA32F);

	m_texture_jfa_0 = createTexture(m_texture_jfa_0, GL_TEXTURE_3D, 2, m_volSize, m_volSize, m_volSize, GL_RGBA32F);
	m_texture_jfa_1 = createTexture(m_texture_jfa_1, GL_TEXTURE_3D, 2, m_volSize, m_volSize, m_volSize, GL_RGBA32F);

	//glBindTexture(GL_TEXTURE_2D, m_texture_initial);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RG, GL_FLOAT, zeroVals.data());
	
	//m_texture_initial_RGB = createTexture(m_texture_initial_RGB, GL_TEXTURE_3D, 2, m_volSize, m_volSize, m_volSize, GL_RGBA32F);
	
	m_texture_output_SDF = createTexture(m_texture_output_SDF, GL_TEXTURE_3D, 1, m_volSize, m_volSize, m_volSize, GL_R32I);

	m_zeroValues.resize(m_volSize * m_volSize * m_volSize * 4, 32768.0f);
	glBindTexture(GL_TEXTURE_3D, m_texture_initial);
	glTexSubImage3D(GL_TEXTURE_3D, 0,0, 0,0 , m_volSize, m_volSize, m_volSize, GL_RGBA, GL_FLOAT, m_zeroValues.data());
	glGenerateMipmap(GL_TEXTURE_3D);
	theError = glGetError();

}   

void gFlood::pushBackTP(float x, float y)
{
	//m_trackedPoints.push_back(x);
	//m_trackedPoints.push_back(y);

	//for (int u = -50; u < 50; u++)
	//{
	//	m_trackedPoints.push_back(x + u);
	//	m_trackedPoints.push_back(y);
	//}

	//for (int v = -50; v < 50; v++)
	//{
	//	m_trackedPoints.push_back(x);
	//	m_trackedPoints.push_back(y + v);
	//}

	//for (auto i : m_trackedPoints)
	//	std::cout << i << " " << std::endl;
}
void gFlood::uploadTP()
{

	//GLenum theError;
	//theError = glGetError();

	wipeFlood();

	int i = 0;
	int j = 35;
	int k = 20;

	std::vector<float> trackedPoints(m_volSize / 2 * 3, 0);
	for (int m = 0; m < m_volSize / 2 * 3; m+=3, i++)
	{
		//trackedPoints[i] = std::rand() % 128;
		trackedPoints[m] = i;
		trackedPoints[m+1] = j;
		trackedPoints[m+2] = k;

	}

	for (int i = 0; i < 30; i += 3)
	{
		std::cout << trackedPoints[i] << " " << trackedPoints[i + 1] << " " << trackedPoints[i + 2] << std::endl;
	}

	//theError = glGetError();



	std::vector<float> points(4,0);
	for (size_t i = 0; i < trackedPoints.size(); i += 3)
	{
		points[0] = i;
		points[1] = j;
		points[2] = k;

		glBindTexture(GL_TEXTURE_3D, m_texture_jfa_1);
		glTexSubImage3D(GL_TEXTURE_3D, 1, points[0], points[1], points[2], 1, 1, 1, GL_RGBA, GL_FLOAT, points.data());
	}
	//	theError = glGetError();

	//glCopyImageSubData(m_texture_initial, GL_TEXTURE_3D, 1, 0, 0, 0,
	//	m_texture_jfa_1, GL_TEXTURE_3D, 1, 0, 0, 0,
	//	m_volSize / 2, m_volSize / 2, m_volSize / 2);

	// 1 + JFA


		jumpFloodProg.use();
		glUniformMatrix4fv(m_trackID, 1, GL_FALSE, glm::value_ptr(m_pose));

		glUniform1f(m_scaleFactorID, (m_volSize / 2.0) / m_volDim);

	int compWidth = divup(m_volSize / 2, 4);
	int compHeight = divup(m_volSize / 2, 4);
	int compDepth = divup(m_volSize / 2, 4);
	//theError = glGetError();

	glUniform1f(m_jumpID, 1);
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_jfaUpdateID);

	glBindImageTexture(0, m_texture_jfa_1, 1, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, m_texture_jfa_0, 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glDispatchCompute(compWidth, compHeight, compDepth);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	//theError = glGetError();

	//cv::Mat prejfaMat = cv::Mat(m_texture_height, m_texture_width, CV_32SC2);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_texture_jfa_0);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RG_INTEGER, GL_INT, prejfaMat.data);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glActiveTexture(0);

	////cv::imshow("dens1", sxx3);   

	//cv::Mat image1[2];
	//cv::split(prejfaMat, image1);
	//cv::imshow("prejfa0", image1[0] * 255);
	//cv::imshow("prejfa1", image1[1] * 255);

}

void gFlood::setFloodInitialRGBTexture(unsigned char * data, int width, int height, int nrChan)
{
	//GLenum err;
	//err = glGetError();

	//err = glGetError();

	//glBindTexture(GL_TEXTURE_2D, m_texture_initial_RGB);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

	////err = glGetError();

	//cv::Mat im0 = cv::Mat(height, width, CV_8UC3);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_texture_initial_RGB);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, im0.data);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glActiveTexture(0);

	////cv::Mat colSplit[2];
	////cv::split(col, colSplit);
	////cv::imshow("col0", colSplit[0] > 0);
	////cv::imshow("col1", colSplit[1] > 0);

	//cv::imshow("mat", im0);
	//cv::waitKey(1);




}

void gFlood::setFloodInitialFromDepth()
{

	wipeFlood();

	int compWidth = divup(840, 4); // image2D dimensions, set this as variable
	int compHeight = divup(484, 4);


	jumpFloodProg.use();
	glUniformMatrix4fv(m_trackID, 1, GL_FALSE, glm::value_ptr(m_pose));

	glUniform1f(m_scaleFactorID, (m_volSize / 2.0) / m_volDim);
	


	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_jfaInitFromDepthID);

	glBindImageTexture(0, m_texture_jfa_1, 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(3, m_textureVertices, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glDispatchCompute(compWidth, compHeight, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//glCopyImageSubData(m_texture_jfa_1, GL_TEXTURE_3D, 1, 0, 0, 0, m_texture_initial_RGB, GL_TEXTURE_3D, 1, 0, 0, 0, m_volSize / 2, m_volSize / 2, m_volSize / 2);

	// 1 + JFA
	compWidth = divup(m_volSize / 2, 4);
	compHeight = divup(m_volSize / 2, 4);
	int compDepth = divup(m_volSize / 2, 4);

	glUniform1f(m_jumpID, 1);
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_jfaUpdateID);

	glBindImageTexture(0, m_texture_jfa_1, 1, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, m_texture_jfa_0, 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glDispatchCompute(compWidth, compHeight, compDepth);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);



}


void gFlood::wipeFlood()
{

	//glCopyImageSubData(m_texture_initial, GL_TEXTURE_3D, 0, 0, 0, 0, m_texture_jfa_0, GL_TEXTURE_3D, 0, 0, 0, 0, m_volSize, m_volSize, m_volSize);
	//glCopyImageSubData(m_texture_initial, GL_TEXTURE_3D, 1, 0, 0, 0, m_texture_jfa_0, GL_TEXTURE_3D, 1, 0, 0, 0, m_volSize / 2, m_volSize / 2, m_volSize / 2);

	//glCopyImageSubData(m_texture_initial, GL_TEXTURE_3D, 0, 0, 0, 0, m_texture_jfa_1, GL_TEXTURE_3D, 0, 0, 0, 0, m_volSize, m_volSize, m_volSize);
	glCopyImageSubData(m_texture_initial, GL_TEXTURE_3D, 1, 0, 0, 0, m_texture_jfa_1, GL_TEXTURE_3D, 1, 0, 0, 0, m_volSize / 2, m_volSize / 2, m_volSize / 2);


}    

void gFlood::clearPoints()
{
	//glBindBuffer(GL_ARRAY_BUFFER, m_trackedPointsBuffer);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, m_trackedPoints.size() * sizeof(float), m_trackedPoints.data());
}

     
void gFlood::jumpFloodCalc()
{

	glBeginQuery(GL_TIME_ELAPSED, timeQuery[0]);

	setFloodInitialFromDepth();
	//uploadTP();

	int compWidth = divup(m_volSize / 2, 4);
	int compHeight = divup(m_volSize / 2, 4);
	int compDepth = divup(m_volSize / 2, 4);

	bool useRandPoints = false;
	jumpFloodProg.use();
	if (useRandPoints)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, m_texture_initial);
		glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_jfaInitID);
		glBindImageTexture(0, m_texture_jfa_0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(1, m_texture_jfa_1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		glDispatchCompute(compWidth, compHeight, compDepth);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}


	//int iterCount = 0;
	//float maxDim = std::max(m_texture_width, m_texture_height);
	
	//int log2ceil = std::ceil(std::log(maxDim) / std::log(2));
	//int passes = log2ceil - 1;
	// using 11 levels that will enable an image up to 4096x4096
	
	//float stepLength = std::pow(std::log(m_volSize) - 1,2);
	
	int numLevels = std::log2(m_volSize / 2);

	for (int level = 0; level < numLevels; level++)
	{
		int stepWidth = std::max(1, (int(m_volSize / 2) >> (level + 1)));
		//std::cout << "step width " << stepWidth << std::endl;
		glUniform1f(m_jumpID, (float)stepWidth);
		glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_jfaUpdateID);

		if (level % 2 == 0)
		{
			glBindImageTexture(0, m_texture_jfa_0, 1, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			glBindImageTexture(1, m_texture_jfa_1, 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		}
		else
		{
			glBindImageTexture(0, m_texture_jfa_1, 1, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			glBindImageTexture(1, m_texture_jfa_0, 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		}



		glDispatchCompute(compWidth, compHeight, compDepth);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}


	// 1 + JFA half reso variant https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=4276119&tag=1

	compWidth = divup(m_volSize, 4);
	compHeight = divup(m_volSize, 4);
	compDepth = divup(m_volSize, 4);

	// UPSCALE
	glUniform1f(m_jumpID, 1.0f);
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_jfaUpscaleID);
	glBindImageTexture(0, m_texture_jfa_0, 1, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, m_texture_jfa_0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glDispatchCompute(compWidth, compHeight, compDepth);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


	// FINAL FLOOD STEP LENGTH 1.0


	glUniform1f(m_jumpID, 1.0f);
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_jfaUpdateID);
	glBindImageTexture(0, m_texture_jfa_0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, m_texture_jfa_1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glDispatchCompute(compWidth, compHeight, compDepth);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);



	glEndQuery(GL_TIME_ELAPSED);
	GLuint available = 0;
	while (!available) {
		glGetQueryObjectuiv(timeQuery[0], GL_QUERY_RESULT_AVAILABLE, &available);
	}
	// elapsed time in nanoseconds
	GLuint64 elapsed;
	glGetQueryObjectui64vEXT(timeQuery[0], GL_QUERY_RESULT, &elapsed);
	std::cout << elapsed / 1000000.0 << std::endl;

	//std::vector<float> vecdata(m_texture_width * m_texture_height * 2);
	//cv::Mat col = cv::Mat(m_texture_height, m_texture_width, CV_32FC2, vecdata.data());


	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_texture_jfa_1);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, col.data);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glActiveTexture(0);

	//cv::Mat colSplit[2];
	//cv::split(col, colSplit);
	//cv::imshow("col0", colSplit[0] * 0.001f);
	//cv::imshow("col1", colSplit[1] * 0.001f);



	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_texture_initial);
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_getColorID);
	glBindImageTexture(1, m_texture_jfa_1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(2, m_texture_output_SDF, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

	glDispatchCompute(compWidth, compHeight, compDepth);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);






	//std::vector<uint8_t> colMatVon(m_texture_width * m_texture_height * 4, 4);
	//cv::Mat colVon = cv::Mat(m_texture_height, m_texture_width, CV_8UC4, colMatVon.data());


	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_texture_output_RGBA);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, colVon.data);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glActiveTexture(0);

	//cv::imshow("cols", colVon);
	//cv::waitKey(1);

}
