#include <vector>
#include <iostream>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"


const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0, 255, 0, 255);

const int width = 800;
const int height = 800;
const int depth = 255;

int* zBuffer = new int[width * height];


Model* model = nullptr;

Vec3f light_dir = Vec3f(0, 0, 1);

Vec3f camera = Vec3f(3, 0, 3);

Vec3f center = Vec3f(0, 0, 0);

Vec3f up = Vec3f(0, 1, 0);

//4d-->3d
//除以最后一个分量（当最后一个分量为0时为向量，不为0时为坐标）
//Vec3f m2v(Matrix m) {
//	return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
//}

//3d-->4d
//添加1以表示坐标
//Matrix v2m(Vec3f v) {
//	Matrix m(4, 1);
//	m[0][0] = v.x;
//	m[1][0] = v.y;
//	m[2][0] = v.z;
//	m[3][0] = 1.f;
//	return m;
//}




//叉乘计算，结果并不归一化
//Vec3f cross(Vec3f t0, Vec3f t1) {
//	float x = t0.y * t1.z - t1.y * t0.z;
//	float y = t0.z * t1.x -  t0.x * t1.z;
//	float z = t0.x * t1.y - t0.y * t1.x;
//	return Vec3f(x, y, z);
//}

////重心坐标计算，返回的是对ABC三点分别的占比
//Vec3f Barycentric(Vec3i* pts, Vec2i P) {
//
//	Vec3f A = Vec3f(pts[0].x, pts[0].y, 0);
//	Vec3f B = Vec3f(pts[1].x, pts[1].y, 0);
//	Vec3f C = Vec3f(pts[2].x, pts[2].y, 0);
//
//	Vec3f s1 = Vec3f(B.x - A.x, C.x - A.x, A.x-P.x);
//	Vec3f s2 = Vec3f(B.y - A.y, C.y - A.y, A.y-P.y);
//
//	Vec3f m = cross(s1, s2);
//
//	if (std::abs(m.z) > 1e-2) {
//		return Vec3f(1 - m.x / m.z - m.y / m.z,m.x / m.z, m.y / m.z);
//	}
//	else {
//		return Vec3f(-1, -1, 0);
//	}
//}




//void Line(Vec2i t0,Vec2i t1, TGAImage& image, const TGAColor color) {
//	
//	int x0 = t0.x;
//	int x1 = t1.x;
//	int y0 = t0.y;
//	int y1 = t1.y;
//
//
//	bool steep = false;
//	
//	//将slope绝对值大于1转为slope小于1
//	if (std::abs(y1 - y0) > std::abs(x1 - x0)) {
//		std::swap(x1, y1);
//		std::swap(x0, y0);
//		steep = true;
//	}
//	if (x1 < x0) {
//		std::swap(x0, x1);
//		std::swap(y0, y1);
//	}
//
//
//	//假设此时已经转换为18区域
//	int dy =std::abs(y1 - y0);
//	int dx = x1 - x0;
//	int Error = 2*dy - dx;
//	for (int x = x0, y = y0; x < x1; x++)
//	{
//		Error += 2 * dy;
//		if(Error>0) {
//			Error -= 2 * dx;
//			y1 > y0 ? y++ : y--;
//		}
//		if (steep) image.set(y, x, color);
//		else image.set(x, y, color);
//	}
//}


