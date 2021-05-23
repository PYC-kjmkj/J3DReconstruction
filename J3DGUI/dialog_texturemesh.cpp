#include "dialog_texturemesh.h"
#include "ui_dialog_texturemesh.h"

Dialog_TextureMesh::Dialog_TextureMesh(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Dialog_TextureMesh)
{
	ui->setupUi(this);
}

Dialog_TextureMesh::~Dialog_TextureMesh()
{
	delete ui;
}

void Dialog_TextureMesh::on_btn_OK_clicked()
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

	Global::textureMeshInputDir = ui->lineEdit_inputDir->text() + "/TIN_Mesh_Refine.J3D";
	Global::textureMeshOutputDir = ui->lineEdit_OutputDir->text() + "/TEXTURE_Mesh.J3D";
	Global::textureMeshWorkingDir = ui->lineEdit_OutputDir->text();

	_mkdir("C:\\ProgramData\\J3DEngine");

	QFile cmdcache("C:\\ProgramData\\J3DEngine\\cmdCache.tmp");

	if (cmdcache.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
	{
		QString head = "texturemesh\n";
		cmdcache.write(head.toUtf8());
		cmdcache.write(Global::textureMeshInputDir.toUtf8());
		cmdcache.write("\n");
		cmdcache.write(Global::textureMeshWorkingDir.toUtf8());
		cmdcache.write("\n");
		cmdcache.write(Global::textureMeshOutputDir.toUtf8());
		cmdcache.write("\n");
		cmdcache.write(ui->comboBox->currentText().toUtf8());
		cmdcache.write("\n");
		cmdcache.close();
		QMessageBox::information(NULL, u8"���", u8"������� ", QMessageBox::Yes, NULL);
		PostThreadMessageA(Global::engineTid, CMD_TEXTUREMESH, 0, 0);
		Global::tasking = true;
		this->close();
	}
	else
	{
		QMessageBox::information(NULL, u8"����", u8"�޷����ʻ����ļ�������Ȩ�ޣ���ʹ�ù���Ա������� ", QMessageBox::Yes, NULL);
	}
}

void Dialog_TextureMesh::on_btn_CANCEL_clicked()
{
	this->close();
}

void Dialog_TextureMesh::on_pushButton_browseInputDir_clicked()
{
	Global::textureMeshInputDir = QFileDialog::getExistingDirectory(this, u8"���ģ���ؽ��ļ�Ŀ¼ ", "", NULL);
	ui->lineEdit_inputDir->setText(Global::textureMeshInputDir);
}

void Dialog_TextureMesh::on_pushButton_browseOutputDir_clicked()
{
	Global::textureMeshOutputDir = QFileDialog::getExistingDirectory(this, u8"����������ģ���ļ�Ŀ¼ ", "", NULL);
	ui->lineEdit_OutputDir->setText(Global::textureMeshOutputDir);
}
