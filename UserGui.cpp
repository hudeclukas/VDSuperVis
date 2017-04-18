#include "UserGui.h"

// This is included here because it is forward declared in
// UserGui.h
#include "ui_UserGui.h"

#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>

#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCubeAxesActor.h>
#include <vtkQuad.h>
#include <vtkTextProperty.h>
#include <vtkCamera.h>
#include <vtkTransform.h>
#include <vtkBillboardTextActor3D.h>

#include <map>

#include <opencv2/opencv.hpp>

#include "UserInputParser.h"
#include "Superpixel.h"
#include "SimilarityMatrix.h"
#include "MouseInteractorStyle.h"



void UserGui::addAxes()
{
	if (!axes) {
		axes = vtkSmartPointer<vtkCubeAxesActor>::New();
		vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
		transform->Scale(1000.0, 1000.0, 1000.0);

		axes->SetUserTransform(transform);
		axes->SetCamera(renderer3D->GetActiveCamera());

		axes->SetBounds(bounds);
		axes->SetZAxisRange(bounds[4] / 1000, bounds[5] / 1000);
		axes->SetTitleOffset(axes->GetLabelOffset() + 3);

		axes->GetXAxesLinesProperty()->SetColor(0, 0, 0);
		axes->GetYAxesLinesProperty()->SetColor(0, 0, 0);
		axes->GetZAxesLinesProperty()->SetColor(0, 0, 0);

		axes->SetXTitle("");
		axes->SetXAxisLabelVisibility(0);

		axes->SetYTitle("");
		axes->SetYAxisLabelVisibility(0);

		axes->SetZTitle("Cosine similarity");
		axes->GetTitleTextProperty(2)->SetColor(0.0, 0.0, 1.0);
		axes->GetLabelTextProperty(2)->SetColor(0.0, 0.0, 1.0);
		axes->GetLabelTextProperty(2)->SetFontSize(15);

		axes->DrawXGridlinesOn();
		axes->DrawYGridlinesOn();
		axes->DrawZGridlinesOn();

		axes->SetGridLineLocation(axes->VTK_GRID_LINES_FURTHEST);
		axes->XAxisMinorTickVisibilityOff();
		axes->YAxisMinorTickVisibilityOff();
	}
	else
	{
		auto bnds = axes->GetBounds();
		if (bnds[0] != bounds[0] ||
			bnds[1] != bounds[1] || 
			bnds[2] != bounds[2] || 
			bnds[3] != bounds[3] || 
			bnds[4] != bounds[4] || 
			bnds[5] != bounds[5] )
		{
			axes->SetBounds(bounds);
			axes->SetZAxisRange(bounds[4] / 1000, bounds[5] / 1000);
		}
	}
	renderer3D->AddActor(axes);
}

