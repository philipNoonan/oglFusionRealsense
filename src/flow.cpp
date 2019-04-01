#include "flow.h"



void gFlow::compileAndLinkShader()
{
	try { 

	disFlowProg.compileShader("shaders/disFlow.cs");
	disFlowProg.link();
	  
	sobelProg.compileShader("shaders/sobelEdge.cs"); 
	sobelProg.link(); 
	 
	variRefineProg.compileShader("shaders/variRefine.cs");   
	variRefineProg.link();  
	            
	extKalmanProg.compileShader("shaders/extendedKalmanFilter.cs");
	extKalmanProg.link(); 

	hpQuadtreeProg.compileShader("shaders/quadtree.cs");
	hpQuadtreeProg.link();

	hpQuadListProg.compileShader("shaders/traverseQuadtree.cs");
	hpQuadListProg.link();

	//jumpFloodProg.compileShader("shaders/jumpFlood.cs");
	//jumpFloodProg.link();

	prefixSumProg.compileShader("shaders/prefixSum2D.cs");
	prefixSumProg.link();

	stdDevProg.compileShader("shaders/stdDev.cs");
	stdDevProg.link();

	renderOffscreenProg.compileShader("shaders/vertShaderOS.vs");
	renderOffscreenProg.compileShader("shaders/fragShaderOS.fs");
	renderOffscreenProg.link();

	 
	densifyRasterProg.compileShader("shaders/vertShaderDensify.vs");
	densifyRasterProg.compileShader("shaders/fragShaderDensify.fs");
    densifyRasterProg.link();


	}            
	catch (GLSLProgramException &e) {        
		std::cerr << e.what() << std::endl;           
		exit(EXIT_FAILURE);          
	}                                                 
}                                                                          
void gFlow::setLocations()
{                        
	//sobel
	m_subroutine_SobelID = glGetSubroutineUniformLocation(sobelProg.getHandle(), GL_COMPUTE_SHADER, "launchSubroutine"); // this is wrong
	m_getGradientsID = glGetSubroutineIndex(sobelProg.getHandle(), GL_COMPUTE_SHADER, "getGradients");
	m_getSmoothnessID = glGetSubroutineIndex(sobelProg.getHandle(), GL_COMPUTE_SHADER, "getSmoothness");
	m_imageType_cov_ID = glGetUniformLocation(sobelProg.getHandle(), "imageType");
	m_level_cov_ID = glGetUniformLocation(sobelProg.getHandle(), "level");
	// disflow
	m_subroutine_DISflowID = glGetSubroutineUniformLocation(disFlowProg.getHandle(), GL_COMPUTE_SHADER, "launchSubroutine"); // this is wrong
	m_makePatchesID = glGetSubroutineIndex(disFlowProg.getHandle(), GL_COMPUTE_SHADER, "makePatches");
	m_makePatchesHorID = glGetSubroutineIndex(disFlowProg.getHandle(), GL_COMPUTE_SHADER, "makePatchesHor");
	m_makePatchesVerID = glGetSubroutineIndex(disFlowProg.getHandle(), GL_COMPUTE_SHADER, "makePatchesVer");
	m_trackID = glGetSubroutineIndex(disFlowProg.getHandle(), GL_COMPUTE_SHADER, "track");
	m_trackPoseID = glGetSubroutineIndex(disFlowProg.getHandle(), GL_COMPUTE_SHADER, "trackPose");


	m_patchInverseSearchID = glGetSubroutineIndex(disFlowProg.getHandle(), GL_COMPUTE_SHADER, "patchInverseSearch");
	m_patchInverseSearchDescentID = glGetSubroutineIndex(disFlowProg.getHandle(), GL_COMPUTE_SHADER, "patchInverseSearchDescent");

	m_densificationID = glGetSubroutineIndex(disFlowProg.getHandle(), GL_COMPUTE_SHADER, "densification");
	m_medianFilterID = glGetSubroutineIndex(disFlowProg.getHandle(), GL_COMPUTE_SHADER, "medianFilter");


	m_sumFlowTextureID = glGetSubroutineIndex(disFlowProg.getHandle(), GL_COMPUTE_SHADER, "sumFlowTexture");

	//m_prefixSum2D_HorID = glGetSubroutineIndex(disFlowProg.getHandle(), GL_COMPUTE_SHADER, "prefixSum2D_hor");
	//m_prefixSum2D_VerID = glGetSubroutineIndex(disFlowProg.getHandle(), GL_COMPUTE_SHADER, "prefixSum2D_ver");
	//patchInverseSearch_fwd1ID = glGetSubroutineIndex(disFlowProg.getHandle(), GL_COMPUTE_SHADER, "patchInverseSearch_fwd1");
	 
	m_patch_sizeID = glGetUniformLocation(disFlowProg.getHandle(), "patch_size");
	m_patch_strideID = glGetUniformLocation(disFlowProg.getHandle(), "patch_stride");
	m_level_dis_ID = glGetUniformLocation(disFlowProg.getHandle(), "level");
	m_iter_dis_ID = glGetUniformLocation(disFlowProg.getHandle(), "iter");
	m_trackWidthID = glGetUniformLocation(disFlowProg.getHandle(), "trackWidth");
	m_texSizeID = glGetUniformLocation(disFlowProg.getHandle(), "texSize");
	m_imageType_dis_ID = glGetUniformLocation(disFlowProg.getHandle(), "imageType");

	m_valAID = glGetUniformLocation(disFlowProg.getHandle(), "valA");
	m_valBID = glGetUniformLocation(disFlowProg.getHandle(), "valB");


	// variref
	m_subroutine_variRefineID = glGetSubroutineUniformLocation(variRefineProg.getHandle(), GL_COMPUTE_SHADER, "launchSubroutine"); // this is wrong
	m_prepareBuffersID = glGetSubroutineIndex(variRefineProg.getHandle(), GL_COMPUTE_SHADER, "prepareBuffers");
	m_computeDataTermID = glGetSubroutineIndex(variRefineProg.getHandle(), GL_COMPUTE_SHADER, "computeDataTerm");
	m_computeSmoothnessTermID = glGetSubroutineIndex(variRefineProg.getHandle(), GL_COMPUTE_SHADER, "computeSmoothnessTerm");

	m_computeSORID = glGetSubroutineIndex(variRefineProg.getHandle(), GL_COMPUTE_SHADER, "computeSOR");
	m_resizeID = glGetSubroutineIndex(variRefineProg.getHandle(), GL_COMPUTE_SHADER, "resize");

	
	m_level_var_ID = glGetUniformLocation(variRefineProg.getHandle(), "level");
	m_flipflopID = glGetUniformLocation(variRefineProg.getHandle(), "flipflop");
	m_iter_var_ID = glGetUniformLocation(variRefineProg.getHandle(), "iter");

	//extended kalman filter


	// jump flood algorithm
	m_subroutine_jumpFloodID = glGetSubroutineUniformLocation(jumpFloodProg.getHandle(), GL_COMPUTE_SHADER, "launchSubroutine"); // this is wrong
	m_jfaInitID = glGetSubroutineIndex(jumpFloodProg.getHandle(), GL_COMPUTE_SHADER, "jumpFloodAlgorithmInit");
	m_jfaUpdateID = glGetSubroutineIndex(jumpFloodProg.getHandle(), GL_COMPUTE_SHADER, "jumpFloodAlgorithmUpdate");

	m_jumpID = glGetUniformLocation(jumpFloodProg.getHandle(), "jump");

	// quadtrees
	m_subroutine_hpQuadtreeID = glGetSubroutineUniformLocation(hpQuadtreeProg.getHandle(), GL_COMPUTE_SHADER, "hpQuadtreeSubroutine");
	m_hpDiscriminatorID = glGetSubroutineIndex(hpQuadtreeProg.getHandle(), GL_COMPUTE_SHADER, "hpDiscriminator");
	m_hpBuilderID = glGetSubroutineIndex(hpQuadtreeProg.getHandle(), GL_COMPUTE_SHADER, "hpBuilder");

	m_hpLevelID = glGetUniformLocation(hpQuadtreeProg.getHandle(), "hpLevel");
	m_quadThreshID = glGetUniformLocation(hpQuadtreeProg.getHandle(), "quadThresh");

	m_subroutine_hpQuadlistID = glGetSubroutineUniformLocation(hpQuadListProg.getHandle(), GL_COMPUTE_SHADER, "quadlistSubroutine");
	m_traverseHPLevelID = glGetSubroutineIndex(hpQuadListProg.getHandle(), GL_COMPUTE_SHADER, "traverseHPLevel");

	m_totalSumID = glGetUniformLocation(hpQuadListProg.getHandle(), "totalSum");
	m_cutoffID = glGetUniformLocation(hpQuadListProg.getHandle(), "cutoff");

	//offscreen rendering
	m_imSizeID = glGetUniformLocation(renderOffscreenProg.getHandle(), "imSize");
	m_texLevelID = glGetUniformLocation(renderOffscreenProg.getHandle(), "texLevel");

	// prefixSum2d 
	m_useRGBAID = glGetUniformLocation(prefixSumProg.getHandle(), "useRGBA");


	//std dev
	m_subroutine_stdDevID = glGetSubroutineUniformLocation(stdDevProg.getHandle(), GL_COMPUTE_SHADER, "launchSubroutine"); // this is wrong
	m_stdFirstID = glGetSubroutineIndex(stdDevProg.getHandle(), GL_COMPUTE_SHADER, "firstPass");
	m_stdSecondID = glGetSubroutineIndex(stdDevProg.getHandle(), GL_COMPUTE_SHADER, "secondPass");
	m_quadListCountID = glGetUniformLocation(stdDevProg.getHandle(), "quadListCount");

	glGenQueries(1, timeQuery);

	variational_refinement_iter = 5;
	variational_refinement_alpha = 20.f;
	variational_refinement_gamma = 10.f;
	variational_refinement_delta = 5.f;

	/* Use separate variational refinement instances for different scales to avoid repeated memory allocation: */
	int max_possible_scales = 10;
	//for (int i = 0; i < max_possible_scales; i++)
//		variational_refinement_processors.push_back(cv::optflow::createVariationalFlowRefinement());

	
	  
} 
void gFlow::allocateOffscreenRendering()
{
	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	glGenTextures(1, &m_textureFlowMinusMeanFlow);
	glBindTexture(GL_TEXTURE_2D, m_textureFlowMinusMeanFlow);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_texture_width, m_texture_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureFlowMinusMeanFlow, 0);

	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, attachments);

	glGenRenderbuffers(1, &m_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_texture_width, m_texture_height); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferQuadlist);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, m_bufferQuadlistMeanTemp);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

}



