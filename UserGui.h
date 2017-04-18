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
class QTreeWidgetItem;
// Forward VTK class declarations
class MouseInteractorStyle;
class vtkCubeAxesActor;
class vtkBillboardTextActor3D;

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
	void selectFirstColorPicker();
	void selectNextColorPicker();
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

	void resetCameraView();
	void clearSegmentsTree();

	void focusSelected(int idx);
	void visualizeMeanSim();
	void visualizeSelectSim();
	void updateSegmentTree(double *values);
private:
	// functions
	void noDataWarning();
	void noSelectWarning();
	void setActorHeight(vtkActor *prop, double h);
	void constrainSpinBoxes(double min, double max);
	void saveCameraParams();
	vtkSmartPointer<vtkActor> generateSquare();
	QTreeWidgetItem * createAddTreeItem(Superpixel s);

	void resetBounds();
	void addActorToBounds(vtkActor *actor);

	// Threads
	QFutureWatcher<void> FutureWatcher;

	// Data
	std::string rgbPath;
	std::string dptPath;
	std::string supPath;
	std::string feaPath;

	std::map<int, Superpixel> superpixels;
	std::map<int, Superpixel> sups;
	cv::Mat similarityMatrix;
	std::vector<double> mean_similarity;
	SimilarityMatrix sm;

	double bounds[6] = { 0,1,0,1,0,1 };

	vtkSmartPointer<vtkActorCollection> segments = vtkSmartPointer<vtkActorCollection>::New();
	vtkSmartPointer<vtkActorCollection> contours = vtkSmartPointer<vtkActorCollection>::New();
	std::map<int,vtkBillboardTextActor3D*> labels;

	vtkSmartPointer<vtkActor> minPlane = nullptr;
	vtkSmartPointer<vtkActor> maxPlane = nullptr;

	// Similarity visualisation
	enum SimilarityType
	{
		MEAN, SELECTED
	};

	SimilarityType simType = MEAN;
	int focusSegmentId = -1;

	// Designer form
	Ui_UserGui *ui;
	QFileDialog *fileDialog;
	QColorDialog *colorDialog;
	QSet<int> selectedInTree;

	// VTK objects
	vtkSmartPointer<vtkCubeAxesActor> axes = nullptr;

	// Interactor
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor3D = vtkSmartPointer<vtkRenderWindowInteractor>::New();

	// Renderer
	vtkSmartPointer<vtkRenderer> renderer3D = nullptr;
	vtkSmartPointer<MouseInteractorStyle> interactorStyle = nullptr;

	double position[3];
};

#endif