// Constructor
UserGui::UserGui() {
	// Setup QT Gui
	{
		this->ui = new Ui_UserGui;
		this->ui->setupUi(this);
		this->ui->progressBar->hide();
		this->fileDialog = new QFileDialog(nullptr, tr("Select image"), QString(), tr("Image Files (*.jpg)"));
		this->colorDialog = new QColorDialog(QColor(255, 0, 0));
		this->colorDialog->setOption(QColorDialog::ShowAlphaChannel);
		this->ui->flipBox->setItemData(0, tr("Base segment view"), Qt::ToolTipRole);
		this->ui->flipBox->setItemData(1, tr("Rotate segments 90deg by X axis"), Qt::ToolTipRole);
		this->ui->flipBox->setItemData(2, tr("Set segments to follow camera"), Qt::ToolTipRole);
		this->ui->segmentsTree->setSortingEnabled(true);
		this->ui->segmentsTree->sortByColumn(0, Qt::AscendingOrder);
	}
	// Create a renderer, render window, and interactor
	{
		renderer3D = vtkSmartPointer<vtkRenderer>::New();
		renderer3D->SetBackground(0.8, 0.8, 0.8);
		this->addAxes();
	}

	// VTK/Qt wedded
	{
		renderer3D->ResetCamera();
		renderer3D->InteractiveOff();
		this->ui->qvtkWidget->GetRenderWindow()->AddRenderer(renderer3D);
	}
	// Set interactor style
	{
		interactorStyle = vtkSmartPointer<MouseInteractorStyle>::New();
		interactorStyle->SetDefaultRenderer(renderer3D);
		this->ui->qvtkWidget->GetInteractor()->SetInteractorStyle(interactorStyle);
	}
	// Save camera position
	{
		this->saveCameraParams();
	}
	// Set up action signals and slots
	{
		connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
		connect(this->ui->actionLoad_data, SIGNAL(triggered()), this, SLOT(loadData()));
		connect(this->ui->actionContour_color, SIGNAL(triggered()), this, SLOT(contourColorPicker()));
		connect(this->ui->actionFilterMin_color, SIGNAL(triggered()), this, SLOT(filterPlaneMinColorPicker()));
		connect(this->ui->actionFilterMax_color, SIGNAL(triggered()), this, SLOT(filterPlaneMaxColorPicker()));
		connect(this->ui->actionFilterMax_color, SIGNAL(triggered()), this, SLOT(filterPlaneMaxColorPicker()));
		connect(this->ui->actionFSelected_color, SIGNAL(triggered()), this, SLOT(selectFirstColorPicker()));
		connect(this->ui->actionNSelected_color, SIGNAL(triggered()), this, SLOT(selectNextColorPicker()));
		connect(this->ui->buttonVisualize, SIGNAL(clicked()), this, SLOT(visualizeData()));
		connect(this->ui->flipBox, SIGNAL(currentIndexChanged(int)), this, SLOT(flipSegments(int)));
		connect(this->ui->checkboxContours, SIGNAL(clicked(bool)), this, SLOT(showContours(bool)));
		connect(this->ui->minSimilarity, SIGNAL(valueChanged(double)), this, SLOT(minFilter(double)));
		connect(this->ui->maxSimilarity, SIGNAL(valueChanged(double)), this, SLOT(maxFilter(double)));
		connect(this->ui->checkBoxPlanes, SIGNAL(clicked(bool)), this, SLOT(filterPlanes(bool)));
		connect(this->ui->checkBoxDelete, SIGNAL(clicked(bool)), this, SLOT(filterDelete(bool)));
		connect(&this->FutureWatcher, SIGNAL(finished()), this, SLOT(visualizeDataFinished()));

		connect(this->ui->buttonResetView, SIGNAL(clicked()), this, SLOT(resetCameraView()));
		connect(this->ui->buttonClearSelect, SIGNAL(clicked()), interactorStyle, SLOT(clearSelection()));
		connect(this->ui->buttonClearSelect, SIGNAL(clicked()), this, SLOT(clearSegmentsTree()));

		connect(this->ui->checkBoxSelect, SIGNAL(clicked(bool)), interactorStyle, SLOT(setSelect(bool)));
		connect(this->ui->checkBoxMultiSelect, SIGNAL(clicked(bool)), interactorStyle, SLOT(setMultiSelect(bool)));
		connect(this->interactorStyle, SIGNAL(pickedActor(int)), this, SLOT(focusSelected(int)));
		connect(this->ui->buttonMeanSim, SIGNAL(clicked()), this, SLOT(visualizeMeanSim()));
		connect(this->ui->buttonSelectSim, SIGNAL(clicked()), this, SLOT(visualizeSelectSim()));
	}
}

void UserGui::slotExit() {
	qApp->exit();
}

void UserGui::noDataWarning()
{	
	QMessageBox::warning(this, tr("Empty data"),
		tr("To change color load data first."), QMessageBox::Ok,
		QMessageBox::Ok);
}

void UserGui::noSelectWarning() {
	QMessageBox::warning(this, tr("Empty selection"),
		tr("To change similarity mapping, select segment first."), QMessageBox::Ok,
		QMessageBox::Ok);
}

