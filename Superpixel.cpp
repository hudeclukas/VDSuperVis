#include "Superpixel.h"
#include <fstream>
#include <string>
#include <vtkPNGReader.h>
#include <vtkJPEGReader.h>
#include <vtkImageData.h>
#include <vector>
#include <map>
#include <vtkVertexGlyphFilter.h>
#include <vtkPointData.h>


Superpixel::Superpixel()
{
	m_colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
	m_colors->SetNumberOfComponents(3);
	m_points = vtkSmartPointer<vtkPoints>::New();
}

Superpixel::~Superpixel() 
{
}

std::map<int, Superpixel> Superpixel::loadSuperpixels(std::string image, std::string indexes, std::string features) 
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
	while(!featuresFile.eof())
	{
		Superpixel sup;
		featuresFile >> sup.m_id >> sup.blue >> sup.green >> sup.red >> sup.depth >> sup.nX >> sup.nY >> sup.nZ;
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
				superpixels[idx].m_points->InsertNextPoint(x, y, idx);
				superpixels[idx].m_colors->InsertNextTypedTuple(color);
			}
		}
	}
	return superpixels;
}

vtkSmartPointer<vtkPolyData> Superpixel::getSuperpixel()
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
