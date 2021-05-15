#define _USE_MATH_DEFINES
#define NOMINMAX
#include <iostream>
#include <Windows.h>
#include "PresetConfigureImages.hpp"
#include "MatchImages.hpp"
#include "StructureFromMotion.hpp"
#include "StructureFromPoses.hpp"
#include "ExportSparseCloud.hpp"
#include "OriginPoints.hpp"
#include "BlocksExchangeInterface.hpp"
#include <algorithm>
#include <direct.h>
#include <fstream>
#include "Global.h"
#include "MVSEngine.h"
#include "third_party/cmdLine/cmdLine.h"


HWND m_hWnd;
int STATE_RETURN;


bool saveTid()
{
	int tid = GetCurrentThreadId();
	ofstream file;
	locale loc = locale::global(locale(""));
	file.open("C:\\ProgramData\\J3DEngine\\ProgramCache.tmp", ios::out | ios::trunc);
	locale::global(loc);
	if (!file.is_open())
		return false;
	file << std::to_string(tid) << endl;
	file.close();
	return true;
}

VOID CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	m_hWnd = FindWindow("Qt5QWindowIcon", "J3D���ӻ�ƽ̨");
	if (!saveTid() || !Global::saveProcess())
		printf("д�뻺��ʧ�ܣ��������Ȩ�ޣ���ʹ�ù���Ա������� \n");
}
void MsgProc(UINT msg, WPARAM wp, LPARAM lp)
{
	if (!m_hWnd) {
		return;
	}

	switch (msg)
	{
	case CMD_MATCHFEATURES:
	{
		Global::process = PROCESSWORKING;
		Global::saveProcess();
		std::string imagesInputDir;
		std::string sensorWidthDataBaseDir = "./SenWidDB.txt";
		std::string matchesOutputDir;
		std::string EigenMatrix;// EigenMatrixFormat"f;0;ppx;0;f;ppy;0;0;1"
		std::string describerMethod;
		std::string featureQuality;
		bool upRight;
		bool forceCompute;
		std::string geometricModel;
		float distanceRatio;
		bool forceMatch;

		printf("\n������У�MATCHFEATURES \n\n");
		ifstream cmdCache;
		cmdCache.open(("C:\\ProgramData\\J3DEngine\\cmdCache.tmp"), ios::in | ios::_Nocreate);
		if (!cmdCache)
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}

		std::string temp;
		getline(cmdCache, temp);
		if (temp != "matchfeature")
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}

		getline(cmdCache, imagesInputDir);
		getline(cmdCache, matchesOutputDir);
		getline(cmdCache, temp);
		EigenMatrix = temp;
		if (EigenMatrix == "NULL")
		{
			EigenMatrix = "";
		}
		getline(cmdCache, describerMethod);
		getline(cmdCache, featureQuality);
		getline(cmdCache, temp);
		if (temp == "0")
			upRight = false;
		else
			upRight = true;
		getline(cmdCache, temp);
		if (temp == "0")
			forceCompute = false;
		else
			forceCompute = true;
		getline(cmdCache, geometricModel);
		getline(cmdCache, temp);
		distanceRatio = atof(temp.c_str());
		getline(cmdCache, temp);
		if (temp == "0")
			forceMatch = false;
		else
			forceMatch = true;

		cmdCache.close();

		STATE_RETURN = LoadingImages(imagesInputDir, matchesOutputDir, sensorWidthDataBaseDir, EigenMatrix, "1.0;1.0;1.0");
		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("����ͼƬʧ��\n");
			Global::process = PROCESSERROR;
			break;
		}
		printf("����ͼƬ�ɹ�\n");
		STATE_RETURN = GetFeatures(
			matchesOutputDir + "/sfm_data.json",
			matchesOutputDir,
			describerMethod,
			"",
			upRight,
			forceCompute);

		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("��ȡ������Ϣʧ��\n");
			Global::process = PROCESSERROR;
			break;

		}
		printf("��ȡ��������ɣ�׼����ʼƥ��������\n");

		STATE_RETURN = GetMatches(
			matchesOutputDir + "/sfm_data.json",
			matchesOutputDir,
			geometricModel,
			"",
			"AUTO",
			-1,
			2048,
			0U,
			distanceRatio, forceMatch);

		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("ƥ��������Ϣʧ��\n");
			Global::process = PROCESSERROR;
			break;
		}
		printf("ƥ�����\n");
		printf("\n�������\n");
		Global::process = PROCESSCLOSE;
		break;
	}

	case CMD_SFMANDSFP:
	{
		Global::process = PROCESSWORKING;
		Global::saveProcess();
		std::string matchesDir, sfmOutputDir;
		int triangulationMethod, resectionMethod;
		printf("\n������У�SFMANDSFP \n\n");
		ifstream cmdCache;
		cmdCache.open(("C:\\ProgramData\\J3DEngine\\cmdCache.tmp"), ios::in | ios::_Nocreate);
		if (!cmdCache)
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}

		std::string temp;
		getline(cmdCache, temp);
		if (temp != "sfmandsfp")
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}

		getline(cmdCache, matchesDir);
		getline(cmdCache, sfmOutputDir);
		getline(cmdCache, temp);
		triangulationMethod = atoi(temp.c_str());
		getline(cmdCache, temp);
		resectionMethod = atoi(temp.c_str());
		cmdCache.close();
		STATE_RETURN = StructureFromMotion
		(
			matchesDir + "\\sfm_data.json",
			matchesDir,
			"",
			sfmOutputDir,
			"",
			"",
			"ADJUST_ALL",
			3,
			true,
			true,
			triangulationMethod,
			resectionMethod);
		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("SFM�ؽ�ʧ��\n");
			Global::process = PROCESSERROR;
			break;

		}

		printf("����SFM���ݾ�ϸ��\n");
		STATE_RETURN = ConvertCoorsToOrigin
		(
			sfmOutputDir + "/sfm_data.bin",
			sfmOutputDir
		);
		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("SFM���ݾ�ϸ��ʧ��\n");
			Global::process = PROCESSERROR;
			break;

		}

		printf("���е�����ɫ\n");
		STATE_RETURN = PrintPointColors(sfmOutputDir + "/sfm_data_local.bin", sfmOutputDir + "/colored.ply");
		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("SFM������ɫʧ��\n");
			Global::process = PROCESSERROR;
			break;
		}
		printf("����SFP�ع�\n");
		STATE_RETURN = StructureFromPoses(
			sfmOutputDir + "/sfm_data_local.bin",
			matchesDir,
			sfmOutputDir + "/robust.bin",
			matchesDir + "/matches.f.bin");
		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("SFP�ع�ʧ��\n");
			Global::process = PROCESSERROR;
			break;
		}
		printf("SFP�ع��ɹ�\n");
		printf("����ϡ��������\n");
		STATE_RETURN = ExportSparseCloud
		(
			sfmOutputDir + "/robust.bin",
			sfmOutputDir + "/SparseCloud.J3D",
			sfmOutputDir + "/undistorted_images",
			sfmOutputDir
		);
		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("���ϡ�����ʧ��\n");
			Global::process = PROCESSERROR;
			break;
		}
		printf("\n�������\n");
		Global::process = PROCESSCLOSE;
		break;
	}

	case CMD_EXPORTDENSECLOUD:
	{
		Global::process = PROCESSWORKING;
		Global::saveProcess();
		std::string densifyInputDir, densifyOutputDir, densifyWorkingDir;
		printf("\n������У�DENSIFYPOINTCLOUD \n\n");
		ifstream cmdCache;
		cmdCache.open(("C:\\ProgramData\\J3DEngine\\cmdCache.tmp"), ios::in | ios::_Nocreate);
		if (!cmdCache)
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}

		std::string temp;
		getline(cmdCache, temp);
		if (temp != "densifypointcloud")
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}

		getline(cmdCache, densifyInputDir);
		getline(cmdCache, densifyWorkingDir);
		getline(cmdCache, densifyOutputDir);
		cmdCache.close();
		char* cmd[7];
		char t[200];
		GetModuleFileNameA(NULL, t, 200);
		cmd[0] = t;
		cmd[1] = "-i";
		cmd[2] = (char*)densifyInputDir.data();
		cmd[3] = "-w";
		cmd[4] = (char*)densifyWorkingDir.data();
		cmd[5] = "-o";
		cmd[6] = (char*)densifyOutputDir.data();
		Global::process = !MVSEngine::DensifyPointCloud(7, cmd);
		break;
	}

	case CMD_RECONSTRUCTMESH:
	{
		Global::process = PROCESSWORKING;
		Global::saveProcess();
		std::string reconstructMeshInputDir, reconstructMeshOutputDir, reconstructMeshWorkingDir;
		printf("\n������У�RECONSTRUCTMESH \n\n");
		ifstream cmdCache;
		cmdCache.open(("C:\\ProgramData\\J3DEngine\\cmdCache.tmp"), ios::in | ios::_Nocreate);
		if (!cmdCache)
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}

		std::string temp;
		getline(cmdCache, temp);
		if (temp != "reconstructmesh")
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}
		getline(cmdCache, reconstructMeshInputDir);
		getline(cmdCache, reconstructMeshWorkingDir);
		getline(cmdCache, reconstructMeshOutputDir);
		cmdCache.close();
		char* cmd[9];
		char t[200];
		GetModuleFileNameA(NULL, t, 200);
		cmd[0] = t;
		cmd[1] = "-i";
		cmd[2] = (char*)reconstructMeshInputDir.data();
		cmd[3] = "-d";
		cmd[4] = "4";
		cmd[5] = "-o";
		cmd[6] = (char*)reconstructMeshOutputDir.data();
		cmd[7] = "-w";
		cmd[8] = (char*)reconstructMeshWorkingDir.data();
		int status = MVSEngine::ReconstructMesh(9, cmd);
		if (STATE_RETURN != EXIT_SUCCESS) {
			Global::process = PROCESSWORKING;
			printf("����ʧ�ܣ�����·�����ļ��Ƿ���ȷ\n");
			break;
		}
		cmd[0] = t;
		cmd[1] = "-i";
		cmd[2] = (char*)reconstructMeshOutputDir.data();
		cmd[3] = "--resolution-level";
		cmd[4] = "2";
		cmd[5] = "-o";
		cmd[6] = (char*)reconstructMeshOutputDir.data();
		cmd[7] = "-w";
		cmd[8] = (char*)reconstructMeshWorkingDir.data();
		Global::process = !MVSEngine::RefineMesh(9, cmd);
		break;
	}

	case CMD_TEXTUREMESH:
	{
		Global::process = PROCESSWORKING;
		Global::saveProcess();
		std::string textureMeshInputDir, textureMeshOutputDir, textureMeshWorkingDir, exportFormat;
		printf("\n������У�TEXTUREMESH \n\n");
		ifstream cmdCache;
		cmdCache.open(("C:\\ProgramData\\J3DEngine\\cmdCache.tmp"), ios::in | ios::_Nocreate);
		if (!cmdCache)
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}
		std::string temp;
		getline(cmdCache, temp);
		if (temp != "texturemesh")
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}
		getline(cmdCache, textureMeshInputDir);
		getline(cmdCache, textureMeshWorkingDir);
		getline(cmdCache, textureMeshOutputDir);
		getline(cmdCache, exportFormat);
		bool isOsgb = false;
		if ("osgb" == exportFormat)
		{
			exportFormat = "obj";
			isOsgb = true;
		}
		cmdCache.close();
		char* cmd[9];
		char t[200];
		GetModuleFileNameA(NULL, t, 200);
		cmd[0] = t;
		cmd[1] = "-i";
		cmd[2] = (char*)textureMeshInputDir.data();
		cmd[3] = "-o";
		cmd[4] = (char*)textureMeshOutputDir.data();
		cmd[5] = "-w";
		cmd[6] = (char*)textureMeshWorkingDir.data();
		cmd[7] = "--export-type";
		cmd[8] = (char*)exportFormat.data();
		int status = MVSEngine::TextureMesh(9, cmd);

		if (status == EXIT_SUCCESS)
		{
			if (isOsgb)
			{
				string cmdt = "osgcv.exe " + textureMeshWorkingDir + "/TEXTURE_Mesh.obj " + textureMeshWorkingDir + "/TEXTURE_Mesh.osgb";
				::system(cmdt.c_str());
			}
			Global::process = PROCESSCLOSE;
		}
		else
		{
			Global::process = PROCESSWORKING;
		}
		break;

	}

	case CMD_FULLAUTO:
	{
		Global::process = PROCESSWORKING;
		Global::saveProcess();


		std::string imagesInputDir;
		std::string sensorWidthDataBaseDir = "./SenWidDB.txt";
		std::string matchesOutputDir;
		std::string EigenMatrix;// EigenMatrixFormat"f;0;ppx;0;f;ppy;0;0;1"
		std::string describerMethod;
		std::string featureQuality;
		bool upRight;
		bool forceCompute;
		std::string geometricModel;
		float distanceRatio;
		bool forceMatch;

		std::string matchesDir, sfmOutputDir;
		int triangulationMethod, resectionMethod;

		std::string densifyInputDir, densifyOutputDir, densifyWorkingDir;

		std::string reconstructMeshInputDir, reconstructMeshOutputDir, reconstructMeshWorkingDir;

		std::string textureMeshInputDir, textureMeshOutputDir, textureMeshWorkingDir, exportFormat;

		printf("\n������У�FULLAUTO \n\n");
		ifstream cmdCache;
		cmdCache.open(("C:\\ProgramData\\J3DEngine\\fullautoCmdCache.tmp"), ios::in | ios::_Nocreate);
		if (!cmdCache)
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}

		std::string temp;
		getline(cmdCache, temp);
		if (temp != "matchfeature")
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}

		getline(cmdCache, imagesInputDir);
		getline(cmdCache, matchesOutputDir);
		getline(cmdCache, EigenMatrix);
		if (EigenMatrix == "NULL")
		{
			EigenMatrix = "";
		}
		getline(cmdCache, describerMethod);
		getline(cmdCache, featureQuality);
		getline(cmdCache, temp);
		if (temp == "0")
			upRight = false;
		else
			upRight = true;
		getline(cmdCache, temp);
		if (temp == "0")
			forceCompute = false;
		else
			forceCompute = true;
		getline(cmdCache, geometricModel);
		getline(cmdCache, temp);
		distanceRatio = atof(temp.c_str());
		getline(cmdCache, temp);
		if (temp == "0")
			forceMatch = false;
		else
			forceMatch = true;
		getline(cmdCache, temp);
		if (temp != "sfmandsfp")
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}

		getline(cmdCache, matchesDir);
		getline(cmdCache, sfmOutputDir);
		getline(cmdCache, temp);
		triangulationMethod = atoi(temp.c_str());
		getline(cmdCache, temp);
		resectionMethod = atoi(temp.c_str());
		getline(cmdCache, temp);
		if (temp != "densifypointcloud")
		{
			printf("����ʧ��,�޷���ȡ�������\n");

			Global::process = PROCESSERROR;
			break;
		}

		getline(cmdCache, densifyInputDir);
		getline(cmdCache, densifyWorkingDir);
		getline(cmdCache, densifyOutputDir);

		getline(cmdCache, temp);
		if (temp != "reconstructmesh")
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}
		getline(cmdCache, reconstructMeshInputDir);
		getline(cmdCache, reconstructMeshWorkingDir);
		getline(cmdCache, reconstructMeshOutputDir);
		getline(cmdCache, temp);
		if (temp != "texturemesh")
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}
		getline(cmdCache, textureMeshInputDir);
		getline(cmdCache, textureMeshWorkingDir);
		getline(cmdCache, textureMeshOutputDir);
		getline(cmdCache, exportFormat);
		cmdCache.close();
		STATE_RETURN = LoadingImages(imagesInputDir, matchesOutputDir, sensorWidthDataBaseDir, EigenMatrix, "1.0;1.0;1.0");
		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("����ͼƬʧ��\n");
			Global::process = PROCESSERROR;
			break;
		}
		printf("����ͼƬ�ɹ�\n");
		STATE_RETURN = GetFeatures(
			matchesOutputDir + "/sfm_data.json",
			matchesOutputDir,
			describerMethod,
			"",
			upRight,
			forceCompute);
		Sleep(3000);

		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("��ȡ������Ϣʧ��\n");
			Global::process = PROCESSERROR;
			break;

		}
		printf("��ȡ��������ɣ�׼����ʼƥ��������\n");
		STATE_RETURN = GetMatches(
			matchesOutputDir + "/sfm_data.json",
			matchesOutputDir,
			geometricModel,
			"",
			"AUTO",
			-1,
			2048,
			0U,
			distanceRatio, forceMatch);
		Sleep(3000);

		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("ƥ��������Ϣʧ��\n");
			Global::process = PROCESSERROR;
			break;
		}
		printf("ƥ�����\n");
		STATE_RETURN = StructureFromMotion(
			matchesDir + "\\sfm_data.json",
			matchesDir,
			"",
			sfmOutputDir,
			"",
			"",
			"ADJUST_ALL",
			3,
			true,
			true,
			triangulationMethod,
			resectionMethod);
		Sleep(3000);
		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("SFM�ؽ�ʧ��\n");
			Global::process = PROCESSERROR;
			break;

		}

		printf("����SFM���ݾ�ϸ��\n");
		STATE_RETURN = ConvertCoorsToOrigin
		(
			sfmOutputDir + "/sfm_data.bin",
			sfmOutputDir
		);
		Sleep(2000);
		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("SFM���ݾ�ϸ��ʧ��\n");
			Global::process = PROCESSERROR;
			break;

		}

		printf("���е�����ɫ\n");
		STATE_RETURN = PrintPointColors(sfmOutputDir + "/sfm_data_local.bin", sfmOutputDir + "/colored.ply");
		Sleep(3000);
		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("SFM������ɫʧ��\n");
			Global::process = PROCESSERROR;
			break;
		}
		printf("����SFP�ع�\n");
		STATE_RETURN = StructureFromPoses(
			sfmOutputDir + "/sfm_data_local.bin",
			matchesDir,
			sfmOutputDir + "/robust.bin",
			matchesDir + "/matches.f.bin");
		Sleep(3000);
		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("SFP�ع�ʧ��\n");
			Global::process = PROCESSERROR;
			break;
		}
		printf("SFP�ع��ɹ�\n");
		printf("����ϡ��������\n");
		STATE_RETURN = ExportSparseCloud
		(
			sfmOutputDir + "/robust.bin",
			sfmOutputDir + "/SparseCloud.J3D",
			sfmOutputDir + "/undistorted_images",
			sfmOutputDir
		);
		Sleep(3000);
		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("���ϡ�����ʧ��\n");
			Global::process = PROCESSERROR;
			break;
		}
		char* cmd[7];
		char t[200];
		GetModuleFileNameA(NULL, t, 200);
		cmd[0] = t;
		cmd[1] = "-i";
		cmd[2] = (char*)densifyInputDir.data();
		cmd[3] = "-w";
		cmd[4] = (char*)densifyWorkingDir.data();
		cmd[5] = "-o";
		cmd[6] = (char*)densifyOutputDir.data();
		STATE_RETURN = MVSEngine::DensifyPointCloud(7, cmd);
		if (STATE_RETURN != EXIT_SUCCESS) {
			Global::process = PROCESSWORKING;
			printf("����ʧ�ܣ�����·�����ļ��Ƿ���ȷ\n");
			break;
		}
		Sleep(5000);
		char* cmd1[9];
		char t1[200];
		GetModuleFileNameA(NULL, t1, 200);
		cmd1[0] = t1;
		cmd1[1] = "-i";
		cmd1[2] = (char*)reconstructMeshInputDir.data();
		cmd1[3] = "-d";
		cmd1[4] = "4";
		cmd1[5] = "-o";
		cmd1[6] = (char*)reconstructMeshOutputDir.data();
		cmd1[7] = "-w";
		cmd1[8] = (char*)reconstructMeshWorkingDir.data();
		STATE_RETURN = MVSEngine::ReconstructMesh(9, cmd1);
		if (STATE_RETURN != EXIT_SUCCESS) {
			Global::process = PROCESSWORKING;
			printf("����ʧ�ܣ�����·�����ļ��Ƿ���ȷ\n");
			break;
		}
		Sleep(5000);
		cmd1[0] = t1;
		cmd1[1] = "-i";
		cmd1[2] = (char*)reconstructMeshOutputDir.data();
		cmd1[3] = "--resolution-level";
		cmd1[4] = "2";
		cmd1[5] = "-o";
		cmd1[6] = (char*)reconstructMeshOutputDir.data();
		cmd1[7] = "-w";
		cmd1[8] = (char*)reconstructMeshWorkingDir.data();
		STATE_RETURN = MVSEngine::RefineMesh(9, cmd1);
		if (STATE_RETURN != EXIT_SUCCESS) {
			Global::process = PROCESSWORKING;
			printf("����ʧ�ܣ�����·�����ļ��Ƿ���ȷ\n");
			break;
		}
		Sleep(5000);
		bool isOsgb = false;
		if ("osgb" == exportFormat)
		{
			exportFormat = "obj";
			isOsgb = true;
		}
		char* cmd2[9];
		char t2[200];
		GetModuleFileNameA(NULL, t2, 200);
		cmd2[0] = t2;
		cmd2[1] = "-i";
		cmd2[2] = (char*)textureMeshInputDir.data();
		cmd2[3] = "-o";
		cmd2[4] = (char*)textureMeshOutputDir.data();
		cmd2[5] = "-w";
		cmd2[6] = (char*)textureMeshWorkingDir.data();
		cmd2[7] = "--export-type";
		cmd2[8] = (char*)exportFormat.data();
		STATE_RETURN = MVSEngine::TextureMesh(9, cmd2);
		if (STATE_RETURN != EXIT_SUCCESS) {
			Global::process = PROCESSWORKING;
			printf("����ʧ�ܣ�����·�����ļ��Ƿ���ȷ\n");
			break;
		}
		if (isOsgb)
		{
			string cmdt = "osgcv.exe " + textureMeshWorkingDir + "/TEXTURE_Mesh.obj " + textureMeshWorkingDir + "/TEXTURE_Mesh.osgb";
			::system(cmdt.c_str());
		}
		Global::process = PROCESSCLOSE;
		break;
	}

	case CMD_IMPORTFROMBE:
	{
		Global::process = PROCESSWORKING;
		Global::saveProcess();
		std::string inputFileName, workDir;
		printf("\n������У�IMPORTFROMBE \n\n");
		ifstream cmdCache;
		cmdCache.open(("C:\\ProgramData\\J3DEngine\\cmdCache.tmp"), ios::in | ios::_Nocreate);
		if (!cmdCache)
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}
		std::string temp;
		getline(cmdCache, temp);
		if (temp != "importfromblocksexchange")
		{
			printf("����ʧ��,�޷���ȡ�������\n");
			Global::process = PROCESSERROR;
			break;
		}
		getline(cmdCache, inputFileName);
		getline(cmdCache, workDir);
		int STATE_RETURN = BlocksExchange2openMVG(inputFileName, workDir);
		if (STATE_RETURN == false)
		{
			printf("����Blocks Exchange�ļ�ʧ��!\n");
			Global::process = PROCESSERROR;
			break;
		}
		Sleep(1000);
		printf("����SFM�������ݾ�ϸ��\n");
		STATE_RETURN = ConvertCoorsToOrigin
		(
			workDir + "/sfm_data.bin",
			workDir
		);
		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("SFM�������ݾ�ϸ��ʧ��\n");
			Global::process = PROCESSERROR;
			break;
		}
		printf("����ϡ��������\n");
		STATE_RETURN = ExportSparseCloud
		(
			workDir + "/sfm_data_local.bin",
			workDir + "/SparseCloud.J3D",
			workDir + "/undistorted_images",
			workDir
		);
		if (STATE_RETURN == EXIT_FAILURE)
		{
			printf("���ϡ�����ʧ��\n");
			Global::process = PROCESSERROR;
			break;
		}
		printf("�ɹ�������ļ�SparseCloud.J3D\n");
		printf("����������ɣ�֮����Լ���ʹ��J3D�������ģ���ؽ�������ӳ�乤��\n");
		printf("\n�������\n");
		Global::process = PROCESSCLOSE;
		break;
	}
	}
	return;
}
int main()
{
	printf("\n���ڳ�ʼ�����棬���Ե�\n");
	MSG msg;
	SetTimer(NULL, 0, 1000, NULL);
	_mkdir("C:\\ProgramData\\J3DEngine");
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;
	GetConsoleMode(hStdin, &mode);
	mode &= ~ENABLE_QUICK_EDIT_MODE;
	mode &= ~ENABLE_INSERT_MODE;
	mode &= ~ENABLE_MOUSE_INPUT;
	SetConsoleMode(hStdin, mode);
	SetConsoleTitle("J3DEngine");
	int tid = GetCurrentThreadId();
	if (!saveTid())
	{
		MessageBoxA(nullptr, "д�����ļ�ʧ�ܣ��������Ȩ�޲�ʹ�ù���Ա������� ", "Ȩ�޴��� ", MB_OK);
		return -1;
	}
	m_hWnd = FindWindow("Qt5QWindowIcon", "J3D���ӻ�ƽ̨");
	if (m_hWnd) {
		printf("�ҵ�J3DGUI����");
		SendMessage(m_hWnd, WM_USER, tid, 0);
	}
	::system("cls");
	printf("\n-----------------------------------\n");
	printf("        ��ӭʹ��J3DEngine V2.0        \n");
	printf("            �����ʼ���ɹ�             \n");
	printf("        ��ʹ��J3DGUI������ָ��       \n");
	printf("     @Basic All rights reserved    \n");
	printf("-----------------------------------\n");
	Global::processProject = 0;
	Global::processState = 0;
	Global::process = PROCESSWORKING;

	while (int a = GetMessage(&msg, NULL, 0, 0))
	{
		if (a != -1)
		{
			if (msg.message == WM_TIMER)
				TimerProc(NULL, 0, 0, 0);
			MsgProc(msg.message, msg.wParam, msg.lParam);
		}

	}

	return 0;
}