void UserGui::contourColorPicker()
{
	if (contours->GetNumberOfItems() == 0) {
		noDataWarning();
		return;
	}
	contours->InitTraversal();
	double old_color[3];
	double old_opacity = 1;
	vtkSmartPointer<vtkProperty> property = contours->GetNextItem()->GetProperty();
	property->GetColor(old_color);
	property->GetOpacity();
	connect(this->colorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(changeContourColor(QColor)));
	if (colorDialog->exec() == QDialog::Rejected) {
		QColor color = QColor(old_color[0] * 255, old_color[1] * 255, old_color[2] * 255, old_opacity * 255);
		changeContourColor(color);
	}
	disconnect(this->colorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(changeContourColor(QColor)));
}

void UserGui::changeContourColor(QColor color) {
	if (!contours) {
		return;
	}
	contours->InitTraversal();
	for (vtkIdType i = 0; i < contours->GetNumberOfItems(); i++) {
		vtkSmartPointer<vtkActor> c = contours->GetNextItem();
		c->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
		c->GetProperty()->SetOpacity(color.alphaF());
	}
	this->ui->qvtkWidget->GetRenderWindow()->Render();
}

void UserGui::filterPlaneMinColorPicker()
{
	if (!minPlane)
	{
		noDataWarning();
		return;
	}
	double old_color[3];
	double old_opacity;
	vtkSmartPointer<vtkProperty> property = minPlane->GetProperty();
	property->GetColor(old_color);
	old_opacity = property->GetOpacity();
	QColor old_qcolor = QColor(old_color[0] * 255, old_color[1] * 255, old_color[2] * 255, old_opacity * 255);
	this->colorDialog->setCurrentColor(QColor(old_qcolor));
	connect(this->colorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(changeFilterPlaneMinColor(QColor)));
	if (colorDialog->exec() == QDialog::Rejected) {
		QColor color = QColor(old_color[0] * 255, old_color[1] * 255, old_color[2] * 255, old_opacity * 255);
		changeFilterPlaneMinColor(color);
	}
	disconnect(this->colorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(changeFilterPlaneMinColor(QColor)));
}

void UserGui::filterPlaneMaxColorPicker() {
	if (!maxPlane) {
		noDataWarning();
		return;
	}
	double old_color[3];
	double old_opacity;
	vtkSmartPointer<vtkProperty> property = maxPlane->GetProperty();
	property->GetColor(old_color);
	old_opacity = property->GetOpacity();
	QColor old_qcolor = QColor(old_color[0] * 255, old_color[1] * 255, old_color[2] * 255, old_opacity * 255);
	this->colorDialog->setCurrentColor(QColor(old_qcolor));
	connect(this->colorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(changeFilterPlaneMaxColor(QColor)));
	if (colorDialog->exec() == QDialog::Rejected) {
		QColor color = QColor(old_color[0] * 255, old_color[1] * 255, old_color[2] * 255, old_opacity * 255);
		changeFilterPlaneMaxColor(color);
	}
	disconnect(this->colorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(changeFilterPlaneMaxColor(QColor)));
}

void UserGui::changeFilterPlaneMinColor(QColor color)
{
	if (!minPlane)	{
		return;
	}
	minPlane->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
	minPlane->GetProperty()->SetOpacity(color.alphaF());
	this->ui->qvtkWidget->GetRenderWindow()->Render();
}

void UserGui::changeFilterPlaneMaxColor(QColor color) {
	if (!maxPlane) {
		return;
	}
	maxPlane->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
	maxPlane->GetProperty()->SetOpacity(color.alphaF());
	this->ui->qvtkWidget->GetRenderWindow()->Render();
}

