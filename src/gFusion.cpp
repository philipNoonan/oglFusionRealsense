﻿#include "gFusion.h"



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
		depthToVertProg.compileShader("shaders/depthToVertMulti.cs");
		depthToVertProg.link();
		
		vertToNormProg.compileShader("shaders/vertToNormMulti.cs");
		vertToNormProg.link();

		trackProg.compileShader("shaders/track.cs");
		trackProg.link();

		reduceProg.compileShader("shaders/reduce.cs");
		reduceProg.link();

		trackSplatProg.compileShader("shaders/trackSplat.cs");
		trackSplatProg.link();

		reduceSplatProg.compileShader("shaders/reduceSplat.cs");
		reduceSplatProg.link();

		integrateProg.compileShader("shaders/integrate.cs");
		integrateProg.link();

		raycastProg.compileShader("shaders/raycast.cs");
		raycastProg.link();

		marchingCubesProg.compileShader("shaders/marchingCubes.cs");
		marchingCubesProg.link();

		prefixSumProg.compileShader("shaders/prefixSum.cs");
		prefixSumProg.link();

		trackSDFProg.compileShader("shaders/trackSDFMulti.cs");
		trackSDFProg.link();

		reduceSDFProg.compileShader("shaders/reduceSDF.cs");
		reduceSDFProg.link();

		mipProg.compileShader("shaders/mip.cs");
		mipProg.link();
		
		
		helpersProg.compileShader("shaders/helpers.cs");
		helpersProg.link();

		bilateralFilterProg.compileShader("shaders/biFilter.cs");
		bilateralFilterProg.link();




		const GLchar* feedbackOutput[] = { "outVertPosConf",
									       "outVertNormRadi",
									       "outVertColTimDev" };

		const GLchar* indexFeedbackOutput[] = { "outVC",
												"outNR",
												"outCTD" };

		depthToBufferProg.compileShader("shaders/depthToBuffer.vs");
		depthToBufferProg.compileShader("shaders/depthToBuffer.gs");
		glTransformFeedbackVaryings(depthToBufferProg.getHandle(), 3, feedbackOutput, GL_INTERLEAVED_ATTRIBS);
		depthToBufferProg.link();

		updateGlobalModelProg.compileShader("shaders/updateGlobalModel.cs");
		//updateGlobalModelProg.compileShader("shaders/updateGlobalModel.vs");
		//updateGlobalModelProg.compileShader("shaders/updateGlobalModel.gs");
		//glTransformFeedbackVaryings(updateGlobalModelProg.getHandle(), 3, feedbackOutput, GL_INTERLEAVED_ATTRIBS);
		updateGlobalModelProg.link();

		dataProg.compileShader("shaders/data.vs");
		dataProg.compileShader("shaders/data.gs");
		//dataProg.compileShader("shaders/data.fs");
		glTransformFeedbackVaryings(dataProg.getHandle(), 3, indexFeedbackOutput, GL_INTERLEAVED_ATTRIBS);
		//dataProg.compileShader("shaders/data.cs");
		dataProg.link();

		cleanGlobalProg.compileShader("shaders/cleanGlobal.vs");
		cleanGlobalProg.compileShader("shaders/cleanGlobal.gs");
		glTransformFeedbackVaryings(cleanGlobalProg.getHandle(), 3, feedbackOutput, GL_INTERLEAVED_ATTRIBS);
		cleanGlobalProg.link();
		
		//initUnstableProg.compileShader("shaders/initUnstable.vs");
		//initUnstableProg.compileShader("shaders/initUnstable.vs");
		//glTransformFeedbackVaryings(initUnstableProg.getHandle(), 3, feedbackOutput, GL_INTERLEAVED_ATTRIBS);
		//initUnstableProg.link();

		createIndexMapProg.compileShader("shaders/createIndexMap.cs");
		//createIndexMapProg.compileShader("shaders/createIndexMap.gs");
		//createIndexMapProg.compileShader("shaders/createIndexMap.fs");
		createIndexMapProg.link();
/*
		createIndexMapProg.compileShader("shaders/createIndexMap.cs");
		createIndexMapProg.link();*/

		//splatterProg.compileShader("shaders/splatterDepth.vs");
		//splatterProg.compileShader("shaders/splatterDepth.fs");
		//splatterProg.link();

		//splatterGlobalProg.compileShader("shaders/splatterGlobal.vs");
		//splatterGlobalProg.compileShader("shaders/splatterGlobal.gs");
		//splatterGlobalProg.compileShader("shaders/splatterGlobal.fs");
		//splatterGlobalProg.link();

		combinedPredictProg.compileShader("shaders/combinedPredict.vs");
		combinedPredictProg.compileShader("shaders/combinedPredict.fs");
		combinedPredictProg.link();



		
	}
	catch (GLSLProgramException &e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	glGenQueries(7, query);

}

void gFusion::setLocations()
{
	depthToVertProg.use();
	m_invkID = glGetUniformLocation(depthToVertProg.getHandle(), "invK");
	m_colorKID = glGetUniformLocation(depthToVertProg.getHandle(), "colorK");

	m_extrinsicsID = glGetUniformLocation(depthToVertProg.getHandle(), "depthToDepth");
	m_camPamsDepthID = glGetUniformLocation(depthToVertProg.getHandle(), "camPamsDepth");
	m_camPamsColorID = glGetUniformLocation(depthToVertProg.getHandle(), "camPamsColor");

	m_imageTypeID = glGetUniformLocation(depthToVertProg.getHandle(), "imageType");
	m_depthScaleID = glGetUniformLocation(depthToVertProg.getHandle(), "depthScale");

	m_numberOfCamerasID_d = glGetUniformLocation(depthToVertProg.getHandle(), "numberOfCameras");
	
	vertToNormProg.use();
	m_numberOfCamerasID_v = glGetUniformLocation(vertToNormProg.getHandle(), "numberOfCameras");


	//std::cout << "invK " << m_invkID << std::endl;
	//std::cout << "camPams " << m_camPamsID << std::endl;
	//std::cout << "imageType " << m_imageTypeID << std::endl;
	//std::cout << "depthScale " << m_depthScaleID << std::endl;
	trackProg.use();
	//m_viewID_t = glGetUniformLocation(trackProg.getHandle(), "view");
	//m_TtrackID = glGetUniformLocation(trackProg.getHandle(), "Ttrack");


	m_distThresh_t = glGetUniformLocation(trackProg.getHandle(), "dist_threshold");
	m_normThresh_t = glGetUniformLocation(trackProg.getHandle(), "normal_threshold");
	m_numberOfCamerasID_tr = glGetUniformLocation(trackProg.getHandle(), "numberOfCameras");
	m_cameraPosesID_tr = glGetUniformLocation(trackProg.getHandle(), "cameraPoses");
	m_inverseCameraPosesID_tr = glGetUniformLocation(trackProg.getHandle(), "inverseCameraPoses");
	//std::cout << "view " << m_viewID_t << std::endl;
	//std::cout << "Ttrack " << m_TtrackID << std::endl;
	//std::cout << "dist_threshold " << m_distThresh_t << std::endl;
	//std::cout << "normal_threshold " << m_normThresh_t << std::endl;
	reduceProg.use();
	m_imageSizeID = glGetUniformLocation(reduceProg.getHandle(), "imageSize");


	trackSplatProg.use();
	m_distThresh_tsp = glGetUniformLocation(trackSplatProg.getHandle(), "dist_threshold");
	m_normThresh_tsp = glGetUniformLocation(trackSplatProg.getHandle(), "normal_threshold");
	m_numberOfCamerasID_tsp = glGetUniformLocation(trackSplatProg.getHandle(), "numberOfCameras");
	m_cameraPosesID_tsp = glGetUniformLocation(trackSplatProg.getHandle(), "cameraPoses");
	m_inverseVPID_tsp = glGetUniformLocation(trackSplatProg.getHandle(), "inverseVP");
	m_camPamID_tsp = glGetUniformLocation(trackSplatProg.getHandle(), "camPam");

	reduceSplatProg.use();
	m_imageSizeID_rsp = glGetUniformLocation(reduceSplatProg.getHandle(), "imageSize");



	//std::cout << "imageSize " << m_imageSizeID << std::endl;
	integrateProg.use();
	m_cameraPosesID_i = glGetUniformLocation(integrateProg.getHandle(), "cameraPoses");
	m_cameraIntrinsicsID_i = glGetUniformLocation(integrateProg.getHandle(), "cameraIntrinsics");
	m_inverseCameraIntrinsicsID_i = glGetUniformLocation(integrateProg.getHandle(), "inverseCameraIntrinsics");
	m_forceIntegrateID = glGetUniformLocation(integrateProg.getHandle(), "forceIntegrate");
	m_d2pID_i = glGetUniformLocation(integrateProg.getHandle(), "d2p");
	m_d2vID_i = glGetUniformLocation(integrateProg.getHandle(), "d2v");

	raycastProg.use();
	m_viewID_r = glGetUniformLocation(raycastProg.getHandle(), "view");
	m_nearPlaneID = glGetUniformLocation(raycastProg.getHandle(), "nearPlane");
	m_farPlaneID = glGetUniformLocation(raycastProg.getHandle(), "farPlane");
	m_stepID = glGetUniformLocation(raycastProg.getHandle(), "step");
	m_largeStepID = glGetUniformLocation(raycastProg.getHandle(), "largeStep");
	m_volDimID_r = glGetUniformLocation(raycastProg.getHandle(), "volDim");
	m_volSizeID_r = glGetUniformLocation(raycastProg.getHandle(), "volSize");
	m_cameraPosesID_r = glGetUniformLocation(raycastProg.getHandle(), "cameraPoses");
	m_numberOfCamerasID_r = glGetUniformLocation(raycastProg.getHandle(), "numberOfCameras");

	//HELPERS
	helpersProg.use();
	m_helpersSubroutineID = glGetSubroutineUniformLocation(helpersProg.getHandle(), GL_COMPUTE_SHADER, "performHelperFunction");
	m_resetVolumeID = glGetSubroutineIndex(helpersProg.getHandle(), GL_COMPUTE_SHADER, "resetVolume");
	m_trackPointsToVertsID = glGetSubroutineIndex(helpersProg.getHandle(), GL_COMPUTE_SHADER, "trackPointsToVerts");
	m_volSizeID_h = glGetUniformLocation(helpersProg.getHandle(), "volSize");
	m_buffer2DWidthID = glGetUniformLocation(helpersProg.getHandle(), "buffer2DWidth");
	m_invKID_h = glGetUniformLocation(helpersProg.getHandle(), "invK");

	// PREFIX SUMS
	prefixSumProg.use();
	m_prefixSumSubroutineID = glGetSubroutineUniformLocation(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "getPrefixSum");
	m_resetSumsArrayID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "resetSumsArray");

	m_forEachGroupID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "forEachGroup");
	m_forEveryGroupID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "forEveryGroup");
	m_forFinalIncrementalSumID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "forFinalIncrementalSum");

	// MARCHING CUBES
	marchingCubesProg.use();
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
	trackSDFProg.use();
	m_TtrackID_t = glGetUniformLocation(trackSDFProg.getHandle(), "Ttrack");
	m_volDimID_t = glGetUniformLocation(trackSDFProg.getHandle(), "volDim");
	m_volSizeID_t = glGetUniformLocation(trackSDFProg.getHandle(), "volSize");
	m_cID = glGetUniformLocation(trackSDFProg.getHandle(), "c");
	m_epsID = glGetUniformLocation(trackSDFProg.getHandle(), "eps");
	m_dMaxID_t = glGetUniformLocation(trackSDFProg.getHandle(), "dMax");
	m_dMinID_t = glGetUniformLocation(trackSDFProg.getHandle(), "dMin");
	m_devNumberTrackSdfID = glGetUniformLocation(trackSDFProg.getHandle(), "devNumber");

	m_numberOfCamerasID_t = glGetUniformLocation(trackSDFProg.getHandle(), "numberOfCameras");
	m_mipLayerID_t = glGetUniformLocation(trackSDFProg.getHandle(), "mip");
	m_cameraPosesID_tsdf = glGetUniformLocation(trackSDFProg.getHandle(), "cameraPoses");

	// REDUCE SDF
	reduceSDFProg.use();
	m_imageSizeID_sdf = glGetUniformLocation(reduceSDFProg.getHandle(), "imageSize");
	m_devNumberReduceSdfID = glGetUniformLocation(reduceSDFProg.getHandle(), "devNumber");

	//INTENSITY PROJECTION
	mipProg.use();
	m_viewID_m = glGetUniformLocation(mipProg.getHandle(), "view");
	m_nearPlaneID_m = glGetUniformLocation(mipProg.getHandle(), "nearPlane");
	m_farPlaneID_m = glGetUniformLocation(mipProg.getHandle(), "farPlane");
	m_stepID_m = glGetUniformLocation(mipProg.getHandle(), "step");
	m_largeStepID_m = glGetUniformLocation(mipProg.getHandle(), "largeStep");
	m_volDimID_m = glGetUniformLocation(mipProg.getHandle(), "volDim");
	m_volSizeID_m = glGetUniformLocation(mipProg.getHandle(), "volSize");

	//splatterProg.use();
	//m_splatterModelID = glGetUniformLocation(splatterProg.getHandle(), "model");
	//m_splatterCamPamID = glGetUniformLocation(splatterProg.getHandle(), "camPam");
	//m_splatterMaxDepthID = glGetUniformLocation(splatterProg.getHandle(), "maxDepth");
	//m_splatterImSizeID = glGetUniformLocation(splatterProg.getHandle(), "imSize");

	depthToBufferProg.use();
	m_camPamID_d2b = glGetUniformLocation(depthToBufferProg.getHandle(), "camPam");
	m_depthScaleID_d2b = glGetUniformLocation(depthToBufferProg.getHandle(), "depthScale");
	m_invKID_d2b = glGetUniformLocation(depthToBufferProg.getHandle(), "invK");
	m_initUnstableID_d2b = glGetUniformLocation(depthToBufferProg.getHandle(), "initUnstable");
	m_frameCountID_d2b = glGetUniformLocation(depthToBufferProg.getHandle(), "frameCount");

	createIndexMapProg.use();
	m_indexInversePoseID = glGetUniformLocation(createIndexMapProg.getHandle(), "inversePose");
	m_indexCamPamID = glGetUniformLocation(createIndexMapProg.getHandle(), "camPam");
	m_indexImSizeID = glGetUniformLocation(createIndexMapProg.getHandle(), "imSize");
	m_indexMaxDepthID = glGetUniformLocation(createIndexMapProg.getHandle(), "maxDepth");
	m_indexTimeID = glGetUniformLocation(createIndexMapProg.getHandle(), "time");
	m_indexTimeDeltaID = glGetUniformLocation(createIndexMapProg.getHandle(), "timeDelta");

	//m_InitUnstableMaxNumVertsID = glGetUniformLocation(initUnstableProg.getHandle(), "maxNumVerts");

	combinedPredictProg.use();
	m_cpMaxNumVertsID = glGetUniformLocation(combinedPredictProg.getHandle(), "maxNumVerts");
	m_cpInversePoseID = glGetUniformLocation(combinedPredictProg.getHandle(), "inversePose");
	m_cpCamPamID = glGetUniformLocation(combinedPredictProg.getHandle(), "camPam");
	m_cpImSizeID = glGetUniformLocation(combinedPredictProg.getHandle(), "imSize");
	m_cpMaxDepthID = glGetUniformLocation(combinedPredictProg.getHandle(), "maxDepth");
	m_cpTimeID = glGetUniformLocation(combinedPredictProg.getHandle(), "time");
	m_cpConfThresholdID = glGetUniformLocation(combinedPredictProg.getHandle(), "confThreshold");
	m_cpMaxTimeID = glGetUniformLocation(combinedPredictProg.getHandle(), "maxTime");
	m_cpTimeDeltaID = glGetUniformLocation(combinedPredictProg.getHandle(), "timeDelta");

	dataProg.use();
	m_dpCamPamID = glGetUniformLocation(dataProg.getHandle(), "camPam");
	m_dpImSizeID = glGetUniformLocation(dataProg.getHandle(), "imSize");
	m_dpScaleID = glGetUniformLocation(dataProg.getHandle(), "scale");
	m_dpTexDimID = glGetUniformLocation(dataProg.getHandle(), "texDim");
	m_dpPoseID = glGetUniformLocation(dataProg.getHandle(), "pose");
	m_dpMaxDepthID = glGetUniformLocation(dataProg.getHandle(), "maxDepth");
	m_dpTimeID = glGetUniformLocation(dataProg.getHandle(), "time");
	m_dpTimeDeltaID = glGetUniformLocation(dataProg.getHandle(), "timeDelta");

	m_dpWeightingID = glGetUniformLocation(dataProg.getHandle(), "weighting");

	updateGlobalModelProg.use();
	m_ugmCamPamID = glGetUniformLocation(updateGlobalModelProg.getHandle(), "camPam");
	m_ugmTimeID = glGetUniformLocation(updateGlobalModelProg.getHandle(), "time");
	m_ugmTimeDeltaID = glGetUniformLocation(updateGlobalModelProg.getHandle(), "timeDelta");
	m_ugmTexDimID = glGetUniformLocation(updateGlobalModelProg.getHandle(), "texDim");
	m_ugmCurrentGlobalNumberID = glGetUniformLocation(updateGlobalModelProg.getHandle(), "currentGlobalCount");
	m_ugmCurrentNewUnstableNumberID = glGetUniformLocation(updateGlobalModelProg.getHandle(), "currentNewUnstableCount");

	cleanGlobalProg.use();
	m_cgTimeID = glGetUniformLocation(cleanGlobalProg.getHandle(), "time");
	m_cgTimeDeltaID = glGetUniformLocation(cleanGlobalProg.getHandle(), "timeDelta");
	m_cgInversePoseID = glGetUniformLocation(cleanGlobalProg.getHandle(), "inversePose");
	m_cgCamPamID = glGetUniformLocation(cleanGlobalProg.getHandle(), "camPam");
	m_cgConfThresholdID = glGetUniformLocation(cleanGlobalProg.getHandle(), "confThreshold");
	m_cgCurrentGlobalNumberID = glGetUniformLocation(cleanGlobalProg.getHandle(), "currentGlobalCount");
	m_cgCurrentNewUnstableNumberID = glGetUniformLocation(cleanGlobalProg.getHandle(), "currentNewUnstableCount");


	bilateralFilterProg.use();
	m_bfDepthScaleID = glGetUniformLocation(bilateralFilterProg.getHandle(), "depthScale");
	m_bfSigmaID = glGetUniformLocation(bilateralFilterProg.getHandle(), "sigma");
	m_bfDSigmaID = glGetUniformLocation(bilateralFilterProg.getHandle(), "bSigma");

}

