#include "Sample.h"

#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPolyDataMapper.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkProperty.h>
#include <vtkSphereSource.h>
#include <vtkWindowToImageFilter.h>
#include <vtkContourFilter.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkCamera.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyleRubberBandPick.h>
#include <vtkDelaunay2D.h>
#include <vtkReverseSense.h>
#include <vtkSurfaceReconstructionFilter.h>
#include <vtkVectorText.h>
#include <vtkAreaPicker.h>
#include <vtkExtractGeometry.h>
#include <vtkPointSource.h>
#include <vtkIdFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkPlanes.h>
#include <vtkRendererCollection.h>
#include <vtkUnstructuredGrid.h>
#include <vtkIdTypeArray.h>
#include <vtkTriangleFilter.h>
#include <vtkPlaneSource.h>
#include <vtkCellPicker.h>
#include <vtkSelectionNode.h>
#include <vtkSelection.h>
#include <vtkExtractSelection.h>
#include <vtkObjectFactory.h>


int showRGBPoints() {
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	points->InsertNextPoint(0.0, 0.0, 0.0);
	points->InsertNextPoint(1.0, 0.0, 0.0);
	points->InsertNextPoint(0.0, 1.0, 0.0);

	vtkSmartPointer<vtkPolyData> pointsPolydata = vtkSmartPointer<vtkPolyData>::New();

	pointsPolydata->SetPoints(points);

	vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
#if VTK_MAJOR_VERSION <= 5
	vertexFilter->SetInputConnection(pointsPolydata->GetProducerPort());
#else
	vertexFilter->SetInputData(pointsPolydata);
#endif
	vertexFilter->Update();

	vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
	polydata->ShallowCopy(vertexFilter->GetOutput());

	// Setup colors
	unsigned char red[3] = { 255, 0, 0 };
	unsigned char green[3] = { 0, 255, 0 };
	unsigned char blue[3] = { 0, 0, 255 };

	vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
	colors->SetNumberOfComponents(3);
	colors->SetName("Colors");
	colors->InsertNextTypedTuple(red);
	colors->InsertNextTypedTuple(green);
	colors->InsertNextTypedTuple(blue);

	polydata->GetPointData()->SetScalars(colors);

	// Visualization
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	mapper->SetInputData(polydata);

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetPointSize(5);

	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	renderer->AddActor(actor);

	renderWindow->Render();
	renderWindowInteractor->Start();

	return EXIT_SUCCESS;
}

