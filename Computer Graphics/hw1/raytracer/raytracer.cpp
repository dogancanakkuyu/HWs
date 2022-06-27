#include <iostream>
#include "parser.h"
#include "ppm.h"
#include <cmath>

typedef unsigned char RGB[3];
int nx,ny,recursionDepth;
float top,bottom,right,left,dist,shadowRayEpsilon;
std::string imageName;
parser::Vec3f e,gaze,up,ambientLight,u,w,m,q;
parser::Vec3i backgroundColor;
std::vector<parser::Vec3f> vertexDatas;
std::vector<parser::Sphere> spheres;
std::vector<parser::Triangle> triangles;
std::vector<parser::Mesh> meshes;
std::vector<parser::Material> materials;
std::vector<parser::PointLight> pointLights;


struct Ray{
    parser::Vec3f o,d;
};
struct Object{
    float t;
    int shapeCheck; //1 for spheres, 2 for triangles, 3 for mesh
    bool intersecting;
    int material_id;
    int index; //For spheres and triangles
    parser::Vec3f normalV;
    parser::Vec3f point;
    parser::Triangle intersectedTriangle; //For meshes
    Object(float t,bool intersecting,int shapeCheck ){
        this->t = t;
        this->intersecting = intersecting;
        this->shapeCheck = shapeCheck;
    }
};


float determinant3x3(parser::Vec3f v1,parser::Vec3f v2,parser::Vec3f v3){
    float result;
    float a,b,c,d,e,f,g,h,i;
    a = v1.x; b = v1.y; c = v1.z; d = v2.x; e = v2.y; f = v2.z; g = v3.x; h = v3.y; i = v3.z;
    result = a*(e*i - h*f) + b*(g*f - d*i) + c*(d*h - e*g);
    return result;
}

parser::Vec3f multS(parser::Vec3f a,float s)
{
    parser::Vec3f result;
    result.x = a.x*s;
    result.y = a.y*s;
    result.z = a.z*s;
    return result;
}

parser::Vec3f addVectors(parser::Vec3f v1,parser::Vec3f v2){
    parser::Vec3f res;
    res.x = v1.x + v2.x;
    res.y = v1.y + v2.y;
    res.z = v1.z + v2.z;
    return res;
}
float dot(parser::Vec3f a,parser::Vec3f b)
{
    return a.x*b.x+a.y*b.y+a.z*b.z;
}

parser::Vec3f cross(parser::Vec3f v1,parser::Vec3f v2){
    parser::Vec3f result;
    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.x;
    return result;
}

parser::Vec3f normalize(parser::Vec3f a)
{
    return multS(a,1.0/sqrt(dot(a,a)));
}

parser::Vec3f normalVectorTriangle(parser::Triangle triangle){
    parser::Vec3f normal,a,b,c;
    parser::Vec3f c_a,b_a;
    a = vertexDatas.at(triangle.indices.v0_id-1);
    b = vertexDatas.at(triangle.indices.v1_id-1);
    c = vertexDatas.at(triangle.indices.v2_id-1);
    b_a = addVectors(b, multS(a,-1));
    c_a = addVectors(c, multS(a,-1));
    normal.x = b_a.y*c_a.z - b_a.z*c_a.y;
    normal.y = b_a.z*c_a.x - b_a.x*c_a.z;
    normal.z = b_a.x*c_a.y - b_a.y*c_a.x;
    return normalize(normal);
}

Ray generateRay(int i,int j){
    Ray result;
    parser::Vec3f s;
    float su,sv;
    su = (i+0.5)*(right-left)/nx;
    sv = (j+0.5)*(top-bottom)/ny;
    s = addVectors(q, addVectors(multS(u,su), multS(up,-sv)));
    result.o = e;
    result.d = addVectors(s, multS(e,-1));
    return result;
}

