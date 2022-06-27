#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <cmath>

#include "Scene.h"
#include "Camera.h"
#include "Color.h"
#include "Mesh.h"
#include "Rotation.h"
#include "Scaling.h"
#include "Translation.h"
#include "Triangle.h"
#include "Vec3.h"
#include "tinyxml2.h"
#include "Helpers.h"

using namespace tinyxml2;
using namespace std;

/*
    Transformations, clipping, culling, rasterization are done here.
    You may define helper functions.
*/

Vec3 Scene::transformVec(Vec3 v0,vector<char> transformChars,vector<int> transformIds){
    Vec4 V = Vec4(v0.x,v0.y,v0.z,1,v0.colorId);
    for (int i = 0; i < transformChars.size() ; i++) {
        if (transformChars.at(i) == 't'){
            Matrix4 translationM = getIdentityMatrix();
            translationM.val[0][3] = translations.at(transformIds.at(i) - 1)->tx;
            translationM.val[1][3] = translations.at(transformIds.at(i) - 1)->ty;
            translationM.val[2][3] = translations.at(transformIds.at(i) - 1)->tz;
            V = multiplyMatrixWithVec4(translationM,V);
        }
        else if (transformChars.at(i) == 's'){
            Matrix4 scalingM = getIdentityMatrix();
            scalingM.val[0][0] = scalings.at(transformIds.at(i) - 1)->sx;
            scalingM.val[1][1] = scalings.at(transformIds.at(i) - 1)->sy;
            scalingM.val[2][2] = scalings.at(transformIds.at(i) - 1)->sz;
            V = multiplyMatrixWithVec4(scalingM,V);
        } else if (transformChars.at(i) == 'r'){
            double R = rotations.at(transformIds.at(i) - 1)->angle * (M_PI / 180.0);
            double ux = rotations.at(transformIds.at(i) - 1)->ux;
            double uy = rotations.at(transformIds.at(i) - 1)->uy;
            double uz = rotations.at(transformIds.at(i) - 1)->uz;
            Vec3 u = Vec3(ux,uy,uz,-1);
            u = normalizeVec3(u);
            double minNo = min(abs(u.x), abs(u.y));
            minNo = min(minNo, abs(u.z));
            Vec3 v,w;
            if (minNo == abs(u.x)){v = Vec3(0,-u.z,u.y,-1);}
            else if (minNo == abs(u.y)){v = Vec3(-u.z,0,u.x,-1);}
            else if (minNo == abs(u.z)){v = Vec3(u.y,-u.x,0,-1);}
            v = normalizeVec3(v);
            w = crossProductVec3(u,v);
            w.colorId = -1;
            w = normalizeVec3(w);
            Matrix4 mMatrix = getIdentityMatrix();
            mMatrix.val[0][0] = u.x,mMatrix.val[0][1] = u.y,mMatrix.val[0][2] = u.z;
            mMatrix.val[1][0] = v.x,mMatrix.val[1][1] = v.y,mMatrix.val[1][2] = v.z;
            mMatrix.val[2][0] = w.x,mMatrix.val[2][1] = w.y,mMatrix.val[2][2] = w.z;
            Matrix4 rMatrix = getIdentityMatrix();
            rMatrix.val[1][1] = cos(R),rMatrix.val[1][2] = -sin(R);
            rMatrix.val[2][1] = sin(R),rMatrix.val[2][2] = cos(R);
            Matrix4 invMatrix = getIdentityMatrix();
            invMatrix.val[0][0] = u.x,invMatrix.val[0][1] = v.x,invMatrix.val[0][2] = w.x;
            invMatrix.val[1][0] = u.y,invMatrix.val[1][1] = v.y,invMatrix.val[1][2] = w.y;
            invMatrix.val[2][0] = u.z,invMatrix.val[2][1] = v.z,invMatrix.val[2][2] = w.z;
            Matrix4 transformation = multiplyMatrixWithMatrix(invMatrix,multiplyMatrixWithMatrix(rMatrix,mMatrix));
            V = multiplyMatrixWithVec4(transformation,V);
        }
    }
    return Vec3(V.x,V.y,V.z,V.colorId);
}