void gFlow::setTexture()
{
	//I1im = cv::Mat(m_texture_height, m_texture_width, CV_8UC4, imageArray);

	//if (nChn == 4)
	//{
	//	glActiveTexture(GL_TEXTURE1);
	//	glBindTexture(GL_TEXTURE_2D, m_textureI1);
	//	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RGBA, GL_UNSIGNED_BYTE, imageArray);
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//}
	//else if (nChn == 3)
	//{
	//	glActiveTexture(GL_TEXTURE1);
	//	glBindTexture(GL_TEXTURE_2D, m_textureI1);
	//	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RGB, GL_UNSIGNED_BYTE, imageArray);
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//}

	////theErr = glGetError(); 
	//glMemoryBarrier(GL_ALL_BARRIER_BITS); 






	  
	//cv::Mat col1 = cv::Mat(m_texture_height / 1, m_texture_width / 1, CV_8UC4);
	//cv::Mat col2 = cv::Mat(m_texture_height / 2, m_texture_width / 2, CV_8UC4);


	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureI1);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, col1.data);
	//glBindTexture(GL_TEXTURE_2D, 0);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureI1);
	//glGetTexImage(GL_TEXTURE_2D, 1, GL_RGBA, GL_UNSIGNED_BYTE, col2.data);
	//glBindTexture(GL_TEXTURE_2D, 0);

	//cv::Mat col2p = cv::Mat(m_texture_height / 1, m_texture_width / 1, CV_8UC4);
	//cv::pyrUp(col2, col2p);

	////theErr = glGetError();
	//   
	//cv::imshow("colo1", col1 - col2p);

	//cv::Mat lap = col1 - col2p;

	//cv::Mat output = cv::Mat(m_texture_height / 1, m_texture_width / 1, CV_8UC4);
	//cv::multiply(lap, col1, output);

	//cv::imshow("coclcocl", output);


	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, m_textureI1);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RGBA, GL_UNSIGNED_BYTE, lap.data);
	//glGenerateMipmap(GL_TEXTURE_2D);

	////theErr = glGetError(); 
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);




	if (firstFrame)
	{

		glCopyImageSubData(m_textureI1, GL_TEXTURE_2D, 0, 0, 0, 0,
			m_textureI0, GL_TEXTURE_2D, 0, 0, 0, 0,
			m_texture_width, m_texture_height, 1);
		firstFrame = false;

		//I1im.copyTo(I0im);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureI0);
	glGenerateMipmap(GL_TEXTURE_2D);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//cv::Mat col0 = cv::Mat(m_texture_height / 1, m_texture_width / 1, CV_8UC4);
	//// 

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureI1);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, col0.data);
	//glBindTexture(GL_TEXTURE_2D, 0);

	//////theErr = glGetError();
	////   
	//cv::imshow("colo0", col0);


	//cv::Mat col = cv::Mat(m_texture_height, m_texture_width, CV_8UC4);


	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureI1);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, col.data);
	//glBindTexture(GL_TEXTURE_2D, 0);
	 
	//cv::imshow("colo", col);
	  

}     
        

void gFlow::setTexture(float * imageArray)
{

	I1im = cv::Mat(m_texture_height, m_texture_width, CV_32FC1, imageArray);


	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_textureI1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RED, GL_FLOAT, imageArray);
	glGenerateMipmap(GL_TEXTURE_2D);

	//theErr = glGetError(); 
	glMemoryBarrier(GL_ALL_BARRIER_BITS);




	if (firstFrame)
	{

		glCopyImageSubData(m_textureI1, GL_TEXTURE_2D, 0, 0, 0, 0,
			m_textureI0, GL_TEXTURE_2D, 0, 0, 0, 0,
			m_texture_width, m_texture_height, 1);
		firstFrame = false;

		//I1im.copyTo(I0im);

	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureI0);
	glGenerateMipmap(GL_TEXTURE_2D);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);



	//cv::Mat col = cv::Mat(m_texture_height / 2, m_texture_width / 2, CV_8UC4);
	////  

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureI1);
	//glGetTexImage(GL_TEXTURE_2D, 1, GL_RGBA, GL_UNSIGNED_BYTE, col.data);
	//glBindTexture(GL_TEXTURE_2D, 0);

	//////theErr = glGetError();
	////   
	//cv::imshow("colo1", col);

	//cv::Mat col0 = cv::Mat(m_texture_height / 2, m_texture_width / 2, CV_8UC4);
	//// 

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureI0);
	//glGetTexImage(GL_TEXTURE_2D, 1, GL_RGBA, GL_UNSIGNED_BYTE, col0.data);
	//glBindTexture(GL_TEXTURE_2D, 0);

	//////theErr = glGetError();
	////   
	//cv::imshow("colo0", col0);


	//cv::Mat col = cv::Mat(m_texture_height, m_texture_width, CV_32FC1);


	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureI1);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, col.data);
	//glBindTexture(GL_TEXTURE_2D, 0);

	//cv::imshow("colo", col);


}
   
GLuint gFlow::createTexture(GLuint ID, GLenum target, int levels, int w, int h, int d, GLuint internalformat)
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

	float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);

	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

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
		glTexStorage3D(target, levels, internalformat, w, h, d);  
	} 
	return texid;      
}            
     
