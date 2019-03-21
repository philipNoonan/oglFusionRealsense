#include "gFusion.h"



gFusion::~gFusion()
{
}

void gFusion::queryDeviceLimits()
{
	GLint sizeSSBO;
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &sizeSSBO);
	std::cout << "GL_MAX_SHADER_STORAGE_BLOCK_SIZE is " << sizeSSBO << " bytes." << std::endl;

	GLint sizeCWGI;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &sizeCWGI);
	std::cout << "GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS is " << sizeCWGI << " invocations." << std::endl;

	GLint sizeShared;
	glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &sizeShared);
	std::cout << "GL_MAX_COMPUTE_SHARED_MEMORY_SIZE is " << sizeShared << " bytes." << std::endl;


	GLint size3D;
	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &size3D);
	std::cout << "GL_MAX_3D_TEXTURE_SIZE is " << size3D << " texels." << std::endl;

}

void gFusion::compileAndLinkShader()
{
	try {
		depthToVertProg.compileShader("shaders/depthToVert.cs");
		depthToVertProg.link();
		
		vertToNormProg.compileShader("shaders/vertToNorm.cs");
		vertToNormProg.link();

		trackProg.compileShader("shaders/track.cs");
		trackProg.link();

		reduceProg.compileShader("shaders/reduce.cs");
		reduceProg.link();

		integrateProg.compileShader("shaders/integrate.cs");
		integrateProg.link();

		raycastProg.compileShader("shaders/raycast.cs");
		raycastProg.link();

		marchingCubesProg.compileShader("shaders/marchingCubes.cs");
		marchingCubesProg.link();

		prefixSumProg.compileShader("shaders/prefixSum.cs");
		prefixSumProg.link();

		trackSDFProg.compileShader("shaders/trackSDF.cs");
		trackSDFProg.link();

		reduceSDFProg.compileShader("shaders/reduceSDF.cs");
		reduceSDFProg.link();

		mipProg.compileShader("shaders/mip.cs");
		mipProg.link();
		
		
		helpersProg.compileShader("shaders/helpers.cs");
		helpersProg.link();


		
	}
	catch (GLSLProgramException &e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	glGenQueries(7, query);

}

void gFusion::setLocations()
{
	m_invkID = glGetUniformLocation(depthToVertProg.getHandle(), "invK");
	m_camPamsID = glGetUniformLocation(depthToVertProg.getHandle(), "camPams");
	m_imageTypeID = glGetUniformLocation(depthToVertProg.getHandle(), "imageType");
	m_depthScaleID = glGetUniformLocation(depthToVertProg.getHandle(), "depthScale");

	//std::cout << "invK " << m_invkID << std::endl;
	//std::cout << "camPams " << m_camPamsID << std::endl;
	//std::cout << "imageType " << m_imageTypeID << std::endl;
	//std::cout << "depthScale " << m_depthScaleID << std::endl;

	m_viewID_t = glGetUniformLocation(trackProg.getHandle(), "view");
	m_TtrackID = glGetUniformLocation(trackProg.getHandle(), "Ttrack");
	m_distThresh_t = glGetUniformLocation(trackProg.getHandle(), "dist_threshold");
	m_normThresh_t = glGetUniformLocation(trackProg.getHandle(), "normal_threshold");

	//std::cout << "view " << m_viewID_t << std::endl;
	//std::cout << "Ttrack " << m_TtrackID << std::endl;
	//std::cout << "dist_threshold " << m_distThresh_t << std::endl;
	//std::cout << "normal_threshold " << m_normThresh_t << std::endl;

	m_imageSizeID = glGetUniformLocation(reduceProg.getHandle(), "imageSize");

	//std::cout << "imageSize " << m_imageSizeID << std::endl;


	m_invTrackID = glGetUniformLocation(integrateProg.getHandle(), "invTrack");
	m_KID = glGetUniformLocation(integrateProg.getHandle(), "Kmat");
	m_muID = glGetUniformLocation(integrateProg.getHandle(), "mu");
	m_maxWeightID = glGetUniformLocation(integrateProg.getHandle(), "maxWeight");
	m_volDimID = glGetUniformLocation(integrateProg.getHandle(), "volDim");
	m_volSizeID = glGetUniformLocation(integrateProg.getHandle(), "volSize");
	m_imageTypeID_i = glGetUniformLocation(integrateProg.getHandle(), "imageType");
	m_depthScaleID_i = glGetUniformLocation(integrateProg.getHandle(), "depthScale");

	//std::cout << "invTrack " << m_invTrackID << std::endl;
	//std::cout << "Kmat " << m_KID << std::endl;
	//std::cout << "mu " << m_muID << std::endl;
	//std::cout << "maxWeight " << m_maxWeightID << std::endl;
	//std::cout << "volDim " << m_volDimID << std::endl;
	//std::cout << "volSize " << m_volSizeID << std::endl;
	//std::cout << "imageType " << m_imageTypeID_i << std::endl;
	//std::cout << "depthScale " << m_depthScaleID_i << std::endl;

	m_viewID_r = glGetUniformLocation(raycastProg.getHandle(), "view");
	m_nearPlaneID = glGetUniformLocation(raycastProg.getHandle(), "nearPlane");
	m_farPlaneID = glGetUniformLocation(raycastProg.getHandle(), "farPlane");
	m_stepID = glGetUniformLocation(raycastProg.getHandle(), "step");
	m_largeStepID = glGetUniformLocation(raycastProg.getHandle(), "largeStep");
	m_volDimID_r = glGetUniformLocation(raycastProg.getHandle(), "volDim");
	m_volSizeID_r = glGetUniformLocation(raycastProg.getHandle(), "volSize");

	//HELPERS
	m_helpersSubroutineID = glGetSubroutineUniformLocation(helpersProg.getHandle(), GL_COMPUTE_SHADER, "performHelperFunction");
	m_resetVolumeID = glGetSubroutineIndex(helpersProg.getHandle(), GL_COMPUTE_SHADER, "resetVolume");
	m_trackPointsToVertsID = glGetSubroutineIndex(helpersProg.getHandle(), GL_COMPUTE_SHADER, "trackPointsToVerts");
	m_volSizeID_h = glGetUniformLocation(helpersProg.getHandle(), "volSize");
	m_buffer2DWidthID = glGetUniformLocation(helpersProg.getHandle(), "buffer2DWidth");
	m_invKID_h = glGetUniformLocation(helpersProg.getHandle(), "invK");

	// PREFIX SUMS
	m_prefixSumSubroutineID = glGetSubroutineUniformLocation(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "getPrefixSum");
	m_resetSumsArrayID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "resetSumsArray");

	m_forEachGroupID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "forEachGroup");
	m_forEveryGroupID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "forEveryGroup");
	m_forFinalIncrementalSumID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "forFinalIncrementalSum");

	// MARCHING CUBES
	m_marchingCubesSubroutineID = glGetSubroutineUniformLocation(marchingCubesProg.getHandle(), GL_COMPUTE_SHADER, "marchingCubesSubroutine");
	m_classifyVoxelID = glGetSubroutineIndex(marchingCubesProg.getHandle(), GL_COMPUTE_SHADER, "launchClassifyVoxel");
	m_compactVoxelsID = glGetSubroutineIndex(marchingCubesProg.getHandle(), GL_COMPUTE_SHADER, "launchCompactVoxels");
	m_generateTrianglesID = glGetSubroutineIndex(marchingCubesProg.getHandle(), GL_COMPUTE_SHADER, "launchGenerateTriangles");

	m_gridSizeID = glGetUniformLocation(marchingCubesProg.getHandle(), "gridSize");
	m_gridSizeShiftID = glGetUniformLocation(marchingCubesProg.getHandle(), "gridSizeShift");
	m_gridSizeMaskID = glGetUniformLocation(marchingCubesProg.getHandle(), "gridSizeMask");
	m_isoValueID = glGetUniformLocation(marchingCubesProg.getHandle(), "isoValue");
	m_numVoxelsID = glGetUniformLocation(marchingCubesProg.getHandle(), "numVoxels");
	m_activeVoxelsID = glGetUniformLocation(marchingCubesProg.getHandle(), "activeVoxels");
	m_maxVertsID = glGetUniformLocation(marchingCubesProg.getHandle(), "maxVerts");
	m_voxelSizeID = glGetUniformLocation(marchingCubesProg.getHandle(), "voxelSize");

	// TRACKSDF
	m_TtrackID_t = glGetUniformLocation(trackSDFProg.getHandle(), "Ttrack");
	m_volDimID_t = glGetUniformLocation(trackSDFProg.getHandle(), "volDim");
	m_volSizeID_t = glGetUniformLocation(trackSDFProg.getHandle(), "volSize");
	m_cID = glGetUniformLocation(trackSDFProg.getHandle(), "c");
	m_epsID = glGetUniformLocation(trackSDFProg.getHandle(), "eps");
	m_imageSizeID_t_sdf = glGetUniformLocation(trackSDFProg.getHandle(), "imageSize");
	// REDUCE SDF
	m_imageSizeID_sdf = glGetUniformLocation(reduceSDFProg.getHandle(), "imageSize");

	//INTENSITY PROJECTION
	m_viewID_m = glGetUniformLocation(mipProg.getHandle(), "view");
	m_nearPlaneID_m = glGetUniformLocation(mipProg.getHandle(), "nearPlane");
	m_farPlaneID_m = glGetUniformLocation(mipProg.getHandle(), "farPlane");
	m_stepID_m = glGetUniformLocation(mipProg.getHandle(), "step");
	m_largeStepID_m = glGetUniformLocation(mipProg.getHandle(), "largeStep");
	m_volDimID_m = glGetUniformLocation(mipProg.getHandle(), "volDim");
	m_volSizeID_m = glGetUniformLocation(mipProg.getHandle(), "volSize");

	//listmode.resize(402653184, 0);
	//glGenBuffers(1, &m_lmbuff_0);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lmbuff_0);
	//glBufferStorage(GL_SHADER_STORAGE_BUFFER, 402653184 * sizeof(int), &listmode[0], GL_DYNAMIC_STORAGE_BIT);

	//glGenBuffers(1, &m_lmbuff_1);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lmbuff_1);
	//glBufferStorage(GL_SHADER_STORAGE_BUFFER, 402653184 * sizeof(int) / 2, &listmode[0], GL_DYNAMIC_STORAGE_BIT);

	//glBufferData(GL_SHADER_STORAGE_BUFFER, 402653184 * sizeof(int), &listmode[0], GL_DYNAMIC_DRAW);




}