bool Scene::backFaceCulling (Vec4 v0,Vec4 v1,Vec4 v2){

    Vec3 vr0 = Vec3(v0.x,v0.y,v0.z,v0.colorId);
    Vec3 vr1 = Vec3(v1.x,v1.y,v1.z,v1.colorId);
    Vec3 vr2 = Vec3(v2.x,v2.y,v2.z,v2.colorId);
    Vec3 v1_0 = subtractVec3(vr1,vr0);
    Vec3 v2_0 = subtractVec3(vr2,vr0);
    Vec3 normal;
    normal.x = v1_0.y * v2_0.z - v1_0.z * v2_0.y;
    normal.y = v1_0.z * v2_0.x - v1_0.x * v2_0.z;
    normal.z = v1_0.x * v2_0.y - v1_0.y * v2_0.x;
    normal = normalizeVec3(normal);
    if (dotProductVec3(normal,vr1) <= 0)
    {
        return true;
    }
    return false;
}


Vec4 Scene::persDivide(Vec4 toBeDivided){
    Vec4 res = toBeDivided;
    res.x /= res.t;
    res.y /= res.t;
    res.z /= res.t;
    res.t /= res.t;
    return res;
}

double Scene::f(int x, int y, int fvx, int fvy, int svx, int svy) {
    return double(x * (fvy - svy) + y * (svx - fvx) + fvx * svy - fvy * svx);
}
void Scene::computeGridColor(Vec4 v0,Vec4 v1,Vec4 v2,int horRes,int verRes){

    int v0_x = round(v0.x),v0_y = round(v0.y),v0_z = round(v0.z);
    int v1_x = round(v1.x),v1_y = round(v1.y),v1_z = round(v1.z);
    int v2_x = round(v2.x),v2_y = round(v2.y),v2_z = round(v2.z);

    int xMin = min(min(v0_x,v1_x),v2_x);
    int yMin = min(min(v0_y,v1_y),v2_y);
    int xMax = max(max(v0_x,v1_x),v2_x);
    int yMax = max(max(v0_y,v1_y),v2_y);
    

    double alpha,beta,gamma;
    
    if(v0_x < 0 && v1_x < 0 && v2_x < 0){
        return;
    } else if (v0_y < 0 && v1_y < 0 && v2_y < 0){
        return;
    }else if(v0_x > horRes && v1_x > horRes && v2_x > horRes){
        return;
    } else if(v0_y > verRes && v1_y > verRes && v2_y > verRes){
        return;
    }
     

    Color c;

    for (int y = max(0,yMin); y <= min(verRes - 1,yMax) ; y++) {
        for (int x = max(0,xMin); x <= min(horRes - 1,xMax) ; x++) {
            alpha = f(x,y,v1_x,v1_y,v2_x,v2_y) / f(v0_x,v0_y,v1_x,v1_y,v2_x,v2_y);
            beta =  f(x,y,v2_x,v2_y,v0_x,v0_y) / f(v1_x,v1_y,v2_x,v2_y,v0_x,v0_y);
            gamma = f(x,y,v0_x,v0_y,v1_x,v1_y) / f(v2_x,v2_y,v0_x,v0_y,v1_x,v1_y);
            if (alpha >= 0 && beta >= 0 && gamma >= 0){
                c.r = round(alpha * colorsOfVertices.at(v0.colorId - 1)->r + beta * colorsOfVertices.at(v1.colorId - 1)->r + gamma * colorsOfVertices.at(v2.colorId - 1)->r);
                c.g = round(alpha * colorsOfVertices.at(v0.colorId - 1)->g + beta * colorsOfVertices.at(v1.colorId - 1)->g + gamma * colorsOfVertices.at(v2.colorId - 1)->g);
                c.b = round(alpha * colorsOfVertices.at(v0.colorId - 1)->b + beta * colorsOfVertices.at(v1.colorId - 1)->b + gamma * colorsOfVertices.at(v2.colorId - 1)->b);
                image[x][y].r = makeBetweenZeroAnd255(c.r);
                image[x][y].g = makeBetweenZeroAnd255(c.g);
                image[x][y].b = makeBetweenZeroAnd255(c.b);


            }
        }
    }



}