////绘制三角形
////顶点组，纹理坐标组，顶点光照强度组，顶点距离组，图片，zBuffer组
//void triangle(Vec3i* pts,Vec2i* uvs,float* intensity,float* distance,  TGAImage& img, int* zBuffer) {
//	//创建包围盒
//	Vec2i bboxMin(pts[0].x, pts[0].y);
//	Vec2i bboxMax(pts[0].x, pts[0].y);
//	
//	//计算包围盒
//	for (int i = 1; i < 3; i++) {
//		bboxMin.x = std::min(bboxMin.x, pts[i].x);
//		bboxMin.y = std::min(bboxMin.y, pts[i].y);
//		bboxMax.x = std::max(bboxMax.x, pts[i].x);
//		bboxMax.y = std::max(bboxMax.y, pts[i].y);
//	}
//
//
//	//对包围盒内所有点进行遍历
//	Vec2i P;
//	for (P.x = bboxMin.x; P.x <= bboxMax.x; P.x++) {
//		for (P.y = bboxMin.y; P.y <= bboxMax.y; P.y++) {
//			
//			Vec3f bc_Screen = Barycentric(pts, P);
//			if (bc_Screen.x >= 0 && bc_Screen.y >= 0 && bc_Screen.z >= 0)
//			{
//				//使用重心计算当前点的深度并与深度图比较
//				int zDepth = bc_Screen.x * pts[0].z + bc_Screen.y * pts[1].z + bc_Screen.z * pts[2].z;
//
//				if (zDepth > zBuffer[P.x + P.y * width]) {
//					zBuffer[P.x + P.y * width] = zDepth;
//
//					//获取纹理坐标
//					Vec2i uv = uvs[0] * bc_Screen.x + uvs[1] * bc_Screen.y + uvs[2] * bc_Screen.z;
//					//获取顶点光照强度
//					float ity = intensity[0] * bc_Screen.x + intensity[1] * bc_Screen.y + intensity[2] * bc_Screen.z;
//					//避免过曝光
//					ity = std::min(1.f, std::abs(ity) + 0.01f);
//					//获取顶点距离
//					float dis = distance[0] * bc_Screen.x + distance[1] * bc_Screen.y + distance[2] * bc_Screen.z;
//
//					ity = ity * (25.f / std::pow(dis, 2.0f));
//					TGAColor color = model->diffuse(uv);
//					//TGAColor color(255, 255, 255, 255);
//					TGAColor bc_Color = TGAColor(color.r * ity, color.g * ity, color.b * ity,color.a * ity);
//					img.set(P.x, P.y, bc_Color);
//
//				}
//			}
//		}
//	}
//}

//视角变换矩阵
//将物体x，y坐标转换到屏幕坐标
//zbuffer(-1,1)转换到0~255
//Matrix viewport(int x, int y, int w, int h) {
//	Matrix m = Matrix::identity(4);
//	//第4列表示平移信息
//	m[0][3] = x + w / 2.f;
//	m[1][3] = y + h / 2.f;
//	m[2][3] = depth / 2.f;
//	//对角线表示缩放信息
//	m[0][0] = w / 2.f;
//	m[1][1] = h / 2.f;
//	m[2][2] = depth / 2.f;
//	return m;
//}


//视角移动矩阵
//更改摄像机视角——实质为物体平移与旋转
//摄像机变换为先旋转后平移，故物体变换为先平移后旋转
//Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
//	//计算出z，根据z和up计算出y，再计算出x
//	//z = (eye-center).normalize
//	//up x z = x
//	//z x x = y
//	Vec3f z = (eye - center).normalize();
//	Vec3f x = cross(up, z).normalize();
//	Vec3f y = cross(z, x).normalize();
//	//旋转矩阵
//	Matrix rotation = Matrix::identity(4);
//	//平移矩阵
//	Matrix translation = Matrix::identity(4);
//
//	//构造逆平移矩阵
//	for (int i = 0; i < 3; i++) {
//		translation[i][3] = -center[i];
//	}
//	//构造逆旋转矩阵
//	for (int i = 0; i < 3; i++) {
//		rotation[0][i] = x[i];
//		rotation[1][i] = y[i];
//		rotation[2][i] = z[i];
//	}
//	//构造平移旋转矩阵并返回，注意矩阵乘法的顺序为从右向左
//	Matrix res = rotation * translation;
//	return res;
//}