GLuint gFusion::createTexture(GLenum target, int levels, int w, int h, int d, GLint internalformat)
{
	GLuint texid;
	glGenTextures(1, &texid);
	glBindTexture(target, texid);

	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
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
	else if (target == GL_TEXTURE_3D)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexStorage3D(target, levels, internalformat, w, h, d);
	}

	return texid;
}

void gFusion::initTextures()
{
	if (m_usingFloatDepth)
	{
		m_textureDepth = createTexture(GL_TEXTURE_2D, 3, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_R32F);
	}
	else
	{
		m_textureDepth = createTexture(GL_TEXTURE_2D, 3, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_R16);
		// https://www.khronos.org/opengl/wiki/Normalized_Integer
		// I think that we have to use normailsed images here, because we cannot use ints for mip maps, they just dont work
	}

	//////////////////
	//   look at normailsed integers and https://www.opengl.org/discussion_boards/showthread.php/165655-Question-about-texture-using-glsl
	//////////////////
	//GLuint m_textureDepthTest;
	//m_textureDepthTest = createTexture(GL_TEXTURE_2D, 3, 64, 64, 1, GL_R16);
	//glBindTexture(GL_TEXTURE_2D, m_textureDepthTest);

	//std::vector<uint16_t> initData(64 * 64, 2);
	//std::vector<uint16_t> outData(16 * 16, 1);

	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 64, GL_RED_INTEGER, GL_UNSIGNED_SHORT, initData.data());
	//glGenerateMipmap(GL_TEXTURE_2D);


	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureDepthTest);
	//glGetTexImage(GL_TEXTURE_2D, 1, GL_RED_INTEGER, GL_UNSIGNED_SHORT, outData.data());
	//glBindTexture(GL_TEXTURE_2D, 0);
	//


	m_textureColor = createTexture(GL_TEXTURE_2D, 1, m_color_width, m_color_height, 1, GL_RGBA8UI);
	m_textureVertex = createTexture(GL_TEXTURE_2D, 3, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F);
	m_textureNormal = createTexture(GL_TEXTURE_2D, 3, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F);
	m_textureReferenceVertex = createTexture(GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F);
	m_textureReferenceNormal = createTexture(GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F);
	m_textureDifferenceVertex = createTexture(GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_R32F);
	m_textureTestImage = createTexture(GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F);
	m_textureTrackImage = createTexture(GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F);
	m_textureTest = createTexture(GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F);
	m_textureMip = createTexture(GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F);


}

void gFusion::initVolume()
{
	//GLenum err = glGetError();
	m_textureVolume = createTexture(GL_TEXTURE_3D, 1, configuration.volumeSize.x, configuration.volumeSize.y, configuration.volumeSize.z, GL_RG16I);
	//err = glGetError();
}

void gFusion::Reset(glm::mat4 pose, bool deleteFlag)
{
	if (deleteFlag)
	{
		glDeleteTextures(1, &m_textureVolume);
		initVolume();
	}
	else
	{
		resetVolume();
	}

	resetTimes();

	poseLibrary.resize(0);
	resetPose(pose);
}

void gFusion::resetVolume()
{
	helpersProg.use();

	int compWidth;
	int compHeight;

	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16I);
	//glBindImageTexture(1, m_textureTestImage, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_resetVolumeID);
	glUniform3fv(m_volSizeID_h, 1, glm::value_ptr(configuration.volumeSize));


	compWidth = divup(configuration.volumeSize.x, 32);
	compHeight = divup(configuration.volumeSize.y, 32);

	glDispatchCompute(compWidth, compHeight, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//cv::Mat test = cv::Mat(configuration.volumeSize.x, configuration.volumeSize.y, CV_32FC1);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureTestImage);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, test.data);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//

	//cv::imshow("test", test);


}

void gFusion::resetPose(glm::mat4 pose)
{
	m_pose = pose;

	std::memcpy(m_pose_eig.data(), glm::value_ptr(pose), 16 * sizeof(float));

	m_cumTwist << 0, 0, 0, 0, 0, 0;

}


void gFusion::allocateBuffers()
{

	// REDUCTION BUFFER OBJECT
	m_reduction.resize(configuration.depthFrameSize.x * configuration.depthFrameSize.y);
	size_t reductionSize = configuration.depthFrameSize.x * configuration.depthFrameSize.y * (sizeof(GLint) + (sizeof(GLfloat) * 7)); // this is the size of one reduction element 1 int + 1 float + 6 float

	glGenBuffers(1, &m_buffer_reduction);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_buffer_reduction);
	glBufferData(GL_SHADER_STORAGE_BUFFER, reductionSize, &m_reduction[0], GL_STATIC_DRAW);

	// OUTPUT DATA FROM REDUCTION BUFFER OBJECT
	m_outputdata.resize(32 * 8);
	glGenBuffers(1, &m_buffer_outputdata);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_buffer_outputdata);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 32 * 8 * sizeof(float), &m_outputdata[0], GL_STATIC_DRAW);

	//// MARCHING CUBES BUFFERS
	//size_t memSize = sizeof(GLuint) * mcubeConfiguration.numVoxels;
	//size_t memSizeVec4 = sizeof(float) * 4 * mcubeConfiguration.maxVerts;

	//glGenBuffers(1, &m_bufferVoxelVerts);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_bufferVoxelVerts);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, memSize, NULL, GL_DYNAMIC_DRAW);
	//
	//glGenBuffers(1, &m_bufferVoxelOccupied);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_bufferVoxelOccupied);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, memSize, NULL, GL_DYNAMIC_DRAW);

	//glGenBuffers(1, &m_bufferVoxelOccupiedScan);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_bufferVoxelOccupiedScan);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, memSize, NULL, GL_DYNAMIC_DRAW);
	//
	//glGenBuffers(1, &m_bufferCompactedVoxelArray);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_bufferCompactedVoxelArray);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, memSize, NULL, GL_DYNAMIC_DRAW);
	//
	//glGenBuffers(1, &m_bufferVoxelVertsScan);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_bufferVoxelVertsScan);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, memSize, NULL, GL_DYNAMIC_DRAW);

	//glGenBuffers(1, &m_bufferPos);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_bufferPos);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, memSizeVec4, NULL, GL_DYNAMIC_DRAW);

	//glGenBuffers(1, &m_bufferNorm);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, m_bufferNorm);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, memSizeVec4, NULL, GL_DYNAMIC_DRAW);

	//glGenBuffers(1, &m_bufferPrefixSumByGroup);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, m_bufferPrefixSumByGroup);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, memSize / 1024, NULL, GL_DYNAMIC_DRAW);



	size_t reductionSDFSize = configuration.depthFrameSize.y * configuration.depthFrameSize.x * (sizeof(GLint) + sizeof(GLfloat) * 8); // this is the size of one reduction element 1 int + 1 float + 1 float + 6 float
	glGenBuffers(1, &m_bufferSDFReduction);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, m_bufferSDFReduction);
	glBufferData(GL_SHADER_STORAGE_BUFFER, reductionSDFSize, NULL, GL_STATIC_DRAW);

	// OUTPUT DATA FROM REDUCTION BUFFER OBJECT
	glGenBuffers(1, &m_bufferSDFoutputdata);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, m_bufferSDFoutputdata);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 32 * 8 * sizeof(float), NULL, GL_STATIC_DRAW);


	// TESTS
	glGenBuffers(1, &m_buffer_testInput);
	glGenBuffers(1, &m_buffer_testOutput);
	glGenBuffers(1, &m_buffer_testMidput);




	m_trackedPoints3D.resize(configuration.depthFrameSize.y * configuration.depthFrameSize.y * 3);
	int xOffTrack = configuration.depthFrameSize.y / 2;
	int yOffTrack = configuration.depthFrameSize.y / 2;
	int xSpacing = 2 * xOffTrack / configuration.depthFrameSize.y;
	int ySpacing = 2 * yOffTrack / configuration.depthFrameSize.y;

	for (int i = 0; i < configuration.depthFrameSize.y * 3; i += 3)
	{
		for (int j = 0; j < configuration.depthFrameSize.y; j++)
		{
			m_trackedPoints3D[j * configuration.depthFrameSize.y * 2 + i] = ((int)configuration.depthFrameSize.x >> 1) - xOffTrack + (i / 3) * xSpacing;
			m_trackedPoints3D[j * configuration.depthFrameSize.y * 2 + i + 1] = ((int)configuration.depthFrameSize.x >> 1) - yOffTrack + j * ySpacing;
			m_trackedPoints3D[j * configuration.depthFrameSize.y * 2 + i + 2] = 0.0f;


		}
	}

	glGenBuffers(1, &m_trackedPoints3DBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_trackedPoints3DBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_trackedPoints3D.size() * sizeof(float), m_trackedPoints3D.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, m_trackedPoints3DBuffer);




}

