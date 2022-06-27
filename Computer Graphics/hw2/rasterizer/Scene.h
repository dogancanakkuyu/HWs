#ifndef _SCENE_H_
#define _SCENE_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "Camera.h"
#include "Color.h"
#include "Mesh.h"
#include "Rotation.h"
#include "Scaling.h"
#include "Translation.h"
#include "Triangle.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Matrix4.h"

using namespace std;

class Scene
{
public:
    Color backgroundColor;
    bool cullingEnabled;

    vector< vector<Color> > image;
    vector< Camera* > cameras;
    vector< Vec3* > vertices;
    vector< Color* > colorsOfVertices;
    vector< Scaling* > scalings;
    vector< Rotation* > rotations;
    vector< Translation* > translations;
    vector< Mesh* > meshes;

    Scene(const char *xmlPath);

    void initializeImage(Camera* camera);
    void forwardRenderingPipeline(Camera* camera);
    int makeBetweenZeroAnd255(double value);
    void writeImageToPPMFile(Camera* camera);
    void convertPPMToPNG(string ppmFileName, int osType);

    void computeGridColor(Vec4 v0, Vec4 v1, Vec4 v2, int horRes, int verRes);

    Vec3 transformVec(Vec3 v0, vector<char> transformChars, vector<int> transformIds);

    bool visible(double den, double num, double &te, double &tl);

    bool clipping(Vec4 &v0, Vec4 &v1);

    void computeLineRasterization(Vec4 &v0, Vec4 &v1, Color &c0, Color &c1,Camera* camera);

    Vec4 persDivide(Vec4 toBeDivided);

    bool backFaceCulling(Vec4 v0, Vec4 v1, Vec4 v2);

    double f(int x, int y, int fvx, int fvy, int svx, int svy);
};

#endif