void gFlow::allocateBuffers()
{ 
	// awesome BUFFER OBJECT
	m_refinementDataTerms.resize(m_texture_width * m_texture_height * 5, 0.25f); // a11, a12, a22, b1, b2
	size_t refineDataTermsSize = m_texture_width * m_texture_height * 5 * sizeof(float);
	glDeleteBuffers(1, &m_buffer_refinement_data_terms);
	glGenBuffers(1, &m_buffer_refinement_data_terms);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer_refinement_data_terms);

	glBufferData(GL_SHADER_STORAGE_BUFFER, refineDataTermsSize, m_refinementDataTerms.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, m_buffer_refinement_data_terms);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	 
	      
	 
	// tracking buffer
	m_trackedPoints.resize(m_texture_height * m_texture_height * 2); 
	int xOffTrack = m_texture_height / 2; 
	int yOffTrack = m_texture_height / 2;
	int xSpacing = 2 * xOffTrack / m_texture_height;
	int ySpacing = 2 * yOffTrack / m_texture_height;
	   
	for (int i = 0; i < m_texture_height * 2; i += 2)
	{
		for (int j = 0; j < m_texture_height; j++)
		{
			m_trackedPoints[j * m_texture_height * 2 + i] = (m_texture_width >> 1) - xOffTrack + (i / 2) * xSpacing;
			m_trackedPoints[j * m_texture_height * 2 + i + 1] = (m_texture_height >> 1) - yOffTrack + j * ySpacing;

		}  
	}     
	
	glDeleteBuffers(1, &m_trackedPointsBuffer);
	glGenBuffers(1, &m_trackedPointsBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_trackedPointsBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_trackedPoints.size() * sizeof(float), m_trackedPoints.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, m_trackedPointsBuffer);
	    


	// quadtrees
	glDeleteBuffers(1, &m_bufferQuadlist);
	glGenBuffers(1, &m_bufferQuadlist);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, m_bufferQuadlist);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 512*512*sizeof(float)*4, NULL, GL_DYNAMIC_DRAW); // some max size

	glDeleteBuffers(1, &m_bufferQuadlistMeanTemp);
	glGenBuffers(1, &m_bufferQuadlistMeanTemp);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, m_bufferQuadlistMeanTemp);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 512 * 512 * sizeof(float) * 4, NULL, GL_DYNAMIC_DRAW); // some max size


	



}  
    
          
void gFlow::allocateTextures(int nChn)
{
	zeroValues.resize(m_texture_width * m_texture_height * 4, 0);
	oneValues.resize(m_texture_width * m_texture_height * 4, 1);

	m_numberHPLevels = GLHelper::numberOfLevels(glm::ivec3(m_texture_width, m_texture_height, 1));
	// WITH OR WITHOUT THIS HINT, MIP MAPPING (at least the rg32f images) IS THE EXACT SAME AS cv::resize cv::INTER_AREA
	//glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	// the incoming texture should have been pre-mipmaped, but all other textures need to be allocated and wiped each frame
	if (nChn == 1)
	{
		m_textureI0 = createTexture(m_textureI0, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_R32F);
		//m_textureI1 = createTexture(m_textureI1, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_R32F);
	}
	else
	{
		m_textureI0 = createTexture(m_textureI0, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_RGB8);
		//m_textureI1 = createTexture(m_textureI1, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_RGBA8);
	}
	
	m_textureI0_prod_xx_yy_xy_aux = createTexture(m_textureI0_prod_xx_yy_xy_aux, GL_TEXTURE_2D, m_numLevels, m_texture_width / m_patch_stride, m_texture_height , 0, GL_RGBA32F);
	m_textureI0_sum_x_y_aux = createTexture(m_textureI0_sum_x_y_aux, GL_TEXTURE_2D, m_numLevels, m_texture_width / m_patch_stride, m_texture_height, 0, GL_RG32F);

	m_textureI0_prod_xx_yy_xy = createTexture(m_textureI0_prod_xx_yy_xy, GL_TEXTURE_2D, m_numLevels, m_texture_width / m_patch_stride, m_texture_height / m_patch_stride, 0, GL_RGBA32F);
	m_textureI0_sum_x_y = createTexture(m_textureI0_sum_x_y, GL_TEXTURE_2D, m_numLevels, m_texture_width / m_patch_stride, m_texture_height / m_patch_stride, 0, GL_RG32F);

	// all mip maps   
	    
	m_textureI0_grad_x_y = createTexture(m_textureI0_grad_x_y, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_RG32F);
	 
	m_textureS_x_y = createTexture(m_textureS_x_y, GL_TEXTURE_2D, m_numLevels, m_texture_width / m_patch_stride, m_texture_height / m_patch_stride, 0, GL_RG32F);
	glBindTexture(GL_TEXTURE_2D, m_textureS_x_y);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width / m_patch_stride, m_texture_height / m_patch_stride, GL_RG, GL_FLOAT, zeroValues.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	m_textureU_x_y = createTexture(m_textureU_x_y, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_RG32F);
	glBindTexture(GL_TEXTURE_2D, m_textureU_x_y);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RG, GL_FLOAT, zeroValues.data());
	glGenerateMipmap(GL_TEXTURE_2D); 

	m_texture_init_U_x_y = createTexture(m_texture_init_U_x_y, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_RG32F);
	glBindTexture(GL_TEXTURE_2D, m_texture_init_U_x_y);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RG, GL_FLOAT, zeroValues.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	m_texture_prefixSumTemp = createTexture(m_texture_prefixSumTemp, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_width, 0, GL_RG32F);
	glBindTexture(GL_TEXTURE_2D, m_texture_prefixSumTemp);
	glGenerateMipmap(GL_TEXTURE_2D);

	m_texture_prefixSum = createTexture(m_texture_prefixSum, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_RG32F);
	glBindTexture(GL_TEXTURE_2D, m_texture_prefixSum);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RG, GL_FLOAT, zeroValues.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	m_texture_prefixSumTempSecondPass = createTexture(m_texture_prefixSumTempSecondPass, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_width, 0, GL_RG32F);
	glBindTexture(GL_TEXTURE_2D, m_texture_prefixSumTempSecondPass);
	glGenerateMipmap(GL_TEXTURE_2D);

	m_texture_prefixSumSecondPass = createTexture(m_texture_prefixSumSecondPass, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_RG32F);
	glBindTexture(GL_TEXTURE_2D, m_texture_prefixSumSecondPass);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RG, GL_FLOAT, zeroValues.data());
	glGenerateMipmap(GL_TEXTURE_2D);


	m_texture_previous_U_x_y = createTexture(m_texture_previous_U_x_y, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_RG32F);

	       
	m_textureWarp_I1 = createTexture(m_textureWarp_I1, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_RGBA8);
	  
	      
	// variational refine  

	//m_textureI_warp = createTexture(GL_TEXTURE_2D, numLevels, m_texture_width, m_texture_height, 0, GL_RGBA8);
	//m_textureI_mix = createTexture(GL_TEXTURE_2D, numLevels, m_texture_width, m_texture_height, 0, GL_RGBA8);
	//m_textureI_diff = createTexture(GL_TEXTURE_2D, numLevels, m_texture_width, m_texture_height, 0, GL_RGBA8);

	m_textureI_mix_diff_warp = createTexture(m_textureI_mix_diff_warp, GL_TEXTURE_2D_ARRAY, m_numLevels, m_texture_width, m_texture_height, 3, GL_RGBA8);

	//m_textureI_warp_grad_x_y = createTexture(GL_TEXTURE_2D, numLevels, m_texture_width, m_texture_height, 0, GL_RG16I);
	//m_textureI_mix_grad_x_y = createTexture(GL_TEXTURE_2D, numLevels, m_texture_width, m_texture_height, 0, GL_RG16I);
	//m_textureI_diff_grad_x_y = createTexture(GL_TEXTURE_2D, numLevels, m_texture_width, m_texture_height, 0, GL_RG16I);

	m_textureI_grads_mix_diff_x_y = createTexture(m_textureI_grads_mix_diff_x_y, GL_TEXTURE_2D_ARRAY, m_numLevels, m_texture_width, m_texture_height, 2, GL_RG32F);
	m_textureI_second_grads_mix_diff_x_y = createTexture(m_textureI_second_grads_mix_diff_x_y, GL_TEXTURE_2D_ARRAY, m_numLevels, m_texture_width, m_texture_height, 2, GL_RG32F);

	m_texture_dup_dvp = createTexture(m_texture_dup_dvp, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_RGBA32F);
	//m_texture_smoothness_ux_uy_vx_vy = createTexture(GL_TEXTURE_2D_ARRAY, numLevels, m_texture_width, m_texture_height, 2, GL_RG16I);
	m_texture_total_flow = createTexture(m_texture_total_flow, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_RG32F);

	m_texture_smoothness_weight = createTexture(m_texture_smoothness_weight, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_R32F);
	m_texture_smoothness_terms = createTexture(m_texture_smoothness_terms, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_RG32F);


	m_texture_temp = createTexture(m_texture_temp, GL_TEXTURE_2D, 1, m_texture_width, m_texture_height, 0, GL_R16I);
	m_texture_temp1 = createTexture(m_texture_temp1, GL_TEXTURE_2D, 1, m_texture_width, m_texture_height, 0, GL_R16I);
	
	m_textureTest = createTexture(m_textureTest, GL_TEXTURE_2D, m_numLevels, m_texture_width / m_patch_stride, m_texture_height / m_patch_stride, 0, GL_R32F);
	   
	m_sumFlow = createTexture(m_sumFlow, GL_TEXTURE_2D, 1, m_texture_width, m_texture_height, 0, GL_RG32F);

	// JFA stuff
	m_texture_jfa_0 = createTexture(m_texture_jfa_0, GL_TEXTURE_2D, 1, m_texture_width, m_texture_height, 0, GL_RG32I);
	m_texture_jfa_1 = createTexture(m_texture_jfa_1, GL_TEXTURE_2D, 1, m_texture_width, m_texture_height, 0, GL_RG32I);

	// QUADTREEE
	m_texture_hpOriginalData = GLHelper::createTexture(m_texture_hpOriginalData, GL_TEXTURE_2D, 1, m_texture_width, m_texture_height, 0, GL_R32F);
	m_texture_hpQuadtree = GLHelper::createTexture(m_texture_hpQuadtree, GL_TEXTURE_2D, m_numberHPLevels +1, 1 << m_numberHPLevels, 1 << m_numberHPLevels, 0, GL_R32F);
	m_textureBLANKFLOW = createTexture(m_textureBLANKFLOW, GL_TEXTURE_2D, 1, m_texture_width, m_texture_height, 0, GL_RG32F);
	glBindTexture(GL_TEXTURE_2D, m_textureBLANKFLOW);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RG, GL_FLOAT, oneValues.data());

	//m_textureFlowMinusMeanFlow = createTexture(m_textureFlowMinusMeanFlow, GL_TEXTURE_2D, m_numLevels, m_texture_width, m_texture_height, 0, GL_RGBA32F);
}   
void gFlow::wipeSumFlow()
{
	glBindTexture(GL_TEXTURE_2D, m_sumFlow);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RG, GL_FLOAT, zeroValues.data());
}

void gFlow::wipeFlow()
{
	glBindTexture(GL_TEXTURE_2D, m_textureU_x_y);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RG, GL_FLOAT, zeroValues.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, m_texture_total_flow);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RG, GL_FLOAT, zeroValues.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, m_texture_init_U_x_y);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RG, GL_FLOAT, zeroValues.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, m_textureTest);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RED, GL_FLOAT, zeroValues.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, m_textureS_x_y);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width / m_patch_stride, m_texture_height / m_patch_stride, GL_RG, GL_FLOAT, zeroValues.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindBuffer(GL_ARRAY_BUFFER, m_trackedPointsBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_trackedPoints.size() * sizeof(float), m_trackedPoints.data());


}    

void gFlow::clearPoints()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_trackedPointsBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_trackedPoints.size() * sizeof(float), m_trackedPoints.data());
}

void gFlow::computeSobel(int level, bool useInfrared)
{ 
	sobelProg.use(); 
	glUniform1i(m_level_cov_ID, level);

	
	//if (useInfrared)
	//{
		//glBindImageTexture(0, m_textureI0, level, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureI0);
		glUniform1i(m_imageType_cov_ID, 5); // image type 0 = color rgba8ui 
	//}
	//else
	//{
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, m_textureI0);
	//	glBindImageTexture(0, m_textureI0, level, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
	//	glUniform1i(m_imageType_cov_ID, 0); // image type 0 = color rgba8ui 
	//}




	glBindImageTexture(4, m_textureI0_grad_x_y, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
	
	int compWidth; 
	int compHeight;
	  
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_getGradientsID);

	compWidth = divup(m_texture_width >> level, 32); // right bitshift does division by powers of 2
	compHeight = divup(m_texture_height >> level, 32);

	    
	   
	glDispatchCompute(compWidth, compHeight, 1);   
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	//       
	//if (level == 1)  
	//{
	//	cv::Mat sx = cv::Mat(m_texture_height >> level, m_texture_width >> level, CV_32FC2);

	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, m_textureI0_grad_x_y);
	//	glGetTexImage(GL_TEXTURE_2D, level, GL_RG, GL_FLOAT, sx.data);// this is importnant, you are using GL_RED_INTEGETER!!!!
	//	glBindTexture(GL_TEXTURE_2D, 0);
	 
	//	cv::Mat threshsx = sx > 0.01;
	//	cv::Mat image00[2]; 
	//	cv::Mat dist;
	//	cv::split(threshsx, image00); 


	//	cv::imshow("ssss0", image00[0]);
	//	cv::imshow("ssss1", image00[1]);

	//	cv::distanceTransform(image00[0], dist, CV_DIST_L2, 3);
	//	cv::normalize(dist, dist, -0.5, 0.5, NORM_MINMAX);

	//	cv::imshow("dist", dist); 

	//	cv::waitKey(1);

	 
	//}




	if (level == 0)  
	{  
//		cv::Mat sx = cv::Mat(m_texture_height >> level, m_texture_width >> level, CV_32FC2);
//		 
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, m_textureI0_grad_x_y);
//		glGetTexImage(GL_TEXTURE_2D, level, GL_RG, GL_FLOAT, sx.data);// this is importnant, you are using GL_RED_INTEGETER!!!!
//		glBindTexture(GL_TEXTURE_2D, 0);
//		 
//		cv::Mat image00[2]; 
//		cv::split(sx, image00); 
//		    
//		  
//		cv::imshow("ssss0", image00[0] / 2 + 0.5);
//		cv::imshow("ssss1", image00[1] / 2 + 0.5);  
//
//		cv::Mat I0xs = cv::Mat(1080 >> level, 1920 >> level, CV_16SC1);
//		cv::Mat I0ys = cv::Mat(1080 >> level, 1920 >> level, CV_16SC1);
//		cv::Mat greyI0;
//
//		cv::Mat I1xs = cv::Mat(1080 >> level, 1920 >> level, CV_16SC1);
//		cv::Mat I1ys = cv::Mat(1080 >> level, 1920 >> level, CV_16SC1);
//		cv::Mat greyI1;
////		 
//		cv::cvtColor(I0im, greyI0, CV_RGBA2GRAY); 
//		cv::cvtColor(I1im, greyI1, CV_RGBA2GRAY); 
////
//		cv::spatialGradient(greyI0, I0xs, I0ys); 
//
//		cv::imshow("sx", I0xs * 128);
//		cv::imshow("sy", I0ys * 128);
		 
