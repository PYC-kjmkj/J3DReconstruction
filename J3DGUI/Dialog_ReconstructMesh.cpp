#include "Dialog_ReconstructMesh.h"
#include "ui_dialog_reconstructmesh.h"

Dialog_ReconstructMesh::Dialog_ReconstructMesh(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Dialog_ReconstructMesh)
{
	ui->setupUi(this);
}

Dialog_ReconstructMesh::~Dialog_ReconstructMesh()
{
	delete ui;
}

void Dialog_ReconstructMesh::on_btn_OK_clicked()
{
	if (Global::GetProcessidFromName("J3DEngine.exe") == 0)
	{
		QMessageBox::critical(this, u8"���� ", u8"δ�ҵ�J3DEngine���� ", QMessageBox::Ok, QMessageBox::Ok);
		return;
	}
	else
		Global::connectEngine();


	if (ui->lineEdit_inputDir->text() == "")
	{
		QMessageBox::critical(this, u8"���� ", u8"δ�����ܼ����ƽ��·�� ", QMessageBox::Ok, QMessageBox::Ok);
		return;
	}
	if (ui->lineEdit_OutputDir->text() == "")
	{
		QMessageBox::critical(this, u8"���� ", u8"δ�������·�� ", QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	Global::reconstructMeshInputDir = ui->lineEdit_inputDir->text() + "/DenseCloud.J3D";
	Global::reconstructMeshOutputDir = ui->lineEdit_OutputDir->text() + "/TIN_Mesh.J3D";
	Global::reconstructMeshWorkingDir = ui->lineEdit_OutputDir->text();
	auto refineDir = Global::reconstructMeshWorkingDir + "/TIN_Mesh_Refine.J3D";
	_mkdir("C:\\ProgramData\\J3DEngine");

	QFile cmdcache("C:\\ProgramData\\J3DEngine\\cmdCache.tmp");

	if (cmdcache.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
	{
		QString head = "reconstructmesh\n";
		cmdcache.write(head.toUtf8());
		cmdcache.write(Global::reconstructMeshInputDir.toUtf8());
		cmdcache.write("\n");
		cmdcache.write(Global::reconstructMeshWorkingDir.toUtf8());
		cmdcache.write("\n");
		cmdcache.write(Global::reconstructMeshOutputDir.toUtf8());
		cmdcache.write("\n");
		cmdcache.write(refineDir.toUtf8());
		cmdcache.write("\n");
		cmdcache.close();
		QMessageBox::information(NULL, u8"���", u8"������� ", QMessageBox::Yes, NULL);
		PostThreadMessageA(Global::engineTid, CMD_RECONSTRUCTMESH, 0, 0);
		Global::tasking = true;
		this->close();
	}
	else
	{
		QMessageBox::information(NULL, u8"����", u8"�޷����ʻ����ļ�������Ȩ�ޣ���ʹ�ù���Ա������� ", QMessageBox::Yes, NULL);
	}

}

void Dialog_ReconstructMesh::on_btn_CANCEL_clicked()
{
	this->close();
}

void Dialog_ReconstructMesh::on_pushButton_browseInputDir_clicked()
{
	Global::reconstructMeshInputDir = QFileDialog::getExistingDirectory(this, u8"����ܼ������ļ�Ŀ¼ ", "", NULL);
	ui->lineEdit_inputDir->setText(Global::reconstructMeshInputDir);
}

void Dialog_ReconstructMesh::on_pushButton_browseOutputDir_clicked()
{
	Global::reconstructMeshOutputDir = QFileDialog::getExistingDirectory(this, u8"������������ģ���ļ�Ŀ¼ ", "", NULL);
	ui->lineEdit_OutputDir->setText(Global::reconstructMeshOutputDir);
}