GLuint gFusion::createTexture(GLenum target, int levels, int w, int h, int d, GLint internalformat, GLenum magFilter, GLenum minFilter)
{
	GLuint texid;
	glGenTextures(1, &texid);
	glBindTexture(target, texid);

	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
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
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		glTexStorage3D(target, levels, internalformat, w, h, d);
	}

	float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);

	return texid;
}

void gFusion::initTextures()
{

	m_sensorsTimestamps.resize(m_numberOfCameras);
	m_previousTime.resize(m_numberOfCameras);

	//m_textureDepth.resize(m_numberOfCameras);
	//m_textureVertex.resize(m_numberOfCameras);
	//m_textureNormal.resize(m_numberOfCameras);
	//m_textureSDFImage.resize(m_numberOfCameras);
	//m_textureTrackImage.resize(m_numberOfCameras);
	// make these a 3d texture of depth
	for (int i = 0; i < m_numberOfCameras; i++)
	{
		//m_textureDepth[i] = createTexture(GL_TEXTURE_2D, 3, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_R16);
		//m_textureVertex[i] = createTexture(GL_TEXTURE_2D, 3, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F);
		//m_textureNormal[i] = createTexture(GL_TEXTURE_2D, 3, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F);

		//m_textureSDFImage[i] = createTexture(GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);
		//m_textureTrackImage[i] = createTexture(GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);

	}

	m_textureColorArray = createTexture(GL_TEXTURE_2D_ARRAY, 3, configuration.depthFrameSize.x, configuration.depthFrameSize.y, m_numberOfCameras, GL_RGBA8, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	
	m_textureDepthArray = createTexture(GL_TEXTURE_2D_ARRAY, 3, configuration.depthFrameSize.x, configuration.depthFrameSize.y, m_numberOfCameras, GL_R16, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	m_textureDepthFilteredArray = createTexture(GL_TEXTURE_2D_ARRAY, 3, configuration.depthFrameSize.x, configuration.depthFrameSize.y, m_numberOfCameras, GL_R16, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);

	m_textureVertexArray = createTexture(GL_TEXTURE_2D_ARRAY, 3, configuration.depthFrameSize.x, configuration.depthFrameSize.y, m_numberOfCameras, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);
	m_textureNormalArray = createTexture(GL_TEXTURE_2D_ARRAY, 3, configuration.depthFrameSize.x, configuration.depthFrameSize.y, m_numberOfCameras, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);

	m_textureSDFImage = createTexture(GL_TEXTURE_2D_ARRAY, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, m_numberOfCameras, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);
	m_textureTrackImage = createTexture(GL_TEXTURE_2D_ARRAY, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, m_numberOfCameras, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);

	m_textureDist = createTexture(GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, m_numberOfCameras, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);

	// https://www.khronos.org/opengl/wiki/Normalized_Integer
	// I think that we have to use normailsed images here, because we cannot use ints for mip maps, they just dont work


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

	m_textureColor = createTexture(GL_TEXTURE_2D, 1, m_color_width, m_color_height, 1, GL_RGBA8UI, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	m_textureReferenceVertexArray = createTexture(GL_TEXTURE_2D_ARRAY, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, m_numberOfCameras, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);
	m_textureReferenceNormalArray = createTexture(GL_TEXTURE_2D_ARRAY, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, m_numberOfCameras, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);
	m_textureDifferenceVertex = createTexture(GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_R32F, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);
	m_textureTestImage = createTexture(GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);
	//m_textureTrackImage = createTexture(GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F);
	m_textureTest = createTexture(GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);
	m_textureMip = createTexture(GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);


}

void gFusion::initVolume()
{
	//GLenum err = glGetError();
	m_textureVolume = createTexture(GL_TEXTURE_3D, 1, configuration.volumeSize.x, configuration.volumeSize.y, configuration.volumeSize.z, GL_RGBA16F, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);
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

	resetSplat();

	resetTimes();

	poseLibrary.resize(0);
	resetPose(pose);
}

void gFusion::resetSplat()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_depth_VBO);
	glBufferData(GL_ARRAY_BUFFER, configuration.depthFrameSize.x * configuration.depthFrameSize.y * sizeof(glm::vec4) * 3, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_newUnstableIndex_VBO);
	glBufferData(GL_ARRAY_BUFFER, configuration.depthFrameSize.x * configuration.depthFrameSize.y * sizeof(glm::vec4) * 3, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_globalTarget_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_bufferSize, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_globalRender_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_bufferSize, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void gFusion::resetVolume()
{
	helpersProg.use();

	int compWidth;
	int compHeight;

	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
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
	size_t reductionSize = m_numberOfCameras * configuration.depthFrameSize.x * configuration.depthFrameSize.y * (sizeof(GLint) + (sizeof(GLfloat) * 7)); // this is the size of one reduction element 1 int + 1 float + 6 float

	glGenBuffers(1, &m_buffer_reduction);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_buffer_reduction);
	glBufferData(GL_SHADER_STORAGE_BUFFER, reductionSize, NULL, GL_DYNAMIC_COPY);

	// OUTPUT DATA FROM REDUCTION BUFFER OBJECT
	m_outputdata.resize(32 * 8);
	glGenBuffers(1, &m_buffer_outputdata);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_buffer_outputdata);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 32 * 8 * sizeof(float), NULL, GL_DYNAMIC_COPY);

	glGenBuffers(1, &m_bufferCameraData);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_bufferCameraData);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(glm::mat4), NULL, GL_DYNAMIC_READ);

	glGenBuffers(1, &m_bufferCameraData_i);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_bufferCameraData_i);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(glm::mat4), NULL, GL_DYNAMIC_READ);

	//glGenBuffers(1, &m_bufferCameraIntrinsics);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_bufferCameraIntrinsics);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(glm::mat4), NULL, GL_DYNAMIC_READ);

	glGenBuffers(1, &m_bufferInverseCameraIntrinsics);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_bufferInverseCameraIntrinsics);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(glm::mat4), NULL, GL_DYNAMIC_READ);

	size_t reductionSDFSize = m_numberOfCameras * configuration.depthFrameSize.y * configuration.depthFrameSize.x * (sizeof(GLint) + sizeof(GLfloat) * 8); // this is the size of one reduction element 1 int + 1 float + 1 float + 6 float
	glGenBuffers(1, &m_bufferSDFReduction);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, m_bufferSDFReduction);
	glBufferData(GL_SHADER_STORAGE_BUFFER, reductionSDFSize, NULL, GL_STATIC_DRAW);

	// OUTPUT DATA FROM REDUCTION BUFFER OBJECT
	glGenBuffers(1, &m_bufferSDFoutputdata);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, m_bufferSDFoutputdata);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 32 * 8 * sizeof(float), NULL, GL_STATIC_DRAW);

	glGenBuffers(1, &m_uboIntegrationConfig);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboIntegrationConfig);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(integrateShaderConfigs), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//unsigned int block_index = glGetUniformBlockIndex(integrateProg.getHandle(), "Configs");

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_uboIntegrationConfig, 0, sizeof(integrateShaderConfigs));

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

	// DEPTH TO BUFFER



	//glGenTransformFeedbacks(1, &m_depth_TFO);

	//glGenBuffers(1, &m_depth_VBO);
	//glBindBuffer(GL_ARRAY_BUFFER, m_depth_VBO);
	//glBufferData(GL_ARRAY_BUFFER, m_numberOfCameras * configuration.depthFrameSize.x * configuration.depthFrameSize.y * sizeof(glm::vec4) * 3, NULL, GL_DYNAMIC_COPY);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	

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