void UserGui::selectFirstColorPicker() {
	auto old_color = interactorStyle->getFirstSelectColor();
	connect(this->colorDialog, SIGNAL(currentColorChanged(QColor)), interactorStyle, SLOT(setFirstSelectColor(QColor)));
	if (colorDialog->exec() == QDialog::Rejected) {
		interactorStyle->setFirstSelectColor(old_color);
	}
	disconnect(this->colorDialog, SIGNAL(currentColorChanged(QColor)), interactorStyle, SLOT(setFirstSelectColor(QColor)));
}

void UserGui::selectNextColorPicker() {
	auto old_color = interactorStyle->getNextSelectColor();
	connect(this->colorDialog, SIGNAL(currentColorChanged(QColor)), interactorStyle, SLOT(setNextSelectColor(QColor)));
	if (colorDialog->exec() == QDialog::Rejected) {
		interactorStyle->setNextSelectColor(old_color);
	}
	disconnect(this->colorDialog, SIGNAL(currentColorChanged(QColor)), interactorStyle, SLOT(setNextSelectColor(QColor)));
}

void UserGui::loadData()
{
	if (fileDialog->exec() == QDialog::Accepted)
	{
		QString file = fileDialog->selectedFiles()[0];
		rgbPath = file.toStdString();
		QDir base = QDir(file);
		base.cdUp();
		fileDialog->setDirectory(base);
		feaPath = base.absolutePath().toStdString() + "/features.txt";
		supPath = base.absolutePath().toStdString() + "/superpixels.png";
		base.cdUp();
		dptPath = base.absolutePath().toStdString() + "/depth/";
		dptPath += QFileInfo(file).fileName().split(".")[0].toStdString() + ".png";

		superpixels = Superpixel::loadSuperpixels(rgbPath, supPath, dptPath, feaPath);
		if (superpixels.size() == 0)
		{
			QMessageBox::warning(this, tr("No superpixels loaded"),
				tr("Find precomputed dataset"), QMessageBox::Ok,
				QMessageBox::Ok);
			return;
		}
		similarityMatrix = sm.getSimilarityMatrix(superpixels);
		ui->buttonVisualize->setDisabled(false);
		ui->rangeSelect->setDisabled(false);
		QString range = QString::number(0) + "-" + QString::number(superpixels.size() - 1);
		ui->rangeSelect->setPlaceholderText(range);
		ui->rangeSelect->setText(range);
	}
}