int showSilhuette() {
	vtkSmartPointer<vtkPolyData> data3d;

	vtkSmartPointer<vtkPolyData> input;
	vtkSmartPointer<vtkSphereSource> sphereSource =
		vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->Update();
	input = sphereSource->GetOutput();


	vtkSmartPointer<vtkPolyData> polydata =
		vtkSmartPointer<vtkPolyData>::New();
	polydata->SetPoints(input->GetPoints());

	// Construct the surface and create isosurface.	
	vtkSmartPointer<vtkSurfaceReconstructionFilter> surf =
		vtkSmartPointer<vtkSurfaceReconstructionFilter>::New();

	surf->SetInputData(polydata);

	vtkSmartPointer<vtkContourFilter> cf =
		vtkSmartPointer<vtkContourFilter>::New();
	cf->SetInputConnection(surf->GetOutputPort());
	cf->SetValue(0, 0.0);

	// Sometimes the contouring algorithm can create a volume whose gradient
	// vector and ordering of polygon (using the right hand rule) are
	// inconsistent. vtkReverseSense cures this problem.
	vtkSmartPointer<vtkReverseSense> reverse =
		vtkSmartPointer<vtkReverseSense>::New();
	reverse->SetInputConnection(cf->GetOutputPort());
	reverse->ReverseCellsOn();
	reverse->ReverseNormalsOn();
	reverse->Update();

	data3d = reverse->GetOutput();

	double bounds_data[6], center_data[3];
	data3d->GetBounds(bounds_data);
	data3d->GetCenter(center_data);

	// Black and white scene with the data in order to print the view
	vtkSmartPointer<vtkPolyDataMapper> mapper_data = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper_data->SetInputData(data3d);

	vtkSmartPointer<vtkActor> actor_data = vtkSmartPointer<vtkActor>::New();
	actor_data->SetMapper(mapper_data);
	actor_data->GetProperty()->SetColor(0, 0, 0);

	vtkSmartPointer<vtkRenderer> tmp_rend = vtkSmartPointer<vtkRenderer>::New();
	tmp_rend->SetBackground(1, 1, 1);

	tmp_rend->AddActor(actor_data);
	tmp_rend->ResetCamera();
	tmp_rend->GetActiveCamera()->SetParallelProjection(1);

	vtkSmartPointer<vtkRenderWindow> tmp_rW = vtkSmartPointer<vtkRenderWindow>::New();
	tmp_rW->SetOffScreenRendering(1);
	tmp_rW->AddRenderer(tmp_rend);

	tmp_rW->Render();

	// Get a print of the window
	vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
	windowToImageFilter->SetInput(tmp_rW);
	windowToImageFilter->SetMagnification(2); //image quality
	windowToImageFilter->Update();

	// Extract the silhouette corresponding to the black limit of the image
	vtkSmartPointer<vtkContourFilter> ContFilter = vtkSmartPointer<vtkContourFilter>::New();
	ContFilter->SetInputConnection(windowToImageFilter->GetOutputPort());
	ContFilter->SetValue(0, 255);
	ContFilter->Update();

	// Make the contour coincide with the data.
	vtkSmartPointer<vtkPolyData> contour = ContFilter->GetOutput();

	double bounds_contour[6], center_contour[3];
	double trans_x = 0., trans_y = 0., trans_z = 0., ratio_x = 0., ratio_y = 0.;
	contour->GetBounds(bounds_contour);

	ratio_x = (bounds_data[1] - bounds_data[0]) / (bounds_contour[1] - bounds_contour[0]);
	ratio_y = (bounds_data[3] - bounds_data[2]) / (bounds_contour[3] - bounds_contour[2]);

	// Rescale the contour so that it shares the same bounds as the
	// input data
	vtkSmartPointer<vtkTransform>transform1 = vtkSmartPointer<vtkTransform>::New();
	transform1->Scale(ratio_x, ratio_y, 1.);

	vtkSmartPointer<vtkTransformPolyDataFilter> tfilter1 = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
#if VTK_MAJOR_VERSION <= 5
	tfilter1->SetInput(contour);
#else
	tfilter1->SetInputData(contour);
#endif
	tfilter1->SetTransform(transform1);
	tfilter1->Update();

	contour = tfilter1->GetOutput();

	// Translate the contour so that it shares the same center as the
	// input data
	contour->GetCenter(center_contour);
	trans_x = center_data[0] - center_contour[0];
	trans_y = center_data[1] - center_contour[1];
	trans_z = center_data[2] - center_contour[2];

	vtkSmartPointer<vtkTransform> transform2 = vtkSmartPointer<vtkTransform>::New();
	transform2->Translate(trans_x, trans_y, trans_z);

	vtkSmartPointer<vtkTransformPolyDataFilter> tfilter2 = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
#if VTK_MAJOR_VERSION <= 5
	tfilter2->SetInput(contour);
#else
	tfilter2->SetInputData(contour);
#endif
	tfilter2->SetTransform(transform2);
	tfilter2->Update();

	contour = tfilter2->GetOutput();

	// Render the result : Input data + resulting silhouette

	// Updating the color of the data
	actor_data->GetProperty()->SetColor(0.9, 0.9, 0.8);

	// Create a mapper and actor of the silhouette
	vtkSmartPointer<vtkPolyDataMapper> mapper_contour =	vtkSmartPointer<vtkPolyDataMapper>::New();
#if VTK_MAJOR_VERSION <= 5
	mapper_contour->SetInput(contour);
#else
	mapper_contour->SetInputData(contour);
#endif

	vtkSmartPointer<vtkActor> actor_contour = vtkSmartPointer<vtkActor>::New();
	actor_contour->SetMapper(mapper_contour);
	actor_contour->GetProperty()->SetLineWidth(2.);

	// 2 renderers and a render window
	vtkSmartPointer<vtkRenderer> renderer1 = vtkSmartPointer<vtkRenderer>::New();
	renderer1->AddActor(actor_data);

	vtkSmartPointer<vtkRenderer> renderer2 = vtkSmartPointer<vtkRenderer>::New();
	renderer2->AddActor(actor_contour);

	vtkSmartPointer<vtkRenderWindow> renderwindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderwindow->SetSize(400, 400);

	renderwindow->AddRenderer(renderer1);
	renderer1->SetViewport(0., 0., 0.5, 1.);

	renderwindow->AddRenderer(renderer2);
	renderer2->SetViewport(0.5, 0., 1., 1.);

	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();

	iren->SetRenderWindow(renderwindow);
	iren->SetInteractorStyle(style);

	renderwindow->Render();
	iren->Start();

	return EXIT_SUCCESS;
}