void gFusion::resetTimes()
{
	raycastTime = 0.0;
	marchingCubesTime = 0.0;
	trackTime = 0.0;
	reduceTime = 0.0;
	integrateTime = 0.0;
	trackSDFTime = 0.0;
	reduceSDFTime = 0.0;
}

void gFusion::getTimes(float arr[])
{
	arr[1] = raycastTime;
	arr[2] = marchingCubesTime;
	arr[3] = trackTime;
	arr[4] = reduceTime;
	arr[5] = integrateTime;
	arr[6] = trackSDFTime;
	arr[7] = reduceSDFTime;

}

void gFusion::printTimes()
{


	std::cout << "raycast " << raycastTime << " ms " << std::endl;
	std::cout << "marchingcubes " << marchingCubesTime << " ms " << std::endl;
	std::cout << "track " << trackTime << " ms " << std::endl;
	std::cout << "reduce " << reduceTime << " ms " << std::endl;
	std::cout << "integrate " << integrateTime << " ms " << std::endl;

}



void gFusion::testPrefixSum()
{
	std::vector<uint32_t> testInput, testOutput, testMidput;
	testInput.resize(128 * 128 * 128, 1);
	testOutput.resize(testInput.size());
	testMidput.resize(testInput.size() / 1024);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, m_buffer_testInput);
	glBufferData(GL_SHADER_STORAGE_BUFFER, testInput.size() * sizeof(uint32_t), &testInput[0], GL_STATIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, m_buffer_testOutput);
	glBufferData(GL_SHADER_STORAGE_BUFFER, testOutput.size() * sizeof(uint32_t), &testOutput[0], GL_STATIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, m_buffer_testMidput);
	glBufferData(GL_SHADER_STORAGE_BUFFER, testMidput.size() * sizeof(uint32_t), &testMidput[0], GL_STATIC_DRAW);

	GLuint query[1];
	glGenQueries(1, query);
	int xthreads = divup(testInput.size(), 1024); 
	prefixSumProg.use();
	glBeginQuery(GL_TIME_ELAPSED, query[0]);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_resetSumsArrayID);
	glDispatchCompute(xthreads, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_forEachGroupID);
	glDispatchCompute(xthreads, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glEndQuery(GL_TIME_ELAPSED);

	int xthreads2 = divup(xthreads, 1024);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_forEveryGroupID);
	glDispatchCompute(xthreads2, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_forFinalIncrementalSumID);
	glDispatchCompute(xthreads, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	std::vector<uint32_t> outputPrefixSum, midPrefixSum;
	outputPrefixSum.resize(testInput.size());
	midPrefixSum.resize(testInput.size() /1024);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer_testOutput);
	void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(outputPrefixSum.data(), outputPrefixSum.size() * sizeof(uint32_t), ptr, outputPrefixSum.size() * sizeof(uint32_t));

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);



	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer_testMidput);
	void *ptr1 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(midPrefixSum.data(), midPrefixSum.size() * sizeof(uint32_t), ptr1, midPrefixSum.size() * sizeof(uint32_t));

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	std::cout << "midprefix " << std::endl;
	for (auto i : midPrefixSum)
		std::cout << i << " ";
	std::cout << std::endl;

	int j = 0, k = -1;
	std::cout << "outprefix " << std::endl;

	//for (auto i : outputPrefixSum)
	//{
	//	std::cout << i << " ";
	//}
	//std::cout << std::endl;
	
	GLuint available = 0;
	while (!available) {
		glGetQueryObjectuiv(query[0], GL_QUERY_RESULT_AVAILABLE, &available);
	}

	// elapsed time in nanoseconds
	GLuint64 elapsed;
	glGetQueryObjectui64vEXT(query[0], GL_QUERY_RESULT, &elapsed);

	double seconds = elapsed / 1000000000.0;

	std::cout << "time elapsed " << seconds << " sec" << std::endl;

}

void gFusion::depthToVertex(float * depthArray)
{
	int compWidth;
	int compHeight; 

	// gltexsubimage2d  
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureDepth);
	if (depthArray != NULL)
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, configuration.depthFrameSize.x, configuration.depthFrameSize.y, GL_RED, GL_FLOAT, depthArray);
	}
	glGenerateMipmap(GL_TEXTURE_2D);

	//std::vector<float> testvec(480 * 848 / 2, 10);


	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureDepth);
	//glGetTexImage(GL_TEXTURE_2D, 1, GL_RED, GL_FLOAT, testvec.data());
	//glBindTexture(GL_TEXTURE_2D, 0);
	//

	//cv::Mat lvl1d = cv::Mat(480 / 2, 848 / 2, CV_32FC1, testvec.data());

	//cv::imshow("lvl1!d", lvl1d);

	depthToVertProg.use();

	for (int i = 0; i < 3; i++) // here 3 is the number of mipmap levels
	{
		compWidth = divup((int)configuration.depthFrameSize.x >> i, 32);
		compHeight = divup((int)configuration.depthFrameSize.y >> i, 32);

		glm::mat4 invK = GLHelper::getInverseCameraMatrix(m_camPamsDepth / float(1 << i));
		glBindImageTexture(0, m_textureDepth, i, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
		glBindImageTexture(2, m_textureVertex, i, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glUniformMatrix4fv(m_invkID, 1, GL_FALSE, glm::value_ptr(invK));
		glUniform4fv(m_camPamsID, 1, glm::value_ptr(m_camPamsDepth));
		glUniform1i(m_imageTypeID, 1); // 1 == type float
		glUniform1f(m_depthScaleID, 1000); // 1000 == each depth unit == 1 mm

		glDispatchCompute(compWidth, compHeight, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}



}

void gFusion::depthToVertex(uint16_t * depthArray)
{
	int compWidth;
	int compHeight;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureDepth);
	if (depthArray != NULL) 
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, configuration.depthFrameSize.x, configuration.depthFrameSize.y, GL_RED, GL_UNSIGNED_SHORT, depthArray);
	}
	glGenerateMipmap(GL_TEXTURE_2D);

	//std::vector<uint16_t> testvec(480 * 848 / 2, 10);


	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureDepth);
	//glGetTexImage(GL_TEXTURE_2D, 1, GL_RED, GL_UNSIGNED_SHORT, testvec.data());
	//glBindTexture(GL_TEXTURE_2D, 0);
	//

	//cv::Mat lvl1d = cv::Mat(480 / 2, 848 / 2, CV_16SC1, testvec.data()); 
	//cv::Mat converted;
	//lvl1d.convertTo(converted, CV_32FC1);
	//cv::imshow("lvl1!d", (converted * 100.0 / 1000000.0 - 0.1) / (0.2 - 0.1));



	depthToVertProg.use();

	for (int i = 0; i < 3; i++) // here 3 is the number of mipmap levels
	{
		compWidth = divup((int)configuration.depthFrameSize.x >> i, 32);
		compHeight = divup((int)configuration.depthFrameSize.y >> i, 32);
		

		glm::mat4 invK = GLHelper::getInverseCameraMatrix(m_camPamsDepth / float(1 << i));
		glBindImageTexture(1, m_textureDepth, i, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
		glBindImageTexture(2, m_textureVertex, i, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glUniformMatrix4fv(m_invkID, 1, GL_FALSE, glm::value_ptr(invK));
		//glUniform4fv(m_camPamsID, 1, glm::value_ptr(m_camPamsDepth));
		glUniform1i(m_imageTypeID, 0); // 0 == type short
		glUniform1f(m_depthScaleID, m_depthUnit / 1000000.0f); // 1000 == each depth unit == 1 mm

		glDispatchCompute(compWidth, compHeight, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}

	//std::vector<float> testvec(480 * 848, 10);


	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureVertex);
	//glGetTexImage(GL_TEXTURE_2D, 2, GL_RGBA, GL_FLOAT, testvec.data());
	//glBindTexture(GL_TEXTURE_2D, 0);
	//

}


void gFusion::vertexToNormal()
{
	int compWidth;
	int compHeight; 

	vertToNormProg.use();

	for (int i = 0; i < 3; i++) // here 3 is the number of mipmap levels
	{
		glBindImageTexture(0, m_textureVertex, i, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(1, m_textureNormal, i, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

		compWidth = divup((int)configuration.depthFrameSize.x >> i, 32);
		compHeight = divup((int)configuration.depthFrameSize.y >> i, 32);

		glDispatchCompute(compWidth, compHeight, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

	}
}

void gFusion::showNormals()
{
#ifdef USE_OPENCV
	cv::Mat lvl0 = cv::Mat(480, 848, CV_32FC4);
	cv::Mat lvl1 = cv::Mat(480 / 2, 848 / 2, CV_32FC4);
	cv::Mat lvl2 = cv::Mat(480 / 4, 848 / 4, CV_32FC4);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureNormal);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, lvl0.data);
	glGetTexImage(GL_TEXTURE_2D, 1, GL_RGBA, GL_FLOAT, lvl1.data);
	glGetTexImage(GL_TEXTURE_2D, 2, GL_RGBA, GL_FLOAT, lvl2.data);
	glBindTexture(GL_TEXTURE_2D, 0);
	

	cv::imshow("lvl0!", lvl0);
	cv::imshow("lvl1!", lvl1);
	cv::imshow("lvl2!", lvl2);



	lvl0 = cv::Mat(480, 848, CV_32FC4);
	lvl1 = cv::Mat(480 / 2, 848 / 2, CV_32FC4);
	lvl2 = cv::Mat(480 / 4, 848 / 4, CV_32FC4);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureVertex);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, lvl0.data);
	glGetTexImage(GL_TEXTURE_2D, 1, GL_RGBA, GL_FLOAT, lvl1.data);
	glGetTexImage(GL_TEXTURE_2D, 2, GL_RGBA, GL_FLOAT, lvl2.data);
	glBindTexture(GL_TEXTURE_2D, 0);
	

	cv::Mat image0[4];
	cv::split(lvl0, image0);
	cv::Mat image1[4];
	cv::split(lvl1, image1);
	cv::Mat image2[4];
	cv::split(lvl2, image2);

	cv::imshow("lvl0", image0[2]);
	cv::imshow("lvl1", image1[2]);
	cv::imshow("lvl2", image2[2]);
#endif

}

