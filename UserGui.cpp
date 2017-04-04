#include "UserGui.h"

// This is included here because it is forward declared in
// UserGui.h
#include "ui_UserGui.h"

#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCubeAxesActor.h>
#include <vtkTextActor.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextProperty.h>
#include <vtkCamera.h>
#include <vtkTransform.h>
#include <vtkStringArray.h>
#include <map>

#include <opencv2/opencv.hpp>

#include "Sample.h"
#include "Superpixel.h"
#include "SimilarityMatrix.h"

void UserGui::addAxes()
{
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->Scale(1000.0, 1000.0, 1000.0);
	vtkSmartPointer<vtkCubeAxesActor> axes = vtkSmartPointer<vtkCubeAxesActor>::New();

	axes->SetUserTransform(transform);
	axes->SetCamera(renderer3D->GetActiveCamera());

	axes->SetBounds(bounds);
	axes->SetLabelOffset(3);

	axes->GetXAxesLinesProperty()->SetColor(0, 0, 0);
	axes->GetYAxesLinesProperty()->SetColor(0, 0, 0);
	axes->GetZAxesLinesProperty()->SetColor(0, 0, 0);

	axes->GetTitleTextProperty(0)->SetColor(1.0, 0.0, 0.0);
	axes->GetLabelTextProperty(0)->SetColor(1.0, 0.0, 0.0);
	axes->GetLabelTextProperty(0)->SetFontSize(15);

	axes->GetTitleTextProperty(1)->SetColor(0.0, 1.0, 0.0);
	axes->GetLabelTextProperty(1)->SetColor(0.0, 1.0, 0.0);
	axes->GetLabelTextProperty(1)->SetFontSize(15);

	axes->GetTitleTextProperty(2)->SetColor(0.0, 0.0, 1.0);
	axes->GetLabelTextProperty(2)->SetColor(0.0, 0.0, 1.0);
	axes->GetLabelTextProperty(2)->SetFontSize(15);

	axes->DrawXGridlinesOn();
	axes->DrawYGridlinesOn();
	axes->DrawZGridlinesOn();

	axes->SetGridLineLocation(axes->VTK_GRID_LINES_FURTHEST);
	axes->XAxisMinorTickVisibilityOff();
	axes->YAxisMinorTickVisibilityOff();
	
	renderer3D->AddActor(axes);
}

// Constructor
UserGui::UserGui() {
	this->ui = new Ui_UserGui;
	this->ui->setupUi(this);
	this->ui->progressBar->hide();
	this->fileDialog = new QFileDialog(nullptr, tr("Select image"), QString(), tr("Image Files (*.jpg)"));


	// Create a renderer, render window, and interactor
	renderer3D = vtkSmartPointer<vtkRenderer>::New();
	renderer3D->SetBackground(0.8, 0.8, 0.8);
	renderer3D->GetActiveCamera()->ParallelProjectionOn();
	renderer3D->GetActiveCamera()->SetViewUp(-1, 0, 0);

	addAxes();

	// VTK/Qt wedded
	renderer3D->ResetCamera();
	renderer3D->InteractiveOff();
	this->ui->qvtkWidget->GetRenderWindow()->AddRenderer(renderer3D);

	// Set up action signals and slots
	connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
	connect(this->ui->actionLoad_data, SIGNAL(triggered()), this, SLOT(loadData()));
	connect(this->ui->buttonVisualize, SIGNAL(clicked()), this, SLOT(visualizationThread()));
	connect(this->ui->buttonFlip, SIGNAL(clicked()), this, SLOT(flipAngle()));
}

void UserGui::slotExit() {
	qApp->exit();
}

void UserGui::loadData()
{
	if (fileDialog->exec())
	{
		QString file = fileDialog->selectedFiles()[0];
		rgbPath = file.toStdString();
		QDir base = QDir(file);
		base.cdUp();
		feaPath = base.absolutePath().toStdString() + "/features.txt";
		supPath = base.absolutePath().toStdString() + "/superpixels.png";
		base.cdUp();
		dptPath = base.absolutePath().toStdString() + "/depth/";
		dptPath += QFileInfo(file).fileName().split(".")[0].toStdString() + ".png";

		superpixels = Superpixel::loadSuperpixels(rgbPath, supPath, dptPath, feaPath);
		similarityMatrix = sm.getSimilarityMatrix(superpixels);
	}
}

