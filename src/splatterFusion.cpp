#include "splatterFusion.h"

namespace rgbd
{
	splatterFusion::splatterFusion()
	{
		vT.push_back(glm::mat4(1.0f));
	}

	splatterFusion::~splatterFusion()
	{
	}

	void splatterFusion::loadShaders(
		std::map<std::string, const gl::Shader::Ptr> &progs,
		const std::string &folderPath
	)
	{
		progs.insert(std::make_pair("BilateralFilter", std::make_shared<gl::Shader>(folderPath + "BilateralFilter.comp")));
		progs.insert(std::make_pair("CASFilter", std::make_shared<gl::Shader>(folderPath + "contrastAdaptiveSharpening.comp")));
		progs.insert(std::make_pair("alignDepthColor", std::make_shared<gl::Shader>(folderPath + "alignDepthColor.comp")));
		progs.insert(std::make_pair("CalcVertexMap", std::make_shared<gl::Shader>(folderPath + "CalcVertexMap.comp")));
		progs.insert(std::make_pair("CalcNormalMap", std::make_shared<gl::Shader>(folderPath + "CalcNormalMap.comp")));
		progs.insert(std::make_pair("VirtualMapGeneration", std::make_shared<gl::Shader>(folderPath + "VirtualMapGeneration.vert", folderPath + "VirtualMapGeneration.frag")));
		progs.insert(std::make_pair("ProjectiveDataAssoc", std::make_shared<gl::Shader>(folderPath + "ProjectiveDataAssoc.comp")));
		progs.insert(std::make_pair("MultiplyMatrices", std::make_shared<gl::Shader>(folderPath + "MultiplyMatrices.comp")));
		progs.insert(std::make_pair("DownSamplingC", std::make_shared<gl::Shader>(folderPath + "DownSamplingC.comp")));
		progs.insert(std::make_pair("DownSamplingD", std::make_shared<gl::Shader>(folderPath + "DownSamplingD.comp")));
		progs.insert(std::make_pair("DownSamplingV", std::make_shared<gl::Shader>(folderPath + "DownSamplingV.comp")));
		progs.insert(std::make_pair("DownSamplingN", std::make_shared<gl::Shader>(folderPath + "DownSamplingN.comp")));
		progs.insert(std::make_pair("IndexMapGeneration", std::make_shared<gl::Shader>(folderPath + "IndexMapGeneration.vert", folderPath + "IndexMapGeneration.frag")));

		//progs.insert(std::make_pair("IndexMapGeneration", std::make_shared<gl::Shader>(folderPath + "IndexMapGeneration.comp")));
		progs.insert(std::make_pair("GlobalMapUpdate", std::make_shared<gl::Shader>(folderPath + "GlobalMapUpdate.comp")));
		progs.insert(std::make_pair("SurfaceSplatting", std::make_shared<gl::Shader>(folderPath + "SurfaceSplatting.vert", folderPath + "SurfaceSplatting.frag")));
		progs.insert(std::make_pair("UnnecessaryPointRemoval", std::make_shared<gl::Shader>(folderPath + "UnnecessaryPointRemoval.comp")));
		progs.insert(std::make_pair("p2pTrack", std::make_shared<gl::Shader>(folderPath + "p2pTrack.comp")));
		progs.insert(std::make_pair("p2pReduce", std::make_shared<gl::Shader>(folderPath + "p2pReduce.comp")));
		progs.insert(std::make_pair("rgbOdometry", std::make_shared<gl::Shader>(folderPath + "rgbOdometry.comp")));
		progs.insert(std::make_pair("rgbOdometryReduce", std::make_shared<gl::Shader>(folderPath + "rgbOdometryReduce.comp")));
		progs.insert(std::make_pair("rgbOdometryStep", std::make_shared<gl::Shader>(folderPath + "rgbOdometryStep.comp")));
		progs.insert(std::make_pair("rgbOdometryStepReduce", std::make_shared<gl::Shader>(folderPath + "rgbOdometryStepReduce.comp")));

	}

	void splatterFusion::init(
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		const glm::mat4 &K,
		const std::map<std::string, const gl::Shader::Ptr> &progs
	)
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		int width = currentFrame.getWidth();
		int height = currentFrame.getHeight();

		icp = std::make_shared<rgbd::PyramidricalICP>(width, height, K, rgbd::FUSIONTYPE::SPLATTER, progs);
		gMap = std::make_shared<rgbd::GlobalMap>(width, height, K, progs);
		rgbOdo = std::make_shared<rgbd::RGBOdometry>(width, height, progs);

		//std::cout << "map initialization..." << std::endl;
		for (int idx = 0; idx < 1; ++idx)
		{
			//std::cout << "Map size: " << gMap->getMapSize() << std::endl;
			gMap->genVirtualFrame(virtualFrame, glm::mat4(1.0f));
			virtualFrame.update();

			gMap->updateGlobalMap(currentFrame, glm::mat4(1.0f), 0);
			gMap->removeUnnecessaryPoints(0);
			gMap->genIndexMap(glm::mat4(1.0f));
		}
		//std::cout << "done!" << std::endl;

