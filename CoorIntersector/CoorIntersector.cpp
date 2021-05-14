#include <iostream>
#include <string>
#include "libs/MVS/Common.h"
#include "libs/MVS/Scene.h"
#include "opencv2/highgui.hpp"
#include "opencv2/opencv.hpp"
#include <vector>
#include <cstdio>
#include "dxflib/dl_dxf.h"
#include "TiffData.hpp"
#include <chrono>
#include <Windows.h>
using namespace MVS;

std::string workDir;
Scene scene;
TiffData tiffData;
int img_index;
std::vector<SEACAVE::Point3d> picked_points;
#define NOW_TIME std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()


void onMouse(int event, int x, int y, int flags, void* param)
{
	if (event == CV_EVENT_LBUTTONUP)
	{
		time_t start_time = NOW_TIME;
		auto p = tiffData.find(scene.images[img_index], x, y);
		if (isnan(p.x) || isnan(p.y) || isnan(p.z))
		{
			printf("�������:x=%d\ty=%d\tӳ�������:lon=0.0\tlat=0.0\talt=0.0\n��괦����Ч��,������\tӳ����ʱ:%dms\n\n", img_index, NOW_TIME - start_time, x, y);
			return;
		}
		picked_points.push_back(p);
		printf("�������:x=%d\ty=%d\tӳ�������:lon=%0.10lf\tlat=%0.10lf\talt=%0.10lf\n��ӵ�ɹ�, Ŀǰ����Ϊ%d\tӳ����ʱ:%dms\n\n", x, y, p.x, p.y, p.z, picked_points.size(), NOW_TIME - start_time);
	}
	else if (event == CV_EVENT_RBUTTONUP)
	{
		if (picked_points.size() > 0)
		{
			picked_points.pop_back();
			printf("ɾ���ɹ�,Ŀǰ�ܵ���Ϊ%d\n", picked_points.size());
		}
		else
		{
			printf("�޷�ɾ��,Ŀǰ�㼯Ϊ��\n");
		}
	}
}