bool Scene::visible(double den,double num,double & te,double & tl){
    if(den > 0){
        double t = num / den;
        if(t > tl){
            return false;
        }
        if(t > te){
            te = t;
        }
    }
    else if(den < 0){
        double t = num / den;
        if(t < te){
            return false;
        }
        if(t < tl){
            tl = t;
        }
    }
    else if(num > 0){
        return false;
    }
    return true;
}
bool Scene::clipping(Vec4 & v0,Vec4 & v1){
    double dx = v1.x - v0.x;
    double dy = v1.y - v0.y;
    double dz = v1.z - v0.z;
    bool visibility = false;
    if (dx == 0 && -1.0 - v0.x > 0){
        return visibility;
    }
    else if (dx ==0 && v0.x - 1.0 > 0){
        return visibility;
    }
    else if(dy == 0 && -1.0 - v0.y > 0){
        return visibility;
    }
    else if(dy == 0 && v0.y - 1.0 > 0){
        return visibility;
    }
    else if(dz == 0 && -1.0 - v0.z > 0){
        return false;
    }
    else if(dz == 0 && v0.z - 1.0 > 0){
        return visibility;
    }
    else{
        double te = 0,tl = 1;
        if (visible(dx,-1.0 - v0.x,te,tl)){
            if(visible(-dx,v0.x - 1.0,te,tl)){
                if(visible(dy,-1.0 - v0.y,te,tl)){
                    if(visible(-dy,v0.y - 1.0,te,tl)){
                        if(visible(dz,-1.0 - v0.z,te,tl)){
                            if(visible(-dz,v0.z - 1.0,te,tl)){
                                visibility = true;
                                if(tl < 1){
                                    v1.x = v0.x + dx * tl;
                                    v1.y = v0.y + dy * tl;
                                    v1.z = v0.z + dz * tl;
                              
                                }
                                if(te > 0){
                                    v0.x = v0.x + dx * te;
                                    v0.y = v0.y + dy * te;
                                    v0.z = v0.z + dz * te;
                                    
                                }
                            }
                        }
                    }
                }
            }
        }

    }
    return visibility;

}