void gFusion::showRaycast()
{
	/*cv::Mat points = cv::Mat(424, 512, CV_32FC4);
	cv::Mat norms = cv::Mat(424, 512, CV_32FC4);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureReferenceVertex);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, points.data);
	glBindTexture(GL_TEXTURE_2D, 0);
	

	cv::Mat pointsDepth[4];
	cv::split(points, pointsDepth);
	cv::imshow("points", pointsDepth[2]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureReferenceNormal);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, norms.data);
	glBindTexture(GL_TEXTURE_2D, 0);
	

	cv::imshow("normals", norms);
*/

}

void gFusion::showDifference()
{
	/*cv::Mat dif = cv::Mat(424, 512, CV_32FC1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureDifferenceVertex);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, dif.data);
	glBindTexture(GL_TEXTURE_2D, 0);
	

	cv::imshow("difs", dif* 10.0f);*/
}

void gFusion::showTrack()
{
	/*cv::Mat trk = cv::Mat(424, 512, CV_32FC4);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureTrackImage);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, trk.data);
	glBindTexture(GL_TEXTURE_2D, 0);
	

	cv::cvtColor(trk, trk, CV_RGBA2BGRA);

	cv::imshow("trk", trk);*/
}

bool gFusion::Track()
{
	glBeginQuery(GL_TIME_ELAPSED, query[0]);

	bool tracked = false;
	glm::mat4 oldPose = m_pose;

	float alignmentEnergy;

	// here we will loop through the layers and number of iterations per layer
	for (int level = configuration.iterations.size() - 1; level >= 0; --level)
	{
		//int level = 0;
		for (int iteration = 0; iteration < configuration.iterations[level]; iteration++)
		{
			std::vector<float> b;
			std::vector<float> C;

			track(level);
			reduce(level);
			getReduction(b, C, alignmentEnergy);


			//std::cout << "level " << level << " iteration " << iteration << " AE:" << alignmentEnergy << std::endl;

			Eigen::Matrix<float, 6, 1> b_icp(b.data());
			Eigen::Matrix<float, 6, 6, Eigen::RowMajor> C_icp(C.data());

			Eigen::Matrix<double, 6, 1> result;
			Eigen::Matrix<double, 6, 6, Eigen::RowMajor> dC_icp = C_icp.cast<double>();
			Eigen::Matrix<double, 6, 1> db_icp = b_icp.cast<double>();


			//result = dC_icp.ldlt().solve(db_icp);
			//result = dC_icp.fullPivLu().solve(db_icp);

			Eigen::JacobiSVD<Eigen::MatrixXd> svd(dC_icp, Eigen::ComputeFullU | Eigen::ComputeFullV);
			result = svd.solve(db_icp); // TODO CHECK THIS WORKS, SHOULD WE MAKE A BACK SUB SOLVER?


			glm::mat4 delta = glm::eulerAngleXYZ(result(3), result(4), result(5));
			delta[3][0] = result(0);
			delta[3][1] = result(1);
			delta[3][2] = result(2);

			m_pose = delta * m_pose;

			if (result.norm() < 1e-4 && result.norm() != 0)
				break;
		}
	}



	////std::cout << alignmentEnergy << std::endl;
	if (alignmentEnergy > 1.0e-3f || alignmentEnergy == 0)
	{
		m_pose = oldPose;
	}
	else
	{
		tracked = true;
		updatePoseFinder();
	}

	m_alignmentEnergy = alignmentEnergy;
	//tracked = true;

	//m_pose = oldPose;

	//std::cout << " p2p " << std::endl;
	//std::cout << m_pose[0][0] << " " << m_pose[1][0] << " " << m_pose[2][0] << " " << m_pose[3][0] << std::endl;
	//std::cout << m_pose[0][1] << " " << m_pose[1][1] << " " << m_pose[2][1] << " " << m_pose[3][1] << std::endl;
	//std::cout << m_pose[0][2] << " " << m_pose[1][2] << " " << m_pose[2][2] << " " << m_pose[3][2] << std::endl;
	//std::cout << m_pose[0][3] << " " << m_pose[1][3] << " " << m_pose[2][3] << " " << m_pose[3][3] << std::endl;

	glEndQuery(GL_TIME_ELAPSED);
	GLuint available = 0;
	while (!available) {
		glGetQueryObjectuiv(query[0], GL_QUERY_RESULT_AVAILABLE, &available);
	}
	// elapsed time in nanoseconds
	GLuint64 elapsed;
	glGetQueryObjectui64vEXT(query[0], GL_QUERY_RESULT, &elapsed);
	trackTime = elapsed / 1000000.0;

	return tracked;
}

void gFusion::track(int layer)
{

	glm::mat4 invK = GLHelper::getInverseCameraMatrix(m_camPamsDepth);
	glm::mat4 oldPose = pose;
	glm::mat4 projectReference = GLHelper::getCameraMatrix(m_camPamsDepth) * glm::inverse(m_pose);

	int compWidth;
	int compHeight;

	trackProg.use();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_buffer_reduction);


	glUniformMatrix4fv(m_viewID_t, 1, GL_FALSE, glm::value_ptr(projectReference));
	glUniformMatrix4fv(m_TtrackID, 1, GL_FALSE, glm::value_ptr(m_pose));
	glUniform1f(m_distThresh_t, configuration.dist_threshold);
	glUniform1f(m_normThresh_t, configuration.normal_threshold);

	glBindImageTexture(0, m_textureVertex, layer, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(1, m_textureNormal, layer, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glBindImageTexture(2, m_textureReferenceVertex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); // the raycasted images do not need to be scaled, since the mipmaped verts are projected back to 512*424 image space resolution in the shader
	glBindImageTexture(3, m_textureReferenceNormal, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	
	glBindImageTexture(4, m_textureDifferenceVertex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
	glBindImageTexture(5, m_textureTrackImage, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	
	compWidth = divup((int)configuration.depthFrameSize.x >> layer, 32); // right bitshift does division by powers of 2
	compHeight = divup((int)configuration.depthFrameSize.y >> layer, 32);

	glDispatchCompute(compWidth, compHeight, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//std::vector<float> testvec(480 * 848*4, 10);


	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureReferenceVertex);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, testvec.data());
	//glBindTexture(GL_TEXTURE_2D, 0);
	//



}

void gFusion::reduce(int layer)
{


	//glBeginQuery(GL_TIME_ELAPSED, query[1]);

	glm::ivec2 imageSize = glm::ivec2((int)configuration.depthFrameSize.x >> layer, (int)configuration.depthFrameSize.y >> layer);

	reduceProg.use();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_buffer_reduction);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_buffer_outputdata);

	glUniform2iv(m_imageSizeID, 1, glm::value_ptr(imageSize));
	//glBindImageTexture(0, m_textureOutputData, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	glDispatchCompute(8, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//glEndQuery(GL_TIME_ELAPSED);
	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query[1], GL_QUERY_RESULT_AVAILABLE, &available);
	//}
	//// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query[1], GL_QUERY_RESULT, &elapsed);
	//reduceTime = elapsed / 1000000.0;

}

Eigen::Matrix4f Twist(const Eigen::Matrix<double, 6, 1> &xi)
{
	//glm::mat4 M;
	//M[0][0] = 0.0f;		M[1][0] = -xi(2);	M[2][0] = xi(1);	M[3][0] = xi(3);
	//M[0][1] = xi(2);	M[1][1] = 0.0f;		M[2][1] = -xi(1);	M[3][1] = xi(4);
	//M[0][2] = -xi(1);	M[1][2] = xi(0);	M[2][2] = 0.0f;		M[3][2] = xi(5);
	//M[0][3] = 0.0f;		M[1][3] = 0.0f;		M[2][3] = 0.0f;		M[3][3] = 0.0f;

	Eigen::Matrix4f M;
	//M << 0.0, -xi(2), xi(1), xi(3),
	//	xi(2), 0.0, -xi(0), xi(4),
	//	-xi(1), xi(0), 0.0, xi(5),
	//	0.0, 0.0, 0.0, 0.0;

	M << 0.0, -xi(2), xi(1), xi(3),
		xi(2), 0.0, -xi(0), xi(4),
		-xi(1), xi(0), 0.0, xi(5),
		0.0, 0.0, 0.0, 0.0;

	//Eigen::Matrix4f Mt = M.transpose();

	return M;
};