void gFusion::uploadDepth(std::vector<rs2::frame_queue> depthQ, int devNumber, glm::vec3 &point)
{


	std::vector<rs2::frame> depthFrame(m_numberOfCameras);


	for (int camNumber = 0; camNumber < m_numberOfCameras; camNumber++)
	{
		//std::cout << "cam number : " << camNumber << std::endl;
		depthQ[camNumber].poll_for_frame(&depthFrame[camNumber]);
		if (depthFrame[camNumber] != NULL)
		{
			if (depthFrame[camNumber].supports_frame_metadata(RS2_FRAME_METADATA_SENSOR_TIMESTAMP))
			{
				auto currentTime = depthFrame[camNumber].get_frame_metadata(RS2_FRAME_METADATA_SENSOR_TIMESTAMP);
				auto deltaTime = currentTime - m_previousTime[camNumber];
				if (deltaTime < 0)
				{
					timeShiftOffsets++;
				}

				m_sensorsTimestamps[camNumber] = currentTime + (timeShiftOffsets * (2 ^ 32));
				m_previousTime[camNumber] = m_sensorsTimestamps[camNumber];
			}


			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureDepthArray);
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, camNumber, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RED, GL_UNSIGNED_SHORT, depthFrame[camNumber].get_data());
		}
		else
		{
			//std::cout << "cam number : " << camNumber << " : was null" << std::endl;
		}

	}

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	if (1)
	{
		//std::cout << "clicky pointy" << std::endl;
		if (depthFrame[devNumber] != NULL)
		{
			const uint16_t* p_depth_frame = reinterpret_cast<const uint16_t*>(depthFrame[devNumber].get_data()); 
//float z = float(depthArray[pointY * depthWidth + pointX]) * (float)cameraInterface.getDepthUnit(cameraDevice) / 1000000.0f;
			int depth_pixel_index = (m_pointY * configuration.depthFrameSize.x + m_pointX);

			glm::vec4 tempPoint(0.0f, 0.0f, 0.0f, 1.0f);

			tempPoint.z = p_depth_frame[depth_pixel_index] * (float)m_depthUnit / 1000000.0f;
			//std::cout << tempPoint.z << std::endl;
			


			//kcamera.fx(), kcamera.fx(), kcamera.ppx(), kcamera.ppy()
			//std::cout << z << std::endl;

			tempPoint.x = (m_pointX - m_camPamsDepth[devNumber].z) * (1.0f / m_camPamsDepth[devNumber].x) * tempPoint.z;
			tempPoint.y = (m_pointY - m_camPamsDepth[devNumber].w) * (1.0f / m_camPamsDepth[devNumber].y) * tempPoint.z;

			m_clickedPoint = false;

			if (devNumber == 0)
			{
				point.x = tempPoint.x;
				point.y = tempPoint.y;
				point.z = tempPoint.z;
			}
			else
			{
				glm::vec4 toutput = m_depthToDepth * tempPoint;
				point.x = toutput.x;
				point.y = toutput.y;
				point.z = toutput.z;
			}

		}
		else
		{
			m_clickedPoint = true;
		}
	}
}