void Scene::computeLineRasterization(Vec4& v0,Vec4& v1,Color& c0,Color& c1,Camera *camera){

    
    double m = (v1.y - v0.y) / (v1.x - v0.x);
    int v0_x = round(v0.x),v0_y = round(v0.y);
    int v1_x = round(v1.x),v1_y = round(v1.y);
    Color c,diff;
    if (-1 <= m &&  m <= 1){
        if(v1_x < v0_x){
            int temp = v0_x;
            int temp2 = v0_y;
            Color temp3 = c0;
            v0_x = v1_x;
            v0_y = v1_y;
            c0 = c1;
            v1_x = temp;
            v1_y = temp2;
            c1 = temp3;
        }

        int y = v0_y;
        int d;
        if(v1_y < v0_y){
            d = 2 * (v0_y - v1_y) + (v1_x - v0_x);
        }
        else{
            d = 2 * (v0_y - v1_y) + (v1_x - v0_x);
        }
        c = c0;
        diff.r = (c1.r - c0.r) / (v1_x - v0_x);
        diff.g = (c1.g - c0.g) / (v1_x - v0_x);
        diff.b = (c1.b - c0.b) / (v1_x - v0_x);
        for (int x = v0_x; x <= v1_x ; x++) {
            if(x >= 0 && x <= camera->horRes -1 && y >= 0 && y <= camera->verRes - 1){
                image[x][y].r = makeBetweenZeroAnd255(c.r);
                image[x][y].g = makeBetweenZeroAnd255(c.g);
                image[x][y].b = makeBetweenZeroAnd255(c.b);
            }

            if(v1_y < v0_y){
                if(-d < 0){
                    y -= 1;
                    d += 2 * ((v0_y - v1_y) - (v1_x - v0_x));
                }
                else{
                    d += 2 * (v0_y - v1_y);
                }
            }
            else{
                if(d < 0){
                    y += 1;
                    d += 2 * ((v0_y - v1_y) + (v1_x - v0_x));
                }
                else{
                    d += 2 * (v0_y - v1_y);
                }
            }
            c.r += diff.r;
            c.g += diff.g;
            c.b += diff.b;
        }
    }
    else{
        if(v1_y < v0_y){
            int temp = v0_x;
            int temp2 = v0_y;
            Color temp3 = c0;
            v0_x = v1_x;
            v0_y = v1_y;
            c0 = c1;
            v1_x = temp;
            v1_y = temp2;
            c1 = temp3;
        }
        int x = v0_x;
        int d;
        if(v1_x < v0_x){
            d = 2 * (v0_x - v1_x) + (v1_y - v0_y);
        }
        else{
            d = 2 * (v0_x - v1_x) + (v1_y - v0_y);
        }
        c = c0;
        diff.r = (c1.r - c0.r) / (v1_y - v0_y);
        diff.g = (c1.g - c0.g) / (v1_y - v0_y);
        diff.b = (c1.b - c0.b) / (v1_y - v0_y);
        for (int y = v0_y; y <= v1_y ; y++) {
            if(x >= 0 && x <= camera->horRes -1 && y >= 0 && y <= camera->verRes - 1){
                image[x][y].r = makeBetweenZeroAnd255(c.r);
                image[x][y].g = makeBetweenZeroAnd255(c.g);
                image[x][y].b = makeBetweenZeroAnd255(c.b);
            }
            if(v1_x < v0_x){
                if(-d < 0){
                    x -= 1;
                    d += 2 * ((v0_x - v1_x) - (v1_y - v0_y));
                }
                else{
                    d += 2 * (v0_x - v1_x);
                }
            }
            else{
                if(d < 0){
                    x += 1;
                    d += 2 * ((v0_x - v1_x) + (v1_y - v0_y));
                }
                else{
                    d += 2 * (v0_x - v1_x);
                }
            }
            c.r += diff.r;
            c.g += diff.g;
            c.b += diff.b;

        }
    }
}

