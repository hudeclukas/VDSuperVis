#include "Superpixel.h"
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include <opencv2/core/core.hpp>

#include <vtkPNGReader.h>
#include <vtkJPEGReader.h>
#include <vtkImageData.h>
#include <vtkTransform.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkPolyDataMapper.h>
#include <vtkWindowToImageFilter.h>
#include <vtkContourFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>

#include <opencv2/imgcodecs.hpp>
#include <vtkDelaunay2D.h>
#include <vtkVectorText.h>

Superpixel::Superpixel()
{
	m_colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
	m_colors->SetNumberOfComponents(3);
	m_points = vtkSmartPointer<vtkPoints>::New();
}

Superpixel::~Superpixel() 
{
}

double* Superpixel::getFeatureArray() const
{
	double *arr = new double[7];
	arr[0] = blue;
	arr[1] = green;
	arr[2] = red;
	arr[3] = depth;
	arr[4] = nX;
	arr[5] = nY;
	arr[6] = nZ;
	return arr;
}

std::string* Superpixel::getFeatureNamesArray() const
{
	std::string *arr = new std::string[7];
	arr[0] = "blue";
	arr[1] = "green";
	arr[2] = "red";
	arr[3] = "depth";
	arr[4] = "nX";
	arr[5] = "nY";
	arr[6] = "nZ";
	return arr;
}

std::map<int, Superpixel> Superpixel::loadSuperpixels(std::string image, std::string indexes, std::string depth, std::string features) 
{
	// Read the image
	vtkSmartPointer<vtkJPEGReader> colorReader = vtkSmartPointer<vtkJPEGReader>::New();
	vtkSmartPointer<vtkPNGReader> indexesReader = vtkSmartPointer<vtkPNGReader>::New();
	if (!colorReader->CanReadFile(image.c_str())) {
		std::cerr << "Error reading file " << image << std::endl;
		return std::map<int, Superpixel>();
	}
	if (!indexesReader->CanReadFile(indexes.c_str())) {
		std::cerr << "Error reading file " << indexes << std::endl;
		return std::map<int, Superpixel>();
	}
	std::map<int, Superpixel> superpixels;

	colorReader->SetFileName(image.c_str());
	colorReader->Update();
	indexesReader->SetFileName(indexes.c_str());
	indexesReader->Update();
	vtkSmartPointer<vtkImageData> colorData;
	colorData = colorReader->GetOutput();
	vtkSmartPointer<vtkImageData> indexesData;
	indexesData = indexesReader->GetOutput();
	auto dim = colorData->GetDimensions();

	std::ifstream featuresFile(features);

	cv::Mat depthMap = cv::imread(depth, cv::IMREAD_UNCHANGED);
	double min, max;
	cv::minMaxLoc(depthMap, &min, &max);
	
	while(!featuresFile.eof())
	{
		Superpixel sup;
		featuresFile >> sup.m_id >> sup.blue >> sup.green >> sup.red >> sup.depth >> sup.nX >> sup.nY >> sup.nZ;
		sup.blue /= 255;
		sup.green /= 255;
		sup.red /= 255;
		sup.depth /= max;
		sup.m_colors->SetName("sup_" + sup.m_id);
		superpixels[sup.m_id] = sup;
	}

	unsigned char * pixels = static_cast<unsigned char*>(colorData->GetScalarPointer(0, 0, 0));
	unsigned short * indices = static_cast<unsigned short*>(indexesData->GetScalarPointer(0, 0, 0));
	for (auto z = 0; z < dim[2]; ++z) {
		for (auto y = 0; y < dim[1]; ++y) {
			for (auto x = 0; x < dim[0]; ++x) {
				unsigned char color[3];
				color[0] = *pixels++;
				color[1] = *pixels++;
				color[2] = *pixels++;
				double idx = *indices++;
				superpixels[idx].m_points->InsertNextPoint(x, y, 0);
				superpixels[idx].m_colors->InsertNextTypedTuple(color);
			}
		}
	}
	return superpixels;
}