int computeSurface()
{
	vtkSmartPointer<vtkPolyData> input;
	vtkSmartPointer<vtkSphereSource> sphereSource =
		vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->Update();
	input = sphereSource->GetOutput();
	

	vtkSmartPointer<vtkPolyData> polydata =
		vtkSmartPointer<vtkPolyData>::New();
	polydata->SetPoints(input->GetPoints());

	// Construct the surface and create isosurface.	
	vtkSmartPointer<vtkSurfaceReconstructionFilter> surf =
		vtkSmartPointer<vtkSurfaceReconstructionFilter>::New();

	surf->SetInputData(polydata);

	vtkSmartPointer<vtkContourFilter> cf =
		vtkSmartPointer<vtkContourFilter>::New();
	cf->SetInputConnection(surf->GetOutputPort());
	cf->SetValue(0, 0.0);

	// Sometimes the contouring algorithm can create a volume whose gradient
	// vector and ordering of polygon (using the right hand rule) are
	// inconsistent. vtkReverseSense cures this problem.
	vtkSmartPointer<vtkReverseSense> reverse =
		vtkSmartPointer<vtkReverseSense>::New();
	reverse->SetInputConnection(cf->GetOutputPort());
	reverse->ReverseCellsOn();
	reverse->ReverseNormalsOn();

	vtkSmartPointer<vtkPolyDataMapper> map =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	map->SetInputConnection(reverse->GetOutputPort());
	map->ScalarVisibilityOff();

	vtkSmartPointer<vtkActor> surfaceActor =
		vtkSmartPointer<vtkActor>::New();
	surfaceActor->SetMapper(map);

	// Create the RenderWindow, Renderer and both Actors
	vtkSmartPointer<vtkRenderer> ren =
		vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renWin =
		vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(ren);
	vtkSmartPointer<vtkRenderWindowInteractor> iren =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);

	// Add the actors to the renderer, set the background and size
	ren->AddActor(surfaceActor);
	ren->SetBackground(.2, .3, .4);

	renWin->Render();
	iren->Start();

	return EXIT_SUCCESS;

}

int triangulateSurface()
{
	// Create points on an XY grid with random Z coordinate
	vtkSmartPointer<vtkPoints> points =
		vtkSmartPointer<vtkPoints>::New();

	unsigned int gridSize = 10;
	for (unsigned int x = 0; x < gridSize; x++) {
		for (unsigned int y = 0; y < gridSize; y++) {
			points->InsertNextPoint(x, y, vtkMath::Random(0.0, 3.0));
		}
	}

	// Add the grid points to a polydata object
	vtkSmartPointer<vtkPolyData> polydata =
		vtkSmartPointer<vtkPolyData>::New();
	polydata->SetPoints(points);

	vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter =
		vtkSmartPointer<vtkVertexGlyphFilter>::New();
	glyphFilter->SetInputData(polydata);
	glyphFilter->Update();

	// Create a mapper and actor
	vtkSmartPointer<vtkPolyDataMapper> pointsMapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	pointsMapper->SetInputConnection(glyphFilter->GetOutputPort());

	vtkSmartPointer<vtkActor> pointsActor =
		vtkSmartPointer<vtkActor>::New();
	pointsActor->SetMapper(pointsMapper);
	pointsActor->GetProperty()->SetPointSize(3);
	pointsActor->GetProperty()->SetColor(1, 0, 0);
	// Triangulate the grid points
	vtkSmartPointer<vtkDelaunay2D> delaunay =
		vtkSmartPointer<vtkDelaunay2D>::New();
	delaunay->SetInputData(polydata);
	delaunay->Update();

	// Create a mapper and actor
	vtkSmartPointer<vtkPolyDataMapper> triangulatedMapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	triangulatedMapper->SetInputConnection(delaunay->GetOutputPort());

	vtkSmartPointer<vtkActor> triangulatedActor =
		vtkSmartPointer<vtkActor>::New();
	triangulatedActor->SetMapper(triangulatedMapper);

	// Create a renderer, render window, and interactor
	vtkSmartPointer<vtkRenderer> renderer =
		vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindow =
		vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	// Add the actor to the scene
	renderer->AddActor(pointsActor);
	renderer->AddActor(triangulatedActor);
	renderer->SetBackground(.3, .6, .3); // Background color green

										 // Render and interact
	renderWindow->Render();
	renderWindowInteractor->Start();

	return EXIT_SUCCESS;
}