void UserGui::visualizeData()
{
	this->resetCameraView();
	if (renderer3D->GetActors()->GetNumberOfItems() > 0)
	{
		renderer3D->RemoveAllViewProps();
	}
	if (minPlane) {
		minPlane->Delete();
		minPlane = nullptr;
	}
	if (maxPlane) {
		maxPlane->Delete();
		maxPlane = nullptr;
	}

	auto range = parseRange(ui->rangeSelect->text());

	this->ui->progressBar->show();
	this->ui->progressBar->setValue(0);
	this->ui->progressBar->setRange(0, range.size());

	// Create a mappers and actors
	sups.clear();
	for (auto i : range) 
	{
		sups[i] = superpixels[i];
	}

	this->resetBounds();
	clearSegmentsTree();
	interactorStyle->clearSelection();

	mean_similarity = sm.getMeanSimilarity();

	for (auto sup : sups) {
		// Map superpixels
		//sup.second.setHeight(1000 * mean_similarity[sup.second.m_id]);
		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputData(sup.second.getSuperpixel());
		vtkSmartPointer<vtkActor> actor_segment = vtkSmartPointer<vtkActor>::New();
		actor_segment->SetMapper(mapper);
		actor_segment->GetProperty()->SetPointSize(2);
		setActorHeight(actor_segment, 1000 * mean_similarity[sup.second.m_id]);
		segments->AddItem(actor_segment);
		// Add the actor to the scene
		renderer3D->AddActor(actor_segment);

		this->addActorToBounds(actor_segment);
		
		// get superpixels contour
		vtkSmartPointer<vtkPolyDataMapper> mapper_contour = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper_contour->SetInputData(sup.second.getContour());
		mapper_contour->ScalarVisibilityOff();
		vtkSmartPointer<vtkActor> actor_contour = vtkSmartPointer<vtkActor>::New();
		actor_contour->SetMapper(mapper_contour);
		actor_contour->GetProperty()->SetLineWidth(2.);
		actor_contour->GetProperty()->SetColor(0.8, 0., 0.);
		actor_contour->SetVisibility(ui->checkboxContours->isChecked());
		setActorHeight(actor_contour, 1000 * mean_similarity[sup.second.m_id]);
		contours->AddItem(actor_contour);
		// Add actor to the scene
		renderer3D->AddActor(actor_contour);

		// add labels to scene
		//vtkSmartPointer<vtkBillboardTextActor3D> actor_label = vtkSmartPointer<vtkBillboardTextActor3D>::New();
		//actor_label->SetInput((std::to_string(sup.second.m_id) + ".").c_str());
		//auto pos = sup.second.getCentroid();
		//pos[2] += 0;
		//actor_label->SetPosition(pos);
		//actor_label->GetTextProperty()->SetFontSize(25);
		//actor_label->GetTextProperty()->BoldOn();
		//actor_label->GetTextProperty()->SetColor(1.0, 1.0, .4);
		//actor_label->GetTextProperty()->SetJustificationToCentered();
		//setActorHeight(actor_contour, 1000 * mean_similarity[sup.second.m_id]);
		//labels[sup.second.m_id] = (actor_label);
		//// Add actor to the scene
		//renderer3D->AddActor(actor_label);

		ui->progressBar->setValue(ui->progressBar->value() + 1);
	}

	interactorStyle->setActors(segments);
	constrainSpinBoxes((bounds[4]-10)/1000, (bounds[5]+10)/1000);
	addAxes();
	this->ui->qvtkWidget->GetRenderWindow()->Render();
	this->renderer3D->ResetCamera();
	saveCameraParams();

	this->ui->qvtkWidget->GetRenderWindow()->Render();

	ui->progressBar->hide();
}

void UserGui::visualizeDataFinished()
{
	this->ui->progressBar->hide();
}

void UserGui::setActorHeight(vtkActor* prop, double h)
{
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->PostMultiply(); //this is the key line
	transform->Translate(0,0,h);
	prop->SetUserTransform(transform);
}


void UserGui::flipAngle(double rotation)
{
	if (!segments)
	{
		return;
	}
	segments->InitTraversal();
	contours->InitTraversal();
	for (vtkIdType i = 0; i < segments->GetNumberOfItems(); i++)
	{
		vtkSmartPointer<vtkActor> a = segments->GetNextItem();
		auto actorTransform = a->GetUserTransform();
		vtkTransform* actorVtkTransform = vtkTransform::SafeDownCast(actorTransform);
		auto positionTransform = actorVtkTransform->GetPosition();
		auto positionCentroid = superpixels[i].getCentroid();
		positionCentroid[2] = positionTransform[2];
		vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
		transform->PostMultiply(); //this is the key line
		transform->Translate(-positionCentroid[0], -positionCentroid[1], -positionCentroid[2]);
		transform->RotateX(rotation);
		transform->Translate(positionCentroid);
		a->SetUserTransform(transform);

		vtkSmartPointer<vtkActor> b = contours->GetNextItem();
		b->SetUserTransform(transform);
	}
	this->ui->qvtkWidget->GetRenderWindow()->Render();
}

void UserGui::flipSegments(int type)
{
	switch (type)
	{
	case 0:
		flipAngle(0);
		return;
	case 1:
		flipAngle(-90);
		return;
	case 2:
		vtkSmartPointer<vtkCamera> camera = renderer3D->GetActiveCamera();
		auto orientation = camera->GetOrientation();
		cout << orientation[0] << " " << orientation[1] << " " << orientation[2] << " " << orientation[3] << endl;
	}
}

