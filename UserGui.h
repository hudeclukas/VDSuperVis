#ifndef USERGUI_H
#define USERGUI_H

#include <vtkSmartPointer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActorCollection.h>

#include <QMainWindow>
#include <QFileDialog>
#include <QColorDialog>
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
	void contourColorPicker();
	void filterPlaneMinColorPicker();
	void filterPlaneMaxColorPicker();
	void changeContourColor(QColor color);
	void changeFilterPlaneMaxColor(QColor color);
	void changeFilterPlaneMinColor(QColor color);
	void loadData();
	void visualizeData();
	void visualizeDataFinished();
	void visualizationThread();
	void flipAngle(double rotation);
	void flipSegments(int type);
	void showContours(bool change);
	void minFilter(double min);
	void maxFilter(double max);
	void filterPlanes(bool check);
	void filterDelete(bool check);
private:
	// functions
	void noDataWarning();
	void resetCameraView();
	void constrainSpinBoxes(double min, double max);
	vtkSmartPointer<vtkActor> generateSquare();

	// Threads
	QFutureWatcher<void> FutureWatcher;

	// Data
	std::string rgbPath;
	std::string dptPath;
	std::string supPath;
	std::string feaPath;

	std::map<int, Superpixel> superpixels;
	cv::Mat similarityMatrix;
	SimilarityMatrix sm;

	double bounds[6] = { 0,1,0,1,0,1 };

	vtkSmartPointer<vtkActorCollection> segments = vtkSmartPointer<vtkActorCollection>::New();
	vtkSmartPointer<vtkActorCollection> contours = vtkSmartPointer<vtkActorCollection>::New();
	vtkSmartPointer<vtkActor> minPlane = nullptr;
	vtkSmartPointer<vtkActor> maxPlane = nullptr;

	// Designer form
	Ui_UserGui *ui;
	QFileDialog *fileDialog;
	QColorDialog *colorDialog;

	// Interactor
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor3D = vtkSmartPointer<vtkRenderWindowInteractor>::New();

	// Renderer
	vtkSmartPointer<vtkRenderer> renderer3D;
};

#endif