void Scene::forwardRenderingPipeline(Camera *camera)
{

    //Form camera transformation matrix
    Matrix4 T = getIdentityMatrix();
    T.val[0][3] = -camera->pos.x,T.val[1][3] = -camera->pos.y,T.val[2][3] = -camera->pos.z;

    Matrix4 R = getIdentityMatrix();
    R.val[0][0] = camera->u.x,R.val[0][1] = camera->u.y,R.val[0][2] = camera->u.z;
    R.val[1][0] = camera->v.x,R.val[1][1] = camera->v.y,R.val[1][2] = camera->v.z;
    R.val[2][0] = camera->w.x,R.val[2][1] = camera->w.y,R.val[2][2] = camera->w.z;

    Matrix4 camTransformation = multiplyMatrixWithMatrix(R,T);
    //Form orthographic projection matrix

    Matrix4 orth = getIdentityMatrix();
    orth.val[0][0] = 2.0 / (camera->right - camera->left);
    orth.val[0][3] = - (camera->right + camera->left) / (camera->right - camera->left);
    orth.val[1][1] = 2.0 / (camera->top - camera->bottom);
    orth.val[1][3] = - (camera->top + camera->bottom) / (camera->top - camera->bottom);
    orth.val[2][2] = -2.0 / (camera->far - camera->near);
    orth.val[2][3] = - (camera->far + camera->near) / (camera->far - camera->near);

    //Form perspective projection matrix

    Matrix4 p2o = getIdentityMatrix();
    p2o.val[0][0] = camera->near,p2o.val[1][1] = camera->near;
    p2o.val[2][2] = camera->far + camera->near;
    p2o.val[2][3] = camera->far * camera->near;
    p2o.val[3][2] = -1.0,p2o.val[3][3] = 0.0;

    Matrix4 per = multiplyMatrixWithMatrix(orth,p2o);

    //Form viewport transformation matrix

    Matrix4 vp = getIdentityMatrix();
    vp.val[0][0] = camera->horRes / 2.0,vp.val[0][3] = (camera->horRes - 1) / 2.0;
    vp.val[1][1] = camera->verRes / 2.0,vp.val[1][3] = (camera->verRes - 1) / 2.0;
    vp.val[2][2] = 0.5,vp.val[2][3] = 0.5;


    for (int i = 0; i < meshes.size() ; i++) {

        vector<int> transformedStatus;
        transformedStatus.reserve(vertices.size());
        vector<Vec3> transformedVertices;
        transformedVertices.reserve(vertices.size());
        for (int j = 0; j < vertices.size() ; ++j) {
            transformedStatus.push_back(0);
            transformedVertices.push_back(Vec3(0,0,0,0));
        }
        for (int j = 0; j < meshes.at(i)->triangles.size() ; j++) {
            int fVId = meshes.at(i)->triangles.at(j).getFirstVertexId();
            if (transformedStatus.at(fVId - 1) == 0) {
                Vec3 V0 = *vertices.at(fVId - 1);
                Vec3 temp = transformVec(V0,meshes.at(i)->transformationTypes,meshes.at(i)->transformationIds);
                transformedStatus.at(fVId - 1) = 1;
                transformedVertices.at(fVId - 1) = temp;
            }
            int sVId = meshes.at(i)->triangles.at(j).getSecondVertexId();
            if (transformedStatus.at(sVId - 1) == 0){
                Vec3 V1 = *vertices.at(sVId - 1);
                Vec3 temp = transformVec(V1,meshes.at(i)->transformationTypes,meshes.at(i)->transformationIds);
                transformedStatus.at(sVId - 1) = 1;
                transformedVertices.at(sVId - 1) = temp;
            }
            int tVId = meshes.at(i)->triangles.at(j).getThirdVertexId();
            if (transformedStatus.at(tVId - 1) == 0){
                Vec3 V2 = *vertices.at(tVId - 1);
                Vec3 temp = transformVec(V2,meshes.at(i)->transformationTypes,meshes.at(i)->transformationIds);
                transformedStatus.at(tVId - 1) = 1;
                transformedVertices.at(tVId - 1) = temp;
            }
        }
        for (int j = 0; j < meshes.at(i)->triangles.size() ; j++){

            int fVId = meshes.at(i)->triangles.at(j).getFirstVertexId();
            int sVId = meshes.at(i)->triangles.at(j).getSecondVertexId();
            int tVId = meshes.at(i)->triangles.at(j).getThirdVertexId();
            Vec3 fV = transformedVertices.at(fVId - 1);
            Vec3 sV = transformedVertices.at(sVId - 1);
            Vec3 tV = transformedVertices.at(tVId - 1);

            Vec4 fV4 = Vec4(fV.x,fV.y,fV.z,1,fV.colorId);
            Vec4 sV4 = Vec4(sV.x,sV.y,sV.z,1,sV.colorId);
            Vec4 tV4 = Vec4(tV.x,tV.y,tV.z,1,tV.colorId);

            // Camera Transformation

            fV4 = multiplyMatrixWithVec4(camTransformation,fV4);
            sV4 = multiplyMatrixWithVec4(camTransformation,sV4);
            tV4 = multiplyMatrixWithVec4(camTransformation,tV4);

            if(camera->projectionType == 0){
                //orthographic projection

                fV4 = multiplyMatrixWithVec4(orth,fV4);
                sV4 = multiplyMatrixWithVec4(orth,sV4);
                tV4 = multiplyMatrixWithVec4(orth,tV4);
            }
            else{
                //perspective projection
                fV4 = multiplyMatrixWithVec4(per,fV4);
                sV4 = multiplyMatrixWithVec4(per,sV4);
                tV4 = multiplyMatrixWithVec4(per,tV4);
            }
            if(meshes.at(i)->type == 1){
                //solid
                fV4 = persDivide(fV4);
                sV4 = persDivide(sV4);
                tV4 = persDivide(tV4);
                if(this->cullingEnabled){
                    if (backFaceCulling(fV4,sV4,tV4)){
                        continue;
                    }
                }
                //viewport transformation
                fV4 = multiplyMatrixWithVec4(vp,fV4);
                sV4 = multiplyMatrixWithVec4(vp,sV4);
                tV4 = multiplyMatrixWithVec4(vp,tV4);
                computeGridColor(fV4,sV4,tV4,camera->horRes,camera->verRes);
            }
            else{
                //wireframe
                if(this->cullingEnabled){
                    if (backFaceCulling(fV4,sV4,tV4)){
                        continue;
                    }
                }
                fV4 = persDivide(fV4);
                sV4 = persDivide(sV4);
                tV4 = persDivide(tV4);
                Vec4 backupfV = fV4;
                Vec4 backupsV = sV4;
                Vec4 backuptV = tV4;
                Color c0 = *colorsOfVertices.at(fV4.colorId - 1);
                Color bc0 = *colorsOfVertices.at(fV4.colorId - 1);
                Color c1 = *colorsOfVertices.at(sV4.colorId - 1);
                Color bc1 = *colorsOfVertices.at(sV4.colorId - 1);
                Color c2 = *colorsOfVertices.at(tV4.colorId - 1);
                Color bc2 = *colorsOfVertices.at(tV4.colorId - 1);
                if(clipping(fV4,sV4)){
                    //viewport transformation
                    fV4 = multiplyMatrixWithVec4(vp,fV4);
                    sV4 = multiplyMatrixWithVec4(vp,sV4);
                    computeLineRasterization(fV4,sV4,c0,c1,camera);
                }
                if(clipping(backupsV,tV4)){
                    //viewport transformation
                    backupsV = multiplyMatrixWithVec4(vp,backupsV);
                    tV4 = multiplyMatrixWithVec4(vp,tV4);
                    computeLineRasterization(backupsV,tV4,bc1,c2,camera);
                }
                if(clipping(backuptV,backupfV)){
                    //viewport transformation
                    backuptV = multiplyMatrixWithVec4(vp,backuptV);
                    backupfV = multiplyMatrixWithVec4(vp,backupfV);
                    computeLineRasterization(backuptV,backupfV,bc2,bc0,camera);
                }

            }
        }

    }


}
/*
    Parses XML file
*/
Scene::Scene(const char *xmlPath)
{
    const char *str;
    XMLDocument xmlDoc;
    XMLElement *pElement;

    xmlDoc.LoadFile(xmlPath);

    XMLNode *pRoot = xmlDoc.FirstChild();

    // read background color
    pElement = pRoot->FirstChildElement("BackgroundColor");
    str = pElement->GetText();
    sscanf(str, "%lf %lf %lf", &backgroundColor.r, &backgroundColor.g, &backgroundColor.b);

    // read culling
    pElement = pRoot->FirstChildElement("Culling");
    if (pElement != NULL) {
        str = pElement->GetText();

        if (strcmp(str, "enabled") == 0) {
            cullingEnabled = true;
        }
        else {
            cullingEnabled = false;
        }
    }

    // read cameras
    pElement = pRoot->FirstChildElement("Cameras");
    XMLElement *pCamera = pElement->FirstChildElement("Camera");
    XMLElement *camElement;
    while (pCamera != NULL)
    {
        Camera *cam = new Camera();

        pCamera->QueryIntAttribute("id", &cam->cameraId);

        // read projection type
        str = pCamera->Attribute("type");

        if (strcmp(str, "orthographic") == 0) {
            cam->projectionType = 0;
        }
        else {
            cam->projectionType = 1;
        }

        camElement = pCamera->FirstChildElement("Position");
        str = camElement->GetText();
        sscanf(str, "%lf %lf %lf", &cam->pos.x, &cam->pos.y, &cam->pos.z);

        camElement = pCamera->FirstChildElement("Gaze");
        str = camElement->GetText();
        sscanf(str, "%lf %lf %lf", &cam->gaze.x, &cam->gaze.y, &cam->gaze.z);

        camElement = pCamera->FirstChildElement("Up");
        str = camElement->GetText();
        sscanf(str, "%lf %lf %lf", &cam->v.x, &cam->v.y, &cam->v.z);

        cam->gaze = normalizeVec3(cam->gaze);
        cam->u = crossProductVec3(cam->gaze, cam->v);
        cam->u = normalizeVec3(cam->u);

        cam->w = inverseVec3(cam->gaze);
        cam->v = crossProductVec3(cam->u, cam->gaze);
        cam->v = normalizeVec3(cam->v);

        camElement = pCamera->FirstChildElement("ImagePlane");
        str = camElement->GetText();
        sscanf(str, "%lf %lf %lf %lf %lf %lf %d %d",
               &cam->left, &cam->right, &cam->bottom, &cam->top,
               &cam->near, &cam->far, &cam->horRes, &cam->verRes);

        camElement = pCamera->FirstChildElement("OutputName");
        str = camElement->GetText();
        cam->outputFileName = string(str);

        cameras.push_back(cam);

        pCamera = pCamera->NextSiblingElement("Camera");
    }

    // read vertices
    pElement = pRoot->FirstChildElement("Vertices");
    XMLElement *pVertex = pElement->FirstChildElement("Vertex");
    int vertexId = 1;

    while (pVertex != NULL)
    {
        Vec3 *vertex = new Vec3();
        Color *color = new Color();

        vertex->colorId = vertexId;

        str = pVertex->Attribute("position");
        sscanf(str, "%lf %lf %lf", &vertex->x, &vertex->y, &vertex->z);

        str = pVertex->Attribute("color");
        sscanf(str, "%lf %lf %lf", &color->r, &color->g, &color->b);

        vertices.push_back(vertex);
        colorsOfVertices.push_back(color);

        pVertex = pVertex->NextSiblingElement("Vertex");

        vertexId++;
    }

    // read translations
    pElement = pRoot->FirstChildElement("Translations");
    XMLElement *pTranslation = pElement->FirstChildElement("Translation");
    while (pTranslation != NULL)
    {
        Translation *translation = new Translation();

        pTranslation->QueryIntAttribute("id", &translation->translationId);

        str = pTranslation->Attribute("value");
        sscanf(str, "%lf %lf %lf", &translation->tx, &translation->ty, &translation->tz);

        translations.push_back(translation);

        pTranslation = pTranslation->NextSiblingElement("Translation");
    }

    // read scalings
    pElement = pRoot->FirstChildElement("Scalings");
    XMLElement *pScaling = pElement->FirstChildElement("Scaling");
    while (pScaling != NULL)
    {
        Scaling *scaling = new Scaling();

        pScaling->QueryIntAttribute("id", &scaling->scalingId);
        str = pScaling->Attribute("value");
        sscanf(str, "%lf %lf %lf", &scaling->sx, &scaling->sy, &scaling->sz);

        scalings.push_back(scaling);

        pScaling = pScaling->NextSiblingElement("Scaling");
    }

    // read rotations
    pElement = pRoot->FirstChildElement("Rotations");
    XMLElement *pRotation = pElement->FirstChildElement("Rotation");
    while (pRotation != NULL)
    {
        Rotation *rotation = new Rotation();

        pRotation->QueryIntAttribute("id", &rotation->rotationId);
        str = pRotation->Attribute("value");
        sscanf(str, "%lf %lf %lf %lf", &rotation->angle, &rotation->ux, &rotation->uy, &rotation->uz);

        rotations.push_back(rotation);

        pRotation = pRotation->NextSiblingElement("Rotation");
    }

    // read meshes
    pElement = pRoot->FirstChildElement("Meshes");

    XMLElement *pMesh = pElement->FirstChildElement("Mesh");
    XMLElement *meshElement;
    while (pMesh != NULL)
    {
        Mesh *mesh = new Mesh();

        pMesh->QueryIntAttribute("id", &mesh->meshId);

        // read projection type
        str = pMesh->Attribute("type");

        if (strcmp(str, "wireframe") == 0) {
            mesh->type = 0;
        }
        else {
            mesh->type = 1;
        }

        // read mesh transformations
        XMLElement *pTransformations = pMesh->FirstChildElement("Transformations");
        XMLElement *pTransformation = pTransformations->FirstChildElement("Transformation");

        while (pTransformation != NULL)
        {
            char transformationType;
            int transformationId;

            str = pTransformation->GetText();
            sscanf(str, "%c %d", &transformationType, &transformationId);

            mesh->transformationTypes.push_back(transformationType);
            mesh->transformationIds.push_back(transformationId);

            pTransformation = pTransformation->NextSiblingElement("Transformation");
        }

        mesh->numberOfTransformations = mesh->transformationIds.size();

        // read mesh faces
        char *row;
        char *clone_str;
        int v1, v2, v3;
        XMLElement *pFaces = pMesh->FirstChildElement("Faces");
        str = pFaces->GetText();
        clone_str = strdup(str);

        row = strtok(clone_str, "\n");
        while (row != NULL)
        {
            int result = sscanf(row, "%d %d %d", &v1, &v2, &v3);

            if (result != EOF) {
                mesh->triangles.push_back(Triangle(v1, v2, v3));
            }
            row = strtok(NULL, "\n");
        }
        mesh->numberOfTriangles = mesh->triangles.size();
        meshes.push_back(mesh);

        pMesh = pMesh->NextSiblingElement("Mesh");
    }
}