///*
//		cv::Scharr(greyI0, I0xs, CV_16S, 1, 0);
//		cv::Scharr(greyI0, I0xs, CV_16S, 1, 0);
//
//		cv::Mat I0xs_grad;
//		cv::convertScaleAbs(I0xs, I0xs_grad);
//		cv::imshow("gryxx", I0xs_grad); 
//
//		cv::spatialGradient(greyI1, I1xs, I1ys);
//		cv::imshow("gryxasdasdasd", I1xs);
//*/
//		I0xs.convertTo(I0xs, CV_32FC1); 
//
//		cv::imshow("difffs0", (image00[0] - I0xs));   
//
//		double min, max;
//		cv::minMaxLoc(image00[0], &min, &max); 
//
//		double min1, max1;
//		cv::minMaxLoc(I0xs, &min1, &max1);
//
//		cv::Scalar mean0 = cv::mean(I0xs);
//		cv::Scalar mean1 = cv::mean(image00[0]);
//
//		std::cout << "mean0 : " << mean0[0] << " mean 1 :" << mean1[0] << " rato : " << mean1[0] / mean0[0] << " min : " << min << " max : " << max << " min : " << min1 << " max : " << max1 << " ratio : " << max / max1 << std::endl;
//

	} 




	//else 
	if (level == 0)
	{
		//cv::Mat sx = cv::Mat(m_texture_height >> level, m_texture_width >> level, CV_32FC2);

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, m_textureI0_grad_x_y);
		//glGetTexImage(GL_TEXTURE_2D, level, GL_RG, GL_FLOAT, sx.data);// this is importnant, you are using GL_RED_INTEGETER!!!!
		//glBindTexture(GL_TEXTURE_2D, 0);
	 //
		//cv::Mat image11[2];
		//cv::split(sx, image11);


		//cv::imshow("s1x", image11[0] / 2 + 0.5);
		//cv::imshow("s1y", image11[1] / 2 + 0.5);
   
	}    
	//else if (level == 6)
	//{
	//	cv::Mat sx = cv::Mat(m_texture_height >> level, m_texture_width >> level, CV_16SC2);

	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, m_textureI0_grad_x_y);
	//	glGetTexImage(GL_TEXTURE_2D, level, GL_RG_INTEGER, GL_SHORT, sx.data);// this is importnant, you are using GL_RED_INTEGETER!!!!
	//	glBindTexture(GL_TEXTURE_2D, 0);
	//	cv::Mat image112[2];
	//	cv::split(sx, image112);

	//	//sx.convertTo(sx, CV_8U, ); 

	//	cv::imshow("s2x", image112[0]);
	//	cv::imshow("s2y", image112[1]);



	//}   
	//   
	// 
	  
	     
	        
}       
    
void gFlow::makePatches(int level)
{
	//MAKE PATCHES
	disFlowProg.use(); 
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_makePatchesID);

	glBindImageTexture(0, m_textureI0_grad_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
	glBindImageTexture(4, m_textureI0_prod_xx_yy_xy, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(5, m_textureI0_sum_x_y, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
	  
	     
	int compWidth, compHeight;
	compWidth = divup(m_texture_width >> level, 16); 
	compHeight = divup(m_texture_height >> level, 16); 
	     
	glDispatchCompute(compWidth, compHeight, 1);
	 
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); 
	  

	////MAKE PATCHES 
	//disFlowProg.use();  
	//glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_makePatchesHorID);

	//glBindImageTexture(0, m_textureI0_grad_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG16I);
	//glBindImageTexture(4, m_textureI0_prod_xx_yy_xy_aux, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	//glBindImageTexture(5, m_textureI0_sum_x_y_aux, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);


	//int compWidth, compHeight;


	//compWidth = 1;
	//compHeight = divup(m_texture_height >> level, 4);
	// 

	//glDispatchCompute(compWidth, compHeight, 1);

	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);



	//glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_makePatchesVerID);

	//glBindImageTexture(0, m_textureI0_grad_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG16I);

	//glBindImageTexture(7, m_textureI0_prod_xx_yy_xy_aux, level, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	//glBindImageTexture(3, m_textureI0_sum_x_y_aux, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);

	//glBindImageTexture(4, m_textureI0_prod_xx_yy_xy, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	//glBindImageTexture(5, m_textureI0_sum_x_y, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

	//compWidth = divup(m_texture_width >> level, 4);
	//compHeight = 1;

	//glDispatchCompute(compWidth, compHeight, 1);

	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


	 
	 




	 




	 

	      
	     
	      
	        
	// if (level == 0)
	// {
	//	cv::Mat sxx = cv::Mat((m_texture_height >> level) / m_patch_stride, (m_texture_width >> level) / m_patch_stride, CV_32FC4);

	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, m_textureI0_prod_xx_yy_xy);
	//	glGetTexImage(GL_TEXTURE_2D, level, GL_RGBA, GL_FLOAT, sxx.data);
	//	glBindTexture(GL_TEXTURE_2D, 0); 
	//	  
	//	cv::Mat image0[4];     
	//	cv::split(sxx, image0);  

	//	cv::imshow("sflowx", image0[0] / 2.0 + 0.5); 
	//	cv::imshow("sflowy", image0[1] / 2.0 + 0.5);
	//	cv::imshow("sflowz", image0[2] / 2.0 + 0.5);

	//	cv::Mat sumxy = cv::Mat((m_texture_height >> level) / m_patch_stride, (m_texture_width >> level) / m_patch_stride, CV_32FC2);

	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, m_textureI0_sum_x_y);
	//	glGetTexImage(GL_TEXTURE_2D, level, GL_RG, GL_FLOAT, sumxy.data);
	//	glBindTexture(GL_TEXTURE_2D, 0);

	//	cv::Mat imsumxy[4]; 
	//	cv::split(sumxy, imsumxy);
	//	//cv::normalize(imsumxy[0], imsumxy[0], 0, 1, cv::NORM_MINMAX); 
	//	//cv::normalize(imsumxy[1], imsumxy[1], 0, 1, cv::NORM_MINMAX);

	//	cv::imshow("imsumx", imsumxy[0]);
	//	cv::imshow("imsumy", imsumxy[1]);
	//}    
	// else if (level == 1) 
	// {
	//	 cv::Mat sxx = cv::Mat((m_texture_height >> level) / m_patch_stride, (m_texture_width >> level) / m_patch_stride, CV_32FC4);

	//	 glActiveTexture(GL_TEXTURE0);
	//	 glBindTexture(GL_TEXTURE_2D, m_textureI0_prod_xx_yy_xy);
	//	 glGetTexImage(GL_TEXTURE_2D, level, GL_RGBA, GL_FLOAT, sxx.data);
	//	 glBindTexture(GL_TEXTURE_2D, 0);

	//	 cv::Mat image0[4];
	//	 cv::split(sxx, image0); 

	//	 cv::imshow("sflowx1", image0[0]);
	//	 cv::imshow("sflowy1", image0[1]);
	//	 cv::imshow("sflowz1", image0[2]);

	//	 cv::Mat sumxy = cv::Mat((m_texture_height >> level) / m_patch_stride, (m_texture_width >> level) / m_patch_stride, CV_32FC2);

	//	 glActiveTexture(GL_TEXTURE0);
	//	 glBindTexture(GL_TEXTURE_2D, m_textureI0_sum_x_y);
	//	 glGetTexImage(GL_TEXTURE_2D, level, GL_RG, GL_FLOAT, sumxy.data);
	//	 glBindTexture(GL_TEXTURE_2D, 0);

	//	 cv::Mat imsumxy[4];
	//	 cv::split(sumxy, imsumxy);
	//	 //cv::normalize(imsumxy[0], imsumxy[0], 0, 1, cv::NORM_MINMAX); 
	//	 //cv::normalize(imsumxy[1], imsumxy[1], 0, 1, cv::NORM_MINMAX);

	//	 cv::imshow("imsumx1", imsumxy[0]);
	//	 cv::imshow("imsumy1", imsumxy[1]);

	//	  
  
	// }  
	   
}                          
                                
                      
bool gFlow::patchInverseSearch(int level, bool useInfrared)
{            
	 	    
		disFlowProg.use();       
		             
		glUniform1i(m_level_dis_ID, level);  

		if (useInfrared)
		{ 
			glUniform1i(m_imageType_dis_ID, 0);
		}
		else
		{
			glUniform1i(m_imageType_dis_ID, 1);
		} 

		//glUniform1i(m_imageFormatID);
		//glUniform1i(m_iter_dis_ID, outer_num_iter);     
		glUniform1f(m_valAID, m_valA);
		glUniform1f(m_valBID, m_valB);
		


		// INVERSE SEARCH 
		glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_patchInverseSearchID);
		    
		glActiveTexture(GL_TEXTURE0); 
		glBindTexture(GL_TEXTURE_2D, m_textureI0);       
		glActiveTexture(GL_TEXTURE1);      
		glBindTexture(GL_TEXTURE_2D, m_textureI1);        
		        
		                      
		glActiveTexture(GL_TEXTURE2);   
		glBindTexture(GL_TEXTURE_2D, m_textureI0_prod_xx_yy_xy);  
		glActiveTexture(GL_TEXTURE3);   
		glBindTexture(GL_TEXTURE_2D, m_textureI0_sum_x_y); 
		//
		glActiveTexture(GL_TEXTURE4);   
		glBindTexture(GL_TEXTURE_2D, m_textureU_x_y); 

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, m_texture_init_U_x_y);
		 
		glBindImageTexture(0, m_textureI0_grad_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
		 
		glBindImageTexture(1, m_textureU_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);   
		glBindImageTexture(2, m_texture_init_U_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);

		glBindImageTexture(4, m_textureI0_prod_xx_yy_xy, level, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(5, m_textureI0_sum_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);

		glBindImageTexture(6, m_textureS_x_y, level, GL_FALSE, 0, GL_READ_WRITE, GL_RG32F);
		 
		glBindImageTexture(3, m_textureTest, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
		     
		                    
		          
		int compWidth = divup(m_texture_width >> level, 16);   
		int compHeight = divup(m_texture_height >> level, 16);  

		glDispatchCompute(compWidth, compHeight, 1);    
		  
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);  


		   
		                                       	    
	return false;  
}   
             
  
bool gFlow::densification(int level)
{  

	disFlowProg.use(); 

	glUniform1i(m_level_dis_ID, level);

	// DENSIFICATION
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_densificationID);
	glUniform1i(m_imageType_dis_ID, 1); // 1 == rgb

	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, m_textureI0); 
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_textureI1); 

	glBindImageTexture(6, m_textureS_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
	glBindImageTexture(1, m_textureU_x_y, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
	    
	int compWidth = divup(m_texture_width >> level, 4);
	int compHeight = divup(m_texture_height >> level, 4);
	       
	glDispatchCompute(compWidth, compHeight, 1);  
	         
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);  
	          
	 
	return false;    
}        

