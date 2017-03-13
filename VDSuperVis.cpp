#include <vtkSmartPointer.h>
#include <vtkImageViewer2.h>
#include <vtkPNGReader.h>
#include <vtkJPEGReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkPointData.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkCamera.h>

int main(int argc, char* argv[]) {
    argc = 4;
    std::string rgbPath = "D:/RGBD datasets/VD/000002_2014-05-26_14-23-37_260595134347_rgbf000103-resize/image/0000103.jpg";
	std::string supPath = "D:/RGBD datasets/VD/000002_2014-05-26_14-23-37_260595134347_rgbf000103-resize/image/superpixels.png";
	std::string feaPath = "D:/RGBD datasets/VD/000002_2014-05-26_14-23-37_260595134347_rgbf000103-resize/image/features.txt";
    // Verify input arguments
    if (argc < 2) {
        std::cout << "Usage: " << argv[0]
            << " Filename(.png)" << std::endl;
        return EXIT_FAILURE;
    }

    // Read the image
    vtkSmartPointer<vtkJPEGReader> jpgReader = vtkSmartPointer<vtkJPEGReader>::New();
    vtkSmartPointer<vtkPNGReader> pngReader = vtkSmartPointer<vtkPNGReader>::New();
    if (!jpgReader->CanReadFile(rgbPath.c_str())) {
        std::cerr << "Error reading file " << rgbPath << std::endl;
        return EXIT_FAILURE;
    }
    if (!pngReader->CanReadFile(supPath.c_str())) {
        std::cerr << "Error reading file " << supPath << std::endl;
        return EXIT_FAILURE;
    }
    jpgReader->SetFileName(rgbPath.c_str());
    jpgReader->Update();
    pngReader->SetFileName(supPath.c_str());
    pngReader->Update();

    vtkSmartPointer<vtkImageData> imageData;
    imageData = jpgReader->GetOutput();
    vtkSmartPointer<vtkImageData> superpixels;
    superpixels = pngReader->GetOutput();
    auto dim = imageData->GetDimensions();
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

	unsigned char * pixels = static_cast<unsigned char*>(imageData->GetScalarPointer(0, 0, 0));
	unsigned short * indexes = static_cast<unsigned short*>(superpixels->GetScalarPointer(0, 0, 0));
	vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
	colors->SetNumberOfComponents(3);
	
	for (auto z = 0; z < dim[2]; ++z)
	{
		for (auto y = 0; y < dim[1]; ++y) 
		{
			for (auto x = 0; x < dim[0]; ++x) 
			{
				unsigned char color[3];
				color[0] = *pixels++;
				color[1] = *pixels++;
				color[2] = *pixels++;
				double idx = *indexes++;
				points->InsertNextPoint(x, y, idx);
				colors->SetName("Image");
				colors->InsertNextTypedTuple(color);
			}
		}
	}
	vtkSmartPointer<vtkPolyData> pointsPolyData = vtkSmartPointer<vtkPolyData>::New();
	pointsPolyData->SetPoints(points);
	vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
	vertexFilter->SetInputData(pointsPolyData);
	vertexFilter->Update();
	vtkSmartPointer<vtkPolyData> imageInSpace = vtkSmartPointer<vtkPolyData>::New();
	imageInSpace->ShallowCopy(vertexFilter->GetOutput());
	imageInSpace->GetPointData()->SetScalars(colors);

	// Create a mapper and actor
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(imageInSpace);
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetPointSize(2);
	// Create a renderer, render window, and interactor
	vtkSmartPointer<vtkRenderer> renderer3D = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindow3D = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow3D->AddRenderer(renderer3D);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor3D = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor3D->SetRenderWindow(renderWindow3D);
	// Add the actor to the scene
	renderer3D->AddActor(actor);
	renderer3D->SetBackground(0.2, 0.2, 0.2); 
	renderer3D->GetActiveCamera()->ParallelProjectionOn();

	// Render and interact
	renderWindow3D->Render();

	renderWindow3D->Start();
	renderWindowInteractor3D->Start();

    return EXIT_SUCCESS;
}