float intersectSphere(Ray r,parser::Sphere s){
    float A,B,C; //constants for the quadratic equation

    float delta;

    parser::Vec3f c;
    c = vertexDatas.at(s.center_vertex_id-1);
    float t,t1,t2;

    C = (r.o.x-c.x)*(r.o.x-c.x)+(r.o.y-c.y)*(r.o.y-c.y)+(r.o.z-c.z)*(r.o.z-c.z)-s.radius*s.radius;

    B = 2*r.d.x*(r.o.x-c.x)+2*r.d.y*(r.o.y-c.y)+2*r.d.z*(r.o.z-c.z);

    A = r.d.x*r.d.x+r.d.y*r.d.y+r.d.z*r.d.z;

    delta = B*B-4*A*C;

    if (delta<0) return -1;
    else if (delta==0)
    {
        t = -B / (2*A);
    }
    else
    {
        delta = sqrt(delta);
        A = 2*A;
        t1 = (-B + delta) / A;
        t2 = (-B - delta) / A;

        if (t1<t2) t=t1; else t=t2;
    }

    return t;
}

float intersectTriangle(Ray r,parser::Triangle triangle){
    float beta,gamma,t;
    parser::Vec3f v1,v2,v3,v4; //v1 = ax-ox ; v2 = ax-cx; v3 = ax-bx; v4 = dx
    float ax,bx,cx,ay,by,cy,az,bz,cz;
    ax = vertexDatas[triangle.indices.v0_id-1].x;
    ay = vertexDatas[triangle.indices.v0_id-1].y;
    az = vertexDatas[triangle.indices.v0_id-1].z;
    bx = vertexDatas[triangle.indices.v1_id-1].x;
    by = vertexDatas[triangle.indices.v1_id-1].y;
    bz = vertexDatas[triangle.indices.v1_id-1].z;
    cx = vertexDatas[triangle.indices.v2_id-1].x;
    cy = vertexDatas[triangle.indices.v2_id-1].y;
    cz = vertexDatas[triangle.indices.v2_id-1].z;
    v1.x = ax-r.o.x; v1.y = ay-r.o.y; v1.z = az-r.o.z;
    v2.x = ax-cx; v2.y = ay-cy; v2.z = az-cz;
    v3.x = ax-bx; v3.y = ay-by; v3.z = az-bz;
    v4.x = r.d.x; v4.y = r.d.y; v4.z = r.d.z;
    beta = determinant3x3(v1,v2,v4) / determinant3x3(v3,v2,v4);
    gamma = determinant3x3(v3,v1,v4) / determinant3x3(v3,v2,v4);
    t = determinant3x3(v3,v2,v1) / determinant3x3(v3,v2,v4);
    if (beta >=0 && gamma >=0 && beta+gamma <=1 ) {
        if (t>0 && t< 100000) {
            return t;
        }
        else{
            return -1;
        }
    }
    else{
        return -1;
    }
}
parser::Vec3f findReceivedRadiance (parser::PointLight pointLight, parser::Vec3f point) {
    parser::Vec3f lightVector = addVectors(pointLight.position, multS(point,-1));
    float distance = dot(lightVector,lightVector);
    return multS(pointLight.intensity,1.0/distance);
}

float distancePointToLight (parser::PointLight pointLight, parser::Vec3f point) {
    parser::Vec3f lightVector = addVectors(pointLight.position, multS(point,-1));
    return sqrt(dot(lightVector,lightVector));
}


parser::Vec3f multV(parser::Vec3f v1,parser::Vec3f v2){
    parser::Vec3f result;
    result.x = v1.x * v2.x;
    result.y = v1.y * v2.y;
    result.z = v1.z * v2.z;
    return result;
}

float roundTo255 (float num) {
    if (num <= 255.0) return num;
    else return 255.0;
}
float max(float f1,float f2){
    if (f1>=f2) return f1;else return f2;
}
bool isShadow(Ray shadowRay, float distanceToLight) {
    for (int j = 0; j <spheres.size() ; ++j) {
        float shadowT = intersectSphere(shadowRay,spheres.at(j));
        if (shadowT > shadowRayEpsilon && shadowT < distanceToLight){
            return true;
        }
    }

    for (int j = 0; j < triangles.size(); ++j) {
        float shadowT = intersectTriangle(shadowRay,triangles.at(j));
        if (shadowT > shadowRayEpsilon && shadowT < distanceToLight){
            return true;
        }
    }

    for (int i = 0; i < meshes.size(); ++i) {
        std::vector<parser::Triangle> trianglesInMesh;
        for (int j = 0; j <meshes.at(i).faces.size() ; j++) {
            parser::Triangle triangle;
            triangle.material_id = meshes.at(i).material_id;
            triangle.indices.v0_id = meshes.at(i).faces.at(j).v0_id;
            triangle.indices.v1_id = meshes.at(i).faces.at(j).v1_id;
            triangle.indices.v2_id = meshes.at(i).faces.at(j).v2_id;
            trianglesInMesh.push_back(triangle);
        }
        for (int j = 0; j <trianglesInMesh.size() ; ++j) {
            float shadowT = intersectTriangle(shadowRay,trianglesInMesh.at(j));
            if (shadowT > shadowRayEpsilon && shadowT < distanceToLight){
                return true;
            }
        }
    }
    return false;

}

