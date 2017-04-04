#pragma once
#include <string>
#include <vector>
#include <map>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPolyData.h>

class Superpixel {
	
	vtkSmartPointer<vtkPoints> m_points;
	vtkSmartPointer<vtkUnsignedCharArray> m_colors;
	vtkSmartPointer<vtkPolyData> m_contour = nullptr;

public:
	int m_id;
	double red, green, blue, depth, nX, nY, nZ;

	Superpixel();
	virtual ~Superpixel();
	double * getFeatureArray() const;

	static std::map<int, Superpixel> loadSuperpixels(std::string image, std::string indexes, std::string depth, std::string features);

	vtkSmartPointer<vtkPolyData> getSuperpixel() const;
	vtkSmartPointer<vtkPolyData> getContour();
	vtkSmartPointer<vtkPolyData> getSurface() const;
	double* getCentroid() const;
	double cosineSimilarity(Superpixel s2);
	void setHeight(double h);
};
