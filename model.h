//#pragma once
//#include <vector>
//#include <string>
//#include "geometry.h"
//#include "tgaimage.h"
//class Model {
//
//	std::vector<std::vector<int> > faces_;
//
//	std::vector<vec3> vertex_{};					//顶点坐标
//	std::vector<vec2> texCoord_{};					//纹理坐标
//	std::vector<vec3> normalLine_{};				//法线
//
//	std::vector<int> face_vertex{};                 //面三角形对应的顶点索引
//	std::vector<int> face_texCoord{};				//面三角形对应的纹理索引
//	std::vector<int> face_normalLine{};				//面三角形对应的法线索引
//
//	TGAImage diffusemap{};                          //漫反射颜色纹理
//	TGAImage normalmap{};							//自然纹理
//	TGAImage specularmap{};							//镜面反射纹理
//
//	void load_texture(const std::string filename, const std::string suffix, TGAImage& img);
//
//public:
//
//	Model(const std::string filename);
//
//	int nverts() const;
//	int nfaces() const;
//	vec3 normal(const int iface, const int nthvert) const;   //每个三角形的中心法线顶点
//	vec3 normal(const vec2& uv)const;						 //从法线贴图纹理中获取法线向量
//	vec3 vert(const int i)const;
//	vec3 vert(const int iface, const int nthvert)const;
//	vec2 uv(const int iface, const int nthvert)const;
//	const TGAImage& diffuse() const { return diffusemap; }
//	const TGAImage& specular() const { return specularmap; }
//};

#ifndef __MODEL_H__
#define __MODEL_H__
#include <vector>
#include <string>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
    std::vector<Vec3f> verts_;
    std::vector<std::vector<Vec3i> > faces_; // attention, this Vec3i means vertex/uv/normal
    std::vector<Vec3f> norms_;
    std::vector<Vec2f> uv_;
    TGAImage diffusemap_;
    TGAImage normalmap_;
    TGAImage specularmap_;
    void load_texture(std::string filename, const char* suffix, TGAImage& img);
public:
    Model(const char* filename);
    ~Model();
    int nverts();
    int nfaces();
    Vec3f norm(int iface, int nthvert);
    Vec3f norm(Vec2f uv);
    Vec3f vert(int i);
    Vec3f vert(int iface, int nthvert);
    Vec2f uv(int iface, int nthvert);
    TGAColor diffuse(Vec2f uv);
    float specular(Vec2f uv);
    std::vector<int> face(int idx);
};
#endif //__MODEL_H__
