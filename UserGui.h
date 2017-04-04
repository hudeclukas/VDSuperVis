#ifndef USERGUI_H
#define USERGUI_H

#include <vtkSmartPointer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActorCollection.h>

#include <QMainWindow>
#include <QFileDialog>
#include <QFutureWatcher>

#include <opencv2/opencv.hpp>

#include "Superpixel.h"
#include "SimilarityMatrix.h"

// Forward Qt class declarations
class Ui_UserGui;

class UserGui : public QMainWindow {
	Q_OBJECT
public:

	void addAxes();
	// Constructor/Destructor
	UserGui();
	~UserGui() {};

	public slots:

	virtual void slotExit();
	void loadData();
	void visualizeData();
	void visualizeDataFinished();
	void visualizationThread();
	void flipAngle();

private:
	// Threads

	// Data
	std::string rgbPath = "D:/RGBD datasets/VD/000002_2014-05-26_14-23-37_260595134347_rgbf000103-resize/image/0000103.jpg";
	std::string dptPath = "D:/RGBD datasets/VD/000002_2014-05-26_14-23-37_260595134347_rgbf000103-resize/depth/0000103.png";
	std::string supPath = "D:/RGBD datasets/VD/000002_2014-05-26_14-23-37_260595134347_rgbf000103-resize/image/superpixels.png";
	std::string feaPath = "D:/RGBD datasets/VD/000002_2014-05-26_14-23-37_260595134347_rgbf000103-resize/image/features.txt";

	std::map<int, Superpixel> superpixels;
	cv::Mat similarityMatrix;
	SimilarityMatrix sm;

	int rotation = -90;
	double bounds[6] = { 0,1,0,1,0,1 };

	vtkSmartPointer<vtkActorCollection> segments;
	vtkSmartPointer<vtkActorCollection> contours;


	// Designer form
	Ui_UserGui *ui;
	QFileDialog *fileDialog;

	// Interactor
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor3D = vtkSmartPointer<vtkRenderWindowInteractor>::New();

	// Renderer
	vtkSmartPointer<vtkRenderer> renderer3D;
};

#endif