bool gFusion::TrackSDF() {

	glBeginQuery(GL_TIME_ELAPSED, query[5]);


	bool tracked = false;
	glm::mat4 oldPose = m_pose;

	glm::mat4 sdfPose = m_pose;

	Eigen::Matrix<float, 4, 4, Eigen::ColMajor> m_pose_eig_prev = m_pose_eig;

	Eigen::Vector3d trans;
	Eigen::Matrix3d rot;

	float alignmentEnergy;

	Eigen::Matrix<double, 6, 1> result;
	result << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
	//glm::mat4 twistMat = Twist(result);
	Eigen::Matrix<double, 6, 1> result_prev = result;
	//Eigen::Matrix4d twistMat = Twist(result);


	// here we will loop through the layers and number of iterations per layer
	for (int level = configuration.iterations.size() - 1; level >= 0; --level)
	{
	//	if (tracked == true)
	//		break;
	//	int level = 0;
		for (int iteration = 0; iteration < configuration.iterations[level]; iteration++)
		{
			Eigen::Matrix4f camToWorld = Twist(result).exp() * m_pose_eig_prev;
			//std::cout << "level " << level << " iter " << iteration << " result " << result.transpose() << std::endl;
			std::vector<float> b;
			std::vector<float> C;

			trackSDF(level, camToWorld);
			reduceSDF(level);

			Eigen::Matrix<double, 6, 6> A0 = Eigen::Matrix<double, 6, 6>::Zero();
			Eigen::Matrix<double, 6, 1> b0 = Eigen::Matrix<double, 6, 1>::Zero();

			//getPreRedu(A0, b0);

			getSDFReduction(b, C, alignmentEnergy);

			//std::cout << alignmentEnergy << std::endl;

			Eigen::Matrix<float, 6, 1> b_icp(b.data());
			Eigen::Matrix<float, 6, 6, Eigen::RowMajor> C_icp(C.data());

			//Eigen::Matrix<double, 6, 1> result;
			Eigen::Matrix<double, 6, 6, Eigen::RowMajor> dC_icp = C_icp.cast<double>();
			Eigen::Matrix<double, 6, 1> db_icp = b_icp.cast<double>();

			double scaling = 1.0 / dC_icp.maxCoeff();

			dC_icp *= scaling;
			db_icp *= scaling;

			dC_icp = dC_icp + (double(iteration)*0.5)*Eigen::MatrixXd::Identity(6, 6);

			//Eigen::JacobiSVD<Eigen::MatrixXd> svd(dC_icp, Eigen::ComputeFullU | Eigen::ComputeFullV);
			//result = svd.solve(db_icp); // TODO CHECK THIS WORKS, SHOULD WE MAKE A BACK SUB SOLVER?
			//Eigen::JacobiSVD<Eigen::MatrixXd> svd(dC_icp, Eigen::ComputeFullU | Eigen::ComputeFullV);
			//result = result - svd.solve(db_icp); // TODO CHECK THIS WORKS, SHOULD WE MAKE A BACK SUB SOLVER?

			result = result - dC_icp.ldlt().solve(db_icp);

			//std::cout  std::endl;

			Eigen::Matrix<double, 6, 1> Change = result - result_prev;
			double Cnorm = Change.norm();

			result_prev = result;
			//std::cout << "AE: " << alignmentEnergy << " snorm : " << Cnorm << " vec " << result.transpose() << std::endl;

			//std::cout << "cnrom :" << Cnorm << std::endl;
			if (alignmentEnergy != 0 && Cnorm < 1e-4)
			{
				//std::cout << "tracked!!! iteration " << iteration << " level " << level << " AE: " << alignmentEnergy << " snorm : " << Cnorm << " vec " << result.transpose() << std::endl;

				//std::cout << "tracked!!! iteration " << iteration << std::endl;
				tracked = true;
				break;
			}


			//m_pose_eig = Twist(result).exp() * m_pose_eig;




			//result = dC_icp.inverse() * db_icp;

			//Eigen::Affine3d aff = eigen_utils::direct_exponential_map(result, 1.0);




			//rot <<	m_pose[0][0], m_pose[1][0], m_pose[2][0],
			//		m_pose[0][1], m_pose[1][1], m_pose[2][1],
			//		m_pose[0][2], m_pose[1][2], m_pose[2][2];

			//trans << m_pose[3][0], m_pose[3][1], m_pose[3][2];

			//Eigen::Matrix3d postRot = aff.rotation().transpose() * rot;
			//Eigen::Vector3d postTrans = trans - aff.rotation().transpose() * aff.translation();

		//	std::cout << postRot << std::endl;
		//	std::cout << postTrans << std::endl;
			//Eigen::JacobiSVD<Eigen::MatrixXd> svd(dC_icp, Eigen::ComputeFullU | Eigen::ComputeFullV);
			//result = svd.solve(db_icp); // TODO CHECK THIS WORKS, SHOULD WE MAKE A BACK SUB SOLVER?
			//result = dC_icp.ldlt().solve(db_icp);

			//glm::mat4 delta = glm::eulerAngleXYZ(result(3), result(4), result(5));
			//delta[3][0] = result(0);
			//delta[3][1] = result(1);
			//delta[3][2] = result(2);

			//sdfPose = delta * sdfPose;

			/*m_pose[0][0] = postRot(0, 0); m_pose[1][0] = postRot(0, 1); m_pose[2][0] = postRot(0, 2); m_pose[3][0] = postTrans(0);
			m_pose[0][1] = postRot(1, 0); m_pose[1][1] = postRot(1, 1); m_pose[2][1] = postRot(1, 2); m_pose[3][1] = postTrans(1);
			m_pose[0][2] = postRot(2, 0); m_pose[1][2] = postRot(2, 1); m_pose[2][2] = postRot(2, 2); m_pose[3][2] = postTrans(2);
			m_pose[0][3] = 0;		  m_pose[1][3] = 0;			m_pose[2][3] = 0;	      m_pose[3][3] = 1.0f;

			if (result(0, 0) < 0.001
				&& result(1, 0) < 0.001
				&& result(2, 0) < 0.001
				&& result(3, 0) < 0.001
				&& result(4, 0) < 0.001
				&& result(5, 0) < 0.001) {
				std::cout << "STOP Gauss Newton at level: " << level << " it: " << iteration << std::endl;
				tracked = true;
				break;

			}*/

			//m_pose = sdfPose;

		//	std::cout << result.norm() << std::endl;
			//if (result.norm() < 1e-2 && result.norm() != 0)
			//{
			//	//std::cout << "converged" << std::endl;
			//	tracked = true;
			//	break;

			//}



			}

		}

		if (std::isnan(result.sum())) result << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;


		if (tracked)
		{
			m_pose_eig = Twist(result).exp() * m_pose_eig_prev;

			std::memcpy(glm::value_ptr(m_pose), m_pose_eig.data(), 16 * sizeof(float));



			//updatePoseFinder();

			m_cumTwist += result;
		}
		else
		{

			m_pose_eig = m_pose_eig_prev;
			m_pose = oldPose;
		}

		m_alignmentEnergy = alignmentEnergy;

	//std::cout << " sdf " << std::endl;
	//std::cout << m_pose[0][0] << " " << m_pose[1][0] << " " << m_pose[2][0] << " " << m_pose[3][0] << std::endl;
	//std::cout << m_pose[0][1] << " " << m_pose[1][1] << " " << m_pose[2][1] << " " << m_pose[3][1] << std::endl;
	//std::cout << m_pose[0][2] << " " << m_pose[1][2] << " " << m_pose[2][2] << " " << m_pose[3][2] << std::endl;
	//std::cout << m_pose[0][3] << " " << m_pose[1][3] << " " << m_pose[2][3] << " " << m_pose[3][3] << std::endl;


	

	//if (alignmentEnergy > 0.5f || alignmentEnergy == 0)
	//{
	//	m_pose = oldPose;
	//}
	//else
	//{
	//	tracked = true;
	//	updatePoseFinder();
	//}


		glEndQuery(GL_TIME_ELAPSED);

		GLuint available = 0;
		while (!available) {
			glGetQueryObjectuiv(query[5], GL_QUERY_RESULT_AVAILABLE, &available);
		}
		// elapsed time in nanoseconds
		GLuint64 elapsed;
		glGetQueryObjectui64vEXT(query[5], GL_QUERY_RESULT, &elapsed);
		trackSDFTime = elapsed / 1000000.0;


	return tracked;
}