Object computeIntersection(Ray r){
    Object obj = Object(60000,false,0);
    float tempT;
    for (int i = 0; i <spheres.size() ; ++i) {
        tempT = intersectSphere(r,spheres.at(i));
        if (tempT < obj.t && tempT>=0){
            obj.index = i;
            obj.t = tempT;
            obj.shapeCheck = 1;
        }
    }
    for (int i = 0; i <triangles.size() ; ++i) {
        tempT = intersectTriangle(r,triangles.at(i));
        if (tempT < obj.t && tempT>=0){
            obj.index = i;
            obj.t = tempT;
            obj.shapeCheck = 2;
        }
    }
    for (int i = 0; i <meshes.size() ; ++i) {
        std::vector<parser::Triangle> trianglesInMesh;
        for (int j = 0; j <meshes.at(i).faces.size() ; j++) {
            parser::Triangle triangle;
            triangle.material_id = meshes.at(i).material_id;
            triangle.indices.v0_id = meshes.at(i).faces.at(j).v0_id;
            triangle.indices.v1_id = meshes.at(i).faces.at(j).v1_id;
            triangle.indices.v2_id = meshes.at(i).faces.at(j).v2_id;
            trianglesInMesh.push_back(triangle);
        }
        for (int j = 0; j <trianglesInMesh.size() ; ++j) {
            tempT = intersectTriangle(r,trianglesInMesh.at(j));
            if (tempT < obj.t && tempT>=0){
                obj.t = tempT;
                obj.shapeCheck = 3;
                obj.intersectedTriangle.material_id = trianglesInMesh.at(j).material_id;
                obj.intersectedTriangle.indices.v0_id = trianglesInMesh.at(j).indices.v0_id;
                obj.intersectedTriangle.indices.v1_id = trianglesInMesh.at(j).indices.v1_id;
                obj.intersectedTriangle.indices.v2_id = trianglesInMesh.at(j).indices.v2_id;
            }
        }
    }
    if (obj.shapeCheck == 1) {
        obj.intersecting = true;
        obj.material_id = spheres.at(obj.index).material_id;
        obj.point = addVectors(r.o,multS(r.d,obj.t));
        obj.normalV = normalize(addVectors(obj.point,multS(vertexDatas.at(spheres.at(obj.index).center_vertex_id-1),-1)));
        return obj;
    }
    else if (obj.shapeCheck == 2) {
        obj.intersecting = true;
        obj.material_id = triangles.at(obj.index).material_id;
        obj.point = addVectors(r.o,multS(r.d,obj.t));
        obj.normalV = normalize(normalVectorTriangle(triangles.at(obj.index)));
        return obj;
    }
    else if (obj.shapeCheck == 3) {
        obj.intersecting = true;
        obj.material_id = obj.intersectedTriangle.material_id;
        obj.point = obj.point = addVectors(r.o,multS(r.d,obj.t));
        obj.normalV = normalize(normalVectorTriangle(obj.intersectedTriangle));
        return obj;
    }
    else {return obj;}
}
Object computeReflectedObjec(Ray recursiveRay){
    Object result =  Object(60000,false,0);
    Object newIntersectedObj = computeIntersection(recursiveRay);
    if (newIntersectedObj.t < result.t){
        result = newIntersectedObj;
    }
    return result;

}
parser::Vec3f computeColor(Ray r,Object obj,int recursionD){
    parser::Vec3f color;
    color.x = backgroundColor.x;
    color.y = backgroundColor.y;
    color.z = backgroundColor.z;
    if (obj.intersecting) {
        //AMBIENT SHADING
        parser::Vec3f ambientCoef = materials.at(obj.material_id-1).ambient;
        color = multV(ambientCoef,ambientLight);
        parser::Vec3f w0 = normalize(addVectors(r.o, multS(obj.point,-1)));
        for (int i = 0; i <pointLights.size() ; ++i) {
            parser::Vec3f L = normalize(addVectors(pointLights.at(i).position,multS(obj.point,-1)));
            Ray shadowRay;
            shadowRay.o = obj.point;
            shadowRay.d = L;
            float distanceToLight = distancePointToLight(pointLights.at(i), obj.point);
            bool inShadow = isShadow(shadowRay,distanceToLight);
            if (inShadow) continue;
            parser::Vec3f H = normalize(addVectors(L, w0));
            float cosa,cosb; //cosa -> angle btw L & N , cosb -> angle btw N & H
            cosa = max((float)0,dot(L,obj.normalV));
            cosb = max((float)0, dot(obj.normalV,H));
            parser::Vec3f diffuseCoef = materials.at(obj.material_id-1).diffuse;
            parser::Vec3f specCoef = materials.at(obj.material_id-1).specular;
            float phongExp = materials.at(obj.material_id-1).phong_exponent;
            parser::Vec3f receivedIrradiance = findReceivedRadiance(pointLights.at(i),obj.point);
            //DIFFUSE SHADING
            color = addVectors(color,multV(diffuseCoef, multS(receivedIrradiance,cosa)));
            //SPECULAR SHADING
            color = addVectors(color,multV(specCoef, multS(receivedIrradiance, pow(cosb,phongExp))));

        }
        //MIRROR
        bool isMirror = materials.at(obj.material_id-1).is_mirror;
        if (isMirror) {
            if (recursionD > 0) {
                parser::Vec3f mirrorCoef = materials.at(obj.material_id-1).mirror;
                float cosTeta = max((float)0,dot(obj.normalV,w0));
                parser::Vec3f wr = normalize(addVectors(multS(w0,-1), multS(obj.normalV,2*cosTeta)));
                Ray recursiveRay;
                recursiveRay.o = addVectors(obj.point, multS(wr,shadowRayEpsilon));
                recursiveRay.d = wr;
                Object recursiveObj = computeReflectedObjec(recursiveRay);
                if (recursiveObj.intersecting) {
                    color = addVectors(color, multV(mirrorCoef, computeColor(recursiveRay,recursiveObj,recursionD-1)));
                }
            }

        }
    }
    return color;




}

