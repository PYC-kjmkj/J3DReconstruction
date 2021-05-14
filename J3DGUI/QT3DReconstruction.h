#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_QT3DReconstruction.h"
#include <Windows.h>
#include "dialog_matchfeature.h"
#include "dialog_sfm.h"
#include "dialog_densifypointcloud.h"
#include "Dialog_ReconstructMesh.h"
#include "dialog_texturemesh.h"
#include "dialog_addsensorwidth.h"
#include "dialog_fullauto.h"
#include "dialog_coor.h"
#include <iostream>
#include "qfiledialog.h"
#include "qmessagebox.h"
#include <qprocess.h>
#include <qdebug.h>
#include <qwindow.h>
#include <qwidget.h>
#include <qtextcodec.h>
#include "Global.h"
#include <qfile.h>
#include <qtimer.h>
#include <qinputdialog.h>
#include <fstream>
#include "MVSEngine.h"
#include "mvsviewer.h"
#include "qevent.h"
#include <boost/program_options.hpp>
#include <time.h>
#include <qevent.h>
#include "Jutil.h"
#include "Common.h"
#include "Scene.h"

class QT3DReconstruction : public QMainWindow
{
	Q_OBJECT

public:
	QT3DReconstruction(QWidget *parent = Q_NULLPTR);
	~QT3DReconstruction();

	bool openView(QString fileName);
	static bool openViewCompatibility(QString fileName);
	bool converseType(QString fileNameSrc, QString fileNameDes);
private slots:

	void on_actionMatchFeature_triggered();
	void on_actionSFM_triggered();
	void on_action_viewMVS_triggered();
	void on_action_addSensorWidth_triggered();
	void on_actionDenseCloud_triggered();
	void timerSlot();
	void on_action_reconstrctMesh_triggered();
	void on_action_triggered();
	void on_actionopen_mvs_file_triggered();
	void on_action_fullauto_triggered();
    void on_action_2_triggered();
    void on_pushButton_camera_clicked();
    void on_pushButton_pointcloud_clicked();
    void on_pushButton_mesh_clicked();
    void on_pushButton_texture_clicked();
    void on_pushButton_pointplus_clicked();
    void on_pushButton_pointsub_clicked();
    void on_pushButton_pointnumplus_clicked();
    void on_pushButton_pointnumsub_clicked();
    void on_pushButton_viewportplus_clicked();
    void on_pushButton_viewportsub_clicked();
    void on_pushButton_export_clicked();
	void on_action_coor_triggered();
    void on_action_import_BlocksExchange_triggered();

private:

	Ui::QT3DReconstructionClass ui;
	Dialog_MatchFeature dlgmf;
	Dialog_SFM dlgsfm;
	Dialog_DensifyPointCloud dlgdense;
	Dialog_ReconstructMesh dlgrm;
	Dialog_TextureMesh dlgtm;
	Dialog_addsensorwidth dlgasw;
	Dialog_FullAuto dlgfa;
	dialog_coor dlgcoor;
	QTimer* timer;
	VIEWER::Scene* J3DViewer;
	bool J3DViewerAva;
	Jutil::SparseFileName J3DFile;
	void closeEvent(QCloseEvent *event);
protected:

};