void UserGui::visualizeData()
{
	if (renderer3D->GetActors()->GetNumberOfItems() > 0)
	{
		renderer3D->RemoveAllViewProps();
	}


	//showSilhuette();
	//computeSurface();
	//triangulateSurface();
	//drawText();
	if (similarityMatrix.empty())
	{
		superpixels = Superpixel::loadSuperpixels(rgbPath, supPath, dptPath, feaPath);
		similarityMatrix = sm.getSimilarityMatrix(superpixels);
	}

	this->ui->buttonVisualize->setDisabled(true);
	this->ui->progressBar->setValue(0);
	this->ui->progressBar->setRange(0, superpixels.size());

	// Create a mappers and actors
	std::map<int, Superpixel> sups;
	for (auto i = 0; i < 15; i++) {
		sups[i] = superpixels[i];
	}

	for (auto i = 0; i < 6; i+=2)
	{
		bounds[i] = LONG_MAX;
		bounds[i + 1] = LONG_MIN;
	}

	for (auto sup : sups) {
		sup.second.setHeight(1000 * similarityMatrix.at<double>(0, sup.second.m_id));
		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputData(sup.second.getSuperpixel());
		vtkSmartPointer<vtkActor> actor_segment = vtkSmartPointer<vtkActor>::New();
		actor_segment->SetMapper(mapper);
		actor_segment->GetProperty()->SetPointSize(2);
		// Add the actor to the scene
		renderer3D->AddActor(actor_segment);
		segments->AddItem(actor_segment);

		double* bnds = actor_segment->GetBounds();
		bounds[0] = bnds[0] < bounds[0] ? bnds[0] : bounds[0];
		bounds[1] = bnds[1] > bounds[1] ? bnds[1] : bounds[1];
		bounds[2] = bnds[2] < bounds[2] ? bnds[2] : bounds[2];
		bounds[3] = bnds[3] > bounds[3] ? bnds[3] : bounds[3];
		bounds[4] = bnds[4] < bounds[4] ? bnds[4] : bounds[4];
		bounds[5] = bnds[5] > bounds[5] ? bnds[5] : bounds[5];

		// get superpixels contour
		vtkSmartPointer<vtkPolyDataMapper> mapper_contour = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper_contour->SetInputData(sup.second.getContour());
		mapper_contour->ScalarVisibilityOff();
		vtkSmartPointer<vtkActor> actor_contour = vtkSmartPointer<vtkActor>::New();
		actor_contour->SetMapper(mapper_contour);
		actor_contour->GetProperty()->SetLineWidth(2.);
		actor_contour->GetProperty()->SetColor(0.8, 0., 0.);
		contours->AddItem(actor_contour);
		// Add actor to the scene
		renderer3D->AddActor(actor_contour);


		ui->progressBar->setValue(ui->progressBar->value() + 1);
	}

	addAxes();
	this->ui->qvtkWidget->GetRenderWindow()->Render();
	renderer3D->ResetCamera();

	this->ui->buttonVisualize->setDisabled(false);
}

void UserGui::visualizeDataFinished()
{
	this->ui->progressBar->hide();
}

void UserGui::flipAngle()
{
	if (!segments)
	{
		return;
	}
	for (vtkIdType i = 0; i < segments->GetNumberOfItems(); i++)
	{
		vtkSmartPointer<vtkActor> a = segments->GetNextActor();
		double* position = superpixels[i].getCentroid();
		vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
		transform->PostMultiply(); //this is the key line
		transform->Translate(-position[0], -position[1], -position[2]);
		transform->RotateX(rotation);
		transform->Translate(position);
		a->SetUserTransform(transform);

		vtkSmartPointer<vtkActor> b = contours->GetNextActor();
		b->SetUserTransform(transform);
	}
	this->ui->qvtkWidget->GetRenderWindow()->Render();
	rotation *= -1;
}

void UserGui::visualizationThread()
{
	//QThread *visualThread = new QThread();
	//this->ui->progressBar->show();
	//connect(visualThread, SIGNAL(started()), this, SLOT(visualizeData()));
	//connect(visualThread, SIGNAL(finished()), this, SLOT(visualizeDataFinished()));
	//visualThread->start();
}
