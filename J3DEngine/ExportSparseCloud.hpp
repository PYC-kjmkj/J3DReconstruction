#pragma once
#include "openMVG/cameras/Camera_Pinhole.hpp"
#include "openMVG/cameras/Camera_undistort_image.hpp"
#include "openMVG/image/image_io.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"

#define _USE_EIGEN
#include <software/SfM/export/InterfaceMVS.h>
#include "third_party/cmdLine/cmdLine.h"
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"
#include "third_party/progress/progress_display.hpp"

#include <atomic>
#include <cstdlib>
#include <string>

#ifdef OPENMVG_USE_OPENMP
#include <omp.h>
#endif

using namespace openMVG;
using namespace openMVG::cameras;
using namespace openMVG::geometry;
using namespace openMVG::image;
using namespace openMVG::sfm;

bool exportToSparse(
	const SfM_Data & sfm_data,
	const std::string & sOutFile,
	const std::string & sOutDir,
	const int iNumThreads = 0
)
{
	if (!stlplus::is_folder(sOutDir))
	{
		stlplus::folder_create(sOutDir);
		if (!stlplus::is_folder(sOutDir))
		{
			std::cerr << "�޷��������Ŀ¼������Ȩ�ޣ���ʹ�ù���Ա������� " << std::endl;
			return false;
		}
	}

	MVS::Interface scene;
	size_t nPoses(0);
	const uint32_t nViews((uint32_t)sfm_data.GetViews().size());

	C_Progress_display my_progress_bar(nViews,
		std::cout, "\n- ��ʼ������VIEWS -\n");


	std::map<openMVG::IndexT, uint32_t> map_intrinsic, map_view;

	for (const auto& intrinsic : sfm_data.GetIntrinsics())
	{
		if (isPinhole(intrinsic.second->getType()))
		{
			const Pinhole_Intrinsic * cam = dynamic_cast<const Pinhole_Intrinsic*>(intrinsic.second.get());
			if (map_intrinsic.count(intrinsic.first) == 0)
				map_intrinsic.insert(std::make_pair(intrinsic.first, scene.platforms.size()));
			MVS::Interface::Platform platform;
			// add the camera
			MVS::Interface::Platform::Camera camera;
			camera.K = cam->K();
			// sub-pose
			camera.R = Mat3::Identity();
			camera.C = Vec3::Zero();
			platform.cameras.push_back(camera);
			scene.platforms.push_back(platform);
		}
	}

	scene.images.reserve(nViews);
	for (const auto& view : sfm_data.GetViews())
	{
		++my_progress_bar;
		const std::string srcImage = stlplus::create_filespec(sfm_data.s_root_path, view.second->s_Img_path);

		if (!stlplus::is_file(srcImage))
		{
			std::cerr << "�޷���ȡ��Ӧ��ͼƬ: " << srcImage << std::endl;
			return false;
		}
		if (sfm_data.IsPoseAndIntrinsicDefined(view.second.get()))
		{
			map_view[view.first] = scene.images.size();

			MVS::Interface::Image image;
			image.name = stlplus::create_filespec(sOutDir, view.second->s_Img_path);
			image.platformID = map_intrinsic.at(view.second->id_intrinsic);
			MVS::Interface::Platform& platform = scene.platforms[image.platformID];
			image.cameraID = 0;
			MVS::Interface::Platform::Pose pose;
			image.poseID = platform.poses.size();
			const openMVG::geometry::Pose3 poseMVG(sfm_data.GetPoseOrDie(view.second.get()));
			pose.R = poseMVG.rotation();
			pose.C = poseMVG.center();
			platform.poses.push_back(pose);
			++nPoses;
			scene.images.emplace_back(image);
		}
		else
		{

			std::cout << "Cannot read the corresponding pose or intrinsic of view " << view.first << std::endl;

		}
	}

	C_Progress_display my_progress_bar_images(sfm_data.views.size(),
		std::cout, "\n- �������� -\n");
	std::atomic<bool> bOk(true);
#ifdef OPENMVG_USE_OPENMP
	const unsigned int nb_max_thread = (iNumThreads > 0) ? iNumThreads : omp_get_max_threads();

#pragma omp parallel for schedule(dynamic) num_threads(nb_max_thread)
#endif
	for (int i = 0; i < static_cast<int>(sfm_data.views.size()); ++i)
	{
		++my_progress_bar_images;

		if (!bOk)
			continue;

		Views::const_iterator iterViews = sfm_data.views.begin();
		std::advance(iterViews, i);
		const View * view = iterViews->second.get();

		const std::string srcImage = stlplus::create_filespec(sfm_data.s_root_path, view->s_Img_path);
		const std::string imageName = stlplus::create_filespec(sOutDir, view->s_Img_path);


		if (sfm_data.IsPoseAndIntrinsicDefined(view))
		{

			const openMVG::cameras::IntrinsicBase * cam = sfm_data.GetIntrinsics().at(view->id_intrinsic).get();
			if (cam->have_disto())
			{
				Image<openMVG::image::RGBColor> imageRGB, imageRGB_ud;
				Image<uint8_t> image_gray, image_gray_ud;
				try
				{
					if (ReadImage(srcImage.c_str(), &imageRGB))
					{
						UndistortImage(imageRGB, cam, imageRGB_ud, BLACK);
						bOk = WriteImage(imageName.c_str(), imageRGB_ud);
					}
					else // If RGBColor reading fails, try to read as gray image
						if (ReadImage(srcImage.c_str(), &image_gray))
						{
							UndistortImage(image_gray, cam, image_gray_ud, BLACK);
							const bool bRes = WriteImage(imageName.c_str(), image_gray_ud);
							bOk = bOk & bRes;
						}
						else
						{
							bOk = false;
						}
				}
				catch (const std::bad_alloc& e)
				{
					bOk = false;
				}
			}
			else
			{
				// just copy image
				stlplus::file_copy(srcImage, imageName);
			}
		}
		else
		{
			// just copy the image
			stlplus::file_copy(srcImage, imageName);
		}
	}

	if (!bOk)
	{
		std::cerr << "�ڴ����"
			<< "���Խ��߳�������" << std::endl;
		return false;
	}

	// define structure
	scene.vertices.reserve(sfm_data.GetLandmarks().size());
	for (const auto& vertex : sfm_data.GetLandmarks())
	{
		const Landmark & landmark = vertex.second;
		MVS::Interface::Vertex vert;
		MVS::Interface::Vertex::ViewArr& views = vert.views;
		for (const auto& observation : landmark.obs)
		{
			const auto it(map_view.find(observation.first));
			if (it != map_view.end()) {
				MVS::Interface::Vertex::View view;
				view.imageID = it->second;
				view.confidence = 0;
				views.push_back(view);
			}
		}
		if (views.size() < 2)
			continue;
		std::sort(
			views.begin(), views.end(),
			[](const MVS::Interface::Vertex::View& view0, const MVS::Interface::Vertex::View& view1)
			{
				return view0.imageID < view1.imageID;
			}
		);
		vert.X = landmark.X.cast<float>();
		scene.vertices.push_back(vert);
	}

	for (size_t p = 0; p < scene.platforms.size(); ++p)
	{
		MVS::Interface::Platform& platform = scene.platforms[p];
		for (size_t c = 0; c < platform.cameras.size(); ++c) {
			MVS::Interface::Platform::Camera& camera = platform.cameras[c];
			// find one image using this camera
			MVS::Interface::Image* pImage(nullptr);
			for (MVS::Interface::Image& image : scene.images)
			{
				if (image.platformID == p && image.cameraID == c && image.poseID != NO_ID)
				{
					pImage = &image;
					break;
				}
			}
			if (!pImage)
			{
				std::cerr << "����: û��ͼƬʹ���� " << c << " ��ƽ̨ " << p << std::endl;
				continue;
			}
			// read image meta-data
			ImageHeader imageHeader;
			ReadImageHeader(pImage->name.c_str(), &imageHeader);
			const double fScale(1.0 / std::max(imageHeader.width, imageHeader.height));
			camera.K(0, 0) *= fScale;
			camera.K(1, 1) *= fScale;
			camera.K(0, 2) *= fScale;
			camera.K(1, 2) *= fScale;
		}
	}


	if (!MVS::ARCHIVE::SerializeSave(scene, sOutFile))
		return false;


	return true;
}