void gFusion::allocateTransformFeedbackBuffers()
{

	//glGenVertexArrays(1, &m_depth_VAO);
	//glBindVertexArray(m_depth_VAO);


	glGenTransformFeedbacks(1, &m_depth_TFO);
	glGenBuffers(1, &m_depth_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_depth_VBO);
	glBufferData(GL_ARRAY_BUFFER, configuration.depthFrameSize.x * configuration.depthFrameSize.y * sizeof(glm::vec4) * 3, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenTransformFeedbacks(1, &m_newUnstable_TFO);
	glGenBuffers(1, &m_newUnstableIndex_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_newUnstableIndex_VBO);
	glBufferData(GL_ARRAY_BUFFER, configuration.depthFrameSize.x * configuration.depthFrameSize.y * sizeof(glm::vec4) * 3, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenTransformFeedbacks(1, &m_globalTarget_TFO);
	glGenBuffers(1, &m_globalTarget_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_globalTarget_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_bufferSize, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenTransformFeedbacks(1, &m_globalRender_TFO); // do i need the TFO here as well?
	glGenBuffers(1, &m_globalRender_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_globalRender_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_bufferSize, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);



	//glGenTransformFeedbacks(1, &m_unstable_TFO);
	//glGenBuffers(1, &m_unstable_VBO);
	//glBindBuffer(GL_ARRAY_BUFFER, m_unstable_VBO);
	//glBufferData(GL_ARRAY_BUFFER, textureDimension * textureDimension * 8, NULL, GL_STREAM_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	//glGenBuffers(1, &m_updateIndex_VBO);
	//glBindBuffer(GL_ARRAY_BUFFER, m_updateIndex_VBO);
	//glBufferData(GL_ARRAY_BUFFER, textureDimension * textureDimension * 16, NULL, GL_STREAM_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);



	//// Atomics
	//glGenBuffers(1, &m_atomicCounterTest);
	//glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterTest);
	//glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
	//glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

}
void gFusion::initSplatterFBOs()
{

	// index map
	//glGenFramebuffers(1, &m_global_FBO);
	//glBindFramebuffer(GL_FRAMEBUFFER, m_global_FBO);

	m_textureGlobalIndexVertConf = GLHelper::createTexture(m_textureGlobalIndexVertConf, GL_TEXTURE_2D, 1, configuration.depthFrameSize.x * indexMapScaleFactor, configuration.depthFrameSize.y * indexMapScaleFactor, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	m_textureGlobalIndexNormRadi = GLHelper::createTexture(m_textureGlobalIndexNormRadi, GL_TEXTURE_2D, 1, configuration.depthFrameSize.x * indexMapScaleFactor, configuration.depthFrameSize.y * indexMapScaleFactor, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	m_textureGlobalIndexColTimDev = GLHelper::createTexture(m_textureGlobalIndexColTimDev, GL_TEXTURE_2D, 1, configuration.depthFrameSize.x * indexMapScaleFactor, configuration.depthFrameSize.y * indexMapScaleFactor, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	m_textureVertexID = GLHelper::createTexture(m_textureVertexID, GL_TEXTURE_2D, 1, configuration.depthFrameSize.x * indexMapScaleFactor, configuration.depthFrameSize.y * indexMapScaleFactor, 1, GL_R32I, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);

	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureGlobalIndexVertConf, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_textureGlobalIndexNormRadi, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_textureGlobalIndexColTimDev, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_textureVertexID, 0);


	//GLenum globalAttachments[4] = { GL_COLOR_ATTACHMENT0,
	//						  GL_COLOR_ATTACHMENT1,
	//						  GL_COLOR_ATTACHMENT2,
	//						  GL_COLOR_ATTACHMENT3 };
	//glDrawBuffers(4, globalAttachments);


	//glGenRenderbuffers(1, &m_global_RBO);
	//glBindRenderbuffer(GL_RENDERBUFFER, m_global_RBO);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, configuration.depthFrameSize.x * indexMapScaleFactor, configuration.depthFrameSize.y * indexMapScaleFactor); // use a single renderbuffer object for both a depth AND stencil buffer.
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_global_RBO); // now actually attach it
	//// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	//	std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);


	// combined
	glGenFramebuffers(1, &m_combined_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_combined_FBO);

	m_textureCombinedIndexVertex = GLHelper::createTexture(m_textureCombinedIndexVertex, GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	m_textureCombinedIndexNormal = GLHelper::createTexture(m_textureCombinedIndexNormal, GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	m_textureCombinedIndexColTim = GLHelper::createTexture(m_textureCombinedIndexColTim, GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	m_textureCombinedIndexConRadDev = GLHelper::createTexture(m_textureCombinedIndexConRadDev, GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureCombinedIndexVertex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_textureCombinedIndexNormal, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_textureCombinedIndexColTim, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_textureCombinedIndexConRadDev, 0);


	GLenum combinedAttachments[4] = {	GL_COLOR_ATTACHMENT0,
										GL_COLOR_ATTACHMENT1,
										GL_COLOR_ATTACHMENT2,
										GL_COLOR_ATTACHMENT3};
	glDrawBuffers(4, combinedAttachments);


	glGenRenderbuffers(1, &m_combined_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_combined_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, configuration.depthFrameSize.x, configuration.depthFrameSize.y); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_combined_RBO); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// depth
	glGenFramebuffers(1, &m_depth_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_depth_FBO);

	m_textureDepthIndexVertConf = GLHelper::createTexture(m_textureDepthIndexVertConf, GL_TEXTURE_2D, configuration.iterations.size(), configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	m_textureDepthIndexNormRadi = GLHelper::createTexture(m_textureDepthIndexNormRadi, GL_TEXTURE_2D, configuration.iterations.size(), configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	m_textureDepthIndexColTimDev = GLHelper::createTexture(m_textureDepthIndexColTimDev, GL_TEXTURE_2D, configuration.iterations.size(), configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureDepthIndexVertConf, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_textureDepthIndexNormRadi, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_textureDepthIndexColTimDev, 0);


	GLenum depthAttachments[3] = { GL_COLOR_ATTACHMENT0,
							  GL_COLOR_ATTACHMENT1,
							  GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, depthAttachments);


	glGenRenderbuffers(1, &m_depth_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depth_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, configuration.depthFrameSize.x, configuration.depthFrameSize.y); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth_RBO); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// old 
	//glGenFramebuffers(1, &m_depth_FBO);
	//glBindFramebuffer(GL_FRAMEBUFFER, m_depth_FBO);

	//m_textureDepthIndexVertConf = GLHelper::createTexture(m_textureDepthIndexVertConf, GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	//m_textureDepthIndexNormRadi = GLHelper::createTexture(m_textureDepthIndexNormRadi, GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	//m_textureDepthIndexColTimDev = GLHelper::createTexture(m_textureDepthIndexColTimDev, GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	//m_textureDepthTime = GLHelper::createTexture(m_textureDepthTime, GL_TEXTURE_2D, 1, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_R32UI, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);

	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureDepthIndexVertConf, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_textureDepthIndexNormRadi, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_textureDepthIndexColTimDev, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_textureDepthTime, 0);


	//GLenum depthAttachments[4] = { GL_COLOR_ATTACHMENT0,
	//						  GL_COLOR_ATTACHMENT1,
	//						  GL_COLOR_ATTACHMENT2,
	//						  GL_COLOR_ATTACHMENT3 };
	//glDrawBuffers(4, depthAttachments);


	//glGenRenderbuffers(1, &m_depth_RBO);
	//glBindRenderbuffer(GL_RENDERBUFFER, m_depth_RBO);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, configuration.depthFrameSize.x, configuration.depthFrameSize.y); // use a single renderbuffer object for both a depth AND stencil buffer.
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth_RBO); // now actually attach it
	//// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	//	std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// FUSE framebuffer
	glGenFramebuffers(1, &m_updateMapIndex_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_updateMapIndex_FBO);

	m_textureUpdateMapIndexVertConf = GLHelper::createTexture(m_textureUpdateMapIndexVertConf, GL_TEXTURE_2D, 1, textureDimension, textureDimension, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	m_textureUpdateMapIndexNormRadi = GLHelper::createTexture(m_textureUpdateMapIndexNormRadi, GL_TEXTURE_2D, 1, textureDimension, textureDimension, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
	m_textureUpdateMapIndexColTimDev = GLHelper::createTexture(m_textureUpdateMapIndexColTimDev, GL_TEXTURE_2D, 1, textureDimension, textureDimension, 1, GL_RGBA32F, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureUpdateMapIndexVertConf, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_textureUpdateMapIndexNormRadi, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_textureUpdateMapIndexColTimDev, 0);


	GLenum updateAttachments[3] = { GL_COLOR_ATTACHMENT0,
							  GL_COLOR_ATTACHMENT1,
							  GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, updateAttachments);


	glGenRenderbuffers(1, &m_updateMapIndex_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_updateMapIndex_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, textureDimension, textureDimension); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_updateMapIndex_RBO); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);







}
void gFusion::initSplatterVAOs()
{
	//GLint maxNumber;
	//glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS, &maxNumber);

	glGenVertexArrays(1, &m_data_VAO);


	glGenVertexArrays(1, &m_depth_VAO);
	glBindVertexArray(m_depth_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_depth_VBO);

	// vertex confidence
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * 3, (GLvoid*)0);
	// normal radius
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * 3, (GLvoid*)(sizeof(glm::vec4) * 1));
	//// color time device
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * 3, (GLvoid*)(sizeof(glm::vec4) * 2));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	glGenVertexArrays(1, &m_globalTarget_VAO);
	glBindVertexArray(m_globalTarget_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_globalTarget_VBO);

	// vertex confidence
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * 3, (GLvoid*)0);
	// normal radius
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * 3, (GLvoid*)(sizeof(glm::vec4) * 1));
	//// color time device
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * 3, (GLvoid*)(sizeof(glm::vec4) * 2));

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);



	glGenVertexArrays(1, &m_globalRender_VAO);
	glBindVertexArray(m_globalRender_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_globalRender_VBO);

	// vertex confidence
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * 3, (GLvoid*)0);
	// normal radius
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * 3, (GLvoid*)(sizeof(glm::vec4) * 1));
	//// color time device
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * 3, (GLvoid*)(sizeof(glm::vec4) * 2));

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void gFusion::uploadDepthToBuffer(std::vector<rs2::frame_queue> depthQ, int devNumber, glm::vec3 &point, uint32_t counter)
{

	m_frameCount = counter;
	std::vector<rs2::frame> depthFrame(m_numberOfCameras);


	for (int camNumber = 0; camNumber < m_numberOfCameras; camNumber++)
	{
		//std::cout << "cam number : " << camNumber << std::endl;
		depthQ[camNumber].poll_for_frame(&depthFrame[camNumber]);
		if (depthFrame[camNumber] != NULL)
		{
			if (depthFrame[camNumber].supports_frame_metadata(RS2_FRAME_METADATA_SENSOR_TIMESTAMP))
			{
				auto currentTime = depthFrame[camNumber].get_frame_metadata(RS2_FRAME_METADATA_SENSOR_TIMESTAMP);
				auto deltaTime = currentTime - m_previousTime[camNumber];
				if (deltaTime < 0)
				{
					timeShiftOffsets++;
				}

				m_sensorsTimestamps[camNumber] = currentTime + (timeShiftOffsets * (2 ^ 32));
				m_previousTime[camNumber] = m_sensorsTimestamps[camNumber];
			}


			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureDepthArray);
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, camNumber, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_RED, GL_UNSIGNED_SHORT, depthFrame[camNumber].get_data());
		}
		else
		{
			//std::cout << "cam number : " << camNumber << " : was null" << std::endl;
		}

	}

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	if (1)
	{
		//std::cout << "clicky pointy" << std::endl;
		if (depthFrame[devNumber] != NULL)
		{
			const uint16_t* p_depth_frame = reinterpret_cast<const uint16_t*>(depthFrame[devNumber].get_data());
			//float z = float(depthArray[pointY * depthWidth + pointX]) * (float)cameraInterface.getDepthUnit(cameraDevice) / 1000000.0f;
			int depth_pixel_index = (m_pointY * configuration.depthFrameSize.x + m_pointX);

			glm::vec4 tempPoint(0.0f, 0.0f, 0.0f, 1.0f);

			tempPoint.z = p_depth_frame[depth_pixel_index] * (float)m_depthUnit / 1000000.0f;
			//std::cout << tempPoint.z << std::endl;

			tempPoint.x = (m_pointX - m_camPamsDepth[devNumber].z) * (1.0f / m_camPamsDepth[devNumber].x) * tempPoint.z;
			tempPoint.y = (m_pointY - m_camPamsDepth[devNumber].w) * (1.0f / m_camPamsDepth[devNumber].y) * tempPoint.z;

			m_clickedPoint = false;

			if (devNumber == 0)
			{
				point.x = tempPoint.x;
				point.y = tempPoint.y;
				point.z = tempPoint.z;
			}
			else
			{
				glm::vec4 toutput = m_depthToDepth * tempPoint;
				point.x = toutput.x;
				point.y = toutput.y;
				point.z = toutput.z;
			}


			// use transform feedback to write verts to buffer rather than sparse image array

			depthToBufferProg.use();
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureDepthArray);
			
			glm::vec4 camPam[4];

			for (int i = 0; i < m_numberOfCameras; i++)
			{
				camPam[i] = glm::vec4(m_camPamsDepth[i].z, m_camPamsDepth[i].w, 1.0f / m_camPamsDepth[i].x, 1.0f / m_camPamsDepth[i].y);
				//camPam[i].z = 1.0f / camPam[i].z;
				//camPam[i].w = 1.0f / camPam[i].w;
			}

			glm::mat4 invK[4];
			for (int i = 0; i < m_numberOfCameras; i++)
			{
				invK[i] = GLHelper::getInverseCameraMatrix(m_camPamsDepth[i]);
			}

			glUniform4fv(m_camPamID_d2b, 4, glm::value_ptr(camPam[0])); 
			glUniform1f(m_depthScaleID_d2b, m_depthUnit / 1000000.0f); // 1000 == each depth unit == 1 mm
			glUniformMatrix4fv(m_invKID_d2b, 4, GL_FALSE, glm::value_ptr(invK[0]));
			glUniform1i(m_initUnstableID_d2b, m_initUnstable);
			glUniform1ui(m_frameCountID_d2b, m_frameCount);
			glBindVertexArray(m_depth_VAO);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_depth_VBO);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_newUnstableIndex_VBO);

			//glBindBuffer(GL_ARRAY_BUFFER, m_depth_VBO);
			glEnable(GL_RASTERIZER_DISCARD);

			glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_depth_TFO);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_depth_VBO);

			// FOR WIPING and drawing DO WE HAVE TO WIPE DEPTH TOO?
			glBindImageTexture(0, m_textureDepthIndexVertConf, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glBindImageTexture(1, m_textureDepthIndexNormRadi, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glBindImageTexture(2, m_textureDepthIndexColTimDev, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glBindImageTexture(3, m_textureCombinedIndexVertex, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glBindImageTexture(4, m_textureCombinedIndexNormal, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glBindImageTexture(5, m_textureCombinedIndexColTim, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glBindImageTexture(6, m_textureCombinedIndexConRadDev, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

			GLuint query;
			glGenQueries(1, &query);
			glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);

			glBeginTransformFeedback(GL_POINTS);

			glDrawArrays(GL_POINTS, 0, m_numberOfCameras * configuration.depthFrameSize.x * configuration.depthFrameSize.y); // multiply this by number of cameras

			glBindTexture(GL_TEXTURE_2D, 0);

			glActiveTexture(GL_TEXTURE0);
			glEndTransformFeedback();

			glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
			glGetQueryObjectuiv(query, GL_QUERY_RESULT, &inputDepthCount);

			//std::cout << "prims : " << inputDepthCount << std::endl;

			glFlush();
			

			glDisable(GL_RASTERIZER_DISCARD);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_textureDepthIndexVertConf);
			glGenerateMipmap(GL_TEXTURE_2D);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_textureDepthIndexNormRadi);
			glGenerateMipmap(GL_TEXTURE_2D);

			//glClearNamedBufferSubData(m_newUnstableIndex_VBO, GL_R32F, 0, textureDimension * textureDimension * 8, GL_FLOAT, )


		}
		else
		{
			m_clickedPoint = true;
		}
	}
}

void gFusion::runBilateralFilter()
{
	bilateralFilterProg.use();

	glBindImageTexture(0, m_textureDepthArray, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R16UI);
	glBindImageTexture(1, m_textureDepthFilteredArray, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R16UI);

	float bfSigma = 15.0f;
	float bfDSigma = 0.05f;

	glUniform1f(m_bfDepthScaleID, m_depthUnit / 1000000.0f); // 1000 == each depth unit == 1 mm

	glUniform1f(m_bfSigmaID, bfSigma);
	glUniform1f(m_bfDSigmaID, bfDSigma);

	int compWidth = divup((int)configuration.depthFrameSize.x, 32); // right bitshift does division by powers of 2
	int compHeight = divup((int)configuration.depthFrameSize.y, 32);

	glDispatchCompute(compWidth, compHeight, 1);

}

void gFusion::initSplatterFusion()
{

	// just copy instead of making another transform feedback
	// make sure the times are set correctly, i.e. make sure the unstable time is set to 1 ratehr than frame time, or whatever it needs to be
	glCopyNamedBufferSubData(m_depth_VBO, m_globalTarget_VBO, 0, 0, inputDepthCount * 3 * sizeof(glm::vec4));
	globalVertCount = inputDepthCount;



}


// makes a splat of global model in current camera view
// fragment shader used to set size of splat in the final image
void gFusion::combinedPredict()
{
	combinedPredictProg.use();

	glBindVertexArray(m_globalTarget_VAO);

	glBindFramebuffer(GL_FRAMEBUFFER, m_combined_FBO);

	glViewport(0, 0, configuration.depthFrameSize.x, configuration.depthFrameSize.y);

	GLfloat black[] = { 0, 0, 0, 1 };
	GLfloat red[] = { 2.0f, 0.0f, 0.0f, 1.0f };
	GLfloat blackDepth[] = { 0.0f };

	//glClearTexImage(m_textureCombinedIndexVertConf, 0, GL_RGBA, GL_FLOAT, black);
	//glClearTexImage(m_textureCombinedIndexNormRadi, 0, GL_RGBA, GL_FLOAT, red);

	//glClearBufferfv(GL_COLOR, 0, black);
	//glClearBufferfv(GL_COLOR, 1, red);
	//glClearBufferfi(GL_DEPTH_STENCIL, 0, 10.0f, 0);

	glClearTexImage(m_textureDist, 0, GL_RGBA, GL_FLOAT, black);

	glBindImageTexture(0, m_textureDist, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glm::mat4 inversePose[4];
	for (int i = 0; i < m_numberOfCameras; i++)
	{
		//model[i] = glm::inverse(m_pose);
		inversePose[i] = glm::inverse(m_pose);

	}
	glm::vec4 camPam[4];
	for (int i = 0; i < m_numberOfCameras; i++)
	{
		camPam[i] = vec4(m_camPamsDepth[i].z, m_camPamsDepth[i].w, m_camPamsDepth[i].x, m_camPamsDepth[i].y);
	}

	glUniformMatrix4fv(m_cpInversePoseID, 4, GL_FALSE, glm::value_ptr(inversePose[0]));
	glUniform4fv(m_cpCamPamID, 1, glm::value_ptr(camPam[0]));
	glUniform2fv(m_cpImSizeID, 1, glm::value_ptr(configuration.depthFrameSize));
	glUniform1f(m_cpMaxDepthID, m_depthMax); 
	glUniform1i(m_cpTimeID, m_frameCount);
	glUniform1f(m_cpConfThresholdID, 10.0f); // NOT YET SET
	glUniform1i(m_cpMaxTimeID, m_frameCount);
	glUniform1i(m_cpTimeDeltaID, m_timeDelta);

	//glBindBuffer(GL_ARRAY_BUFFER, m_globalTarget_VBO);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_globalTarget_VBO);


	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);

	if (m_initUnstable == 1)
	{
		glDrawArrays(GL_POINTS, 0, inputDepthCount); // on first run, there are no verts in the global buffer, so we take the number of verts in the depth frame (i.e. the last frame that was initunstabled to the global buffer)
	}
	else
	{
		//glDrawTransformFeedback(GL_POINTS, m_globalTarget_TFO);
		glDrawArrays(GL_POINTS, 0, globalVertCount); // this should include stable and unstable points, but only stable points get 
	}
	//glDrawTransformFeedback(GL_POINTS, m_globalTarget_TFO);

	//GET CURRENT SIZE OF GLOBAL COUNT HERE THEN PASS IT TO THE UPDATE MODEL BIT BELOW

	//glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	//glGetQueryObjectuiv(query, GL_QUERY_RESULT, &globalVertCount);

	//std::cout << "globs : " << globalVertCount << " : input : " << inputDepthCount << std::endl;


	//glFinish();
	glDisable(GL_PROGRAM_POINT_SIZE);
	glDisable(GL_POINT_SPRITE);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//std::vector<float> testvec(848 * 480 * 4, 10);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureDist);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, testvec.data());
	//glBindTexture(GL_TEXTURE_2D, 0);
	//cv::Mat combo = cv::Mat(480, 848, CV_32FC4, testvec.data());
	//cv::imshow("combined", combo);
	//cv::waitKey(1);

}

void gFusion::predictIndices()
{
	// here is were we project to a 4x index map
	//glBindVertexArray(m_global_VAO);

	//glBindFramebuffer(GL_FRAMEBUFFER, m_global_FBO);

	GLfloat black[] = {0,0,0, 1 };
	GLint blackInt[] = { 0 };

	glClearTexImage(m_textureGlobalIndexVertConf, 0, GL_RGBA, GL_FLOAT, black);
	glClearTexImage(m_textureGlobalIndexNormRadi, 0, GL_RGBA, GL_FLOAT, black);

	glClearTexImage(m_textureGlobalIndexColTimDev, 0, GL_RGBA, GL_FLOAT, black);
	glClearTexImage(m_textureVertexID, 0, GL_RED_INTEGER, GL_INT, blackInt);


	//glClear(GL_DEPTH_BUFFER_BIT);

	createIndexMapProg.use();
	glClearTexImage(m_textureDist, 0, GL_RGBA, GL_FLOAT, black);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_globalTarget_VBO);

	glm::mat4 model[4];

	for (int i = 0; i < m_numberOfCameras; i++)
	{
		model[i] = glm::inverse(m_pose);
	}

	glm::vec4 camPam[4];

	for (int i = 0; i < m_numberOfCameras; i++)
	{
		camPam[i] = glm::vec4(m_camPamsDepth[i].z, m_camPamsDepth[i].w, m_camPamsDepth[i].x, m_camPamsDepth[i].y);
	}

	glUniformMatrix4fv(m_indexInversePoseID, 1, GL_FALSE, glm::value_ptr(model[0]));
	glUniform4fv(m_indexCamPamID, 1, glm::value_ptr(camPam[0] * indexMapScaleFactor));
	glUniform2fv(m_indexImSizeID, 1, glm::value_ptr((glm::vec2(configuration.depthFrameSize * indexMapScaleFactor))));
	glUniform1f(m_indexMaxDepthID, m_depthMax);
	glUniform1i(m_indexTimeID, m_frameCount); 
	glUniform1i(m_indexTimeDeltaID, m_timeDelta);

	//glBindBuffer(GL_ARRAY_BUFFER, m_globalTarget_VBO);

	glBindImageTexture(0, m_textureGlobalIndexVertConf,  0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); // to check for depth overwrites
	glBindImageTexture(1, m_textureGlobalIndexNormRadi,  0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(2, m_textureGlobalIndexColTimDev, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(3, m_textureVertexID,             0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32I);

	glBindImageTexture(4, m_textureDist, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);


	//glEnable(GL_RASTERIZER_DISCARD);

	//glDrawArrays(GL_POINTS, 0, inputDepthCount);
	//glDrawTransformFeedback(GL_POINTS, m_globalTarget_TFO); // over all the global verts // this works, but nsite cant see it, theres a documented bug/issue on this. its the way nsite grabs the frames or something
		

	int xthreads;
	xthreads = divup(globalVertCount, 1024);

	//std::cout << "xthreads : " << xthreads << std::endl;

	glDispatchCompute(xthreads, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//	std::vector<float> testvec(848 * 480 * 4, 10);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureDist);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, testvec.data());
	//glBindTexture(GL_TEXTURE_2D, 0);
	//cv::Mat combo = cv::Mat(480, 848, CV_32FC4, testvec.data());
	//cv::imshow("combined", combo * 1000);
	//cv::waitKey(1);

}



// using the textures generated by combined predict and depth to vertex
// perform depth to depth icp
bool gFusion::TrackSplat()
{
	//glBeginQuery(GL_TIME_ELAPSED, query[0]);
	bool tracked = false;
	glm::mat4 oldPose = m_pose;

	float alignmentEnergy;
	float lastICPCount;

	m_splatPose = m_pose;


	// here we will loop through the layers and number of iterations per layer
	for (int level = configuration.iterations.size() - 1; level >= 0; --level)
	{
		//int level = 0;
		for (int iteration = 0; iteration < configuration.iterations[level]; iteration++)
		{
			std::vector<float> b;
			std::vector<float> C;

			//for (int i = 0; i < m_numberOfCameras; i++)
			//{
			trackSplat(level);
			reduceSplat(level);

			//}
			//for (int i = 0; i < m_numberOfCameras; i++)
			//{
			//}
			getReduction(b, C, alignmentEnergy, lastICPCount);



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

			m_splatPose = delta * m_splatPose;

			//std::cout << " old " << glm::to_string(m_splatPose) << std::endl;


			//std::cout << "level " << level << " iteration " << iteration << " AE:" << alignmentEnergy <<  " RESNORM  : " << result.norm() << std::endl;


			if (result.norm() < 1.0e-4 && result.norm() != 0)
				break;
		}
	}

	//std::cout << " " << std::endl;
	//std::cout << "new " << glm::to_string(glm::transpose(m_pose)[0]) << std::endl;
	//std::cout << "new " << glm::to_string(glm::transpose(m_pose)[1]) << std::endl;
	//std::cout << "new " << glm::to_string(glm::transpose(m_pose)[2]) << std::endl;
	//std::cout << "new " << glm::to_string(glm::transpose(m_pose)[3]) << std::endl;
	//std::cout << " " << std::endl;
	//std::cout << "spl " << glm::to_string(glm::transpose(m_splatPose)[0]) << std::endl;
	//std::cout << "spl " << glm::to_string(glm::transpose(m_splatPose)[1]) << std::endl;
	//std::cout << "spl " << glm::to_string(glm::transpose(m_splatPose)[2]) << std::endl;
	//std::cout << "spl " << glm::to_string(glm::transpose(m_splatPose)[3]) << std::endl;
	//std::cout << " " << std::endl;


	//std::cout << "AE : " << alignmentEnergy << " " << lastICPCount << std::endl;
	if (alignmentEnergy > 2.0e-2f || alignmentEnergy == 0 || isnan(alignmentEnergy))
	{
		m_splatPose = oldPose;
	}
	else
	{
		tracked = true;
		//updatePoseFinder();
	}

	m_alignmentEnergy = alignmentEnergy;

	m_pose = m_splatPose;
	//tracked = true;

	//std::cout << glm::to_string(m_pose) << std::endl;

	//glEndQuery(GL_TIME_ELAPSED);
	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query[0], GL_QUERY_RESULT_AVAILABLE, &available);
	//}
	//// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query[0], GL_QUERY_RESULT, &elapsed);
	//trackTime = elapsed / 1000000.0;

	return tracked;
}

void gFusion::trackSplat(int level)
{
	int devNumber = 0; // FIX ME 
	glm::mat4 invK = GLHelper::getInverseCameraMatrix(m_camPamsDepth[devNumber]);
	//glm::mat4 oldPose = pose;
	//glm::mat4 projectReference = GLHelper::getCameraMatrix(m_camPamsDepth[devNumber]) * glm::inverse(m_pose);

	glm::vec4 camPam[4];
	for (int i = 0; i < m_numberOfCameras; i++)
	{
		camPam[i] = vec4(m_camPamsDepth[i].z, m_camPamsDepth[i].w, m_camPamsDepth[i].x, m_camPamsDepth[i].y);
	}


	GLfloat black[] = { 0, 0, 0, 1 };
	glClearTexImage(m_textureDist, 0, GL_RGBA, GL_FLOAT, black);


	int compWidth;
	int compHeight;

	trackSplatProg.use();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_buffer_reduction);

	glm::mat4 cameraPoses[4];
	cameraPoses[0] = m_splatPose;
	cameraPoses[1] = m_splatPose * m_depthToDepth;

	glm::mat4 inverseVP[4];
	for (int i = 0; i < m_numberOfCameras; i++)
	{
		inverseVP[i] = GLHelper::getCameraMatrix(m_camPamsDepth[i]) * glm::inverse(cameraPoses[i]);
	}
	glUniform4fv(m_camPamID_tsp, 1, glm::value_ptr(camPam[0]));

	glProgramUniformMatrix4fv(trackSplatProg.getHandle(), m_cameraPosesID_tsp, 1, GL_FALSE, glm::value_ptr(cameraPoses[0]));
	glProgramUniformMatrix4fv(trackSplatProg.getHandle(), m_inverseVPID_tsp, 1, GL_FALSE, glm::value_ptr(inverseVP[0]));

	//glUniformMatrix4fv(m_viewID_tsp, 1, GL_FALSE, glm::value_ptr(projectReference));
	//glUniformMatrix4fv(m_TtrackID_tsp, 1, GL_FALSE, glm::value_ptr(m_pose));
	glUniform1f(m_distThresh_tsp, configuration.dist_threshold);
	glUniform1f(m_normThresh_tsp, configuration.normal_threshold);
	glUniform1i(m_numberOfCamerasID_tsp, m_numberOfCameras);

	glBindImageTexture(0, m_textureDepthIndexVertConf, level, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, m_textureDepthIndexNormRadi, level, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);

	glBindImageTexture(2, m_textureCombinedIndexVertex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F); // the raycasted images do not need to be scaled, since the mipmaped verts are projected back to 512*424 image space resolution in the shader
	glBindImageTexture(3, m_textureCombinedIndexNormal, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glBindImageTexture(4, m_textureDifferenceVertex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
	glBindImageTexture(5, m_textureTrackImage, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

	glBindImageTexture(6, m_textureDist, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	compWidth = divup((int)configuration.depthFrameSize.x >> level, 32); // right bitshift does division by powers of 2
	compHeight = divup((int)configuration.depthFrameSize.y >> level, 32);

	glDispatchCompute(compWidth, compHeight, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//if (level == 0)
	//{
	//	std::vector<float> testvec(848 * 480 * 4, 10);
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, m_textureDist);
	//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, testvec.data());
	//	glBindTexture(GL_TEXTURE_2D, 0);
	//	cv::Mat combo = cv::Mat(480, 848, CV_32FC4, testvec.data());
	//	cv::imshow("combined", combo);
	//	cv::waitKey(1);
	//}


	//size_t reductionSize = m_numberOfCameras * configuration.depthFrameSize.x * configuration.depthFrameSize.y * (sizeof(GLint) + (sizeof(GLfloat) * 7)); // this is the size of one reduction element 1 int + 1 float + 6 float
	//std::vector<gTrackData> trackData(reductionSize);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer_reduction);
	//void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	////memcpy_s(outputData.data(), outputData.size() * sizeof(float), ptr, outputData.size() * sizeof(float));
	//memcpy(trackData.data(), ptr, trackData.size() * sizeof(gTrackData));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


}

void gFusion::reduceSplat(int level)
{

	//glBeginQuery(GL_TIME_ELAPSED, query[1]);

	glm::ivec2 imageSize = glm::ivec2((int)(configuration.depthFrameSize.x * m_numberOfCameras) >> level, (int)configuration.depthFrameSize.y >> level);

	reduceSplatProg.use();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_buffer_reduction);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_buffer_outputdata);

	glUniform2iv(m_imageSizeID_rsp, 1, glm::value_ptr(imageSize));
	//glBindImageTexture(0, m_textureOutputData, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	glDispatchCompute(8, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void gFusion::fuse()
{
	dataProg.use();

	glEnable(GL_RASTERIZER_DISCARD);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_newUnstable_TFO);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_newUnstableIndex_VBO);

	glBindVertexArray(m_globalTarget_VAO);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureVertexID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_textureGlobalIndexVertConf);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_textureGlobalIndexNormRadi);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_textureGlobalIndexColTimDev);

	GLfloat black[] = { 0, 0, 0, 1 };
	glClearTexImage(m_textureDist, 0, GL_RGBA, GL_FLOAT, black);

	glBindImageTexture(0, m_textureDist, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glm::mat4 pose[4];

	for (int i = 0; i < m_numberOfCameras; i++)
	{
		pose[i] = (m_pose);
	}

	glm::vec4 camPam[4];

	for (int i = 0; i < m_numberOfCameras; i++)
	{
		camPam[i] = glm::vec4(m_camPamsDepth[i].z, m_camPamsDepth[i].w, 1.0f / m_camPamsDepth[i].x, 1.0f / m_camPamsDepth[i].y);
	}

	glUniformMatrix4fv(m_dpPoseID, 1, GL_FALSE, glm::value_ptr(pose[0]));
	glUniform4fv(m_dpCamPamID, 1, glm::value_ptr(camPam[0]));
	glUniform1f(m_dpMaxDepthID, m_depthMax);
	glUniform1ui(m_dpTimeID, m_frameCount);

	glUniform1f(m_dpScaleID, 4.0f);
	glUniform1f(m_dpWeightingID, 1.0f); // NOT YET SET

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_depth_VBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_globalTarget_VBO);

	GLuint query;
	glGenQueries(1, &query);
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
	glBeginTransformFeedback(GL_POINTS);

	glDrawArrays(GL_POINTS, 0, inputDepthCount); // this should be the number of valid depth points in the active region/volume

	glEndTransformFeedback();

	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &newUnstableCount);

	glFlush();

	glDisable(GL_RASTERIZER_DISCARD);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

	//std::vector<float> testvec(848 * 480 * 4, 10);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureDist);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, testvec.data());
	//glBindTexture(GL_TEXTURE_2D, 0);
	//cv::Mat combo = cv::Mat(480, 848, CV_32FC4, testvec.data());
	//cv::imshow("combined", combo * 100.0f);
	//cv::waitKey(1);


	// CLEAN GLOBAL MODEL
	// elastic fusion uses two passes here, firstly to clean the global model of close points, or if the normals of any of the verts are very different from their surrounding verts as seen in the index map
	// on the second pass the the new unstable points are added to the global model
	// i think they must use the fact that their new unstable map is the same size as the global model, otherwise the points would drop off, right?
	// we should be able to just use one pass with global count + new unstable count, rendering to a fresh tiktok buffer
	
	cleanGlobalProg.use();
	glBindVertexArray(m_globalTarget_VAO);

	glClearTexImage(m_textureDist, 0, GL_RGBA, GL_FLOAT, black);

	glEnable(GL_RASTERIZER_DISCARD);
	
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_globalRender_TFO);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_globalRender_VBO);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_newUnstableIndex_VBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_globalTarget_VBO);

	glBindImageTexture(4, m_textureDist, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);


	glBindImageTexture(0, m_textureGlobalIndexVertConf,  0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, m_textureGlobalIndexNormRadi,  0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(2, m_textureGlobalIndexColTimDev, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(3, m_textureVertexID,             0, GL_TRUE, 0, GL_READ_ONLY, GL_R32I);
	
	glm::mat4 inversePose[4];

	for (int i = 0; i < m_numberOfCameras; i++)
	{
		inversePose[i] = glm::inverse(m_pose);
	}

	glUniformMatrix4fv(m_cgInversePoseID, 4, GL_FALSE, glm::value_ptr(inversePose[0]));
	glUniform4fv(m_cgCamPamID, 1, glm::value_ptr(camPam[0]));
	glUniform1i(m_cgTimeID, m_frameCount);
	glUniform1i(m_cgTimeDeltaID, m_timeDelta);
	glUniform1f(m_cgConfThresholdID, 10.0f); // NOT YET SET PROPERLY
	glUniform1ui(m_cgCurrentGlobalNumberID, globalVertCount);

	GLuint finalQuery, f2q;
	glGenQueries(1, &finalQuery);
	glGenQueries(1, &f2q);

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, finalQuery);

	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, globalVertCount + newUnstableCount); // contains 
	glEndTransformFeedback();

	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glGetQueryObjectuiv(finalQuery, GL_QUERY_RESULT, &finalVertCount);



	std::cout << "GC : " << globalVertCount << " : UC : " << newUnstableCount << " : FVC : " << finalVertCount << std::endl;
	globalVertCount = finalVertCount;

	glFlush();
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, f2q);

	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, globalVertCount); // contains 
	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glGetQueryObjectuiv(f2q, GL_QUERY_RESULT, &finalVertCount);

	std::cout << "GC : " << globalVertCount << " : F2VC : " << finalVertCount << std::endl;

	globalVertCount = finalVertCount;

	glDisable(GL_RASTERIZER_DISCARD);
	glFlush();
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

	globalVertCount = finalVertCount;

	std::swap(m_globalRender_TFO, m_globalTarget_TFO);
	std::swap(m_globalRender_VBO, m_globalTarget_VBO);
	std::swap(m_globalRender_VAO, m_globalTarget_VAO);

	//std::vector<float> testvec(848 * 480 * 4, 10);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureDist);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, testvec.data());
	//glBindTexture(GL_TEXTURE_2D, 0);
	//cv::Mat combo = cv::Mat(480, 848, CV_32FC4, testvec.data());
	//cv::imshow("combined", combo);
	//cv::waitKey(1);

}