		glDisable(GL_CULL_FACE);

	}

	void splatterFusion::renderGlobalMap(glm::mat4 renderPose, const rgbd::Frame &globalFrame)
	{
		gMap->genVirtualFrame(globalFrame, renderPose);
	}

	void splatterFusion::performColorTracking(
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		const gl::Texture::Ptr &gradientMap,
		glm::mat4 &pose,
		glm::vec4 cam // cx, cy, fx, fy
		)
	{

		float sigma;
		float rgbError;

		glm::mat3 K = glm::mat3(1.0f);
		K[0][0] = cam.z;
		K[1][1] = cam.w;
		K[2][0] = cam.x;
		K[2][1] = cam.y;

		Eigen::Matrix<double, 3, 3, Eigen::RowMajor> K_eig = Eigen::Matrix<double, 3, 3, Eigen::RowMajor>::Zero();

		K_eig(0, 0) = cam.z;
		K_eig(1, 1) = cam.w;
		K_eig(0, 2) = cam.x;
		K_eig(1, 2) = cam.y;
		K_eig(2, 2) = 1;

		glm::mat4 resultRt = glm::mat4(1.0f);
		Eigen::Matrix<double, 4, 4, Eigen::RowMajor> resultRt_eig = Eigen::Matrix<double, 4, 4, Eigen::RowMajor>::Identity();


		Eigen::Matrix<float, 3, 3, Eigen::RowMajor> Rprev;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				Rprev(i, j) = pose[j][i];
			}
		}

		Eigen::Vector3f tprev;
		tprev(0) = pose[3][0];
		tprev(1) = pose[3][1];
		tprev(2) = pose[3][2];

		Eigen::Matrix<float, 3, 3, Eigen::RowMajor> Rcurr = Rprev;
		Eigen::Vector3f tcurr = tprev;

		for (int iter = 0; iter < 1; iter++)
		{
			Eigen::Matrix<double, 4, 4, Eigen::RowMajor> Rt_eig = resultRt_eig.inverse();

			Eigen::Matrix<double, 3, 3, Eigen::RowMajor> R_eig = Rt_eig.topLeftCorner(3, 3);

			Eigen::Matrix<double, 3, 3, Eigen::RowMajor> KRK_inv_eig = K_eig * R_eig * K_eig.inverse();

			glm::mat3 KRK_inv;// = K * R  * glm::inverse(K);


			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					KRK_inv[i][j] = KRK_inv_eig(j, i);
				}
			}

			glm::mat4 Rt = glm::inverse(resultRt);
			glm::mat3 R = glm::mat3(Rt);

			//glm::mat3 KRK_inv = K * R  * glm::inverse(K);

			glm::vec3 kT = glm::vec3(Rt[3][0], Rt[3][1], Rt[3][2]);

			kT = K * kT;

			Eigen::Isometry3f rgbodomiso3f;

			rgbOdo->computeResiduals(
				currentFrame,
				gradientMap,
				kT,
				KRK_inv,
				sigma,
				rgbError
			);

			rgbOdo->computeStep(
				currentFrame,
				gradientMap,
				cam,
				sigma,
				rgbError,
				resultRt,
				rgbodomiso3f
			);

			Eigen::Isometry3f currentT;
			currentT.setIdentity();
			currentT.rotate(Rprev);
			currentT.translation() = tprev;

			currentT = currentT * rgbodomiso3f.inverse();

			tcurr = currentT.translation();
			Rcurr = currentT.rotation();


		}

		if (sigma == 0 || (tcurr - tprev).norm() > 0.3 || isnan(tcurr(0)))
		{
			Rcurr = Rprev;
			tcurr = tprev;
		}

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				pose[i][j] = Rcurr(j, i);
			}
		}

		pose[3][0] = tcurr(0);
		pose[3][1] = tcurr(1);
		pose[3][2] = tcurr(2);

		//std::cout << tcurr << std::endl;
		std::cout << glm::to_string(glm::transpose(pose)) << std::endl;


	}


	glm::mat4 splatterFusion::calcDevicePose(
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		bool &tracked
	)
	{

		//GLuint query;
		//glGenQueries(1, &query);
		//glBeginQuery(GL_TIME_ELAPSED, query);



		clock_t start_icp = clock();
		//static glm::mat4 T(1.0f);
		//bool tracked = true;
		icp->calc(currentFrame, virtualFrame, T, tracked); // 0.1 ms
		if (tracked)
		{
			vT.push_back(vT.back() * T);
		}
		else
		{
			//std::cout << "tracking lost" << std::endl;
		}


		//glEndQuery(GL_TIME_ELAPSED);
		//GLuint available = 0;
		//while (!available) {
		//	glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
		//}
		//// elapsed time in nanoseconds
		//GLuint64 elapsed;
		//glGetQueryObjectui64vEXT(query, GL_QUERY_RESULT, &elapsed);
		//std::cout << "calc time : " << elapsed / 1000000.0 << std::endl;

		//clock_t start_update_frame2 = clock();
		virtualFrame.update(); // 0.06 ms
		//std::cout << "  Update frame #2: " << (clock() - start_update_frame2) / (double)CLOCKS_PER_SEC << " sec" << std::endl;



		//std::cout << "  ICP: " << (clock() - start_icp) / (double)CLOCKS_PER_SEC << " sec" << std::endl;
		//std::cout << glm::to_string(T) << std::endl;
		//std::cout << glm::to_string(vT.back()) << std::endl;

		return vT.back();
	}

	void splatterFusion::updateGlobalMap(
		const rgbd::Frame &currentFrame,
		const rgbd::Frame &virtualFrame,
		bool integrate
	)
	{





		glm::mat4 invT = glm::inverse(vT.back());



		//clock_t start_idx_map = clock();
		gMap->genIndexMap(invT); // 1 - 2 ms 
		//std::cout << "  Index map: " << (clock() - start_idx_map) / (double)CLOCKS_PER_SEC << " sec" << std::endl;




		if (integrate)
		{


			//clock_t start_update_map = clock();
			gMap->updateGlobalMap(currentFrame, vT.back(), static_cast<int>(vT.size())); // 2 ms
			//std::cout << "  Update map: " << (clock() - start_update_map) / (double)CLOCKS_PER_SEC << " sec" << std::endl;
			//std::cout << "  --> Map size: " << gMap->getMapSize() << std::endl;



			//clock_t start_remove_pts = clock();
			gMap->removeUnnecessaryPoints(static_cast<int>(vT.size())); // 3.5 ms
			//std::cout << "  Remove points: " << (clock() - start_remove_pts) / (double)CLOCKS_PER_SEC << " sec" << std::endl;

			//std::cout << "  --> Removed map size: " << gMap->getMapSize() << std::endl;



		}





		//clock_t start_virtual_frame = clock();
		gMap->genVirtualFrame(virtualFrame, invT); // 1.5 ms
		//std::cout << "  Virtual frame: " << (clock() - start_virtual_frame) / (double)CLOCKS_PER_SEC << " sec" << std::endl;


	}

	void splatterFusion::exportGlobalVertexMap()
	{
		std::vector<glm::vec4> outputVerts;
		std::vector<glm::vec3> outputNorms;
		std::vector<glm::vec3> outputColor;

		gMap->exportPointCloud(outputVerts, outputNorms, outputColor);


		std::string modelFileName = "data/meshes/splatterVertsBin.ply";

		std::ofstream outFile(modelFileName, std::ios::out | std::ios::binary);

		if (!outFile)
		{
			//cerr << "Error opening output file: " << FileName << "!" << endl;
			printf("Error opening output file: %s!\n", modelFileName);
			exit(1);
		}

		int pointNum = outputVerts.size();

		outFile << "ply" << std::endl;
		
		// https://stackoverflow.com/questions/8571089/how-can-i-find-endian-ness-of-my-pc-programmatically-using-c
		int num = 1;
		if (*(char *)&num == 1)
		{
			outFile << "format binary_little_endian 1.0" << std::endl;
		}
		else
		{
			outFile << "format binary_big_endian 1.0" << std::endl;
		}

		// outFile << "format ascii 1.0" << std::endl;

		outFile << "element vertex " << pointNum << std::endl;
		outFile << "property float x" << std::endl;
		outFile << "property float y" << std::endl;
		outFile << "property float z" << std::endl;
		outFile << "property uchar red" << std::endl;
		outFile << "property uchar green" << std::endl;
		outFile << "property uchar blue" << std::endl;
		outFile << "property float nx" << std::endl;
		outFile << "property float ny" << std::endl;
		outFile << "property float nz" << std::endl;
		outFile << "property float radius" << std::endl;

		outFile << "end_header" << std::endl;

		for (int i = 0; i < outputVerts.size(); i++) 
		{
			outFile.write((char*)&outputVerts[i].x, sizeof(float));
			outFile.write((char*)&outputVerts[i].y, sizeof(float));
			outFile.write((char*)&outputVerts[i].z, sizeof(float));

			unsigned char r = int(outputColor[i].x * 255.0f);
			unsigned char g = int(outputColor[i].y * 255.0f);
			unsigned char b = int(outputColor[i].z * 255.0f);

			outFile.write((char*)&r, sizeof(unsigned char));
			outFile.write((char*)&g, sizeof(unsigned char));
			outFile.write((char*)&b, sizeof(unsigned char));

			outFile.write((char*)&outputNorms[i].x, sizeof(float));
			outFile.write((char*)&outputNorms[i].y, sizeof(float));
			outFile.write((char*)&outputNorms[i].z, sizeof(float));



			outFile.write((char*)&outputVerts[i].w, sizeof(float));

		}
		
		outFile.close();
	}

	void splatterFusion::clear()
	{
		gMap->clearAll();
		std::cout << "Global map buffers cleared" << std::endl;
		vT.resize(1, glm::mat4(1.0f));
		T = glm::mat4(1.0f);
		std::cout << "Transform cleared" << std::endl;
	}
}