void UserGui::showContours(bool change)
{
	if (!contours) {
		return;
	}
	contours->InitTraversal();
	for (vtkIdType i = 0; i < contours->GetNumberOfItems(); i++) {
		vtkSmartPointer<vtkActor> c = contours->GetNextItem();
		c->SetVisibility(change);
	}
	this->ui->qvtkWidget->GetRenderWindow()->Render();
}

void UserGui::constrainSpinBoxes(double min, double max)
{
	auto minSpin = ui->minSimilarity;
	auto maxSpin = ui->maxSimilarity;
	minSpin->setMinimum(min);
	minSpin->setMaximum(max);
	maxSpin->setMinimum(min);
	maxSpin->setMaximum(max);
	minSpin->setValue(min);
	maxSpin->setValue(max);
}

vtkSmartPointer<vtkActor> UserGui::generateSquare()
{
	// Square
	// Create four points (must be in counter clockwise order)
	double p0[3] = { 0.0, 0.0, 0.0 };
	double p1[3] = { 1.0, 0.0, 0.0 };
	double p2[3] = { 1.0, 1.0, 0.0 };
	double p3[3] = { 0.0, 1.0, 0.0 };
	// Add the points to a vtkPoints object
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	points->InsertNextPoint(p0);
	points->InsertNextPoint(p1);
	points->InsertNextPoint(p2);
	points->InsertNextPoint(p3);
	// Create a quad on the four points
	vtkSmartPointer<vtkQuad> quad = vtkSmartPointer<vtkQuad>::New();
	quad->GetPointIds()->SetId(0, 0);
	quad->GetPointIds()->SetId(1, 1);
	quad->GetPointIds()->SetId(2, 2);
	quad->GetPointIds()->SetId(3, 3);
	// Create a cell array to store the quad in
	vtkSmartPointer<vtkCellArray> quads = vtkSmartPointer<vtkCellArray>::New();
	quads->InsertNextCell(quad);
	// Create a polydata to store everything in
	vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
	// Add the points and quads to the dataset
	polydata->SetPoints(points);
	polydata->SetPolys(quads);
	// Setup actor and mapper
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(polydata);
	vtkSmartPointer<vtkActor> square_actor = vtkSmartPointer<vtkActor>::New();
	square_actor->SetMapper(mapper);
	return square_actor;
}

void UserGui::minFilter(double min)
{
	if (minPlane == nullptr)
	{
		minPlane = generateSquare();
		minPlane->SetScale(bounds[1] - bounds[0], bounds[3] - bounds[2], 1);
		minPlane->GetProperty()->SetColor(0, 0, 1);
		minPlane->GetProperty()->SetOpacity(0.9);
		renderer3D->AddActor(minPlane);
	}
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->PostMultiply();
	double x = bounds[0];
	double y = bounds[2];
	transform->Translate(x, y, min * 1000);
	minPlane->SetUserTransform(transform);
	filterPlanes(ui->checkBoxPlanes->isChecked());
	filterDelete(ui->checkBoxDelete->isChecked());
	this->ui->qvtkWidget->GetRenderWindow()->Render();
}

void UserGui::maxFilter(double max)
{
	if (maxPlane == nullptr) {
		maxPlane = generateSquare();
		maxPlane->SetScale(bounds[1] - bounds[0], bounds[3] - bounds[2], 1);
		maxPlane->GetProperty()->SetColor(1, 0, 0);
		maxPlane->GetProperty()->SetOpacity(0.9);
		renderer3D->AddActor(maxPlane);
	}
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->PostMultiply();
	double x = bounds[0];
	double y = bounds[2];
	transform->Translate(x, y, max * 1000);
	maxPlane->SetUserTransform(transform);
	filterPlanes(ui->checkBoxPlanes->isChecked());
	filterDelete(ui->checkBoxDelete->isChecked());
	this->ui->qvtkWidget->GetRenderWindow()->Render();
}

