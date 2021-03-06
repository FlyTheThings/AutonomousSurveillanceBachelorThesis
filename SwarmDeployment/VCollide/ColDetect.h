#pragma once
#include "Triangle3D.h"
#include "Rectangle2D.h"
#include "VCollide.H"

class ColDetect
{
public:
	static bool coldetect(Triangle3D tri1, Triangle3D tri2);
	static bool coldetect(Rectangle2D tri1, Rectangle2D tri2);
	static bool coldetectWithoutTransformation(Rectangle2D rect1, Rectangle2D rect2);	//a little bit faster than coldetect
	static bool coldetect(Triangle3D tri1, Triangle3D tri2, double *trans1, double *trans2);
	ColDetect();
	virtual ~ColDetect();

protected:
	static void addTris(double* tris, int n, VCollide& vc);
	static void addPolygon(std::vector<Triangle3D> triangles, VCollide& vc);
	static bool coldetect(int ntri1, int ntri2, int ntrans1, int ntrans2, double* tri1, double* tri2, double* trans1, double* trans2);
	static void update(double *tr, int t, int ntrans, double vc_trans[][4]);
};