void gFusion::depthToVertex(float * depthArray)
{
	//int compWidth;
	//int compHeight; 

	//// gltexsubimage2d  
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureDepth);
	//if (depthArray != NULL)
	//{
	//	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, configuration.depthFrameSize.x, configuration.depthFrameSize.y, GL_RED, GL_FLOAT, depthArray);
	//}
	//glGenerateMipmap(GL_TEXTURE_2D);

	////std::vector<float> testvec(480 * 848 / 2, 10);


	////glActiveTexture(GL_TEXTURE0);
	////glBindTexture(GL_TEXTURE_2D, m_textureDepth);
	////glGetTexImage(GL_TEXTURE_2D, 1, GL_RED, GL_FLOAT, testvec.data());
	////glBindTexture(GL_TEXTURE_2D, 0);
	////

	////cv::Mat lvl1d = cv::Mat(480 / 2, 848 / 2, CV_32FC1, testvec.data());

	////cv::imshow("lvl1!d", lvl1d);

	//depthToVertProg.use();

	//for (int i = 0; i < 3; i++) // here 3 is the number of mipmap levels
	//{
	//	compWidth = divup((int)configuration.depthFrameSize.x >> i, 32);
	//	compHeight = divup((int)configuration.depthFrameSize.y >> i, 32);

	//	glm::mat4 invK = GLHelper::getInverseCameraMatrix(m_camPamsDepth / float(1 << i));
	//	glBindImageTexture(0, m_textureDepth, i, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
	//	glBindImageTexture(2, m_textureVertex, i, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	//	glUniformMatrix4fv(m_invkID, 1, GL_FALSE, glm::value_ptr(invK));
	//	glUniform4fv(m_camPamsDepthID, 1, glm::value_ptr(m_camPamsDepth));
	//	glUniform1i(m_imageTypeID, 1); // 1 == type float
	//	glUniform1f(m_depthScaleID, 1000); // 1000 == each depth unit == 1 mm

	//	glDispatchCompute(compWidth, compHeight, 1);
	//	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	//}



}