void UserGui::filterPlanes(bool check)
{
	if (minPlane != nullptr && maxPlane != nullptr)
	{
		if (ui->minSimilarity->value() * 1000 >= bounds[4]) 
		{
			minPlane->SetVisibility(check);
		} 
		else
		{
			minPlane->SetVisibility(false);
		}
		if (ui->maxSimilarity->value() * 1000 <= bounds[5])
		{
			maxPlane->SetVisibility(check);
		} 
		else
		{
			maxPlane->SetVisibility(false);
		}
	}
	this->ui->qvtkWidget->GetRenderWindow()->Render();
}

void UserGui::filterDelete(bool deleteCheck)
{
	if (!segments) {
		return;
	}
	segments->InitTraversal();
	contours->InitTraversal();
	for (vtkIdType i = 0; i < segments->GetNumberOfItems(); i++) {
		vtkSmartPointer<vtkActor> a = segments->GetNextItem();
		vtkSmartPointer<vtkActor> b = contours->GetNextItem();
		if (deleteCheck) {
			auto height = a->GetBounds()[4];
			double min = ui->minSimilarity->value();
			double max = ui->maxSimilarity->value();
			if (height < min * 1000 || max * 1000 < height) {
				a->SetVisibility(false);
				b->SetVisibility(false);
			} 
			else
			{
				a->SetVisibility(true);
				if (ui->checkboxContours->isChecked())
					b->SetVisibility(true);
			}
		}
		else
		{
			a->SetVisibility(true);
			if (ui->checkboxContours->isChecked())
				b->SetVisibility(true);
		}
	}
	this->ui->qvtkWidget->GetRenderWindow()->Render();
}

void UserGui::resetCameraView()
{
	auto camera = renderer3D->GetActiveCamera();
	camera->SetPosition(position);
	camera->SetViewUp(0, 1, 0);
	renderer3D->ResetCamera();

	this->ui->qvtkWidget->GetRenderWindow()->Render();
}

void UserGui::clearSegmentsTree()
{
	this->ui->segmentsTree->clear();
	this->selectedInTree.clear();
}

void UserGui::saveCameraParams()
{
	auto camera = renderer3D->GetActiveCamera();
	camera->ParallelProjectionOn();
	camera->GetPosition(position);
}

void UserGui::visualizationThread()
{
	//this->ui->progressBar->show();
	//QFuture<void> future = QtConcurrent::run(this, visualizeData);
	//this->FutureWatcher.setFuture(future);
}

QTreeWidgetItem* UserGui::createAddTreeItem(Superpixel s) {	
	QTreeWidgetItem *rootItem = new QTreeWidgetItem(ui->segmentsTree);
	rootItem->setText(0, QString::number(s.m_id));
	switch (simType)
	{
	case MEAN : 
		rootItem->setText(1, QString::number(mean_similarity[s.m_id]));
		break;
	case SELECTED :
		auto simRow = similarityMatrix.ptr<double>(focusSegmentId);
		rootItem->setText(1, QString::number(simRow[s.m_id]));
		break;
	}
	auto features = s.getFeatureArray();
	auto featureNames = s.getFeatureNamesArray();
	for (int i = 0; i < 7; i++) {
		QTreeWidgetItem *childItem = new QTreeWidgetItem();
		childItem->setText(0, QString::fromStdString(featureNames[i]));
		childItem->setText(1, QString::number(features[i]));
		rootItem->addChild(childItem);
	}
	return rootItem;
}

void UserGui::focusSelected(int idx) {
	bool multiSelect = ui->checkBoxMultiSelect->isChecked();
	if (!multiSelect)
	{
		ui->segmentsTree->clear();
	}
	int i = 0;
	for (auto sup : sups)
	{
		if (i == idx)
		{
			QTreeWidgetItem *item = nullptr;
			if (!selectedInTree.contains(sup.second.m_id)) 
			{
				item = this->createAddTreeItem(sup.second);
				if (multiSelect) 
				{
					selectedInTree.insert(sup.second.m_id);
				} else
				{
					focusSegmentId = sup.second.m_id;
				}
			}
			if (item) {
				if (!multiSelect) 
				{
					ui->segmentsTree->expandAll();
				}
				else 
				{
					ui->segmentsTree->collapseItem(item);
				}
			}
			return;
		}
		++i;
	}
}