int drawText()
{
	// Create text
	vtkSmartPointer<vtkVectorText> textSource = vtkSmartPointer<vtkVectorText>::New();
	textSource->SetText("Hello");
	textSource->Update();

	// Create a mapper and actor
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(textSource->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(1.0, 0.0, 0.0);

	// Create a renderer, render window, and interactor
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	// Add the actor to the scene
	renderer->AddActor(actor);
	renderer->SetBackground(1, 1, 1); // Background color white

									  // Render and interact
	renderWindow->Render();
	renderWindowInteractor->Start();

	return EXIT_SUCCESS;
}

// Define interaction style
class InteractorStyle : public vtkInteractorStyleRubberBandPick {
public:
	static InteractorStyle* New();
	vtkTypeMacro(InteractorStyle, vtkInteractorStyleRubberBandPick);

	InteractorStyle() {
		this->SelectedMapper = vtkSmartPointer<vtkDataSetMapper>::New();
		this->SelectedActor = vtkSmartPointer<vtkActor>::New();
		this->SelectedActor->SetMapper(SelectedMapper);
	}

	virtual void OnLeftButtonUp() {
		// Forward events
		vtkInteractorStyleRubberBandPick::OnLeftButtonUp();

		vtkPlanes* frustum = static_cast<vtkAreaPicker*>(this->GetInteractor()->GetPicker())->GetFrustum();

		vtkSmartPointer<vtkExtractGeometry> extractGeometry = vtkSmartPointer<vtkExtractGeometry>::New();
		extractGeometry->SetImplicitFunction(frustum);
		extractGeometry->SetInputData(this->Points);

		extractGeometry->Update();

		vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
		glyphFilter->SetInputConnection(extractGeometry->GetOutputPort());
		glyphFilter->Update();

		vtkPolyData* selected = glyphFilter->GetOutput();
		std::cout << "Selected " << selected->GetNumberOfPoints() << " points." << std::endl;
		std::cout << "Selected " << selected->GetNumberOfCells() << " cells." << std::endl;

		this->SelectedMapper->SetInputData(selected);
		this->SelectedMapper->ScalarVisibilityOff();

		vtkIdTypeArray* ids = vtkIdTypeArray::SafeDownCast(selected->GetPointData()->GetArray("OriginalIds"));
		for (vtkIdType i = 0; i < ids->GetNumberOfTuples(); i++) {
			std::cout << "Id " << i << " : " << ids->GetValue(i) << std::endl;
		}

		this->SelectedActor->GetProperty()->SetColor(1.0, 0.0, 0.0); //(R,G,B)
		this->SelectedActor->GetProperty()->SetPointSize(3);

		this->CurrentRenderer->AddActor(SelectedActor);
		this->GetInteractor()->GetRenderWindow()->Render();
		this->HighlightProp(NULL);
	}

	void SetPoints(vtkSmartPointer<vtkPolyData> points) { this->Points = points; }
private:
	vtkSmartPointer<vtkPolyData> Points;
	vtkSmartPointer<vtkActor> SelectedActor;
	vtkSmartPointer<vtkDataSetMapper> SelectedMapper;

};
vtkStandardNewMacro(InteractorStyle);

int selectObjects() {
	vtkSmartPointer<vtkPointSource> pointSource = vtkSmartPointer<vtkPointSource>::New();
	pointSource->SetNumberOfPoints(20);
	pointSource->Update();

	vtkSmartPointer<vtkIdFilter> idFilter = vtkSmartPointer<vtkIdFilter>::New();
	idFilter->SetInputConnection(pointSource->GetOutputPort());
	idFilter->SetIdsArrayName("OriginalIds");
	idFilter->Update();

	vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
	surfaceFilter->SetInputConnection(idFilter->GetOutputPort());
	surfaceFilter->Update();

	vtkPolyData* input = surfaceFilter->GetOutput();

	// Create a mapper and actor
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(input);

	mapper->ScalarVisibilityOff();

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	// Visualize 
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);

	vtkSmartPointer<vtkAreaPicker> areaPicker = vtkSmartPointer<vtkAreaPicker>::New();
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetPicker(areaPicker);
	renderWindowInteractor->SetRenderWindow(renderWindow);

	renderer->AddActor(actor);
	//renderer->SetBackground(1,1,1); // Background color white

	renderWindow->Render();

	vtkSmartPointer<InteractorStyle> style = vtkSmartPointer<InteractorStyle>::New();
	style->SetPoints(input);
	renderWindowInteractor->SetInteractorStyle(style);

	renderWindowInteractor->Start();

	return EXIT_SUCCESS;
}

// Catch mouse events
class MouseInteractorStyle2 : public vtkInteractorStyleTrackballCamera {
public:
	static MouseInteractorStyle2* New();