void gFusion::trackSDF(int layer, Eigen::Matrix4f camToWorld)
{


	trackSDFProg.use();

	int xthreads, ythreads;
	xthreads = divup((int)configuration.depthFrameSize.x >> layer, 32); // right bitshift does division by powers of 2
	ythreads = divup((int)configuration.depthFrameSize.y >> layer, 32);
	glm::ivec2 imageSize = glm::ivec2((int)configuration.depthFrameSize.x >> layer, (int)configuration.depthFrameSize.y >> layer);

	// bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_textureVolume);

	// bind images
	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16I);
	glBindImageTexture(1, m_textureVertex, layer, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(2, m_textureNormal, layer, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glBindImageTexture(3, m_textureTest, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(4, m_textureTrackImage, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	// bind buffers
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, m_bufferSDFReduction);


	// set uniforms
	//glUniformMatrix4fv(m_TtrackID_t, 1, GL_FALSE, glm::value_ptr(m_pose));
	glUniformMatrix4fv(m_TtrackID_t, 1, GL_FALSE, camToWorld.data());
	glUniform2iv(m_imageSizeID_t_sdf, 1, glm::value_ptr(imageSize));

	glUniform1f(m_cID, 0.02f * 0.1f);
	glUniform1f(m_epsID, 10e-9);
	glUniform3fv(m_volDimID_t, 1, glm::value_ptr(configuration.volumeDimensions));
	glUniform3fv(m_volSizeID_t, 1, glm::value_ptr(configuration.volumeSize));

	glDispatchCompute(xthreads, ythreads, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);


	//if (layer == 0)
	//{
	//	cv::Mat testIm(480, 848, CV_32FC4);
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, m_textureTest);
	//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, testIm.data);
	//	glBindTexture(GL_TEXTURE_2D, 0);

	//	cv::imshow("testim1", testIm);
	//	cv::waitKey(1);
	//}





}

void gFusion::reduceSDF(int layer)
{
	//glBeginQuery(GL_TIME_ELAPSED, query[6]);

	glm::ivec2 imageSize = glm::ivec2((int)configuration.depthFrameSize.x >> layer, (int)configuration.depthFrameSize.y >> layer);

	reduceSDFProg.use();

	// bind buffers
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, m_bufferSDFReduction);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, m_bufferSDFoutputdata);

	glUniform2iv(m_imageSizeID_sdf, 1, glm::value_ptr(imageSize));
	//glBindImageTexture(0, m_textureOutputData, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	glDispatchCompute(8, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//glEndQuery(GL_TIME_ELAPSED);

	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query[6], GL_QUERY_RESULT_AVAILABLE, &available);
	//}
	//// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query[6], GL_QUERY_RESULT, &elapsed);
	//reduceSDFTime = elapsed / 1000000.0;

}

void gFusion::getReduction(std::vector<float>& b, std::vector<float>& C, float &alignmentEnergy)
{
	outputData.resize(32 * 8);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer_outputdata);
	void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(outputData.data(), outputData.size() * sizeof(float), ptr, outputData.size() * sizeof(float));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//for (auto i : outputData)
	//	std::cout << i << " ";

	//std::cout << std::endl;
	// output data has 32 cols and 8 rows
	// sum the 8 rows up to the top row

	for (int row = 1; row < 8; row++)
	{
		for (int col = 0; col < 32; col++)
		{
			outputData[col + 0 * 32] = outputData[col + row * 32];
		}
	}


	std::vector<float>::const_iterator first0 = outputData.begin() + 1;
	std::vector<float>::const_iterator last0 = outputData.begin() + 28;

	std::vector<float> vals(first0, last0);

	std::vector<float>::const_iterator first1 = vals.begin();
	std::vector<float>::const_iterator last1 = vals.begin() + 6;
	std::vector<float>::const_iterator last2 = vals.begin() + 6 + 21;

	std::vector<float> bee(first1, last1);
	std::vector<float> Cee = makeJTJ(std::vector<float>(last1, last2));

	b = bee;
	C = Cee;

	alignmentEnergy = sqrt(outputData[0] / outputData[28]);
}

void gFusion::getPreRedu(Eigen::Matrix<double, 6, 6> &A, Eigen::Matrix<double, 6, 1> &b)
{
	std::vector<float> outputSDFData;
	size_t reductionSDFSize = configuration.depthFrameSize.y * configuration.depthFrameSize.x * 8; // this is the size of one reduction element 1 float + 1 float + 6 float
	outputSDFData.resize(reductionSDFSize);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferSDFReduction);
	void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(outputSDFData.data(), reductionSDFSize * sizeof(GLfloat), ptr, reductionSDFSize * sizeof(GLfloat));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//Eigen::Matrix<double, 6, 6> A = Eigen::Matrix<double, 6, 6>::Zero();
	//Eigen::Matrix<double, 6, 1> b = Eigen::Matrix<double, 6, 1>::Zero();
	Eigen::Matrix<double, 6, 1> SDF_derivative = Eigen::Matrix<double, 6, 1>::Zero();

	for (int i = 0; i < 848 * 480 * 8; i += 8)
	{
		SDF_derivative << outputSDFData[i + 2], outputSDFData[i + 3], outputSDFData[i + 4], outputSDFData[i + 5], outputSDFData[i + 6], outputSDFData[i + 7];
		A = A + SDF_derivative * SDF_derivative.transpose();
		b = b + (outputSDFData[i + 1] * SDF_derivative);
	}



}

void gFusion::getSDFReduction(std::vector<float>& b, std::vector<float>& C, float &alignmentEnergy)
{
	std::vector<float> outputSDFData;
	outputSDFData.resize(32 * 8);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferSDFoutputdata);
	void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(outputSDFData.data(), outputSDFData.size() * sizeof(float), ptr, outputSDFData.size() * sizeof(float));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


	for (int row = 1; row < 8; row++)
	{
		for (int col = 0; col < 32; col++)
		{
			outputSDFData[col + 0 * 32] = outputSDFData[col + row * 32];
		}
	}

	std::vector<float>::const_iterator first0 = outputSDFData.begin() + 1;
	std::vector<float>::const_iterator last0 = outputSDFData.begin() + 28;

	std::vector<float> vals(first0, last0);

	std::vector<float>::const_iterator first1 = vals.begin();
	std::vector<float>::const_iterator last1 = vals.begin() + 6;
	std::vector<float>::const_iterator last2 = vals.begin() + 6 + 21;

	std::vector<float> bee(first1, last1);
	std::vector<float> Cee = makeJTJ(std::vector<float>(last1, last2));

	b = bee;
	C = Cee;

	alignmentEnergy = sqrt(outputSDFData[0] / outputSDFData[28]);


}

void gFusion::integrate()
{

	//GLenum errInt = glGetError();

	glBeginQuery(GL_TIME_ELAPSED, query[2]);

	integrateProg.use();

	glm::mat4 integratePose = glm::inverse(m_pose);
	glm::mat4 K = GLHelper::getCameraMatrix(m_camPamsDepth); // make sure im set

	// bind uniforms
	glUniformMatrix4fv(m_invTrackID, 1, GL_FALSE, glm::value_ptr(integratePose));
	glUniformMatrix4fv(m_KID, 1, GL_FALSE, glm::value_ptr(K));
	glUniform1f(m_muID, configuration.mu);
	glUniform1f(m_maxWeightID, configuration.maxWeight);
	glUniform3fv(m_volDimID, 1, glm::value_ptr(configuration.volumeDimensions));
	glUniform3fv(m_volSizeID, 1, glm::value_ptr(configuration.volumeSize));
	//bind image textures
	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16I);
	//errInt = glGetError();
	if (m_usingFloatDepth)
	{
		glUniform1i(m_imageTypeID_i, 1); // image type 1 == float
		//glUniform1f(m_depthScaleID, m_depthUnit / 1000000.0f); // 1000 == each depth unit == 1 mm // FIX ME WHEN GOING BACK TO KINECT

		glBindImageTexture(1, m_textureDepth, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	}
	else
	{
		
		glUniform1i(m_imageTypeID_i, 0); // image type 0 == short
		glUniform1f(m_depthScaleID, m_depthUnit / 1000000.0f); // 1000 == each depth unit == 1 mm
		glBindImageTexture(2, m_textureDepth, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
	}
	//errInt = glGetError();

	glBindImageTexture(3, m_textureVertex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);


	int xWidth;
	int yWidth;


	xWidth = divup(configuration.volumeSize.x, 32);
	yWidth = divup(configuration.volumeSize.y, 32);

	xWidth = divup(configuration.depthFrameSize.x, 32);
	yWidth = divup(configuration.depthFrameSize.y, 32);

	glDispatchCompute(xWidth, yWidth, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glEndQuery(GL_TIME_ELAPSED);


	GLuint available = 0;
	while (!available) {
		glGetQueryObjectuiv(query[2], GL_QUERY_RESULT_AVAILABLE, &available);
	}

	// elapsed time in nanoseconds
	GLuint64 elapsed;
	glGetQueryObjectui64vEXT(query[2], GL_QUERY_RESULT, &elapsed);

	integrateTime = elapsed / 1000000.0;

	//std::cout << "i time " << integrateTime << std::endl;


}

void gFusion::raycast()
{


	glBeginQuery(GL_TIME_ELAPSED, query[3]);

	raycastProg.use();

	glm::mat4 invK = GLHelper::getInverseCameraMatrix(m_camPamsDepth);
	glm::mat4 view = m_pose * invK;

	float step = configuration.stepSize();

	// bind uniforms
	glUniformMatrix4fv(m_viewID_r, 1, GL_FALSE, glm::value_ptr(view));
	glUniform1f(m_nearPlaneID, configuration.nearPlane);
	glUniform1f(m_farPlaneID, configuration.farPlane);
	glUniform1f(m_stepID, step);
	glUniform1f(m_largeStepID, 0.75f * configuration.mu);
	glUniform3fv(m_volDimID_r, 1, glm::value_ptr(configuration.volumeDimensions));
	glUniform3fv(m_volSizeID_r, 1, glm::value_ptr(configuration.volumeSize));

	//bind image textures
	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16I);
	glBindImageTexture(1, m_textureReferenceVertex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(2, m_textureReferenceNormal, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);



	//glBindImageTexture(3, m_textureTestImage, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	

	//glBindImageTexture(1, m_textureVolumeSliceNorm, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	//glBindImageTexture(2, m_textureVolumeColor, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// bind the volume texture for 3D sampling
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_textureVolume);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, m_textureFloodSDF);


	/*glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, m_textureVolumeColor);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_textureColor);*/

	int xWidth;
	int yWidth;


	xWidth = divup(configuration.depthFrameSize.x, 32);
	yWidth = divup(configuration.depthFrameSize.y, 32);


	glDispatchCompute(xWidth, yWidth, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glEndQuery(GL_TIME_ELAPSED);
	GLuint available = 0;
	while (!available) {
		glGetQueryObjectuiv(query[3], GL_QUERY_RESULT_AVAILABLE, &available);
	}
	// elapsed time in nanoseconds
	GLuint64 elapsed;
	glGetQueryObjectui64vEXT(query[3], GL_QUERY_RESULT, &elapsed);
	raycastTime = elapsed / 1000000.0;

	//std::cout << raycastTime << std::endl;

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureTestImage);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, testIm2.data);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//

	//cv::imshow("testim2", testIm2);



}