void UserGui::resetBounds() {
	for (auto i = 0; i < 6; i += 2) {
		bounds[i] = LONG_MAX;
		bounds[i + 1] = LONG_MIN;
	}
}

void UserGui::addActorToBounds(vtkActor* actor) {
	auto bnds = actor->GetBounds();
	bounds[0] = bnds[0] < bounds[0] ? bnds[0] : bounds[0];
	bounds[1] = bnds[1] > bounds[1] ? bnds[1] : bounds[1];
	bounds[2] = bnds[2] < bounds[2] ? bnds[2] : bounds[2];
	bounds[3] = bnds[3] > bounds[3] ? bnds[3] : bounds[3];
	bounds[4] = bnds[4] < bounds[4] ? bnds[4] : bounds[4];
	bounds[5] = bnds[5] > bounds[5] ? bnds[5] : bounds[5];
}

void UserGui::visualizeMeanSim()
{
	simType = MEAN;
	resetBounds();
	segments->InitTraversal();
	contours->InitTraversal();
	for (auto sup : sups)
	{
		vtkActor* propA = segments->GetNextItem();
		vtkActor* propC = contours->GetNextItem();
		//vtkActor* propL = vtkActor::SafeDownCast(labels[sup.second.m_id]);
		double h = 1000 * mean_similarity[sup.second.m_id];
		setActorHeight(propA, h);
		setActorHeight(propC, h);
		//setActorHeight(propL, h);
		addActorToBounds(propA);
	}
	addAxes();
	constrainSpinBoxes((bounds[4] - 10) / 1000, (bounds[5] + 10) / 1000);
	interactorStyle->ReHighlightProps();
	renderer3D->ResetCamera();
	ui->qvtkWidget->GetRenderWindow()->Render();
	updateSegmentTree(mean_similarity.data());
}

void UserGui::visualizeSelectSim()
{
	simType = SELECTED;
	resetBounds();
	if (focusSegmentId != -1) {
		ui->buttonSelectSim->setText(ui->buttonSelectSim->toolTip() + ": " + QString::number(focusSegmentId));
		auto simRow = similarityMatrix.ptr<double>(focusSegmentId);
		segments->InitTraversal();
		contours->InitTraversal();
		for (auto sup : sups) {
			vtkActor* propA = segments->GetNextItem();
			vtkActor* propC = contours->GetNextItem();
			//vtkActor* propL = vtkActor::SafeDownCast(labels[sup.second.m_id]);
			double h = 1000 * simRow[sup.second.m_id];
			setActorHeight(propA, h);
			setActorHeight(propC, h);
			//setActorHeight(propL, h);
			addActorToBounds(propA);
		}
		addAxes();
		constrainSpinBoxes((bounds[4] - 10) / 1000, (bounds[5] + 10) / 1000);
		interactorStyle->ReHighlightProps();
		renderer3D->ResetCamera();
		ui->qvtkWidget->GetRenderWindow()->Render();
		updateSegmentTree(simRow);
	} else {
		noSelectWarning();
	}
}

void UserGui::updateSegmentTree(double *values)
{
	auto segtree = this->ui->segmentsTree;
	for (int i = 0; i < segtree->topLevelItemCount(); i++)
	{
		auto rootItem = this->ui->segmentsTree->topLevelItem(i);
		auto idx = rootItem->text(i).toInt();
		rootItem->setText(1, QString::number(values[idx]));
		// All children are always same
		//double *featArr = sups[idx].getFeatureArray();
		//for (auto j = 0; j < rootItem->childCount(); j++)
		//{
		//	rootItem->child(j)->setText(1, QString::number(featArr[j]));
		//}
	}
}