	MouseInteractorStyle2() {
		selectedMapper = vtkSmartPointer<vtkDataSetMapper>::New();
		selectedActor = vtkSmartPointer<vtkActor>::New();
	}

	virtual void OnLeftButtonDown() {
		// Get the location of the click (in window coordinates)
		int* pos = this->GetInteractor()->GetEventPosition();

		vtkSmartPointer<vtkCellPicker> picker =
			vtkSmartPointer<vtkCellPicker>::New();
		picker->SetTolerance(0.0005);

		// Pick from this location.
		picker->Pick(pos[0], pos[1], 0, this->GetDefaultRenderer());

		double* worldPosition = picker->GetPickPosition();
		std::cout << "Cell id is: " << picker->GetCellId() << std::endl;

		if (picker->GetCellId() != -1) {

			std::cout << "Pick position is: " << worldPosition[0] << " " << worldPosition[1]
				<< " " << worldPosition[2] << endl;

			vtkSmartPointer<vtkIdTypeArray> ids =
				vtkSmartPointer<vtkIdTypeArray>::New();
			ids->SetNumberOfComponents(1);
			ids->InsertNextValue(picker->GetCellId());

			vtkSmartPointer<vtkSelectionNode> selectionNode =
				vtkSmartPointer<vtkSelectionNode>::New();
			selectionNode->SetFieldType(vtkSelectionNode::CELL);
			selectionNode->SetContentType(vtkSelectionNode::INDICES);
			selectionNode->SetSelectionList(ids);

			vtkSmartPointer<vtkSelection> selection =
				vtkSmartPointer<vtkSelection>::New();
			selection->AddNode(selectionNode);

			vtkSmartPointer<vtkExtractSelection> extractSelection =
				vtkSmartPointer<vtkExtractSelection>::New();
#if VTK_MAJOR_VERSION <= 5
			extractSelection->SetInput(0, this->Data);
			extractSelection->SetInput(1, selection);
#else
			extractSelection->SetInputData(0, this->Data);
			extractSelection->SetInputData(1, selection);
#endif
			extractSelection->Update();

			// In selection
			vtkSmartPointer<vtkUnstructuredGrid> selected =
				vtkSmartPointer<vtkUnstructuredGrid>::New();
			selected->ShallowCopy(extractSelection->GetOutput());

			std::cout << "There are " << selected->GetNumberOfPoints()
				<< " points in the selection." << std::endl;
			std::cout << "There are " << selected->GetNumberOfCells()
				<< " cells in the selection." << std::endl;


#if VTK_MAJOR_VERSION <= 5
			selectedMapper->SetInputConnection(
				selected->GetProducerPort());
#else
			selectedMapper->SetInputData(selected);
#endif

			selectedActor->SetMapper(selectedMapper);
			selectedActor->GetProperty()->EdgeVisibilityOn();
			selectedActor->GetProperty()->SetEdgeColor(1, 0, 0);
			selectedActor->GetProperty()->SetLineWidth(3);

			this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(selectedActor);

		}
		// Forward events
		vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
	}

	vtkSmartPointer<vtkPolyData> Data;
	vtkSmartPointer<vtkDataSetMapper> selectedMapper;
	vtkSmartPointer<vtkActor> selectedActor;

};

vtkStandardNewMacro(MouseInteractorStyle2);

int selectActors() {
	vtkSmartPointer<vtkPlaneSource> planeSource =
		vtkSmartPointer<vtkPlaneSource>::New();
	planeSource->Update();

	vtkSmartPointer<vtkTriangleFilter> triangleFilter =
		vtkSmartPointer<vtkTriangleFilter>::New();
	triangleFilter->SetInputConnection(planeSource->GetOutputPort());
	triangleFilter->Update();

	vtkSmartPointer<vtkPolyDataMapper> mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(triangleFilter->GetOutputPort());

	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	actor->GetProperty()->SetColor(0, 1, 0); //green
	actor->SetMapper(mapper);

	vtkSmartPointer<vtkRenderer> renderer =
		vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindow =
		vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);

	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);
	renderWindowInteractor->Initialize();

	// Set the custom stype to use for interaction.
	vtkSmartPointer<MouseInteractorStyle2> style =
		vtkSmartPointer<MouseInteractorStyle2>::New();
	style->SetDefaultRenderer(renderer);
	style->Data = triangleFilter->GetOutput();

	renderWindowInteractor->SetInteractorStyle(style);

	renderer->AddActor(actor);
	renderer->ResetCamera();

	renderer->SetBackground(0, 0, 1); // Blue

	renderWindow->Render();
	renderWindowInteractor->Start();

	return EXIT_SUCCESS;
}