void gFlow::medianFilter(int level)
{
	disFlowProg.use();
	glUniform1i(m_level_dis_ID, level);
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_medianFilterID);

	glBindImageTexture(2, m_textureU_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
	glBindImageTexture(1, m_texture_total_flow, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

	int compWidth = divup(m_texture_width >> level, 4);
	int compHeight = divup(m_texture_height >> level, 4);

	glDispatchCompute(compWidth, compHeight, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


	glMemoryBarrier(GL_ALL_BARRIER_BITS);



	glCopyImageSubData(m_texture_total_flow, GL_TEXTURE_2D, level, 0, 0, 0,
		m_textureU_x_y, GL_TEXTURE_2D, level, 0, 0, 0,
		m_texture_width >> level, m_texture_height >> level, 1);
	//}


	glMemoryBarrier(GL_ALL_BARRIER_BITS);


}
 
void gFlow::calcStandardDeviation(int level)
{
	//glClearTexSubImage(m_texture_prefixSumTemp,
	//	0,
	//	0,
	//	0,
	//	0,
	//	2048,
	//	2048,
	//	1,
	//	GL_FLOAT  ,
	//	GL_RG32F,
	//	NULL);

	// get image of prefix sums
	prefixSumProg.use();
	glUniform1i(m_useRGBAID, 0);

	glBindImageTexture(0, m_textureU_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
	glBindImageTexture(2, m_texture_prefixSumTemp, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

	glDispatchCompute(m_texture_width, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glBindImageTexture(0, m_texture_prefixSumTemp, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
	glBindImageTexture(2, m_texture_prefixSum, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

	glDispatchCompute(m_texture_width, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	////std::vector<float> col4(m_texture_height * m_texture_width * 4, -1.0);
	//cv::Mat colVonfir = cv::Mat(m_texture_height, m_texture_width, CV_32FC2);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_texture_prefixSum);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, colVonfir.data);
	//glBindTexture(GL_TEXTURE_2D, 0);

	//cv::Mat imagesfir[2];

	//cv::split(colVonfir, imagesfir);
	////cv::flip(imagesfir[0], imagesfir[0], 0);

	//cv::imshow("imssec", imagesfir[0]/ 10000000.0f);
	//cv::waitKey(1);


	// get buffer array of sums of quads
	stdDevProg.use();
	glBindImageTexture(0, m_texture_prefixSum, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bufferQuadlist);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_bufferQuadlistMeanTemp);
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_stdFirstID);
	glUniform1ui(m_quadListCountID, m_quadlistCount);


	int widthQuadList = GLHelper::divup(m_quadlistCount, 1024);
	glDispatchCompute(widthQuadList, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//std::vector<float> outputData0(m_quadlistCount * 4, -1.0);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferQuadlist);
	//void *ptr0 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(outputData0.data(), outputData0.size() * sizeof(float), ptr0, outputData0.size() * sizeof(float));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//std::vector<float> outputData(m_quadlistCount*2,-1.0);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferQuadlistMeanTemp);
	//void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(outputData.data(), outputData.size() * sizeof(float), ptr, outputData.size() * sizeof(float));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//cv::Mat outMeanImage = cv::Mat(2048, 2048, CV_32FC3);
	//int j = 0;
	//for (int i = 0; i < outputData0.size(); i+=4, j+=2)
	//{
	//	int xPos = outputData0[i];
	//	int yPos = outputData0[i + 1];
	//	int lod = outputData0[i + 2];

	//	float quadSideLength = float(pow(2, lod));

	//	float xOrigin = xPos * quadSideLength + (quadSideLength * 0.5f);
	//	float yOrigin = yPos * quadSideLength + (quadSideLength * 0.5f);

	//	float xFlow = outputData[j] * 0.1;
	//	float yFlow = outputData[j + 1] * 0.1;


	//	cv::circle(outMeanImage, cv::Point2f(xOrigin, yOrigin), quadSideLength / 2.0, cv::Scalar(xFlow * xFlow, yFlow * yFlow, 0), -1);
	//}

	//cv::imshow("cirlse", outMeanImage);
	//cv:waitKey(1);




	// get image of (x - xHat)^2
	renderOffscreenProg.use();

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, m_texture_width, m_texture_height);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureU_x_y);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	glBindVertexArray(m_VAO);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferQuadlist);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferQuadlistMeanTemp);

	glm::vec2 imageSize = glm::vec2(m_texture_width >> level, m_texture_height >> level);
	glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));
	glUniform1i(m_texLevelID, level);

	glDrawArrays(GL_POINTS, 0, m_quadlistCount);
	//glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	////std::vector<float> col4(m_texture_height * m_texture_width * 4, -1.0);
	//cv::Mat colVon = cv::Mat(m_texture_height, m_texture_width, CV_32FC4);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureFlowMinusMeanFlow);
	////glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, col4.data());
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, colVon.data);

	//glBindTexture(GL_TEXTURE_2D, 0);

	//cv::Mat images[4];

	//cv::split(colVon, images);
	////cv::flip(images[1], images[1], 0);
	////cv::flip(images[0], images[0], 0);

	//cv::imshow("ims", 0.1 *(images[0] + images[1]));
 //   cv::waitKey(1);


//	cv::Mat outImage = images[0].clone();

	//for (int j = 0; j < 1080; j++)
	//{
	//	for (int i = 1; i < 1920; i++)
	//	{
	//		outImage.at<float>(j, i) = outImage.at<float>(j, i - 1) + outImage.at<float>(j, i);
	//	}
	//}

	//for (int i = 0; i < 1920; i++)
	//{
	//	for (int j = 1; j < 1080; j++)
	//	{
	//		outImage.at<float>(j, i) = outImage.at<float>(j - 1, i) + outImage.at<float>(j, i);
	//	}
	//}
	//cv::imshow("imssum", outImage / 10000000.0f);
	//cv::waitKey(1);
	// get prefix sum of (x - xHat)^2 

	//prefixSumProg.use();
	//glUniform1i(m_useRGBAID, 1);

	//glBindImageTexture(1, m_textureFlowMinusMeanFlow, level, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	//glBindImageTexture(2, m_texture_prefixSumTempSecondPass, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

	//glDispatchCompute(m_texture_width, 1, 1);
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//glUniform1i(m_useRGBAID, 0);

	//glBindImageTexture(0, m_texture_prefixSumTempSecondPass, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
	//glBindImageTexture(2, m_texture_prefixSumSecondPass, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

	//glDispatchCompute(m_texture_width, 1, 1);
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);

	////std::vector<float> col2(m_texture_height * m_texture_width * 2, -1.0);
	////cv::Mat colVonsec = cv::Mat(m_texture_height, m_texture_width, CV_32FC2, col2.data());

	////glActiveTexture(GL_TEXTURE0);
	////glBindTexture(GL_TEXTURE_2D, m_texture_prefixSumSecondPass);
	////glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, colVonsec.data);
	////glBindTexture(GL_TEXTURE_2D, 0);

	////cv::Mat imagessec[2];

	////cv::split(colVonsec, imagessec);
	////cv::flip(imagessec[0], imagessec[0], 0);

	////cv::imshow("imssec", imagessec[0]*0.00001f);
	////cv::waitKey(1);


	//// get sum of (x - xHat)^2 for each quad
	//stdDevProg.use();
	//glBindImageTexture(0, m_texture_prefixSumSecondPass, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bufferQuadlist);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_bufferQuadlistMeanTemp);
	//glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_stdSecondID);
	//glUniform1ui(m_quadListCountID, m_quadlistCount);

	//glDispatchCompute(widthQuadList, 1, 1);
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//// re-pass this texture into the stdDev shader (but flag it to need to divide by N - 1, then take the square root!!!!) to get the sum of (x - xHat)^2 for each quad

	//// the buffer output of this shader contains the std dev of each quad

	//std::vector<float> outputDatastdedv(m_quadlistCount * 2, -1.0);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferQuadlistMeanTemp);
	//void *ptr1 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(outputDatastdedv.data(), outputDatastdedv.size() * sizeof(float), ptr1, outputDatastdedv.size() * sizeof(float));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//	std::vector<float> outputData0(m_quadlistCount * 4, -1.0);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferQuadlist);
	//void *ptr0 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(outputData0.data(), outputData0.size() * sizeof(float), ptr0, outputData0.size() * sizeof(float));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//std::vector<float> outputData(m_quadlistCount*2,-1.0);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferQuadlistMeanTemp);
	//void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(outputData.data(), outputData.size() * sizeof(float), ptr, outputData.size() * sizeof(float));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//cv::Mat outxMeanImage = cv::Mat(1080, 1920, CV_32FC3);
	//int j = 0;
	//for (int i = 0; i < outputData0.size(); i+=4, j+=2)
	//{
	//	int xPos = outputData0[i];
	//	int yPos = outputData0[i + 1];
	//	int lod = outputData0[i + 2];

	//	float quadSideLength = float(pow(2, lod));

	//	float xOrigin = xPos * quadSideLength + (quadSideLength * 0.5f);
	//	float yOrigin = yPos * quadSideLength + (quadSideLength * 0.5f);

	//	float xFlow = outputData[j] * 0.1;
	//	float yFlow = outputData[j + 1] * 0.1;


	//	cv::circle(outxMeanImage, cv::Point2f(xOrigin, yOrigin), quadSideLength / 2.0, cv::Scalar(xFlow * xFlow + yFlow * yFlow, xFlow * xFlow + yFlow * yFlow, xFlow * xFlow + yFlow * yFlow), -1);
	//}

	//cv::imshow("cirlse2", outxMeanImage);
	//cv:waitKey(1);


}




