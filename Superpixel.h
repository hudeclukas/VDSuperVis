#pragma once
#include <string>
#include <map>
#include <vtkSmartPointer.h>

class vtkActor;
class vtkPoints;
class vtkBillboardTextActor3D;
class vtkPolyData;
class vtkUnsignedCharArray;

class Superpixel {
	
	vtkSmartPointer<vtkPoints> m_points;
	vtkSmartPointer<vtkUnsignedCharArray> m_colors;
	vtkSmartPointer<vtkPolyData> m_contour = nullptr;

	bool m_hide = false;
	bool m_hideContour = false;
	bool m_hideLabel = false;

public:
	int m_id;
	bool m_border;
	double red, green, blue, depth, nX, nY, nZ;

	static int min_id;
	static int max_id;

	vtkSmartPointer<vtkActor> segment = nullptr;
	vtkSmartPointer<vtkActor> contour = nullptr;
	vtkSmartPointer<vtkBillboardTextActor3D> label = nullptr;


	Superpixel();
	virtual ~Superpixel();
	static std::map<int, Superpixel> loadSuperpixels(std::string image, std::string indexes, std::string depth, std::string features);

	// data methods
	double * getFeatureArray() const;
	std::string * getFeatureNamesArray() const;
	double cosineSimilarity(Superpixel s2);

	// visualization methods
	vtkSmartPointer<vtkPolyData> getSuperpixel() const;
	vtkSmartPointer<vtkPolyData> getContour();
	vtkSmartPointer<vtkPolyData> getSurface() const;
	double* getCentroid() const;
	void setHeight(double h);

	void setVisibility(bool v);
	void setContourVisibility(bool v);
	void setLabelVisibility(bool v);
	void show();
	void hide();
	void showContour();
	void hideContour();
	void showLabel();
	void hideLabel();
};