void gFusion::depthToVertex()
{
	int compWidth;
	int compHeight;

	depthToVertProg.use();


	compWidth = divup((int)configuration.depthFrameSize.x, 32);
	compHeight = divup((int)configuration.depthFrameSize.y, 32);

	glm::mat4 invK[4];
	for (int i = 0; i < m_numberOfCameras; i++)
	{
		invK[i] = GLHelper::getInverseCameraMatrix(m_camPamsDepth[i]); 
	}
	//glm::mat4 colorK = GLHelper::getCameraMatrix(m_camPamsColor[0]);
	// invK can be sent to the shader, and only the m[0][0] and m[1][1] need to be divided by their mip level (1 << i)


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureDepthArray);
	glBindImageTexture(0, m_textureVertexArray, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(1, m_textureVertexArray, 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(2, m_textureVertexArray, 2, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glUniformMatrix4fv(m_invkID, 4, GL_FALSE, glm::value_ptr(invK[0]));

	//glUniformMatrix4fv(m_colorKID, 1, GL_FALSE, glm::value_ptr(colorK));

	//glUniformMatrix4fv(m_extrinsicsID, 1, GL_FALSE, glm::value_ptr(m_depthToDepth));

	glUniform1f(m_depthScaleID, m_depthUnit / 1000000.0f); // 1000 == each depth unit == 1 mm
	glUniform1i(m_numberOfCamerasID_d, m_numberOfCameras);
	glDispatchCompute(compWidth, compHeight, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);


}

void gFusion::vertexToNormal()
{
	int compWidth;
	int compHeight;

	vertToNormProg.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureVertexArray);
	glBindImageTexture(0, m_textureNormalArray, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(1, m_textureNormalArray, 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(2, m_textureNormalArray, 2, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	compWidth = divup((int)configuration.depthFrameSize.x, 32);
	compHeight = divup((int)configuration.depthFrameSize.y, 32);
	glUniform1i(m_numberOfCamerasID_v, m_numberOfCameras);

	glDispatchCompute(compWidth, compHeight, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);


}

bool gFusion::Track()
{


	shaderConfigs.d2p = 1;
	shaderConfigs.d2v = 0;

	glBeginQuery(GL_TIME_ELAPSED, query[0]);

	bool tracked = false;
	glm::mat4 oldPose = m_pose;
	m_splatPose = m_pose;
	float alignmentEnergy;
	float lastICPCount;

	// here we will loop through the layers and number of iterations per layer
	for (int level = configuration.iterations.size() - 1; level >= 0; --level)
	{
		//int level = 0;
		for (int iteration = 0; iteration < configuration.iterations[level]; iteration++)
		{
			std::vector<float> b;
			std::vector<float> C;

			//for (int i = 0; i < m_numberOfCameras; i++)
			//{
				track(0, level);
				reduce(0, level);

			//}
			for (int i = 0; i < m_numberOfCameras; i++)
			{
			}
			getReduction(b, C, alignmentEnergy, lastICPCount);


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

			if (result.norm() < 1e-5 && result.norm() != 0)
				break;
		}
	}


	//std::cout << "p2p " << glm::to_string(m_pose) << std::endl;
	std::cout << alignmentEnergy << " : icp count " << lastICPCount <<  std::endl;
	if (alignmentEnergy > 2.0e-2f || alignmentEnergy == 0)
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

void gFusion::track(int devNumber, int layer)
{

	//glm::mat4 invK = GLHelper::getInverseCameraMatrix(m_camPamsDepth[devNumber]);
	//glm::mat4 oldPose = pose;
	//glm::mat4 projectReference = GLHelper::getCameraMatrix(m_camPamsDepth[devNumber]) * glm::inverse(m_pose);

	int compWidth;
	int compHeight;

	trackProg.use();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_buffer_reduction);

	glm::mat4 cameraPoses[4];
	cameraPoses[0] = m_pose;
	cameraPoses[1] = m_pose * m_depthToDepth;


	glm::mat4 inverseCameraPoses[4];
	for (int i = 0; i < m_numberOfCameras; i++)
	{
		inverseCameraPoses[i] = GLHelper::getCameraMatrix(m_camPamsDepth[i]) * glm::inverse(cameraPoses[i]);
	}

	glProgramUniformMatrix4fv(trackProg.getHandle(), m_cameraPosesID_tr, 4, GL_FALSE, glm::value_ptr(cameraPoses[0]));
	glProgramUniformMatrix4fv(trackProg.getHandle(), m_inverseCameraPosesID_tr, 4, GL_FALSE, glm::value_ptr(inverseCameraPoses[0]));

	//glUniformMatrix4fv(m_viewID_t, 1, GL_FALSE, glm::value_ptr(projectReference));
	//glUniformMatrix4fv(m_TtrackID, 1, GL_FALSE, glm::value_ptr(m_pose));
	glUniform1f(m_distThresh_t, configuration.dist_threshold);
	glUniform1f(m_normThresh_t, configuration.normal_threshold);
	glUniform1i(m_numberOfCamerasID_tr, m_numberOfCameras);

	glBindImageTexture(0, m_textureVertexArray, layer, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(1, m_textureNormalArray, layer, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

	glBindImageTexture(2, m_textureReferenceVertexArray, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F); // the raycasted images do not need to be scaled, since the mipmaped verts are projected back to 512*424 image space resolution in the shader
	glBindImageTexture(3, m_textureReferenceNormalArray, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);
	
	glBindImageTexture(4, m_textureDifferenceVertex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
	glBindImageTexture(5, m_textureTrackImage, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

	GLfloat black[] = { 0, 0, 0, 1 };
	glClearTexImage(m_textureDist, 0, GL_RGBA, GL_FLOAT, black);

	glBindImageTexture(6, m_textureDist, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	
	compWidth = divup((int)configuration.depthFrameSize.x >> layer, 32); // right bitshift does division by powers of 2
	compHeight = divup((int)configuration.depthFrameSize.y >> layer, 32);

	glDispatchCompute(compWidth, compHeight, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//std::vector<float> testvec(480 * 848*4, 10);

	//if (layer == 0)
	//{
	//	std::vector<float> testvec(848 * 480 * 4, 10);
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, m_textureDist);
	//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, testvec.data());
	//	glBindTexture(GL_TEXTURE_2D, 0);
	//	cv::Mat combo = cv::Mat(480, 848, CV_32FC4, testvec.data());
	//	cv::imshow("combined", combo);
	//	cv::waitKey(1);
	//}


	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureReferenceVertex);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, testvec.data());
	//glBindTexture(GL_TEXTURE_2D, 0);
	//



}

void gFusion::reduce(int devNumber, int layer)
{


	//glBeginQuery(GL_TIME_ELAPSED, query[1]);

	glm::ivec2 imageSize = glm::ivec2((int)(configuration.depthFrameSize.x * m_numberOfCameras) >> layer, (int)configuration.depthFrameSize.y >> layer);

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

Eigen::Matrix4f TwistLoc(const Eigen::Matrix<double, 6, 1> &xi)
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


void gFusion::checkDepthToDepth()
{

	// loop through multiple trackSDF untill the error drops below thresh
	// keep trackPose [0] the same, just modify trackPose[1] ... i.e. find depthToDepth
	// instead of just doing fusion 1x per camera per frame, use the combined texture arrays to read into the shader all the images from all the cameras to output to the reduction buffer a combined reduction which has buffer size relative to number of cameras
	trackSDFProg.use();

	int xthreads, ythreads;
	xthreads = divup((int)configuration.depthFrameSize.x, 32); // right bitshift does division by powers of 2
	ythreads = divup((int)configuration.depthFrameSize.y, 32);
	glm::ivec2 imageSize = glm::ivec2((int)configuration.depthFrameSize.x, (int)configuration.depthFrameSize.y);

	// bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_textureVolume);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureDepthArray);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureVertexArray);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureNormalArray);

	// bind images
	glBindImageTexture(0, m_textureVolume, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA16F);
	glBindImageTexture(1, m_textureSDFImage, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(2, m_textureTrackImage, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	// bind buffers
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, m_bufferSDFReduction);

	glm::mat4 trackPose;
	std::memcpy(glm::value_ptr(trackPose), m_pose_eig.data(), 16 * sizeof(float));

	glm::mat4 cameraPoses[4];
	cameraPoses[0] = trackPose;
	cameraPoses[1] = trackPose * m_depthToDepth;

	glProgramUniformMatrix4fv(trackSDFProg.getHandle(), m_cameraPosesID_tsdf, 4, GL_FALSE, glm::value_ptr(cameraPoses[0]));



	glUniform1f(m_dMaxID_t, configuration.dMax);
	glUniform1f(m_dMinID_t, configuration.dMin);
	glUniform1i(m_numberOfCamerasID_t, m_numberOfCameras);

	glUniform1i(m_mipLayerID_t, 0);

	glUniform1f(m_cID, 0.02f * 0.1f);
	glUniform1f(m_epsID, 10e-9);
	glUniform3fv(m_volDimID_t, 1, glm::value_ptr(configuration.volumeDimensions));
	glUniform3fv(m_volSizeID_t, 1, glm::value_ptr(configuration.volumeSize));

	glDispatchCompute(xthreads, ythreads, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}


bool gFusion::TrackSDF() {

	glBeginQuery(GL_TIME_ELAPSED, query[5]);

	shaderConfigs.d2p = 0;
	shaderConfigs.d2v = 1;

	bool tracked = false;
	glm::mat4 oldPose = m_pose;

	glm::mat4 sdfPose = m_pose;

	Eigen::Matrix<float, 4, 4, Eigen::ColMajor> m_pose_eig_prev = m_pose_eig;

	Eigen::Vector3d trans;
	Eigen::Matrix3d rot;

	float alignmentEnergy;
	double Cnorm;
	Eigen::Matrix<double, 6, 1> result;
	result << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
	//glm::mat4 twistMat = Twist(result);
	Eigen::Matrix<double, 6, 1> result_prev = result;
	//Eigen::Matrix4d twistMat = Twist(result);

	glm::vec3 AEVec;
	glm::dvec3 cNvec;

	// here we will loop through the layers and number of iterations per layer
	for (int level = configuration.iterations.size() - 1; level >= 0; --level)
	{
	//	if (tracked == true)
	//		break;
	//	int level = 0;
		for (int iteration = 0; iteration < configuration.iterations[level]; iteration++)
		{
			Eigen::Matrix4f camToWorld = TwistLoc(result).exp() * m_pose_eig_prev;
			//std::cout << "level " << level << " iter " << iteration << " result " << result.transpose() << std::endl;
			std::vector<float> b;
			std::vector<float> C;

			//for (int i = 0; i < m_numberOfCameras; i++)
			//{
			//	trackSDF(i, level, camToWorld);
			//}

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

			dC_icp = dC_icp + (double(iteration)*1.0)*Eigen::MatrixXd::Identity(6, 6);

			//Eigen::JacobiSVD<Eigen::MatrixXd> svd(dC_icp, Eigen::ComputeFullU | Eigen::ComputeFullV);
			//result = svd.solve(db_icp); // TODO CHECK THIS WORKS, SHOULD WE MAKE A BACK SUB SOLVER?
			//Eigen::JacobiSVD<Eigen::MatrixXd> svd(dC_icp, Eigen::ComputeFullU | Eigen::ComputeFullV);
			//result = result - svd.solve(db_icp); // TODO CHECK THIS WORKS, SHOULD WE MAKE A BACK SUB SOLVER?

			result = result - dC_icp.ldlt().solve(db_icp);

			//std::cout  std::endl;

			Eigen::Matrix<double, 6, 1> Change = result - result_prev;
			Cnorm = Change.norm();

			result_prev = result;
			//std::cout << "AE: " << alignmentEnergy << " snorm : " << Cnorm << " vec " << result.transpose() << std::endl;

			//std::cout << " cnrom : " << Cnorm;
			if (alignmentEnergy != 0 && Cnorm < 1e-4)
			{
				//std::cout << " break " << iteration << " " << level << std::endl;
				//std::cout << "tracked!!! iteration " << iteration << " level " << level << " AE: " << alignmentEnergy << " snorm : " << Cnorm << " vec " << result.transpose() << std::endl;

				//std::cout << "tracked!!! iteration " << iteration << std::endl;
				tracked = true;
				break;
			}
			else
			{
				tracked = false;
			}

		}

	}

		if (std::isnan(result.sum())) result << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;

		//std::cout << "AE: " << alignmentEnergy << " snorm : " << Cnorm << std::endl;

		if (tracked)
		{
			m_pose_eig = TwistLoc(result).exp() * m_pose_eig_prev;

			std::memcpy(glm::value_ptr(m_pose), m_pose_eig.data(), 16 * sizeof(float));



			//updatePoseFinder();

			m_cumTwist += result;
		}
		else
		{


		}
		if (std::isnan(result.sum())) result << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
		m_pose_eig = TwistLoc(result).exp() * m_pose_eig_prev;

		std::memcpy(glm::value_ptr(m_pose), m_pose_eig.data(), 16 * sizeof(float));
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
	// instead of just doing fusion 1x per camera per frame, use the combined texture arrays to read into the shader all the images from all the cameras to output to the reduction buffer a combined reduction which has buffer size relative to number of cameras
	trackSDFProg.use();

	int xthreads, ythreads;
	xthreads = divup((int)configuration.depthFrameSize.x >> layer, 32); // right bitshift does division by powers of 2
	ythreads = divup((int)configuration.depthFrameSize.y >> layer, 32);
	glm::ivec2 imageSize = glm::ivec2((int)configuration.depthFrameSize.x >> layer, (int)configuration.depthFrameSize.y >> layer);
	
	// bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_textureVolume);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureDepthArray);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureVertexArray);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureNormalArray);

	// bind images
	glBindImageTexture(0, m_textureVolume, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA16F);
	glBindImageTexture(1, m_textureSDFImage, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(2, m_textureTrackImage, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	
	// bind buffers
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, m_bufferSDFReduction);

	glm::mat4 trackPose;
	std::memcpy(glm::value_ptr(trackPose), camToWorld.data(), 16 * sizeof(float));

	glm::mat4 cameraPoses[4];
	cameraPoses[0] = trackPose;
	cameraPoses[1] = trackPose * m_depthToDepth;

	glProgramUniformMatrix4fv(trackSDFProg.getHandle(), m_cameraPosesID_tsdf, 4, GL_FALSE, glm::value_ptr(cameraPoses[0]));

	

	glUniform1f(m_dMaxID_t, configuration.dMax);
	glUniform1f(m_dMinID_t, configuration.dMin);
	glUniform1i(m_numberOfCamerasID_t, m_numberOfCameras);

	glUniform1i(m_mipLayerID_t, layer);

	glUniform1f(m_cID, 0.02f * 0.1f);
	glUniform1f(m_epsID, 10e-9);
	glUniform3fv(m_volDimID_t, 1, glm::value_ptr(configuration.volumeDimensions));
	glUniform3fv(m_volSizeID_t, 1, glm::value_ptr(configuration.volumeSize));

	glDispatchCompute(xthreads, ythreads, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

}

void gFusion::trackSDF(int devNumber, int layer, Eigen::Matrix4f camToWorld)
{


	//trackSDFProg.use();

	//int xthreads, ythreads;
	//xthreads = divup((int)configuration.depthFrameSize.x >> layer, 32); // right bitshift does division by powers of 2
	//ythreads = divup((int)configuration.depthFrameSize.y >> layer, 32);
	//glm::ivec2 imageSize = glm::ivec2((int)configuration.depthFrameSize.x >> layer, (int)configuration.depthFrameSize.y >> layer);

	//// bind textures
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_3D, m_textureVolume);

	//// bind images
	//glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	//glBindImageTexture(1, m_textureVertex[devNumber], layer, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	//glBindImageTexture(2, m_textureNormal[devNumber], layer, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	//glBindImageTexture(3, m_textureSDFImage, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	//glBindImageTexture(4, m_textureTrackImage, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	//// bind buffers
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, m_bufferSDFReduction);

	//// set uniforms
	////glUniformMatrix4fv(m_TtrackID_t, 1, GL_FALSE, glm::value_ptr(m_pose));

	//glm::mat4 trackPose;
	//std::memcpy(glm::value_ptr(trackPose), camToWorld.data(), 16 * sizeof(float));


	//if (devNumber > 0)
	//{
	//	trackPose = trackPose * m_depthToDepth;
	//}

	//glUniformMatrix4fv(m_TtrackID_t, 1, GL_FALSE, glm::value_ptr(trackPose));
	//glUniform2iv(m_imageSizeID_t_sdf, 1, glm::value_ptr(imageSize));
	//
	//glUniform1f(m_dMaxID_t, configuration.dMax);
	//glUniform1f(m_dMinID_t, configuration.dMin);
	//glUniform1i(m_devNumberTrackSdfID, devNumber);

	//glUniform1f(m_cID, 0.02f * 0.1f);
	//glUniform1f(m_epsID, 10e-9);
	//glUniform3fv(m_volDimID_t, 1, glm::value_ptr(configuration.volumeDimensions));
	//glUniform3fv(m_volSizeID_t, 1, glm::value_ptr(configuration.volumeSize));

	//glDispatchCompute(xthreads, ythreads, 1);
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);

}

void gFusion::reduceSDF(int layer)
{
	//glBeginQuery(GL_TIME_ELAPSED, query[6]);

	glm::ivec2 imageSize = glm::ivec2((int)(configuration.depthFrameSize.x * m_numberOfCameras) >> layer, (int)configuration.depthFrameSize.y >> layer);

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

void gFusion::getReduction(std::vector<float>& b, std::vector<float>& C, float &alignmentEnergy, float &lastICPCount)
{
	outputData.resize(32 * 8);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer_outputdata);
	void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(outputData.data(), outputData.size() * sizeof(float), ptr, outputData.size() * sizeof(float));
	memcpy(outputData.data(), ptr, outputData.size() * sizeof(float));

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
	lastICPCount = outputData[28];
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

// here we want to integrate the depth data from multiple viewports into a single tsdf
// generating a SDF from jump flood would get too slow at high resolutions
// this will try and perform a running cumulative fusion from each camera using the standard integration but without modifying the gloabal tsdf untill each camera has been integrated and fused
// each camera 'integrates' against the current frames tsdf but does not immediately overwrite it
// a running fusion of the current frames tsdf is fused against other cameras tp prevent the tsdf being set to zero for voxels that are occluded from their own view but are perfectly valid in other views
void gFusion::integrate(bool forceIntegrate)
{
	glBeginQuery(GL_TIME_ELAPSED, query[2]);
	integrateProg.use();

	glm::mat4 d2d = (m_depthToDepth);

	glm::mat4 integratePose = glm::inverse(m_pose * d2d);
	glm::mat4 K = GLHelper::getCameraMatrix(m_camPamsDepth[0]); // make sure im set!!!!!!!!!!!!!!!!!!
	glm::mat4 invK = GLHelper::getInverseCameraMatrix(m_camPamsDepth[0]);


	glm::mat4 cameraPoses[4];
	cameraPoses[0] = glm::inverse(m_pose);
	cameraPoses[1] = glm::inverse(m_pose * d2d);

	glm::mat4 cameraIntrinsics[4];
	glm::mat4 inverseCameraIntrinsics[4];

	for (int i = 0; i < m_numberOfCameras; i++)
	{
		cameraIntrinsics[i] = GLHelper::getCameraMatrix(m_camPamsDepth[i]);
		inverseCameraIntrinsics[i] = GLHelper::getInverseCameraMatrix(m_camPamsDepth[i]);
	}
	glm::vec4 vDim = glm::vec4(configuration.volumeDimensions, 0.0f);
	glm::vec4 vSize = glm::vec4(configuration.volumeSize, 0.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureDepthArray);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureTrackImage);

	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

	// bind uniforms
	glProgramUniformMatrix4fv(integrateProg.getHandle(), m_cameraPosesID_i, 4, GL_FALSE, glm::value_ptr(cameraPoses[0]));
	glProgramUniformMatrix4fv(integrateProg.getHandle(), m_cameraIntrinsicsID_i, 4, GL_FALSE, glm::value_ptr(cameraIntrinsics[0]));
	glProgramUniformMatrix4fv(integrateProg.getHandle(), m_inverseCameraIntrinsicsID_i, 4, GL_FALSE, glm::value_ptr(inverseCameraIntrinsics[0]));

	shaderConfigs.numberOfCameras = m_numberOfCameras;
	shaderConfigs.dMax = configuration.dMax;
	shaderConfigs.dMin = configuration.dMin;
	shaderConfigs.maxWeight = configuration.maxWeight;
	shaderConfigs.depthScale = m_depthUnit / 1000000.0f;
	shaderConfigs.volDim = configuration.volumeDimensions.x;
	shaderConfigs.volSize = configuration.volumeSize.x;


	glBindBuffer(GL_UNIFORM_BUFFER, m_uboIntegrationConfig);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(integrateShaderConfigs), &shaderConfigs, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glProgramUniform1i(integrateProg.getHandle(), m_forceIntegrateID, forceIntegrate);

	//glProgramUniform1f(integrateProg.getHandle(), m_dMaxID_i, configuration.dMax);
	//glProgramUniform1f(integrateProg.getHandle(), m_dMinID_i, configuration.dMin);
	//glProgramUniform1f(integrateProg.getHandle(), m_maxWeightID, configuration.maxWeight);
	//glProgramUniform4fv(integrateProg.getHandle(), m_volDimID, 1, glm::value_ptr(vDim));
	//glProgramUniform4fv(integrateProg.getHandle(), m_volSizeID, 1, glm::value_ptr(vSize));
	//glProgramUniform1f(integrateProg.getHandle(), m_depthScaleID, m_depthUnit / 1000000.0f); // 1000 == each depth unit == 1 mm

	int xWidth;
	int yWidth;
	xWidth = divup(configuration.volumeSize.x, 32);
	yWidth = divup(configuration.volumeSize.y, 32);

	glDispatchCompute(xWidth, yWidth, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glEndQuery(GL_TIME_ELAPSED);

	//GLint i;
	//GLint count;

	//GLint size; // size of the variable
	//GLenum type; // type of the variable (float, vec3 or mat4, etc)

	//const GLsizei bufSize = 16; // maximum name length
	//GLchar name[bufSize]; // variable name in GLSL

	//GLsizei length; // name length
	//glGetProgramiv(integrateProg.getHandle(), GL_ACTIVE_UNIFORMS, &count);
	//printf("Active Uniforms: %d\n", count);

	//for (i = 0; i < count; i++)
	//{
	//	glGetActiveUniform(integrateProg.getHandle(), (GLuint)i, bufSize, &length, &size, &type, name);

	//	printf("Uniform #%d Type: %u Name: %s\n", i, type, name);
	//}


	GLuint available = 0;
	while (!available) {
		glGetQueryObjectuiv(query[2], GL_QUERY_RESULT_AVAILABLE, &available);
	}

	// elapsed time in nanoseconds
	GLuint64 elapsed;
	glGetQueryObjectui64vEXT(query[2], GL_QUERY_RESULT, &elapsed);

	integrateTime = elapsed / 1000000.0;


}

void gFusion::integrate(int devNumber)
{
	////GLenum errInt = glGetError();

	//glBeginQuery(GL_TIME_ELAPSED, query[2]);

	//integrateProg.use();
	//glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_integrateStandardID);

	//glm::mat4 d2d = (m_depthToDepth);



	//if (devNumber == 0)
	//{
	//	d2d = glm::mat4(1.0f);
	//}


	//glm::mat4 integratePose = glm::inverse(m_pose * d2d);

	////glm::mat4 cameraPoses[4];
	////cameraPoses[0] = integratePose;
	////cameraPoses[1] = glm::inverse(d2d);



	////glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_bufferCameraData); // this could just be uniform buffer rather than shader storage bufer
	////glBufferData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(glm::mat4), glm::value_ptr(cameraPoses[0]), GL_DYNAMIC_READ); // 4 x mat4

	//


	//glm::mat4 K = GLHelper::getCameraMatrix(m_camPamsDepth[devNumber]); // make sure im set
	//glm::mat4 K0;
	//GLHelper::projectionFromIntrinsics(K0, m_camPamsDepth[devNumber].x, m_camPamsDepth[devNumber].y, 1.0, m_camPamsDepth[devNumber].y, m_camPamsDepth[devNumber].z, 848, 480, 0.001, 1000.0);

	//integratePose[3][3] = configuration.dMax;
	//integratePose[2][3] = configuration.dMin;
	//integratePose[1][3] = devNumber;
	//integratePose[0][3] = m_numberOfCameras;

	//// bind uniforms
	//glUniformMatrix4fv(m_invTrackID, 1, GL_FALSE, glm::value_ptr(integratePose));
	//glUniformMatrix4fv(m_KID, 1, GL_FALSE, glm::value_ptr(K));
	//glUniform1f(m_muID, configuration.mu);

	////glm::vec4 volVox = glm::vec4(64.5 * 1.0 / 128.0, 64.5 * 1.0 / 128.0, 0.0, 1.0);

	////glm::vec4 worldPos = m_pose * volVox;

	////glm::vec4 clipPos = K0 * worldPos;

	////std::cout << glm::to_string(clipPos) << std::endl;

	////glUniform1i(m_cameraDeviceID_i, devNumber);
	////glUniform1i(m_numberOfCamerasID_i, m_numberOfCameras);

	//glUniform1f(m_dMaxID_i, configuration.dMax);
	//glUniform1f(m_dMinID_i, configuration.dMin);

	//glUniform1f(m_maxWeightID, configuration.maxWeight);

	//glUniform3fv(m_volDimID, 1, glm::value_ptr(configuration.volumeDimensions));
	//glUniform3fv(m_volSizeID, 1, glm::value_ptr(configuration.volumeSize));
	////bind image textures
	//glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
	////errInt = glGetError();
	//if (m_usingFloatDepth)
	//{
	//	glUniform1i(m_imageTypeID_i, 1); // image type 1 == float
	//	//glUniform1f(m_depthScaleID, m_depthUnit / 1000000.0f); // 1000 == each depth unit == 1 mm // FIX ME WHEN GOING BACK TO KINECT

	//	glBindImageTexture(1, m_textureDepth[devNumber], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	//}
	//else
	//{
	//	
	//	glUniform1i(m_imageTypeID_i, 0); // image type 0 == short
	//	glUniform1f(m_depthScaleID, m_depthUnit / 1000000.0f); // 1000 == each depth unit == 1 mm
	//	glBindImageTexture(2, m_textureDepth[devNumber], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
	//}
	////errInt = glGetError();

	//glBindImageTexture(3, m_textureVertex[devNumber], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);


	//int xWidth;
	//int yWidth;


	//xWidth = divup(configuration.volumeSize.x, 32);
	//yWidth = divup(configuration.volumeSize.y, 32);

	////xWidth = divup(configuration.depthFrameSize.x, 32);
	////yWidth = divup(configuration.depthFrameSize.y, 32);

	//
	//glDispatchCompute(xWidth, yWidth, 1);
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//glEndQuery(GL_TIME_ELAPSED);


	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query[2], GL_QUERY_RESULT_AVAILABLE, &available);
	//}

	//// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query[2], GL_QUERY_RESULT, &elapsed);

	//integrateTime = elapsed / 1000000.0;

	////std::cout << "i time " << integrateTime << std::endl;


}

void gFusion::raycast(int devNumber)
{


	glBeginQuery(GL_TIME_ELAPSED, query[3]);

	raycastProg.use();

	glm::mat4 invK = GLHelper::getInverseCameraMatrix(m_camPamsDepth[devNumber]);
	glm::mat4 view = m_pose * invK;

	float step = configuration.stepSize();

	glm::mat4 cameraPoses[4];
	cameraPoses[0] = m_pose * invK;
	cameraPoses[1] = m_pose * m_depthToDepth * invK;
	glProgramUniformMatrix4fv(raycastProg.getHandle(), m_cameraPosesID_r, 4, GL_FALSE, glm::value_ptr(cameraPoses[0]));

	// bind uniforms
	glUniformMatrix4fv(m_viewID_r, 1, GL_FALSE, glm::value_ptr(view));
	glUniform1f(m_nearPlaneID, configuration.nearPlane);
	glUniform1f(m_farPlaneID, configuration.farPlane);
	glUniform1f(m_stepID, step);
	glUniform1f(m_largeStepID, 0.75f * configuration.mu);
	glUniform3fv(m_volDimID_r, 1, glm::value_ptr(configuration.volumeDimensions));
	glUniform3fv(m_volSizeID_r, 1, glm::value_ptr(configuration.volumeSize));
	glUniform1i(m_numberOfCamerasID_r, m_numberOfCameras);

	//bind image textures
	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	glBindImageTexture(1, m_textureReferenceVertexArray, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(2, m_textureReferenceNormalArray, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);



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

	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	glBindImageTexture(1, m_textureMip, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glm::mat4 invK = GLHelper::getInverseCameraMatrix(m_camPamsDepth[0]);
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
	// we mask the incoming infrared and/or depth and/or color depending on the track texture
	// we combine these images with the current transformation




	/*gPosePair newPose;
	GLuint new_textureID;
	new_textureID = createTexture(GL_TEXTURE_2D, 0, configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1, GL_R32F);

	glCopyImageSubData(m_textureDepth, GL_TEXTURE_2D, 0, 0, 0, 0,
		new_textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
		configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1);

	newPose.textureID = new_textureID;
	newPose.pose = m_pose;
	poseLibrary.push_back(newPose);*/

}
/// Function called when tracking is lost
bool gFusion::recoverPose(glm::mat4 recoveryPose)
{
	bool foundPose = false;

	auto oldPose = m_pose;

	m_pose = recoveryPose;
	std::memcpy(m_pose_eig.data(), glm::value_ptr(m_pose), 16 * sizeof(float));


	//std::cout << glm::to_string(m_pose) << std::endl;
	foundPose = TrackSDF();




	//int currentLibrarySize = poseLibrary.size();
	//std::cout << "lib size : " << currentLibrarySize << std::endl;
	//if (currentLibrarySize == 0)
	//{
	//	std::cout << "You should not have arrived here" << std::endl;
	//	return foundPose;
	//}
	//else
	//{
	//	for (int i = 0; i < currentLibrarySize; i++)
	//	{
	//		m_pose = poseLibrary[i].pose;
	//		glCopyImageSubData(poseLibrary[i].textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
	//			m_textureDepth, GL_TEXTURE_2D, 0, 0, 0, 0,
	//			configuration.depthFrameSize.x, configuration.depthFrameSize.y, 1);
	//		//raycast();
	//		//depthToVertex(NULL);
	//		//vertexToNormal();
	//		//showNormals();
	//		//foundPose = Track();
	//		foundPose = TrackSDF();
	//		if (foundPose)
	//		{
	//			std::cout << "found a pose " << std::endl;
	//			break;
	//		}

	//	}



	//}

	return foundPose;
}

void gFusion::trackPoints3D(GLuint trackedPoints2Dbuffer)
{

//	int compWidth;
//	int compHeight;
//
//	helpersProg.use();
//
//	glm::mat4 invK = GLHelper::getInverseCameraMatrix(m_camPamsDepth);
//	glUniformMatrix4fv(m_invKID_h, 1, GL_FALSE, glm::value_ptr(invK));
//	glUniform1i(m_buffer2DWidthID, configuration.depthFrameSize.y * 2);
//
//	// BIND IMAGE
//	glBindImageTexture(1, m_textureDepth, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
//	// BIND BUFFERS
//	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, trackedPoints2Dbuffer);
//	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_trackedPoints3DBuffer);
//
//	compWidth = divup(configuration.depthFrameSize.x, 32); 
//	compHeight = divup(configuration.depthFrameSize.y, 32);
//
//	glDispatchCompute(compWidth, compHeight, 1);
//	glMemoryBarrier(GL_ALL_BARRIER_BITS);
///*
//
//	std::vector<float> outputData(m_depth_height * m_depth_height *2);
//
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, trackedPoints2Dbuffer);
//	void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
//	memcpy_s(outputData.data(), outputData.size() * sizeof(float), ptr, outputData.size() * sizeof(float));
//	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
//
//	cv::Mat blank = cv::Mat(424, 512, CV_8UC4);
//	for (int i = 0; i < m_depth_height * m_depth_height * 2; i += 2)
//	{
//
//		cv::circle(blank, cv::Point2f(outputData[i], outputData[i + 1]), 2, cv::Scalar(255, 128, 128, 255));
//
//	}    
//
//	cv::imshow("tracawked", blank);*/
//
//

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