//标准的高洛德着色器
struct GouraudShader:public IShader{
	//顶点着色器会将数据写入varying_intensity
	//片元着色器从varying_intensity中读取数据
	Vec3f varying_intensity;
	mat<2, 3, float>varying_uv;
	//接受两个变量（面序号与顶点序号）
	virtual Vec4f vertex(int iface, int nthvert) {
		//根据面序号和顶点序号读取模型对应顶点，并扩展到四维
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		//变换顶点坐标到屏幕坐标（视角矩阵 * 投影矩阵 * 变换矩阵 * v）
		mat<4, 4, float>uniform_M = Projection * ModelView;
		mat<4, 4, float> uniform_MIT = ModelView.invert_transpose();
		gl_Vertex = Viewport * uniform_M * gl_Vertex;
		//计算光照强度（顶点法向量*光照方向）
		Vec3f normal = proj<3>(embed<4>(model->norm(iface, nthvert))).normalize();
		varying_intensity[nthvert] = std::max(0.f, model->norm(iface, nthvert) * light_dir);
		return gl_Vertex;
	}
	//根据传入的重心坐标（bar）、颜色和varying_intensity计算出当前像素的颜色
	virtual bool fragment(Vec3f bar, TGAColor& color) {
		Vec2f uv = varying_uv * bar;
		TGAColor c = model->diffuse(uv);
		float intensity = varying_intensity * bar;
		color = c * intensity;
		return false;
	}
};

//将一定阈值内的光照强度替换的着色器
struct ToonShader : public IShader {
	mat<3, 3, float>varying_tri;
	Vec3f varying_ity;

	virtual ~ToonShader() {}

	virtual Vec4f vertex(int iface, int nthvert) {
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
		gl_Vertex = Projection * ModelView * gl_Vertex;
		varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));

		varying_ity[nthvert] = model->norm(iface, nthvert) * light_dir;

		gl_Vertex = Viewport * gl_Vertex;
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color) {
		float intensity = varying_ity * bar;
		if (intensity > .85)intensity = 1;
		else if (intensity > .60)intensity = .80;
		else if (intensity > .45)intensity = .60;
		else if (intensity > .30)intensity = .45;
		else if (intensity > .15)intensity = .30;
		color = TGAColor(255, 155, 0) * intensity;
		return false;
	}
};

//不对法向量插值，法向量来源于三角形边的叉积的着色器
struct FlatShader : public IShader{
	//三个顶点的信息
	mat<3, 3, float>varying_tri;

	virtual ~FlatShader(){}

	virtual Vec4f vertex(int iface, int nthvert) {
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
		gl_Vertex = Projection * ModelView * gl_Vertex;
		varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
		gl_Vertex = Viewport * gl_Vertex;
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color) {
		Vec3f n = cross(varying_tri.col(1) - varying_tri.col(0), varying_tri.col(2) - varying_tri.col(0)).normalize();
		float intensity = n * light_dir;
		color = TGAColor(255, 255, 255) * intensity;
		return false;
	}
};

//Phong氏着色的着色器
struct PhongShader : public IShader{
	mat<2, 3, float> varying_uv;
	mat<4, 4, float>uniform_M = Projection * ModelView;
	mat<4, 4, float>uniform_MIT = ModelView.invert_transpose();

	virtual ~PhongShader(){}

	virtual Vec4f vertex(int iface, int nthvert) {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
		return Viewport * Projection * ModelView * gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color) {
		Vec2f uv = varying_uv * bar;
		Vec3f n = proj<3>(uniform_MIT * embed<4>(model->norm(uv))).normalize();
		Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();
		Vec3f r = (n * (n * l * .2f) - l).normalize(); //反射光
		float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
		float diff = std::max(0.f, n * l);
		TGAColor c = model->diffuse(uv);
		color = c;
		for (int i = 0; i < 3; i++) {
			color[i] = std::min<float>(5 + c[i] * (diff + .6 * spec), 255);
		}
		return false;
	}
};