bool readImg(int idx)
{
	const char* img_name = scene.images[idx].name.c_str();
	cv::Mat img = cv::imread(img_name);
	if (img.empty())return false;
	cv::destroyWindow(scene.images[img_index].name.c_str());
	img_index = idx;
	cv::namedWindow(img_name, CV_WINDOW_NORMAL);
	cv::resizeWindow(img_name, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	cv::moveWindow(img_name, 0, 0);
	cv::imshow(img_name, img);
	cv::setMouseCallback(img_name, onMouse);
	return true;
}

bool write_to_dxf(const std::string& fileName)
{
	DL_Dxf* dxf = new DL_Dxf();
	DL_Codes::version exportVersion = DL_Codes::AC1015;
	DL_WriterA* dw = dxf->out(fileName.c_str(), exportVersion);
	if (dw == NULL) {
		printf("�޷������Ŀ��·��������·����ʹ�ù���ԱȨ������\n");
		return false;
	}
	dxf->writeHeader(*dw);
	dw->sectionEnd();
	dw->sectionTables();
	dxf->writeVPort(*dw);

	dw->tableLinetypes(3);
	dxf->writeLinetype(*dw, DL_LinetypeData("BYBLOCK", "BYBLOCK", 0, 0, 0.0));
	dxf->writeLinetype(*dw, DL_LinetypeData("BYLAYER", "BYLAYER", 0, 0, 0.0));
	dxf->writeLinetype(*dw, DL_LinetypeData("CONTINUOUS", "Continuous", 0, 0, 0.0));
	dw->tableEnd();

	int numberOfLayers = 3;
	dw->tableLayers(numberOfLayers);

	dxf->writeLayer(*dw,
		DL_LayerData("0", 0),
		DL_Attributes(
			std::string(""),      // leave empty
			DL_Codes::black,        // default color
			100,                  // default width
			"CONTINUOUS", 1.0));       // default line style

	dxf->writeLayer(*dw,
		DL_LayerData("mainlayer", 0),
		DL_Attributes(
			std::string(""),
			DL_Codes::white,
			100,
			"CONTINUOUS", 1.0));

	dxf->writeLayer(*dw,
		DL_LayerData("textlayer", 0),
		DL_Attributes(
			std::string(""),
			DL_Codes::white,
			100,
			"CONTINUOUS", 1.0));

	dw->tableEnd();

	dw->tableStyle(1);
	dxf->writeStyle(*dw, DL_StyleData("standard", 0, 2.5, 1.0, 0.0, 0, 2.5, "txt", ""));
	dw->tableEnd();

	dxf->writeView(*dw);
	dxf->writeUcs(*dw);

	dw->tableAppid(1);
	dxf->writeAppid(*dw, "ACAD");
	dw->tableEnd();

	dxf->writeDimStyle(*dw, 1, 1, 1, 1, 1);

	dxf->writeBlockRecord(*dw);
	dxf->writeBlockRecord(*dw, "myblock1");
	dxf->writeBlockRecord(*dw, "myblock2");
	dw->tableEnd();

	dw->sectionEnd();

	dw->sectionBlocks();
	dxf->writeBlock(*dw, DL_BlockData("*Model_Space", 0, 0.0, 0.0, 0.0));
	dxf->writeEndBlock(*dw, "*Model_Space");
	dxf->writeBlock(*dw, DL_BlockData("*Paper_Space", 0, 0.0, 0.0, 0.0));
	dxf->writeEndBlock(*dw, "*Paper_Space");
	dxf->writeBlock(*dw, DL_BlockData("*Paper_Space0", 0, 0.0, 0.0, 0.0));
	dxf->writeEndBlock(*dw, "*Paper_Space0");

	dxf->writeBlock(*dw, DL_BlockData("myblock1", 0, 0.0, 0.0, 0.0));
	// ...
	// write block entities e.g. with dxf->writeLine(), ..
	// ...
	dxf->writeEndBlock(*dw, "myblock1");

	dxf->writeBlock(*dw, DL_BlockData("myblock2", 0, 0.0, 0.0, 0.0));
	// ...
	// write block entities e.g. with dxf->writeLine(), ..
	// ...
	dxf->writeEndBlock(*dw, "myblock2");

	dw->sectionEnd();
	dw->sectionEntities();


	double sum = 0;
	int len = picked_points.size();
	for (int i = 0; i < len; i++)
		sum += picked_points[i].y;
	double avg = sum / len;
	sum = 0;
	for (int i = 0; i < len; i++)
		sum += pow(picked_points[i].y - avg, 2);
	double gap = sqrt(sum / len) / 20;
	for (int i = 0; i < picked_points.size(); i++)
	{
		auto& point = picked_points[i];
		dxf->writePoint(
			*dw,
			DL_PointData(point.x,
				point.y,
				point.z),
			DL_Attributes("mainlayer", 255, -1, "BYLAYER", 1.0));

		dxf->writeText(*dw, DL_TextData(point.x, point.y + gap, point.z,
			0.0, 0.0, 0.0,
			gap, 0,
			0, 0, 0,
			std::to_string(i),
			"Standard",
			0),
			DL_Attributes("textlayer", 255, -1, "BYLAYER", 1.0)
		);
	}
	dw->sectionEnd();

	dxf->writeObjects(*dw);
	dxf->writeObjectsEnd(*dw);

	dw->dxfEOF();
	dw->close();
	delete dw;
	delete dxf;
	return true;
}


int main(int argc, char* argv[])
{
	workDir = argv[1];
	std::string tif_filename = argv[2];
	if (workDir.length() == 0 || tif_filename.length() == 0)
	{
		printf("�������!\n");
		return -1;
	}
	FILE* origin_coor = fopen((workDir + "/local_frame_origin.txt").c_str(), "rt");
	double og_x, og_y, og_z;
	if (origin_coor == nullptr)
	{
		std::cout << "�Ҳ�������ӳ��Ԫ����local_frame_origin.txt, Ĭ��ʹ��ԭ����Ϊ�����" << std::endl;
		og_x = og_y = og_z = 0;
	}
	else
	{
		fscanf(origin_coor, "%lf\n%lf\n%lf", &og_x, &og_y, &og_z);
	}
	printf("DSM�ļ�·��:%s\nSFM�ؽ�Ŀ¼:%s\n��ʼ��ȡ�ļ������Ժ�...", tif_filename.c_str(), workDir.c_str());

	if (!tiffData.read(tif_filename))
	{
		return -1;
	}
	printf("��ȡ��tiff���� ��ȣ�%d �߶ȣ�%d\n", tiffData.Width(), tiffData.Height());
	tiffData.setOrigin(og_x, og_y, og_z);

	scene = MVS::Scene(0);
	if (!scene.Load(workDir + "\\SparseCloud.J3D", false))
	{
		printf("��SparseCloud.J3Dʧ�ܣ����鹤��Ŀ¼\n");
		return -1;
	}

	for (int i = 0; i < scene.images.size(); i++)
	{
		printf("%d : %s\n", i, scene.images[i].name.c_str());
	}
	std::cout << "�����ǿ���ӳ����Ƭ��ţ���������ҪԤ����ͼƬ���(0 - " << scene.images.size() - 1 << ")" << std::endl;
	std::cout << "������ѡ��Ŀ��㣬��������ֵ��ӵ��㼯��" << std::endl;
	std::cout << "����Ҽ�����ɾ����һ������ӵ��㼯�еĵ�" << std::endl;
	std::cout << "���� -/+�л�ͼƬ; p��ʾ��ѡ�еĵ㼯��Ϣ; o�����dxf�ļ�" << std::endl;
	int idx = 0;
	std::cin >> idx;
	if (idx <= 0 || idx >= scene.images.size())idx = 0;

	readImg(idx);
	while (true)
	{
		auto key = cv::waitKey(10);
		switch (key)
		{
		case '-':
		{
			readImg(img_index ? img_index - 1 : scene.images.size() - 1);
			break;
		}
		case '=':
		case '+':
		{
			readImg(((scene.images.size() - 1) == img_index) ? 0 : img_index + 1);
			break;
		}
		case 'p':
		{
			printf("Ŀǰ��ѡ�еĵ㼯����:\n--------------------------------------------------\n");
			for (int i = 0; i < picked_points.size(); i++)
			{
				SEACAVE::Point3d& point = picked_points[i];
				printf("���:%d\tlon = %0.8lf\tlat = %0.8lf\talt = %0.8lf\n", i, point.x, point.y, point.z);
			}
			printf("--------------------------------------------------\n");
			break;
		}
		case 'o':
		{
			if (picked_points.size() == 0)
			{
				printf("û�е���������\n");
				break;
			}
			if (write_to_dxf(workDir + "/picked_points.dxf"))
			{
				printf("�ѳɹ����浽�ļ�picked_points.dxf\n");
			}
			break;
		}
		default:
			break;
		}
		if (key == 27)
		{
			break;
		}

	}
	return 0;
}