void gFlow::variationalRefinement(int level)
{
	cv::Mat I0imq = cv::Mat(m_texture_height >> level, m_texture_width >> level, CV_8UC4);
	cv::Mat I1imq = cv::Mat(m_texture_height >> level, m_texture_width >> level, CV_8UC4);

	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, m_textureI0);
	glGetTexImage(GL_TEXTURE_2D, level, GL_RGBA, GL_UNSIGNED_BYTE, I0imq.data);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureI1);
	glGetTexImage(GL_TEXTURE_2D, level, GL_RGBA, GL_UNSIGNED_BYTE, I1imq.data);
	glBindTexture(GL_TEXTURE_2D, 0);

	cv::Mat I0C1;
	cv::cvtColor(I0imq, I0C1, cv::COLOR_BGRA2GRAY);
	
	cv::Mat I1C1;
	cv::cvtColor(I1imq, I1C1, cv::COLOR_BGRA2GRAY);

	cv::Mat sxx3 = cv::Mat(m_texture_height >> level, m_texture_width >> level, CV_32FC2);
	cv::Mat sxx4 = cv::Mat(m_texture_height >> level, m_texture_width >> level, CV_32FC2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureU_x_y);
	glGetTexImage(GL_TEXTURE_2D, level, GL_RG, GL_FLOAT, sxx3.data);
	glBindTexture(GL_TEXTURE_2D, 0);
	    
	//cv::imshow("dens1", sxx3);  
	      
	cv::Mat image2[2];  
	cv::split(sxx3, image2);  

	 
	//variational_refinement_processors[0]->calcUV(I0C1, I1C1,
	//	image2[0], image2[1]);

	//cv::merge(image2, 2, sxx3);

	////glBindTexture(GL_TEXTURE_2D, m_textureU_x_y);
	////glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width >> level, m_texture_height >> level, GL_RG, GL_FLOAT, sxx3.ptr());
	//  

	glBindTexture(GL_TEXTURE_2D, m_textureU_x_y);
	//if (imageArray != NULL)
	//{

	cv::merge(image2, 2, sxx4);

	glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, m_texture_width >> level, m_texture_height >> level, GL_RG, GL_FLOAT, sxx4.data);

	//if (level == 0)
	//{
	//	cv::Mat mag, ang;
	//	cv::Mat hsv_split[3], hsv;
	//	cv::Mat rgb;
	//	cv::cartToPolar(image2[0], image2[1], mag, ang, true);
	//	cv::normalize(mag, mag, 0, 1, cv::NORM_MINMAX);
	//	hsv_split[0] = ang;
	//	hsv_split[1] = mag;
	//	hsv_split[2] = cv::Mat::ones(ang.size(), ang.type());
	//	cv::merge(hsv_split, 3, hsv);
	//	cv::cvtColor(hsv, rgb, cv::COLOR_HSV2BGR);
	//	cv::imshow("flowvar", rgb);
	//}

	 

}