int main(int argc, char* argv[])
{
    // Sample usage for reading an XML scene file
    parser::Scene scene;

    scene.loadFromXml(argv[1]);
    std::vector<parser::Camera> cameras = scene.cameras;
    spheres = scene.spheres;
    vertexDatas = scene.vertex_data;
    triangles = scene.triangles;
    meshes = scene.meshes;
    materials = scene.materials;
    ambientLight = scene.ambient_light;
    pointLights = scene.point_lights;
    backgroundColor = scene.background_color;
    shadowRayEpsilon = scene.shadow_ray_epsilon;
    recursionDepth = scene.max_recursion_depth;
    for (int i = 0; i <cameras.size() ; ++i) {
        left = cameras[i].near_plane.x;
        right = cameras[i].near_plane.y;
        bottom = cameras[i].near_plane.z;
        top = cameras[i].near_plane.w;
        e = cameras[i].position;
        gaze = cameras[i].gaze;
        w = normalize(multS(gaze,-1));
        up = normalize(cameras[i].up);
        u = normalize(cross(up,w));
        dist = cameras[i].near_distance;
        m = addVectors(e, multS(w,-1*dist));
        q = addVectors(addVectors(m, multS(u,left)), multS(up,top));
        nx = cameras[i].image_width;
        ny = cameras[i].image_height;
        imageName = cameras[i].image_name;
        // The code below creates a test pattern and writes
        // it to a PPM file to demonstrate the usage of the
        // ppm_write function.
        //
        // Normally, you would be running your ray tracing
        // code here to produce the desired image.

        int width = nx, height = ny;

        unsigned char* image = new unsigned char [width * height * 3];
        int imageIndex = 0;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {

                Ray myray = generateRay(x,y);
                parser::Vec3f rayColor;
                rayColor = computeColor(myray, computeIntersection(myray),recursionDepth);
                image[imageIndex++] = roundTo255(rayColor.x);
                image[imageIndex++] = roundTo255(rayColor.y);
                image[imageIndex++] = roundTo255(rayColor.z);

            }
        }
        write_ppm(imageName.c_str(), image, width, height);
    }



}
