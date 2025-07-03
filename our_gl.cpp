#include <cmath>
#include <limits>
#include <cstdlib>
#include "our_gl.h"
#include <algorithm>
Matrix ModelView;
Matrix Viewport;
Matrix Projection;

IShader::~IShader() {}

//视口变换矩阵
void viewport(int x, int y, int w, int h) {
	Viewport = Matrix::identity();
	//第4列表示平移信息
	Viewport[0][3] = x + w / 2.f;
	Viewport[1][3] = y + h / 2.f;
	Viewport[2][3] = 255 / 2.f;
	//对角线表示缩放信息
	Viewport[0][0] = w / 2.f;
	Viewport[1][1] = h / 2.f;
	Viewport[2][2] = 255 / 2.f;
}

//透视投影矩阵
void projection(float coeff) {
	Projection = Matrix::identity();
	Projection[3][2] = coeff;
}

//物体变换矩阵
void lookat(Vec3f eye, Vec3f center, Vec3f up) {
	//计算出z，根据z和up计算出y，再计算出x
	//z = (eye-center).normalize
	//up x z = x
	//z x x = y
	Vec3f z = (eye - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();
	//旋转矩阵
	Matrix rotation = Matrix::identity();
	//平移矩阵
	Matrix translation = Matrix::identity();

	//构造逆平移矩阵
	for (int i = 0; i < 3; i++) {
		translation[i][3] = -center[i];
	}
	//构造逆旋转矩阵
	for (int i = 0; i < 3; i++) {
		rotation[0][i] = x[i];
		rotation[1][i] = y[i];
		rotation[2][i] = z[i];
	}
	//构造平移旋转矩阵并返回，注意矩阵乘法的顺序为从右向左
	ModelView = rotation * translation;
}

//重心坐标计算，返回的是对ABC三点分别的占比
//Vec3f Barycentric(Vec3i* pts, Vec2i P) {
//
//	Vec3f A = Vec3f(pts[0].x, pts[0].y, 0);
//	Vec3f B = Vec3f(pts[1].x, pts[1].y, 0);
//	Vec3f C = Vec3f(pts[2].x, pts[2].y, 0);
//
//	Vec3f s1 = Vec3f(B.x - A.x, C.x - A.x, A.x - P.x);
//	Vec3f s2 = Vec3f(B.y - A.y, C.y - A.y, A.y - P.y);
//
//	Vec3f m = cross(s1, s2);
//
//	if (std::abs(m.z) > 1e-2) {
//		return Vec3f(1 - m.x / m.z - m.y / m.z, m.x / m.z, m.y / m.z);
//	}
//	else {
//		return Vec3f(-1, -1, 0);
//	}
//}

//计算质心坐标
Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P) {
	Vec3f s[2];
	for (int i = 2; i--; ) {
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}
	Vec3f u = cross(s[0], s[1]);
	if (std::abs(u[2]) > 1e-2)
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1);
}



//三角形绘制
//绘制三角形
//顶点组，纹理坐标组，顶点光照强度组，顶点距离组，图片，zBuffer组
void triangle(Vec4f* pts, IShader& shader, TGAImage& image, TGAImage& zBuffer) {
	//创建包围盒
	//Vec2i bboxMin(pts[0].x, pts[0].y);
	//Vec2i bboxMax(pts[0].x, pts[0].y);

	////计算包围盒
	//for (int i = 1; i < 3; i++) {
	//	bboxMin.x = std::min(bboxMin.x, pts[i].x);
	//	bboxMin.y = std::min(bboxMin.y, pts[i].y);
	//	bboxMax.x = std::max(bboxMax.x, pts[i].x);
	//	bboxMax.y = std::max(bboxMax.y, pts[i].y);
	//}

	//初始化包围盒
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			//这里pts除以了最后一个分量，实现了透视中的缩放，所以作为边界框
			bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
			bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
		}
	}


	//对包围盒内所有点进行遍历
	//Vec2i P;
	//for (P.x = bboxMin.x; P.x <= bboxMax.x; P.x++) {
	//	for (P.y = bboxMin.y; P.y <= bboxMax.y; P.y++) {

	//		Vec3f bc_Screen = Barycentric(pts, P);
	//		if (bc_Screen.x >= 0 && bc_Screen.y >= 0 && bc_Screen.z >= 0)
	//		{
	//			//使用重心计算当前点的深度并与深度图比较
	//			int zDepth = bc_Screen.x * pts[0].z + bc_Screen.y * pts[1].z + bc_Screen.z * pts[2].z;

	//			if (zDepth > zBuffer[P.x + P.y * width]) {
	//				zBuffer[P.x + P.y * width] = zDepth;

	//				//获取纹理坐标
	//				Vec2i uv = uvs[0] * bc_Screen.x + uvs[1] * bc_Screen.y + uvs[2] * bc_Screen.z;
	//				//获取顶点光照强度
	//				float ity = intensity[0] * bc_Screen.x + intensity[1] * bc_Screen.y + intensity[2] * bc_Screen.z;
	//				//避免过曝光
	//				ity = std::min(1.f, std::abs(ity) + 0.01f);
	//				//获取顶点距离
	//				float dis = distance[0] * bc_Screen.x + distance[1] * bc_Screen.y + distance[2] * bc_Screen.z;

	//				ity = ity * (25.f / std::pow(dis, 2.0f));
	//				TGAColor color = model->diffuse(uv);
	//				//TGAColor color(255, 255, 255, 255);
	//				TGAColor bc_Color = TGAColor(color.r * ity, color.g * ity, color.b * ity, color.a * ity);
	//				img.set(P.x, P.y, bc_Color);

	//			}
	//		}
	//	}
	//}

	//当前像素坐标P、颜色color
	Vec2i P;
	TGAColor color;

	//遍历包围盒
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {

			//c为当前P对应的质心坐标
			//这里pts除以了最后一个分量，实现了透视中的缩放，所以用于判断P是否在三角形内
			Vec3f c = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), proj<2>(P));

			//插值计算P的zbuffer
			//pts[i]为三角形的三个顶点
			//pts[i][2]为三角形的z信息(0~255)
			//pts[i][3]为三角形的投影系数(1-z/c)
			float z_P = (pts[0][2] / pts[0][3]) * c.x + (pts[0][2] / pts[1][3]) * c.y + (pts[0][2] / pts[2][3]) * c.z;

			int frag_depth = std::max(0, std::min(255, int(z_P + .5)));

			//P的任一质心分量小于0或者zbuffer小于已有zbuffer，不渲染
			if (c.x < 0 || c.y < 0 || c.z<0 || zBuffer.get(P.x, P.y)[0]>frag_depth) continue;
			//调用片元着色器计算当前像素颜色
			bool discard = shader.fragment(c, color);
			if (!discard) {
				//zbuffer
				zBuffer.set(P.x, P.y, TGAColor(frag_depth));
				//为像素设置颜色
				image.set(P.x, P.y, color);
			}
		}
	}
}