void gFusion::intensityProjection()
{
	mipProg.use();

	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16I);
	glBindImageTexture(1, m_textureMip, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glm::mat4 invK = GLHelper::getInverseCameraMatrix(m_camPamsDepth);
	glm::mat4 view = m_pose * invK;

	float step = configuration.stepSize();

	// bind uniforms
	glUniformMatrix4fv(m_viewID_m, 1, GL_FALSE, glm::value_ptr(view));
	glUniform1f(m_nearPlaneID_m, configuration.nearPlane);
	glUniform1f(m_farPlaneID_m, configuration.farPlane);
	glUniform1f(m_stepID_m, step);
	glUniform1f(m_largeStepID_m, 0.75f * configuration.mu);
	glUniform3fv(m_volDimID_m, 1, glm::value_ptr(configuration.volumeDimensions));
	glUniform3fv(m_volSizeID_m, 1, glm::value_ptr(configuration.volumeSize));


	int xWidth;
	int yWidth;


	xWidth = divup(configuration.depthFrameSize.x, 32);
	yWidth = divup(configuration.depthFrameSize.y, 32);


	glDispatchCompute(xWidth, yWidth, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureMip);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, mipMat.data);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//

	//cv::imshow("ints", mipMat * 10.0f);

}
//
//GLuint gFusion::prefixSum(GLuint inputBuffer, GLuint outputBuffer)
//{
//	// reduction sum
//	prefixSumProg.use();
//	int xthreads = divup(mcubeConfiguration.numVoxels, 1024); // 1024 is the localworkgroupsize inside the shader
//
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, inputBuffer);
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, outputBuffer);
//
//	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_resetSumsArrayID);
//	glDispatchCompute(1, 1, 1);
//	glMemoryBarrier(GL_ALL_BARRIER_BITS);
//
//	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_forEachGroupID);
//	glDispatchCompute(xthreads, 1, 1);
//	glMemoryBarrier(GL_ALL_BARRIER_BITS);
//
//	int xthreads2 = divup(xthreads, 1024);
//
//	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_forEveryGroupID);
//	glDispatchCompute(xthreads2, 1, 1);
//	glMemoryBarrier(GL_ALL_BARRIER_BITS);
//
//	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_forFinalIncrementalSumID);
//	glDispatchCompute(xthreads, 1, 1);
//	glMemoryBarrier(GL_ALL_BARRIER_BITS);
//
//	uint32_t lastElement, lastScanElement;
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
//	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, (mcubeConfiguration.numVoxels - 1) * sizeof(uint32_t), sizeof(uint32_t), &lastScanElement);
//
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputBuffer);
//	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, (mcubeConfiguration.numVoxels - 1) * sizeof(uint32_t), sizeof(uint32_t), &lastElement);
//
//	return lastElement + lastScanElement;
//
//}

//void gFusion::marchingCubes()
//{


	//glBeginQuery(GL_TIME_ELAPSED, query[4]);

	//int threads = 128;

	//// CLASSIFY VOXEL
	//marchingCubesProg.use();
	//// BIND TEXTURES
	//glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16I);
	//glBindImageTexture(1, m_textureEdgeTable, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	//glBindImageTexture(2, m_textureTriTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	//glBindImageTexture(3, m_textureNumVertsTable, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	//// BIND BUFFERS
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_bufferVoxelVerts);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_bufferVoxelOccupied);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_bufferVoxelOccupiedScan);
	//// SET UNIFORMS
	//glUniform3uiv(m_gridSizeID, 1, glm::value_ptr(mcubeConfiguration.gridSize));
	//glUniform3uiv(m_gridSizeShiftID, 1, glm::value_ptr(mcubeConfiguration.gridSizeShift));
	//glUniform3uiv(m_gridSizeMaskID, 1, glm::value_ptr(mcubeConfiguration.gridSizeMask));
	//glUniform1f(m_isoValueID, mcubeConfiguration.isoValue);
	//glUniform1ui(m_numVoxelsID, mcubeConfiguration.numVoxels);

	//int xthreads = divup(mcubeConfiguration.numVoxels, threads);
	//int ythreads = 1;
	//if (xthreads > 65535)
	//{
	//	ythreads = xthreads / 32768;
	//	xthreads = 32768;
	//}
	//// LAUNCH SHADER
	//glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_classifyVoxelID);
	//glDispatchCompute(xthreads, ythreads, 1);
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//// PREFIX SUM
	//mcubeConfiguration.activeVoxels = prefixSum(m_bufferVoxelOccupied, m_bufferVoxelOccupiedScan);
	////std::cout << "active voxels " << mcubeConfiguration.activeVoxels << std::endl;

	//// COMPACT VOXELS
	//marchingCubesProg.use();
	//// BIND TEXTURES
	//// BIND BUFFERS
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_bufferVoxelOccupied);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_bufferVoxelOccupiedScan);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_bufferCompactedVoxelArray);
	//// BIND UNIFORMS
	//glUniform1ui(m_numVoxelsID, mcubeConfiguration.numVoxels);

	//xthreads = divup(mcubeConfiguration.numVoxels, threads);
	//ythreads = 1;
	//if (xthreads > 65535)
	//{
	//	ythreads = xthreads / 32768;
	//	xthreads = 32768;
	//}
	//// LAUNCH SHADER
	//glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_compactVoxelsID);
	//glDispatchCompute(xthreads, ythreads, 1);
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//// PREFIX SUM
	//m_totalVerts = prefixSum(m_bufferVoxelVerts, m_bufferVoxelVertsScan);
	////std::cout << "total verts " << m_totalVerts << std::endl;

	//// GENERATE TRIANGLES
	//marchingCubesProg.use();
	//// BIND TEXTURES
	//glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16I);
	//glBindImageTexture(1, m_textureEdgeTable, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	//glBindImageTexture(2, m_textureTriTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	//glBindImageTexture(3, m_textureNumVertsTable, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	//// BIND BUFFERS
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_bufferCompactedVoxelArray);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_bufferVoxelVertsScan);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_bufferPos);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, m_bufferNorm);
	//// BIND UNIFORMS
	//glUniform3uiv(m_gridSizeID, 1, glm::value_ptr(mcubeConfiguration.gridSize));
	//glUniform3uiv(m_gridSizeShiftID, 1, glm::value_ptr(mcubeConfiguration.gridSizeShift));
	//glUniform3uiv(m_gridSizeMaskID, 1, glm::value_ptr(mcubeConfiguration.gridSizeMask));
	//glUniform1f(m_isoValueID, mcubeConfiguration.isoValue);
	//glUniform1ui(m_numVoxelsID, mcubeConfiguration.numVoxels);
	//glUniform1ui(m_activeVoxelsID, mcubeConfiguration.activeVoxels);
	//glUniform1ui(m_maxVertsID, mcubeConfiguration.maxVerts);
	//glUniform3fv(m_voxelSizeID, 1, glm::value_ptr(mcubeConfiguration.voxelSize));
	//
	//xthreads = divup(mcubeConfiguration.activeVoxels, threads);
	//ythreads = 1;
	//while (xthreads > 65535)
	//{
	//	xthreads /= 2;
	//	ythreads *= 2;
	//}
	//// LAUNCH SHADER
	//glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_generateTrianglesID);
	//glDispatchCompute(xthreads, ythreads, 1);
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//glEndQuery(GL_TIME_ELAPSED);


	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query[4], GL_QUERY_RESULT_AVAILABLE, &available);
	//}

	//// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query[4], GL_QUERY_RESULT, &elapsed);

	////marchingCubesTime = elapsed / 1000000.0;

	//std::cout << "time elapsed " << marchingCubesTime << " ms" << std::endl;


//}

void gFusion::updatePoseFinder()
{
	// if tracking is success, see if the glm pose is norm distance away from any in the library, if so, a new posepair should be added to the library
	int currentLibrarySize = poseLibrary.size();

	if (currentLibrarySize == 0)
	{
		addPoseToLibrary();
	}
	else
	{
		for (int i = 0; i < currentLibrarySize; i++)
		{
			glm::vec3 libTran = glm::vec3(poseLibrary[i].pose[3][0], poseLibrary[i].pose[3][1], poseLibrary[i].pose[3][2]);
			glm::vec3 curTran = glm::vec3(m_pose[3][0], m_pose[3][1], m_pose[3][2]);

			float theNorm = glm::l2Norm(libTran, curTran);

			if (theNorm < 0.2f)
			{
				return;
			}
		}
		// if here, then no matching pose was found in library
		addPoseToLibrary();
	}

}