void gFlow::variRef(int level)
{
	glBindTexture(GL_TEXTURE_2D, m_texture_total_flow);
	glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, m_texture_width >> level, m_texture_height >> level, GL_RG, GL_FLOAT, zeroValues.data());

	glBindTexture(GL_TEXTURE_2D, m_texture_dup_dvp);
	glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, m_texture_width >> level, m_texture_height >> level, GL_RGBA, GL_FLOAT, zeroValues.data());

	
	flipflop = 0; 

	variRefineProg.use();

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_prepareBuffersID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureI0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_textureI1); // CHANGE ME TO I1 EXT 

	glBindImageTexture(0, m_textureU_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
	//glBindImageTexture(2, m_textureWarp_I1, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	 
	glBindImageTexture(2, m_textureI_mix_diff_warp, level, GL_TRUE, NULL, GL_WRITE_ONLY, GL_RGBA8);
	 
	glUniform1i(m_level_var_ID, level);

	int compWidth = divup(m_texture_width >> level, 32);
	int compHeight = divup(m_texture_height >> level, 32);

	glDispatchCompute(compWidth, compHeight, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	 
	//if (level == 0)
	//{

	//	std::vector<uchar> vari_vec((m_texture_height >> level) * (m_texture_width >> level) * 4 * 3);
	//	cv::Mat vari_Mat = cv::Mat(m_texture_height >> level, m_texture_width >> level, CV_8UC4);

	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureI_mix_diff_warp);
	//	glGetTexImage(GL_TEXTURE_2D_ARRAY, level, GL_RGBA, GL_UNSIGNED_BYTE, vari_vec.data());// this is importnant, you are using GL_RED_INTEGETER!!!!
	//	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	//	memcpy_s(vari_Mat.data, vari_vec.size() * sizeof(uchar) / 3, vari_vec.data() + vari_vec.size() * 1 / 3, vari_vec.size() * sizeof(uchar) / 3);

	//	cv::imshow("varima", vari_Mat);
	//} 
	 


	// SPIN UP THE SOBEL FOR ALL THESE LOVELY IMAGES

	sobelProg.use();

	glBindImageTexture(0, m_textureI_mix_diff_warp, level, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
	glBindImageTexture(4, m_textureI_grads_mix_diff_x_y, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_getGradientsID);
	glUniform1i(m_imageType_cov_ID, 0); // image type 0 = color rgba8ui

	glDispatchCompute(compWidth, compHeight, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);



	glBindImageTexture(0, m_textureI_mix_diff_warp, level, GL_FALSE, 1, GL_READ_ONLY, GL_RGBA8);
	glBindImageTexture(4, m_textureI_grads_mix_diff_x_y, level, GL_FALSE, 1, GL_WRITE_ONLY, GL_RG32F);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_getGradientsID);
	glUniform1i(m_imageType_cov_ID, 0); // image type 0 = color rgba8ui

	glDispatchCompute(compWidth, compHeight, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	 


	glBindImageTexture(1, m_textureI_grads_mix_diff_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
	glBindImageTexture(5, m_textureI_second_grads_mix_diff_x_y, level, GL_TRUE, NULL, GL_WRITE_ONLY, GL_RG32F);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_getGradientsID);
	glUniform1i(m_imageType_cov_ID, 2); // image type 2 = color rg32f

	glDispatchCompute(compWidth, compHeight, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	  

	for (int inner_iter = 0; inner_iter < level + 1; inner_iter++) // this is a problem, < 1 or < level + 1
	{

		sobelProg.use();

		glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_getSmoothnessID);

		if (inner_iter == 0)
		{
			glBindImageTexture(1, m_textureU_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F); // SHOULD THIS BE DELTA FLOW
		}
		else
		{
			glBindImageTexture(1, m_texture_total_flow, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F); // SHOULD THIS BE DELTA FLOW

		}

		glBindImageTexture(6, m_texture_smoothness_weight, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

		glUniform1i(m_imageType_cov_ID, 1); // image type 0 = color rgba8ui

		glDispatchCompute(compWidth, compHeight, 1);


		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		variRefineProg.use();
		glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_computeSmoothnessTermID);

		//	m_texture_smoothness_terms
		glBindImageTexture(5, m_texture_smoothness_weight, level, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
		glBindImageTexture(6, m_texture_smoothness_terms, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

		glDispatchCompute(compWidth, compHeight, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		  
		variRefineProg.use();   
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_buffer_refinement_data_terms);

		glUniform1i(m_flipflopID, flipflop);    
		glUniform1i(m_iter_var_ID, inner_iter);  
		  
		glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_computeDataTermID);
		//glBindImageTexture(0, m_textureU_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F); // SHOULD THIS BE DELTA FLOW 
		glBindImageTexture(1, m_texture_dup_dvp, level, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); // SHOULD THIS BE DELTA FLOW 
		//glBindImageTexture(7, m_texture_total_flow, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F); // SHOULD THIS BE DELTA FLOW
		glBindImageTexture(3, m_textureI_grads_mix_diff_x_y, level, GL_TRUE, NULL, GL_READ_ONLY, GL_RG32F);
		glBindImageTexture(4, m_textureI_second_grads_mix_diff_x_y, level, GL_TRUE, NULL, GL_READ_ONLY, GL_RG32F);
		


		glDispatchCompute(compWidth, compHeight, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		variRefineProg.use(); 

		 
		  
		glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_computeSORID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_buffer_refinement_data_terms);

		glUniform1i(m_flipflopID, flipflop);
		glUniform1i(m_iter_var_ID, inner_iter);
		glUniform1i(m_level_var_ID, level);

		//	m_texture_smoothness_terms
		glBindImageTexture(0, m_textureU_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F); // SHOULD THIS BE DELTA FLOW

		glBindImageTexture(1, m_texture_dup_dvp, level, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(7, m_texture_total_flow, level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F); // SHOULD THIS BE DELTA FLOW
		glBindImageTexture(6, m_texture_smoothness_terms, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);

		glDispatchCompute(compWidth, compHeight, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		flipflop = !flipflop;
		 
		
		 
	}
	    

	if (level == 0)
	{
		////cv::Mat deltasuv = cv::Mat(m_texture_height >> level, m_texture_width >> level, CV_32FC4);


		////glActiveTexture(GL_TEXTURE0);
		////glBindTexture(GL_TEXTURE_2D, m_texture_dup_dvp);
		////glGetTexImage(GL_TEXTURE_2D, level, GL_RGBA, GL_FLOAT, deltasuv.data);// this is importnant, you are using GL_RED_INTEGETER!!!!
		////glBindTexture(GL_TEXTURE_2D, 0);

		////cv::Mat imdel[4];
		////cv::split(deltasuv, imdel);

		////cv::imshow("dv", imdel[1]);  
		////cv::imshow("du", imdel[2]);

			//cv::Mat totflow = cv::Mat(m_texture_height >> level, m_texture_width >> level, CV_32FC2);

			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, m_texture_total_flow);
			//glGetTexImage(GL_TEXTURE_2D, level, GL_RG, GL_FLOAT, totflow.data);// this is importnant, you are using GL_RED_INTEGETER!!!!
			//glBindTexture(GL_TEXTURE_2D, 0);

			//cv::Mat tofl[2];
			//cv::split(totflow, tofl);  

			////cv::imshow("wwee", tofl[0] - tofl[1]);
			////cv::imshow("dwerwev", tofl[1]); 

		 //  
			//cv::Mat mag, ang; 
			//cv::Mat hsv_split[3], hsv;
			//cv::Mat rgb; 
			//cv::cartToPolar(tofl[0], tofl[1], mag, ang, true);
			//cv::normalize(mag, mag, 0, 1, cv::NORM_MINMAX);
			//hsv_split[0] = ang;
			//hsv_split[1] = mag;
			//hsv_split[2] = cv::Mat::ones(ang.size(), ang.type()); 
			//cv::merge(hsv_split, 3, hsv);
			//cv::cvtColor(hsv, rgb, cv::COLOR_HSV2BGR);
			//cv::imshow("totflowrgb", rgb);

	} 
	 
	//if (level > 0)
	//{
	//	variRefineProg.use();
	//	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_resizeID);

	//	glBindImageTexture(7, m_texture_total_flow, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F); // SHOULD THIS BE DELTA FLOW
	//	glBindImageTexture(0, m_textureU_x_y, level - 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F); // SHOULD THIS BE DELTA FLOW

	//	glDispatchCompute(compWidth, compHeight, 1);
	//	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//}
	//else
	//{
	glCopyImageSubData(m_texture_total_flow, GL_TEXTURE_2D, level, 0, 0, 0,
		m_textureU_x_y, GL_TEXTURE_2D, level, 0, 0, 0,
		m_texture_width >> level, m_texture_height >> level , 1);
	//}


	glMemoryBarrier(GL_ALL_BARRIER_BITS);


	 

	  
	    
}      
 
void gFlow::sumFlowTexture()
{
	disFlowProg.use();
	glm::ivec2 imageSize(m_texture_width, m_texture_height);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_sumFlowTextureID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureU_x_y);
	glUniform2iv(m_texSizeID, 1, glm::value_ptr(imageSize));

	glBindImageTexture(0, m_sumFlow, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG32F);

	int compWidth = divup(m_texture_width, 4);
	int compHeight = divup(m_texture_height, 4);

	glDispatchCompute(compWidth, compHeight, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void gFlow::swapTextures()
{
	// SWAP TEXTURES 
	glCopyImageSubData(m_textureI1, GL_TEXTURE_2D, 0, 0, 0, 0,
		m_textureI0, GL_TEXTURE_2D, 0, 0, 0, 0,
		m_texture_width, m_texture_height, 1);

}  
   
//void gDisOptFlow::resizeFlow(int level)
//{
//	disFlowProg.use();
//  
//	glUniform1i(m_level_dis_ID, level);
//  
//	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_resizeID); 
//
//	glBindImageTexture(2, m_textureU_x_y, level, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
//	glBindImageTexture(1, m_textureU_x_y, level - 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
//
//   
//	int compWidth = divup(m_texture_width >> level, 4);
//	int compHeight = divup(m_texture_height >> level, 4); 
// 
//	glDispatchCompute(compWidth, compHeight, 1);
//
//	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
//
//
//}   
     
void gFlow::jumpFloodCalc()
{
	glBindTexture(GL_TEXTURE_2D, m_texture_jfa_0);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RG_INTEGER, GL_INT, zeroValuesInt.data());

	jumpFloodProg.use();

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_jfaInitID);

	glBindImageTexture(0, m_texture_jfa_0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32I);
	
	int compWidth = divup(m_texture_width, 32);
	int compHeight = divup(m_texture_width, 32); 
	 
	glDispatchCompute(compWidth, compHeight, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	cv::Mat prejfaMat = cv::Mat(m_texture_height, m_texture_width, CV_32SC2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture_jfa_0);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RG_INTEGER, GL_INT, prejfaMat.data);
	glBindTexture(GL_TEXTURE_2D, 0);
	 
	//cv::imshow("dens1", sxx3);   

	//cv::Mat image1[2];
	//cv::split(prejfaMat, image1);
	//cv::imshow("prejfa0", image1[0] * 255);
	//cv::imshow("prejfa1", image1[1] * 255);



	int iterCount = 0;
	for (int jumpLength = m_texture_width / 2; jumpLength > 0; jumpLength >>= 1, iterCount++)
	{
		glUniform1i(m_jumpID, jumpLength);
		glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_jfaUpdateID);

		if (iterCount % 2 == 0)
		{
			glBindImageTexture(0, m_texture_jfa_0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32I);
			glBindImageTexture(1, m_texture_jfa_1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32I);
		}
		else 
		{
			glBindImageTexture(0, m_texture_jfa_1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32I);
			glBindImageTexture(1, m_texture_jfa_0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32I);
		}

		compWidth = divup(m_texture_width, 32);
		compHeight = divup(m_texture_height, 32);

		glDispatchCompute(compWidth, compHeight, 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
	



	 

	cv::Mat jfaMat = cv::Mat(m_texture_height, m_texture_width, CV_32SC2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture_jfa_1);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RG_INTEGER, GL_INT, jfaMat.data);
	glBindTexture(GL_TEXTURE_2D, 0);

	//cv::imshow("dens1", sxx3);  

	cv::Mat image2[2];
	cv::split(jfaMat, image2);
	//cv::imshow("jfa0", image2[0] * 25);
	//cv::imshow("jfa1", image2[1] * 25);

	cv::Mat outfloat = image2[0].mul(image2[1]);
	outfloat.convertTo(outfloat, CV_32FC1, 0.000005f); 

	cv::imshow("jfa2", outfloat);

}

bool gFlow::calc(bool useInfrared) 
{   
	  
	glBeginQuery(GL_TIME_ELAPSED, timeQuery[0]);

	for (int level = 0; level <= m_numLevels; level++)
		computeSobel(level, useInfrared);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);  
	   
	glBindTexture(GL_TEXTURE_2D, m_textureU_x_y);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_texture_width, m_texture_height, GL_RG, GL_FLOAT, zeroValues.data());
	glGenerateMipmap(GL_TEXTURE_2D);
	  
	double totalTime = 0;

	for (int level = m_numLevels - 1; level > -1; level--)
	{

		//glBindTexture(GL_TEXTURE_2D, m_textureS_x_y);
		//glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, (m_texture_width / m_patch_stride) >> level,( m_texture_height / m_patch_stride) >> level, GL_RG, GL_FLOAT, zeroValues.data());

		makePatches(level);




		patchInverseSearch(level, useInfrared);




		densification(level);

		medianFilter(level);

		//if (level == 0)
		//{
		//	calcStandardDeviation(level);
		//}


		//if (level > 1)
		//{
		//	variRef(level);  // mine, broken ish  slower   

		//}



		//if (level > -1) // dont need to densify finest level?   
		//{
		//	variationalRefinement(level); // opencv, slow
		//}
		//	//variRef(level);  // mine, broken ish  slower   


		//	//variRef(level);  // mine, broken ish  slower   

		//	//
		if (level == 0)
		{
			cv::Mat ssdMat = cv::Mat((m_texture_height >> level) / m_patch_stride, (m_texture_width >> level) / m_patch_stride, CV_32FC1);


			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_textureTest);
			glGetTexImage(GL_TEXTURE_2D, level, GL_RED, GL_FLOAT, ssdMat.data);
			glBindTexture(GL_TEXTURE_2D, 0);

			cv::namedWindow("ssd", WINDOW_NORMAL);
			cv::imshow("ssd", ssdMat);
			cv::waitKey(1);
		}




		//}
		  

		 
		                                     
	}        
	glMemoryBarrier(GL_ALL_BARRIER_BITS); 

	sumFlowTexture();

	// COPY FLOW TOP LEVEL AND MAKE MIPMAPS FOR INITIAL FLOW
	//for (int level = 0; level < m_numLevels; level++)
	//{
	glEndQuery(GL_TIME_ELAPSED);
	GLuint available = 0;
	while (!available) {
		glGetQueryObjectuiv(timeQuery[0], GL_QUERY_RESULT_AVAILABLE, &available);
	}

	// elapsed time in nanoseconds
	GLuint64 elapsed;
	glGetQueryObjectui64vEXT(timeQuery[0], GL_QUERY_RESULT, &elapsed);
	totalTime += elapsed / 1000000.0;


	int level = 0;
		glCopyImageSubData(m_textureU_x_y, GL_TEXTURE_2D, level, 0, 0, 0,
			               m_texture_init_U_x_y, GL_TEXTURE_2D, level, 0, 0, 0,
			               m_texture_width >> level, m_texture_height >> level, 1);

		glBindTexture(GL_TEXTURE_2D, m_texture_init_U_x_y);
		glGenerateMipmap(GL_TEXTURE_2D);
		 

	//}  
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	
	m_timeElapsed = totalTime;
	if (swapCounter % 1 == 0)
		swapTextures(); 
	  
	//I1im.copyTo(I0im);

	swapCounter++; 
	  
	return false; 
}
void gFlow::setupEKF()
{
	img = cv::Mat(600, 800, CV_8UC3);
	KF.resize(18);
	mousev.resize(18);
	kalmanv.resize(18);
	measurement = cv::Mat(2, 1, CV_32FC1); measurement.setTo(Scalar(0));

	for (int i = 0; i < KF.size(); i++)
	{
		KF[i].init(4, 2, 2);


		// intialization of KF...
		KF[i].transitionMatrix = (Mat_<float>(4, 4) << 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1);

		KF[i].statePre.at<float>(0) = 0;
		KF[i].statePre.at<float>(1) = 0;
		KF[i].statePre.at<float>(2) = 0;
		KF[i].statePre.at<float>(3) = 0;
		setIdentity(KF[i].measurementMatrix); // 4 cols 2 rows
		setIdentity(KF[i].processNoiseCov, Scalar::all(1)); // 4 cols 4 rows
		setIdentity(KF[i].measurementNoiseCov, Scalar::all(1)); // 2 cols 2 rows
		setIdentity(KF[i].errorCovPost, Scalar::all(.1)); // 4 cols 4 rows
													   // Image to show mouse tracking

		mousev[i].clear();
		kalmanv[i].clear();
	}

}
void gFlow::track(GLuint bufferToTrack, int numPoints)
{
	disFlowProg.use();
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_trackPoseID);


	// bind flow texture 
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, m_sumFlow);

	glm::ivec2 imageSize(m_texture_width, m_texture_height);
	glUniform2iv(m_texSizeID, 1, glm::value_ptr(imageSize));


	// bind ssbo for tracked points
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, bufferToTrack);

	int compWidth = divup(numPoints * 3, 4);
	int compHeight = divup(1, 4);
	glDispatchCompute(compWidth, compHeight, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

/// KALMAN FILTERING
	//std::vector<float> outputData(18 * 3);
	//std::vector<float> outputDataEst;

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferToTrack);
	//void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(outputData.data(), outputData.size() * sizeof(float), ptr, outputData.size() * sizeof(float));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	//outputDataEst = outputData;
	//int idx = 0;
	//for (int i = 0; i < KF.size(); i++, idx += 3)
	//{
	//	Mat temp = KF[i].statePost;
	//	// First predict, to update the internal statePre variable
	//	Mat prediction = KF[i].predict();
	//	Point predictPt(prediction.at<float>(0), prediction.at<float>(1));

	//	// Get mouse point
	//	//GetCursorPos(&mousePos);
	//	measurement(0) = outputData[idx];
	//	measurement(1) = outputData[idx + 1];

	//	// The update phase 
	//	Mat estimated = KF[i].correct(measurement);

	//	Point statePt(estimated.at<float>(0), estimated.at<float>(1));
	//	Point measPt(measurement(0), measurement(1));


	//	mousev[i].push_back(measPt);
	//	kalmanv[i].push_back(statePt);
	//	outputDataEst[idx] = statePt.x;
	//	outputDataEst[idx+1] = statePt.y;


	//	drawCross(statePt, Scalar(255, 255, 255), 5);
	//	//drawCross(measPt, Scalar(0, 0, 255), 5);

	//	/*for (int i = 0; i < mousev.size() - 1; i++)
	//	line(img, mousev[i], mousev[i + 1], Scalar(255, 255, 0), 1);*/

	//	int startDrawVal = kalmanv[i].size() > 100 ? kalmanv[i].size() - 20 : 0;

	//	for (int j = startDrawVal; j < kalmanv[i].size() - 1; j++)
	//		line(img, kalmanv[i][j], kalmanv[i][j + 1], Scalar(0, 155, 255), 1);

	//}

	//// plot points
	//imshow("kalman", img);
	//img = Scalar::all(0);

	//cv::waitKey(1);

	//glBindBuffer(GL_ARRAY_BUFFER, bufferToTrack);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, outputDataEst.size() * sizeof(float), outputDataEst.data());



}