int main(int argc, char** argv) {


	////初始化zBuffer
	//for (int i = 0; i < width * height; i++) {
	//	zBuffer[i] == INT16_MIN;
	//}

	////读取模型
	//if (2 == argc) {
	//	model = new Model(argv[1]);
	//}
	//else {
	//	model = new Model("african_head.obj");
	//}

	////初始化透视矩阵
	//Matrix Projection = Matrix::identity(4);
	////初始化视角矩阵
	//Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	////投影矩阵[3][2]=-1/c，c为相机z坐标
	//Projection[3][2] = -1.f / camera.z;

	////模型变换矩阵(摄像机移动矩阵)
	//Matrix ModelView = lookat(camera, center, up);



	//for (int i = 0; i < model->nfaces(); i++)
	//{
	//	std::vector<int> face = model->face(i); //当前面缓存,包含该面的顶点索引
	//	Vec3i screen_coords[3];
	//	//Vec3f world_coords[3];
	//	//将世界坐标（-1到1之间，所以要先+1）转换为屏幕坐标，此处未采用透视

	//	float intensity[3];
	//	float distance[3];

	//	for (int j = 0; j < 3; j++)
	//	{
	//		Vec3f v = model->vert(face[j]);

	//		//screen_coords[j] = Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);

	//		//视角矩阵 * 投影矩阵 * 坐标（从右向左）
	//		//screen_coords[j] = m2v(ViewPort * Projection * v2m(v));


	//		//Matrix(v)的作用是将v的坐标转换为1x4的矩阵坐标
	//		Matrix m_v = ModelView * Matrix(v);

	//		screen_coords[j] = Vec3f(ViewPort * Projection * m_v);

	//		//计算各顶点光照强度
	//		intensity[j] = model->norm(i, j) * light_dir;

	//		Vec3f new_v = Vec3f(m_v);

	//		//计算各顶点与摄像机距离
	//		distance[j] = std::pow((std::pow(new_v.x - camera.x, 2.0f) + std::pow(new_v.y - camera.y, 2.0f) + std::pow(new_v.z - camera.z, 2.0f)), 0.5f);

	//		//处理摄像机移动
	//		//world_coords[j] = v;
	//	}

	//	//得到法线方向并归一化
	//	//Vec3f n = cross((world_coords[2] - world_coords[0]), (world_coords[1] - world_coords[0]));
	//	//n.normalize();

	//	//光照强度
	//	//float intensity = n * light_dir;
	//	//intensity = std::min(std::abs(intensity), 1.f);
	//	//if (intensity > 0)
	//	{

	//		Vec2i uv[3];
	//		for (int k = 0; k < 3; k++) {
	//			uv[k] = model->uv(i, k);
	//		}

	//		triangle(screen_coords,uv,intensity,distance, image,zBuffer);
	//	}
	//}

	//image.flip_vertically(); //翻转，以左下角为坐标原点
	//image.write_tga_file("output.tga");
	//return 0;




	//加载模型
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/african_head/african_head.obj");
	}
	//初始化变换矩阵、投影矩阵、视角矩阵
	lookat(camera, center, up);
	projection(-1.f / (camera - center).norm());
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	light_dir.normalize();
	//初始化image和zBuffer
	TGAImage image(800, 800, TGAImage::RGB);
	TGAImage zBuffer(width, height, TGAImage::GRAYSCALE);
	//实例化高洛德着色
	//GouraudShader shader;
	//实例化布林峰（Phong）着色
	//PhongShader shader;
	//实例化Toon着色
	ToonShader shader;
	//实例化Flat着色
	//FlatShader shader;
	//以模型面作为循环控制量
	for (int i = 0; i < model->nfaces(); i++) {
		Vec4f screen_coords[3];
		for (int j = 0; j < 3; j++) {
			//通过顶点着色器读取模型顶点
			//变换顶点坐标到屏幕坐标（视角矩阵*投影矩阵*变换矩阵*v） **并非真正屏幕坐标（没有除以最后一个分量）
			//计算光照强度
			screen_coords[j] = shader.vertex(i, j);
		}

		//遍历完三个顶点后，一个三角形的光栅化完成
		//通过片元着色器绘制并着色三角形
		triangle(screen_coords, shader, image, zBuffer);
	}

	image.flip_vertically();
	zBuffer.flip_vertically();
	image.write_tga_file("output.tga");
	zBuffer.write_tga_file("zBuffer.tga");

	delete model;
	return 0;
}