void gFusion::addPoseToLibrary()
{

	gPosePair newPose;
	GLuint new_textureID;
	new_textureID = createTexture(GL_TEXTURE_2D, 0, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_R32F);

	glCopyImageSubData(m_textureDepth, GL_TEXTURE_2D, 0, 0, 0, 0,
		new_textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
		configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1);

	newPose.textureID = new_textureID;
	newPose.pose = m_pose;
	poseLibrary.push_back(newPose);

}
/// Function called when tracking is lost
bool gFusion::recoverPose()
{
	bool foundPose = false;
	int currentLibrarySize = poseLibrary.size();
	std::cout << currentLibrarySize << std::endl;
	if (currentLibrarySize == 0)
	{
		std::cout << "You should not have arrived here" << std::endl;
		return foundPose;
	}
	else
	{
		for (int i = 0; i < currentLibrarySize; i++)
		{
			m_pose = poseLibrary[i].pose;
			glCopyImageSubData(poseLibrary[i].textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
				m_textureDepth, GL_TEXTURE_2D, 0, 0, 0, 0,
				configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1);
			//raycast();
			//depthToVertex(NULL);
			//vertexToNormal();
			//showNormals();
			//foundPose = Track();
			foundPose = TrackSDF();
			if (foundPose)
			{
				std::cout << "found a pose " << std::endl;
				break;
			}

		}



	}

	return foundPose;
}

void gFusion::trackPoints3D(GLuint trackedPoints2Dbuffer)
{

	int compWidth;
	int compHeight;

	helpersProg.use();

	glm::mat4 invK = GLHelper::getInverseCameraMatrix(m_camPamsDepth);
	glUniformMatrix4fv(m_invKID_h, 1, GL_FALSE, glm::value_ptr(invK));
	glUniform1i(m_buffer2DWidthID, configuration.depthFrameSize.y * 2);

	// BIND IMAGE
	glBindImageTexture(1, m_textureDepth, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	// BIND BUFFERS
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, trackedPoints2Dbuffer);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_trackedPoints3DBuffer);

	compWidth = divup(configuration.depthFrameSize.x, 32); 
	compHeight = divup(configuration.depthFrameSize.y, 32);

	glDispatchCompute(compWidth, compHeight, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
/*

	std::vector<float> outputData(m_depth_height * m_depth_height *2);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, trackedPoints2Dbuffer);
	void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(outputData.data(), outputData.size() * sizeof(float), ptr, outputData.size() * sizeof(float));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	cv::Mat blank = cv::Mat(424, 512, CV_8UC4);
	for (int i = 0; i < m_depth_height * m_depth_height * 2; i += 2)
	{

		cv::circle(blank, cv::Point2f(outputData[i], outputData[i + 1]), 2, cv::Scalar(255, 128, 128, 255));

	}    

	cv::imshow("tracawked", blank);*/



}





void gFusion::exportSurfaceAsStlBinary()
{

	//std::string modelFileName = "data/meshes/marchingCubesBin.stl";

	//std::ofstream outFile(modelFileName, std::ios::out | std::ios::binary);

	//if (!outFile)
	//{
	//	//cerr << "Error opening output file: " << FileName << "!" << endl;
	//	printf("Error opening output file: %s!\n", modelFileName);
	//	exit(1);
	//}

	//// copy cuda device to host


	//////
	//// Header
	//////

	//char hdr[80];

	//const int pointNum = mcubeConfiguration.maxVerts;
	//uint32_t NumTri = mcubeConfiguration.maxVerts / 3;
	//uint32_t attributeByteCount = 0;

	////outFile.write(hdr, 80);
	////outFile.write((char*)&NumTri, sizeof(uint));

	//// h_data is the posVbo, i.e. the array of verts of length maxVerts, sparse
	//// h_compVoxelArray is 
	//// h_voxelVerts is the number of verts in each voxel, i.e. an array of length volume height * width * depth with ints inside saying how many verts are inside each voxel. should this be 1 if one vox only contains one vert

	//std::vector<uint32_t> h_compVoxelArray, h_voxelVertsScan, h_voxelVerts;
	//h_compVoxelArray.resize(mcubeConfiguration.numVoxels);
	//h_voxelVertsScan.resize(mcubeConfiguration.numVoxels);
	//h_voxelVerts.resize(mcubeConfiguration.numVoxels);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferCompactedVoxelArray);
	//void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(h_compVoxelArray.data(), h_compVoxelArray.size() * sizeof(uint32_t), ptr, h_compVoxelArray.size() * sizeof(uint32_t));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferVoxelVertsScan);
	//void *ptr1 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(h_voxelVertsScan.data(), h_voxelVertsScan.size() * sizeof(uint32_t), ptr1, h_voxelVertsScan.size() * sizeof(uint32_t));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferVoxelVerts);
	//void *ptr2 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(h_voxelVerts.data(), h_voxelVerts.size() * sizeof(uint32_t), ptr2, h_voxelVerts.size() * sizeof(uint32_t));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//std::vector<float> hData, hDataNorm;
	//hData.resize(mcubeConfiguration.maxVerts * 4);
	//hDataNorm.resize(mcubeConfiguration.maxVerts * 4);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferPos);
	//void *ptr3 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(hData.data(), hData.size() * sizeof(float), ptr3, hData.size() * sizeof(float));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferNorm);
	//void *ptr4 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(hDataNorm.data(), hDataNorm.size() * sizeof(float), ptr4, hDataNorm.size() * sizeof(float));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	//outFile << "solid phils stler" << std::endl;

	////std::cout << " verts ";
	////for (int i = 0; i < m_totalVerts * 4; i++)
	////	std::cout << hData[i] << " ";

	//for (int i = 0; i < m_totalVerts; i++) 
	//{
	//	uint32_t voxel = h_compVoxelArray[i];
	//	uint32_t index = h_voxelVertsScan[voxel]; // index is the start of the array from where your triangles are for current voxel, you need to go from this point to index + numvertinbox * 4
	//	uint32_t numVertInVox = h_voxelVerts[voxel];

	//	// each vertex is separated in hData by 4
	//	//	vertex1 = hData[0] hData[1] hData[2]
	//	//	vertex2 = hData[4] hData[5] hData[6]
	//	//	vertex3 = hData[8] hData[9] hData[10]
	//	// the next triangle starts at hData[12]
	//		





	//	for (int j = 0; j < numVertInVox * 4; j += 12) // j is an incrementer of vertexes, previously we used a float 4
	//	{
	//		outFile << "facet normal " << hDataNorm[index * 4 + j + 0] * -1.0f << " " << hDataNorm[index * 4 + j + 1] * -1.0f << " " << hDataNorm[index * 4 + j + 2] * -1.0f << std::endl;
	//		outFile << "outer loop" << std::endl;
	//		outFile << "vertex " << hData[index * 4 + j + 0] << " " << hData[index * 4 + j + 1] << " " << hData[index * 4 + j + 2] << std::endl;
	//		outFile << "vertex " << hData[index * 4 + j + 4] << " " << hData[index * 4 + j + 5] << " " << hData[index * 4 + j + 6] << std::endl;
	//		outFile << "vertex " << hData[index * 4 + j + 8] << " " << hData[index * 4 + j + 9] << " " << hData[index * 4 + j + 10] << std::endl;
	//		outFile << "endloop" << std::endl;
	//		outFile << "endfacet" << std::endl;

	//	}

	//	//for (int j = 0; j < numVertInVox; j += 3) // oh yeah
	//	//{


	//	//	outFile.write((char*)&hDataNorm[index + (j * 3) + 0], sizeof(float));
	//	//	outFile.write((char*)&hDataNorm[index + (j * 3) + 1], sizeof(float));
	//	//	outFile.write((char*)&hDataNorm[index + (j * 3) + 2], sizeof(float));

	//	//	outFile.write((char*)&hData[index + (j * 3) + 0], sizeof(float));
	//	//	outFile.write((char*)&hData[index + (j * 3) + 0], sizeof(float));
	//	//	outFile.write((char*)&hData[index + (j * 3) + 0], sizeof(float));

	//	//	outFile.write((char*)&hData[index + (j * 3) + 1], sizeof(float));
	//	//	outFile.write((char*)&hData[index + (j * 3) + 1], sizeof(float));
	//	//	outFile.write((char*)&hData[index + (j * 3) + 1], sizeof(float));

	//	//	outFile.write((char*)&hData[index + (j * 3) + 2], sizeof(float));
	//	//	outFile.write((char*)&hData[index + (j * 3) + 2], sizeof(float));
	//	//	outFile.write((char*)&hData[index + (j * 3) + 2], sizeof(float));

	//	//	outFile.write((char*)&attributeByteCount, sizeof(uint));

	//	//}

	//}




	//outFile.close();



}

void gFusion::testLargeUpload()
{
	GLuint queryUpload[1];
	glGenQueries(1, queryUpload);


	glBeginQuery(GL_TIME_ELAPSED, queryUpload[0]);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lmbuff_0);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 402653184 * sizeof(int), &listmode[0]);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lmbuff_1);
	//glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 402653184 * sizeof(int) / 2, &listmode[402653184 / 2]);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);


	glEndQuery(GL_TIME_ELAPSED);

	GLuint available = 0;
	while (!available) {
		glGetQueryObjectuiv(queryUpload[0], GL_QUERY_RESULT_AVAILABLE, &available);
	}
	// elapsed time in milliseconds?
	GLuint64 elapsed;
	glGetQueryObjectui64vEXT(queryUpload[0], GL_QUERY_RESULT, &elapsed);
	double uploadTime = elapsed / 1000000.0;
	std::cout << "uptime " << uploadTime << std::endl;

}