void gFlow::track()
{   
	 
	disFlowProg.use();  
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_trackID);
	
	glUniform1i(m_trackWidthID, m_texture_height * 2);
	 
	// bind flow texture 
	glActiveTexture(GL_TEXTURE6);

	glm::ivec2 imageSize(m_texture_width, m_texture_height);
	glUniform2iv(m_texSizeID, 1, glm::value_ptr(imageSize));

	
	glBindTexture(GL_TEXTURE_2D, m_textureU_x_y);
	// bind ssbo for tracked points
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_trackedPointsBuffer);

	int compWidth = divup(m_texture_height, 4);
	int compHeight = divup(m_texture_height, 4);
	glDispatchCompute(compWidth, compHeight, 1);  
	 
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//std::vector<float> outputData(m_texture_height * m_texture_height *2);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_trackedPointsBuffer);
	//void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(outputData.data(), outputData.size() * sizeof(float), ptr, outputData.size() * sizeof(float));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//cv::Mat blank = cv::Mat(424, 512, CV_8UC4);
	//for (int i = 0; i < m_texture_height * m_texture_height * 2; i += 2)
	//{

	//	cv::circle(blank, cv::Point2f(outputData[i], outputData[i + 1]), 2, cv::Scalar(255, 128, 128, 255));

	//}    

	//cv::imshow("tracked", blank);
	 

	// 



	//cv::Mat blank = cv::Mat(1080, 1920, CV_8UC4);  
	//  
	//if (swapCounter % 100 == 0)
	//{
	//	for (int i = 0; i < 100; i++)
	//	{
	//		for (int j = 0; j < 100; j++)
	//		{
	//			m_trackedPoints[j * 100 + i].x = (1920 >> 1) - 500 + (i * 10);
	//			m_trackedPoints[j * 100 + i].y = (1080 >> 1) - 500 + (j * 10);

	//		}
	//	}
	//}  
	// 
	//cv::Mat totflow = cv::Mat(m_texture_height >> 0, m_texture_width >> 0, CV_32FC2);

	////glActiveTexture(GL_TEXTURE0);
	////glBindTexture(GL_TEXTURE_2D, m_texture_dup_dvp); // there is a problem with totral flow, the x and y comp are the same!!!!!
	////glGetTexImage(GL_TEXTURE_2D, 1, GL_RG, GL_FLOAT, totflow.data);// this is importnant, you are using GL_RED_INTEGETER!!!!
	////glBindTexture(GL_TEXTURE_2D, 0);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureU_x_y); // there is a problem with totral flow, the x and y comp are the same!!!!!
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, totflow.data);// this is importnant, you are using GL_RED_INTEGETER!!!!
	//glBindTexture(GL_TEXTURE_2D, 0);

	//float readFlow[2];  
	//glBindTexture(GL_TEXTURE_2D, m_textureU_x_y);

	//for (int i = 0; i < 10000; i++)
	//{

	//	cv::Point2f currFlow = totflow.at<cv::Point2f>(int(m_trackedPoints[i].y), int(m_trackedPoints[i].x));
	//	m_trackedPoints[i] += currFlow;

	//	m_trackedPoints[i].x = m_trackedPoints[i].x < 0 ? 0 : m_trackedPoints[i].x;
	//	m_trackedPoints[i].y = m_trackedPoints[i].y < 0 ? 0 : m_trackedPoints[i].y;

	//	m_trackedPoints[i].x = m_trackedPoints[i].x > blank.cols - 10 ? blank.cols - 10 : m_trackedPoints[i].x;
	//	m_trackedPoints[i].y = m_trackedPoints[i].y > blank.rows - 10 ? blank.rows - 10 : m_trackedPoints[i].y;

	//	cv::circle(blank, cv::Point2f(m_trackedPoints[i].x, m_trackedPoints[i].y), 2, cv::Scalar(255, 128, 128, 255));

	//}

	//cv::imshow("tracked", blank);


	//return 1;

	 
}


void gFlow::buildQuadtree()
{
	//cv::Mat sp_noise = cv::Mat::zeros(512, 512, CV_32F);
	//cv::randu(sp_noise, 0.0f, 1.0f);
	//sp_noise.at<float>(1, 1) = 1.0f;

	//sp_noise.at<float>(56, 56) = 1.0f;
	//cv::Mat black = sp_noise < 0.999f;
	//cv::Mat white = sp_noise > 0.999f;

	//cv::Mat sp_img = cv::Mat(512, 512, CV_32F);
	//sp_img.setTo(1.0f, white);
	//sp_img.setTo(0.0f, black);

	//cv::imshow("noise", sp_img);
	//cv::waitKey(1);
	//cv::Mat sp_img = cv::Mat(512, 512, CV_32F);



	//glBindTexture(GL_TEXTURE_2D, m_texture_hpOriginalData);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 512, 512, GL_RED, GL_FLOAT, sp_noise.data);
	//	glBeginQuery(GL_TIME_ELAPSED, timeQuery[0]);


	// we need to pass the shader the distance image as a float image binding point, or texture unit (prob texture unit)

	// run the hpDisriminator subroutine
	hpQuadtreeProg.use();

	glBindImageTexture(1, m_texture_hpQuadtree, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture_hpQuadtree);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, m_texture_hpOriginalData);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_textureI0_grad_x_y);

	glm::uvec3 nthreads = GLHelper::divup(glm::uvec3((1 << m_numberHPLevels), (1 << m_numberHPLevels), 1), glm::uvec3(8,8,1));

	glUniform1i(m_quadThreshID, m_valA);


	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_hpDiscriminatorID);
	glDispatchCompute(nthreads.x, nthreads.y, nthreads.z);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);


	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, m_texture_hpQuadtree);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, sp_img.data);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//cv::namedWindow("noise", WINDOW_NORMAL);

	//cv::imshow("noise", sp_img  + 1.0f);
	//cv::waitKey(1);

	// run the hpBuilder subroutine on each level

	/// Do other levels of histopyr
	for (int i = 0; i < m_numberHPLevels; i++)
	{
		glm::uvec3 nthreads = GLHelper::divup(glm::uvec3(((1 << m_numberHPLevels) >> i) / 2, ((1 << m_numberHPLevels) >> i) / 2, 1), glm::uvec3(8, 8, 1));

		glUniform1i(m_hpLevelID, i);

		glBindImageTexture(1, m_texture_hpQuadtree, i+1, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

		glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_hpBuilderID);
		glDispatchCompute(nthreads.x, nthreads.y, nthreads.z);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}

	//glEndQuery(GL_TIME_ELAPSED);
	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(timeQuery[0], GL_QUERY_RESULT_AVAILABLE, &available);
	//}

	// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(timeQuery[0], GL_QUERY_RESULT, &elapsed);
	//double totalTime = elapsed / 1000000.0;

	//std::cout << "time " << totalTime << " ms" << std::endl;

	std::vector<float> sumData(1, 3);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture_hpQuadtree);
	glGetTexImage(GL_TEXTURE_2D, m_numberHPLevels, GL_RED, GL_FLOAT, sumData.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	//std::cout << "sum " << sumData[0] << std::endl;
	m_quadlistCount = sumData[0];

	// read the top value from the mipmap level

	// trigger that may threads on quadlist subroutine

	hpQuadListProg.use();
	nthreads = GLHelper::divup(glm::uvec3(sumData[0], 1, 1), glm::uvec3(32, 1, 1));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture_hpQuadtree);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bufferQuadlist);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_traverseHPLevelID);
	glUniform1ui(m_totalSumID, sumData[0]);
	glUniform1ui(m_cutoffID, m_cutoff);

	glDispatchCompute(nthreads.x, nthreads.y, nthreads.z);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);






	//std::vector<float> posData(sumData[0] * 4);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferPos);
	//void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(posData.data(), posData.size() * sizeof(float), ptr, posData.size() * sizeof(float));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//cv::Mat qtreeImage = cv::Mat(2048, 2048, CV_32F);
	//for (int i = 0; i < posData.size(); i+=4)
	//{

	//	int lod = posData[i + 2];
	//	int quadSideLength = std::pow(2, lod);

	//	int originX = posData[i] * quadSideLength;
	//	int originY = posData[i + 1] * quadSideLength;

	//	cv::line(qtreeImage, cv::Point2i(originX, originY), cv::Point2i(originX + quadSideLength, originY), 1.0);
	//	cv::line(qtreeImage, cv::Point2i(originX + quadSideLength, originY), cv::Point2i(originX + quadSideLength, originY + quadSideLength), 1.0);
	//	cv::line(qtreeImage, cv::Point2i(originX + quadSideLength, originY + quadSideLength), cv::Point2i(originX, originY + quadSideLength), 1);
	//	cv::line(qtreeImage, cv::Point2i(originX, originY + quadSideLength), cv::Point2i(originX, originY), 1);

	//}
	//cv::Mat noiseIm = (sp_img + 1.0f);

	//cv::Mat im1, im2;
	//cv::normalize(noiseIm, im1, 0, 1, cv::NORM_MINMAX);
	//cv::flip(im1, im1f, 1);
	//cv::flip(im1f, im1, 0);




	//cv::normalize(qtreeImage, im2, 0, 1, cv::NORM_MINMAX);



	//cv::Mat outIm = cv::Mat(512, 512, CV_8UC3);
	//cv::addWeighted(im1, 1.5, im2, 0.3, 0.0, outIm);
	//cv::namedWindow("quadtree", WINDOW_NORMAL);
	//cv::imshow("quadtree", qtreeImage);
	//cv::waitKey(1);

}