int ExportSparseCloud(
	std::string sSfM_Data_Filename,
	std::string sOutFile,
	std::string sOutDir,
	std::string workDir,
	int iNumThreads = 0 //only use openmp
)
{
	Global::processProject = SPARSE;
	Global::processState = 0;
	Global::saveProcess();
	if (stlplus::extension_part(sOutFile) != "J3D") {
		std::cerr << std::endl
			<< "��Ч������ļ���չ��: " << sOutFile << std::endl;
		return EXIT_FAILURE;
	}


	SfM_Data sfm_data;
	if (!Load(sfm_data, sSfM_Data_Filename, ESfM_Data(ALL))) {
		std::cerr << std::endl
			<< "����SfM_Data�ļ� \"" << sSfM_Data_Filename << "\" �޷���ȡ." << std::endl;
		return EXIT_FAILURE;
	}

	if (!exportToSparse(sfm_data, sOutFile, sOutDir, iNumThreads))
	{
		std::cerr << std::endl
			<< "�޷�д���ļ�������Ȩ�ޣ���ʹ�ù���Ա������� " << std::endl;
		return EXIT_FAILURE;
	}

	std::vector<std::string> fileNames;
	Global::getFiles(workDir.c_str(), fileNames);
	for (int i = 0; i < fileNames.size(); i++)
	{
		std::string& fn = fileNames[i];
		auto pos = fn.rfind('.');
		if (pos != fn.npos)
		{
			std::string ext = fn.substr(pos + 1, fn.size());
			if (ext == "feat" || ext == "desc" || ext == "svg" || ext == "html")
			{
				remove(fn.c_str());
			}
		}

	}

	Global::processState = 100;
	Global::saveProcess();
	return EXIT_SUCCESS;
}