vtkSmartPointer<vtkPolyData> Superpixel::getSuperpixel() const
{
	vtkSmartPointer<vtkPolyData> pointsPolyData = vtkSmartPointer<vtkPolyData>::New();
	pointsPolyData->SetPoints(m_points);
	vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
	vertexFilter->SetInputData(pointsPolyData);
	vertexFilter->Update();
	vtkSmartPointer<vtkPolyData> superpixel = vtkSmartPointer<vtkPolyData>::New();
	superpixel->ShallowCopy(vertexFilter->GetOutput());
	superpixel->GetPointData()->SetScalars(m_colors);
	return superpixel;
}

vtkSmartPointer<vtkPolyData> Superpixel::getContour()
{
	if (m_contour != nullptr)
	{
		return m_contour;
	}

	vtkSmartPointer<vtkPolyData> data3D = getSurface();

	double bounds_data[6], center_data[3];
	data3D->GetBounds(bounds_data);
	data3D->GetCenter(center_data);

	// Black and white scene with the data in order to print the view
	vtkSmartPointer<vtkPolyDataMapper> mapper_data = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper_data->SetInputData(data3D);

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
	double trans_x, trans_y, trans_z, ratio_x, ratio_y;
	contour->GetBounds(bounds_contour);

	ratio_x = (bounds_data[1] - bounds_data[0]) / (bounds_contour[1] - bounds_contour[0]);
	ratio_y = (bounds_data[3] - bounds_data[2]) / (bounds_contour[3] - bounds_contour[2]);

	// Rescale the contour so that it shares the same bounds as the
	// input data
	vtkSmartPointer<vtkTransform>transform1 = vtkSmartPointer<vtkTransform>::New();
	transform1->Scale(ratio_x, ratio_y, 1.);

	vtkSmartPointer<vtkTransformPolyDataFilter> tfilter1 = vtkSmartPointer<vtkTransformPolyDataFilter>::New();

	tfilter1->SetInputData(contour);
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

	tfilter2->SetInputData(contour);
	tfilter2->SetTransform(transform2);
	tfilter2->Update();

	m_contour = vtkSmartPointer<vtkPolyData>::New();
	m_contour->ShallowCopy(tfilter2->GetOutput());
	
	return m_contour;
}

vtkSmartPointer<vtkPolyData> Superpixel::getSurface() const
{
	// Triangulate the grid points
	vtkSmartPointer<vtkDelaunay2D> delaunay = vtkSmartPointer<vtkDelaunay2D>::New();
	delaunay->SetInputData(getSuperpixel());
	delaunay->Update();

	vtkSmartPointer<vtkPolyData> out = vtkSmartPointer<vtkPolyData>::New();
	out->ShallowCopy(delaunay->GetOutput());

	return out;
}

double* Superpixel::getCentroid() const
{
	double *sum = new double[3];
	sum[0] = 0; sum[1] = 0; sum[2] = 0;
	vtkSmartPointer<vtkDataArray> points = m_points->GetData();
	for (vtkIdType i = 0; i < points->GetNumberOfTuples(); i++) {
		double *point = points->GetTuple(i);
		sum[0] += point[0];
		sum[1] += point[1];
		sum[2] += point[2];
	}
	sum[0] /= points->GetNumberOfTuples();
	sum[1] /= points->GetNumberOfTuples();
	sum[2] /= points->GetNumberOfTuples();
	return sum;
}

double Superpixel::cosineSimilarity(Superpixel s2)
{
	double * arr1 = this->getFeatureArray();
	double * arr2 = s2.getFeatureArray();

	double dot = 0.0, denom_a = 0.0, denom_b = 0.0;
	for (unsigned int i = 0u; i < 7; ++i) {
		dot += arr1[i] * arr2[i];
		denom_a += arr1[i] * arr1[i];
		denom_b += arr2[i] * arr2[i];
	}
	return dot / (sqrt(denom_a) * sqrt(denom_b));
}

void Superpixel::setHeight(double h)
{
	vtkSmartPointer<vtkDataArray> points = m_points->GetData();
	vtkSmartPointer<vtkPoints> newpoints = vtkSmartPointer<vtkPoints>::New();
	for (vtkIdType i = 0; i < points->GetNumberOfTuples(); i++)
	{
		double* point = points->GetTuple(i);
		point[2] = h;
		newpoints->InsertNextPoint(point);
	}
	m_points->DeepCopy(newpoints);
}