//#include <vtkPointSource.h>
//#include <vtkPolyData.h>
//#include <vtkSmartPointer.h>
//#include <vtkPolyDataMapper.h>
//#include <vtkActor.h>
//#include <vtkRenderWindow.h>
//#include <vtkRenderer.h>
//#include <vtkRenderWindowInteractor.h>
//#include <vtkProperty.h>
//
//int main(int, char *[]) {
//    // Create a point cloud
//    vtkSmartPointer<vtkPointSource> pointSource =
//        vtkSmartPointer<vtkPointSource>::New();
//    pointSource->SetCenter(0.0, 0.0, 0.0);
//    pointSource->SetNumberOfPoints(50);
//    pointSource->SetRadius(5.0);
//    pointSource->Update();
//
//    // Create a mapper and actor
//    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//    mapper->SetInputConnection(pointSource->GetOutputPort());
//
//    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
//    actor->SetMapper(mapper);
//    actor->GetProperty()->SetPointSize(3);
//
//    // Create a renderer, render window, and interactor
//    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
//    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
//    renderWindow->AddRenderer(renderer);
//    
//    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
//    renderWindowInteractor->SetRenderWindow(renderWindow);
//
//    // Add the actor to the scene
//    renderer->AddActor(actor);
//    renderer->SetBackground(.3, .6, .3); // Background color green
//                                         // Render and interact
//    renderWindow->Render();
//    renderWindowInteractor->Start();
//
//    return EXIT_SUCCESS;
//}
//
//#include <vtkVersion.h>
//#include <vtkSmartPointer.h>
//#include <vtkPointData.h>
//#include <vtkIdTypeArray.h>
//#include <vtkDataSetSurfaceFilter.h>
//#include <vtkRendererCollection.h>
//#include <vtkProperty.h>
//#include <vtkPlanes.h>
//#include <vtkObjectFactory.h>
//#include <vtkPolyDataMapper.h>
//#include <vtkActor.h>
//#include <vtkRenderWindow.h>
//#include <vtkRenderer.h>
//#include <vtkRenderWindowInteractor.h>
//#include <vtkPolyData.h>
//#include <vtkPointSource.h>
//#include <vtkInteractorStyleRubberBandPick.h>
//#include <vtkAreaPicker.h>
//#include <vtkExtractGeometry.h>
//#include <vtkDataSetMapper.h>
//#include <vtkUnstructuredGrid.h>
//#include <vtkVertexGlyphFilter.h>
//#include <vtkIdFilter.h>
//
//// Define interaction style
//class InteractorStyle : public vtkInteractorStyleRubberBandPick {
//public:
//    static InteractorStyle* New();
//    vtkTypeMacro(InteractorStyle, vtkInteractorStyleRubberBandPick);
//
//    InteractorStyle() {
//        this->SelectedMapper = vtkSmartPointer<vtkDataSetMapper>::New();
//        this->SelectedActor = vtkSmartPointer<vtkActor>::New();
//        this->SelectedActor->SetMapper(SelectedMapper);
//    }
//
//    virtual void OnLeftButtonUp() {
//        // Forward events
//        vtkInteractorStyleRubberBandPick::OnLeftButtonUp();
//
//        vtkPlanes* frustum = static_cast<vtkAreaPicker*>(this->GetInteractor()->GetPicker())->GetFrustum();
//
//        vtkSmartPointer<vtkExtractGeometry> extractGeometry =
//            vtkSmartPointer<vtkExtractGeometry>::New();
//        extractGeometry->SetImplicitFunction(frustum);
//#if VTK_MAJOR_VERSION <= 5
//        extractGeometry->SetInput(this->Points);
//#else
//        extractGeometry->SetInputData(this->Points);
//#endif
//        extractGeometry->Update();
//
//        vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter =
//            vtkSmartPointer<vtkVertexGlyphFilter>::New();
//        glyphFilter->SetInputConnection(extractGeometry->GetOutputPort());
//        glyphFilter->Update();
//
//        vtkPolyData* selected = glyphFilter->GetOutput();
//        std::cout << "Selected " << selected->GetNumberOfPoints() << " points." << std::endl;
//        std::cout << "Selected " << selected->GetNumberOfCells() << " cells." << std::endl;
//#if VTK_MAJOR_VERSION <= 5
//        this->SelectedMapper->SetInput(selected);
//#else
//        this->SelectedMapper->SetInputData(selected);
//#endif
//        this->SelectedMapper->ScalarVisibilityOff();
//
//        vtkIdTypeArray* ids = vtkIdTypeArray::SafeDownCast(selected->GetPointData()->GetArray("OriginalIds"));
//        for (vtkIdType i = 0; i < ids->GetNumberOfTuples(); i++) {
//            std::cout << "Id " << i << " : " << ids->GetValue(i) << std::endl;
//        }
//
//        this->SelectedActor->GetProperty()->SetColor(1.0, 0.0, 0.0); //(R,G,B)
//        this->SelectedActor->GetProperty()->SetPointSize(3);
//
//        this->CurrentRenderer->AddActor(SelectedActor);
//        this->GetInteractor()->GetRenderWindow()->Render();
//        this->HighlightProp(NULL);
//    }
//
//    void SetPoints(vtkSmartPointer<vtkPolyData> points) { this->Points = points; }
//private:
//    vtkSmartPointer<vtkPolyData> Points;
//    vtkSmartPointer<vtkActor> SelectedActor;
//    vtkSmartPointer<vtkDataSetMapper> SelectedMapper;
//
//};
//vtkStandardNewMacro(InteractorStyle);
//
//int main(int, char *[]) {
//    vtkSmartPointer<vtkPointSource> pointSource =
//        vtkSmartPointer<vtkPointSource>::New();
//    pointSource->SetNumberOfPoints(20);
//    pointSource->Update();
//
//    vtkSmartPointer<vtkIdFilter> idFilter =
//        vtkSmartPointer<vtkIdFilter>::New();
//    idFilter->SetInputConnection(pointSource->GetOutputPort());
//    idFilter->SetIdsArrayName("OriginalIds");
//    idFilter->Update();
//
//    vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter =
//        vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
//    surfaceFilter->SetInputConnection(idFilter->GetOutputPort());
//    surfaceFilter->Update();
//
//    vtkPolyData* input = surfaceFilter->GetOutput();
//
//    // Create a mapper and actor
//    vtkSmartPointer<vtkPolyDataMapper> mapper =
//        vtkSmartPointer<vtkPolyDataMapper>::New();
//#if VTK_MAJOR_VERSION <= 5
//    mapper->SetInputConnection(input->GetProducerPort());
//#else
//    mapper->SetInputData(input);
//#endif
//    mapper->ScalarVisibilityOff();
//
//    vtkSmartPointer<vtkActor> actor =
//        vtkSmartPointer<vtkActor>::New();
//    actor->SetMapper(mapper);
//    actor->GetProperty()->SetPointSize(5);
//
//    // Visualize
//    vtkSmartPointer<vtkRenderer> renderer =
//        vtkSmartPointer<vtkRenderer>::New();
//    vtkSmartPointer<vtkRenderWindow> renderWindow =
//        vtkSmartPointer<vtkRenderWindow>::New();
//    renderWindow->AddRenderer(renderer);
//
//    vtkSmartPointer<vtkAreaPicker> areaPicker =
//        vtkSmartPointer<vtkAreaPicker>::New();
//    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
//        vtkSmartPointer<vtkRenderWindowInteractor>::New();
//    renderWindowInteractor->SetPicker(areaPicker);
//    renderWindowInteractor->SetRenderWindow(renderWindow);
//
//    renderer->AddActor(actor);
//    //renderer->SetBackground(1,1,1); // Background color white
//
//    renderWindow->Render();
//
//    vtkSmartPointer<InteractorStyle> style =
//        vtkSmartPointer<InteractorStyle>::New();
//    style->SetPoints(input);
//    renderWindowInteractor->SetInteractorStyle(style);
//
//    renderWindowInteractor->Start();
//
//    return EXIT_SUCCESS;
//}