/*
    Initializes image with background color
*/
void Scene::initializeImage(Camera *camera)
{
    if (this->image.empty())
    {
        for (int i = 0; i < camera->horRes; i++)
        {
            vector<Color> rowOfColors;

            for (int j = 0; j < camera->verRes; j++)
            {
                rowOfColors.push_back(this->backgroundColor);
            }

            this->image.push_back(rowOfColors);
        }
    }
    else
    {
        for (int i = 0; i < camera->horRes; i++)
        {
            for (int j = 0; j < camera->verRes; j++)
            {
                this->image[i][j].r = this->backgroundColor.r;
                this->image[i][j].g = this->backgroundColor.g;
                this->image[i][j].b = this->backgroundColor.b;
            }
        }
    }
}

/*
    If given value is less than 0, converts value to 0.
    If given value is more than 255, converts value to 255.
    Otherwise returns value itself.
*/
int Scene::makeBetweenZeroAnd255(double value)
{
    if (value >= 255.0)
        return 255;
    if (value <= 0.0)
        return 0;
    return (int)(value);
}

/*
    Writes contents of image (Color**) into a PPM file.
*/
void Scene::writeImageToPPMFile(Camera *camera)
{
    ofstream fout;

    fout.open(camera->outputFileName.c_str());

    fout << "P3" << endl;
    fout << "# " << camera->outputFileName << endl;
    fout << camera->horRes << " " << camera->verRes << endl;
    fout << "255" << endl;

    for (int j = camera->verRes - 1; j >= 0; j--)
    {
        for (int i = 0; i < camera->horRes; i++)
        {
            fout << makeBetweenZeroAnd255(this->image[i][j].r) << " "
                 << makeBetweenZeroAnd255(this->image[i][j].g) << " "
                 << makeBetweenZeroAnd255(this->image[i][j].b) << " ";
        }
        fout << endl;
    }
    fout.close();
}

/*
    Converts PPM image in given path to PNG file, by calling ImageMagick's 'convert' command.
    os_type == 1        -> Ubuntu
    os_type == 2        -> Windows
    os_type == other    -> No conversion
*/
void Scene::convertPPMToPNG(string ppmFileName, int osType)
{
    string command;

    // call command on Ubuntu
    if (osType == 1)
    {
        command = "convert " + ppmFileName + " " + ppmFileName + ".png";
        system(command.c_str());
    }

        // call command on Windows
    else if (osType == 2)
    {
        command = "magick convert " + ppmFileName + " " + ppmFileName + ".png";
        system(command.c_str());
    }

        // default action - don't do conversion
    else
    {
    }
}