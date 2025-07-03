//#pragma once
//#include <vector>
//#include <string>
//#include "geometry.h"
//#include "tgaimage.h"
//class Model {
//
//	std::vector<std::vector<int> > faces_;
//
//	std::vector<vec3> vertex_{};					//��������
//	std::vector<vec2> texCoord_{};					//��������
//	std::vector<vec3> normalLine_{};				//����
//
//	std::vector<int> face_vertex{};                 //�������ζ�Ӧ�Ķ�������
//	std::vector<int> face_texCoord{};				//�������ζ�Ӧ����������
//	std::vector<int> face_normalLine{};				//�������ζ�Ӧ�ķ�������
//
//	TGAImage diffusemap{};                          //��������ɫ����
//	TGAImage normalmap{};							//��Ȼ����
//	TGAImage specularmap{};							//���淴������
//
//	void load_texture(const std::string filename, const std::string suffix, TGAImage& img);
//
//public:
//
//	Model(const std::string filename);
//
//	int nverts() const;
//	int nfaces() const;
//	vec3 normal(const int iface, const int nthvert) const;   //ÿ�������ε����ķ��߶���
//	vec3 normal(const vec2& uv)const;						 //�ӷ�����ͼ�����л�ȡ��������
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
