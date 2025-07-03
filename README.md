# 通关TinyRenderer

## Lesson（0）前言

在完成对GAMES101课程的学习后，必须意识到的一件事情是，如果只是单纯听完闫老师的二十八个小时课程却毫无个人练习敲代码的话，那不能称之为学习过计算机图形学。

对我来说，时间会显得稍微有些紧张，所以我没有选择跟随完成GAMES101课程的课后作业，而是针对Github上ssloy的wiki [Lesson 0: getting started · ssloy/tinyrenderer Wiki](https://github.com/ssloy/tinyrenderer/wiki/Lesson-0:-getting-started) 来巩固图形学的理解。

在Lesson 0中，ssloy提到，**Only when you suffer through all the tiny details you will learn what is going on**. 只有通过一点点细节的积累，才能说是理解你所想学的东西，才能知道它到底是如何跑起来的。

ssloy贴心提供了关于tgaimage的源文件和头文件，不必去理会它具体的代码是如何实现的，只需要将其当做一个黑箱运行即可，这和本课程并没有太多的关联。

首先，先将完整的代码下载下来，一点点去看一点点去理解（也许是把C++再进一步掌握？至少对我来说是这样），有无法理解的代码就问问AI，差不多熟悉之后就可以试着自己来写源代码了。

## Lesson（1）布雷森汉姆算法（Bresenham’s Line）绘制直线

直线类型确定：

![](G:/Github Projects/TinyRenderer/TinyRenderer对应图像/Bresenham算法绘制直线.jpg)

首先考虑以x0，y0为原点来确定直线的区域，并转换到1区域。

转换以及相关处理：

对于斜率绝对值大于1的部分，直接交换xy（作关于y=x的对称）使其均位于1458，并记录steep=true。

对于在Y轴左侧的部分，直接交换起点和终点即可。

对于在X轴下侧的部分，直接镜像处理，根据y1-y0的正负性来决定y的增减即可。

执行绘制方案：

![](G:/Github Projects/TinyRenderer/TinyRenderer对应图像/绘制方案.jpg)

对于1区域，以x0，y0为原点，通过计算F(x0+1)与(y0+0.5)的值比较（同时以E作为误差统计），即dy/dx*(x0+1)=y0+dy/dx<y0+0.5 => **2dy<dx**，记E=2dy-dx。

现在我们考虑E<0的情况，此时取y=y0，下一次的计算变为dy/dx*(x0+2) = y0+2dy/dx < y0+0.5 => **4dy < dx**  记E_1 = 4dy-dx = E+2dy。也就是说，如果**y轴无移动**，那么误差增加**2dy**。

然后考虑E>0的情况，此时取y = y0+1，下一次计算变为dy/dx*(x0+2) = y0+2dy/dx < y+0.5 =>

**4dy < 3dx** 记E_1 = 4dy-3dx = E+2(dy-dx)。也就是说，如果**y轴有移动**，那么误差增加**2(dy-dx)**。

关于1、8区域的相关代码：

```c++
	int dy =std::abs(y1 - y0);
	int dx = x1 - x0;
	int Error = 2*dy - dx;
	for (int x = x0, y = y0; x < x1; x++)
	{
		Error += 2 * dy;
		if(Error>0) {
			Error -= 2 * dx;
			y1 > y0 ? y++ : y--;
		}
		if (steep) image.set(y, x, color);
		else image.set(x, y, color);
	}
```

那么，关于直线的绘制就完成哩！

下一步，学会读取obj文件。

因为时间关系，干脆就直接copy作者wiki提供的几何与TGAImage、model相关函数了。

也是成功跑出人像。

## Lesson（2）三角形光栅化与背面剔除

今天的任务是绘制一个填充的三角形。

我们先扩展一下Line()方法，让其支持二维顶点输入。这个比较简单，就在这略过了。

测试代码：

```c++
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& img, TGAColor color) {
	Line(t0, t1, img, color);
	Line(t1, t2, img, color);
	Line(t0, t2, img, color);
}

int main(int argc, char** argv) {
	TGAImage image(500, 500, TGAImage::RGB);


	Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
	Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
	Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
	triangle(t0[0], t0[1], t0[2], image, red);
	triangle(t1[0], t1[1], t1[2], image, white);
	triangle(t2[0], t2[1], t2[2], image, green);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}
```

绘制结果：

<img src="G:/Github Projects/TinyRenderer/TinyRenderer对应图像/未填充的三角形.jpg" style="zoom:50%;" />

那么该如何去填充这个三角形呢？容易想到的是，使用一条条横线填充。

横线左右两端点A与B计算如下：

<img src="G:/Github Projects/TinyRenderer/TinyRenderer对应图像/横线法三角形计算.jpg" style="zoom:50%;" />

将输入的三个顶点按y值从大到小排为T2，T1，T0，通过绿色来求出A的坐标，当A的Y小于T1时B坐标使用黄色来求，反之使用粉色来求。

有没有更好一点的方法？事实上，横线法已经属于很古老的办法了，目前基本上是使用重心法来填充。

<img src="G:/Github Projects/TinyRenderer/TinyRenderer对应图像/重心法三角形计算.jpg" style="zoom:50%;" />

基于给定的三个顶点创建一个长方形包围盒，对包围盒内每一个点进行遍历，若该点位于三角形内则渲染，反之则不渲染。关于怎么判断点是否在三角形内，可以采用重心计算。

那什么是重心计算？对于任一点，向量AP= n * AB + m * AC，将其展开，A - P = n * (A-B) + m * (A - C)，合并，即P = (1-m-n) * A + n * B + m * C，由叉积可知：AP x AB 、 BP x BC 、 CP x CA，当结果同向时，P在三角形内。那么你问我...叉乘怎么算？用矩阵啊（没学线性代数导致的）！

反正就是算出来只要满足 1- m - n >0、n>0、m>0就说明P在三角形内部。再分别使用x与y并结合AP(x,y) = n* AB(x,y) + m*AC(x,y)写成矩阵形式，可以发现[m,n,1]同时垂直于AB(x,y)、AC(x,y)、AP(x,y)，所以...让[AB(x),AC(x),PA(x)]与[AB(y),AC(y),PA(y)]进行叉乘，得到结果[a,b,c]，归一化，得到m=a/c，n=b/c。

通过以上方式，我们得到了m，n，对m、n进行判断，便可得知**P是否在三角形内部以及ABC三点对P的占比**（重要！）。

相关代码如下：

```c++
//叉乘计算，结果并不归一化
Vec3f cross(Vec3f t0, Vec3f t1) {
	float x = t0.y * t1.z - t1.y * t0.z;
	float y = t0.z * t1.x -  t0.x * t1.z;
	float z = t0.x * t1.y - t0.y * t1.x;
	return Vec3f(x, y, z);
}

//重心坐标计算，返回的是对ABC三点分别的占比
Vec3f Barycentric(Vec2i* pts, Vec2i P) {

	Vec3f A = Vec3f(pts[0].x, pts[0].y, 0);
	Vec3f B = Vec3f(pts[1].x, pts[1].y, 0);
	Vec3f C = Vec3f(pts[2].x, pts[2].y, 0);

	Vec3f s1 = Vec3f(B.x - A.x, C.x - A.x, A.x-P.x);
	Vec3f s2 = Vec3f(B.y - A.y, C.y - A.y, A.y-P.y);

	Vec3f m = cross(s1, s2);

	if (std::abs(m.z) > 1e-2) {
		return Vec3f(1 - m.x / m.z - m.y / m.z,m.x / m.z, m.y / m.z);
	}
	else {
		return Vec3f(-1, -1, 0);
	}
}

void triangle(Vec2i* pts, TGAImage& img, TGAColor color) {
	//创建包围盒
	Vec2i bboxMin(pts[0].x, pts[0].y);
	Vec2i bboxMax(pts[0].x, pts[0].y);
	
	//计算包围盒
	for (int i = 1; i < 3; i++) {
		bboxMin.x = std::min(bboxMin.x, pts[i].x);
		bboxMin.y = std::min(bboxMin.y, pts[i].y);
		bboxMax.x = std::max(bboxMax.x, pts[i].x);
		bboxMax.y = std::max(bboxMax.y, pts[i].y);
	}


	//对包围盒内所有点进行遍历
	Vec2i P;
	for (P.x = bboxMin.x; P.x <= bboxMax.x; P.x++) {
		for (P.y = bboxMin.y; P.y <= bboxMax.y; P.y++) {
			Vec3f bc_Screen = Barycentric(pts, P);
			if (bc_Screen.x >= 0 && bc_Screen.y >= 0 && bc_Screen.z >= 0)
			{
				img.set(P.x, P.y, color);
			}
		}
	}
}
```

测试代码：

```c++
int main(int argc, char** argv) {
	TGAImage image(500, 500, TGAImage::RGB);

	Model* model = new Model("african_head.obj");

	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		Vec2i screen_coords[3];
		for (int j = 0; j < 3; j++) {
			Vec3f world_coords = model->vert(face[j]);
			screen_coords[j] = Vec2i((world_coords.x + 1.) * width / 2., (world_coords.y + 1.) * height / 2.);
		}
		triangle(screen_coords, image, TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}
```

效果图像：

<img src="G:/Github Projects/TinyRenderer/TinyRenderer对应图像/七彩人头.jpg" style="zoom:50%;" />

太丑了，干脆引入光照吧！

引入平行光(0,0,-1)，再用简单的点乘来决定每个面的光强占比。

计算如图所示：

![](G:/Github Projects/TinyRenderer/TinyRenderer对应图像/光强计算.jpg)

关于法线方向，模型已经给出，此处仅需作简单计算。

```c++
	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i); //当前面缓存,包含该面的顶点索引
		Vec2i screen_coords[3];
		Vec3f world_coords[3];
       //将世界坐标（-1到1之间，所以要先+1）转换为屏幕坐标，此处未采用透视
		for (int j = 0; j < 3; j++)
		{
			Vec3f v = model->vert(face[j]);
			screen_coords[j] = Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);
			world_coords[j] = v;
		}

		//得到法线方向并归一化
		Vec3f n = cross((world_coords[2] - world_coords[0]), (world_coords[1] - world_coords[0]));
		n.normalize();

		//光照强度
		float intensity = n * light_dir;
		if (intensity > 0)
		{
			triangle(screen_coords, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
		}
	}
```

得到图像：

<img src="G:/Github Projects/TinyRenderer/TinyRenderer对应图像/简单光照处理后的图像.jpg" style="zoom:50%;" />

观察可以发现，嘴巴处仍然有问题，这是因为没有做背面剔除（Z-buffer算法）。

## Lesson（3）背面剔除与Z-buffer算法

Z-buffer算法，与画家算法不同，注重的是一个深度图比较。

具体是指为整个渲染屏幕的每个像素点指定一个深度（即z方向的大小），世界坐标z越远离屏幕时，其z值也越小。

在未使用Z-buffer算法时，会出现这样的情况：

A遮挡在B前面，但由于A先进行了光栅化，B后进行了光栅化，导致实际渲染出来的结果是B遮挡在A前面，与设想不符。

使用Z-buffer算法后，就可以免于去讨论上述遮挡的问题，只需要关注好深度的计算即可。

关于Z-buffer缓冲区设定，可以依据屏幕的大小，指定好一个数组：int* zBuffer = new int[width * height];

对于屏幕坐标为(x,y)的像素点，其当前渲染的深度为zBuffer[x+y*width];

每次渲染时比较该渲染点深度和屏幕像素点当前已记录深度，若更浅则更新深度并渲染。



```c++
//使用重心计算当前点的深度并与深度图比较
int zDepth = bc_Screen.x * wts[0].z + bc_Screen.y * wts[1].z + bc_Screen.z * wts[2].z;
if (zDepth > zBuffer[P.x + P.y * width]) {
				zBuffer[P.x + P.y * width] = zDepth;
				TGAColor bc_Color = TGAColor(color.r * bc_Screen.x, color.g * bc_Screen.y, color.b * bc_Screen.z, 255);
				img.set(P.x, P.y, color);
}
```

得到结果图如下：

<img src="G:/Github Projects/TinyRenderer/TinyRenderer对应图像/背面剔除后的图像.jpg" style="zoom: 50%;" />



可以观察到很明显的改善。

## Lesson（4）透视投影与纹理

在前边的渲染中，采用的是正交投影，而我们对世界的观察遵循的是透视投影，为此，需要进行视角变换。

![](G:/Github Projects/TinyRenderer/TinyRenderer对应图像/旋转二维矩阵变换.jpg)

通过一个单位正方形的旋转，我们得到了旋转矩阵：
$$
 \begin{bmatrix} cosθ & -sinθ\\ sinθ & cosθ\\ \end{bmatrix} 
$$
但是二维矩阵只能解决旋转和缩放，为了能实现任意变换，需通过三维矩阵：
$$
\begin{bmatrix} a & b & m\\ c & d & n\\ 0 & 0 & 1 \end{bmatrix} \begin{bmatrix} x\\ y\\ 1\\ \end{bmatrix} = \begin{bmatrix} ax+by+m\\ cx+dy+n\\ 1 \end{bmatrix}
变换矩阵\quad原坐标\quad变换后坐标
$$

$$
满足\begin{bmatrix} x\\ y\\ 1\\ \end{bmatrix}\equiv\begin{bmatrix} 2x\\ 2y\\ 2\\ \end{bmatrix}
$$

最后一项非零的时候，表示为点的坐标；最后一项为零的时候，表示为一个向量。

矩阵仍然满足数学上的向量与坐标运算：
$$
向量+向量 \begin{bmatrix} x_1\\ y_1\\ 0\\ \end{bmatrix}+\begin{bmatrix} x_2\\ y_2\\ 0\\ \end{bmatrix} = \begin{bmatrix} x_1+x_2\\ y_1+y_2\\ 0\\ \end{bmatrix}
\\
向量+坐标\begin{bmatrix} x_1\\ y_1\\ 0\\ \end{bmatrix}+\begin{bmatrix} x_2\\ y_2\\ 1\\ \end{bmatrix} = \begin{bmatrix} x_1+x_2\\ y_1+y_2\\ 1\\ \end{bmatrix}
\\
坐标+坐标\begin{bmatrix} x_1\\ y_1\\ 1\\ \end{bmatrix}+\begin{bmatrix} x_2\\ y_2\\ 1\\ \end{bmatrix} = \begin{bmatrix} x_1+x_2\\ y_1+y_2\\ 2\\ \end{bmatrix}\equiv\begin{bmatrix}\frac{x_1+x_2}{2}\\ \frac{y_1+y_2}{2}\\ 1\\ \end{bmatrix}
$$
将二维矩阵用三维矩阵取代后，最后一行[0 0 1]中，对于前面两个0的用途仍然未知，对此，我们假设不做任何变换，仅仅更改其值：
$$
\begin{bmatrix} 1 & 0 & 0\\ 0 & 1 & 0\\ 2 & 0 & 1 \end{bmatrix} \begin{bmatrix} x\\ y\\ 1\\ \end{bmatrix} = \begin{bmatrix} x\\ y\\ 2x+1 \end{bmatrix}\equiv\begin{bmatrix} \frac{x}{2x+1} \\ \frac{y}{2x+1} \\1 \end{bmatrix}
$$
![](G:/Github Projects/TinyRenderer/TinyRenderer对应图像/二维透视变换.jpg)

可见，随着x的增加，对应的图像在不停变小，这恰好就是透视的表现。

有了二维，那么就可以扩展到三维（对应四维矩阵变换）：
$$
缩放： \begin{bmatrix}  a & 0 & 0 & 0\\  0 & b & 0 & 0\\  0 & 0 & c & 0\\  0 & 0 & 0 & 1\\  \end{bmatrix}  \begin{bmatrix}  x\\  y\\ z\\ 1\\  \end{bmatrix}  =  \begin{bmatrix}  ax\\  by\\ cz\\ 1  \end{bmatrix}
\\
平移： \begin{bmatrix}  1 & 0 & 0 & m\\  0 & 1 & 0 & n\\  0 & 0 & 1 & o\\  0 & 0 & 0 & 1\\  \end{bmatrix}  \begin{bmatrix}  x\\ y\\ z\\ 1\\  \end{bmatrix}  =  \begin{bmatrix}  x+m\\  y+n\\ z+o\\ 1  \end{bmatrix} 
\\
绕y轴旋转： \begin{bmatrix}  cosθ & 0 & sinθ & 0\\  0 & 1 & 0 & 0\\  -sinθ & 0 & cosθ & 0\\  0 & 0 & 0 & 1\\  \end{bmatrix}  \begin{bmatrix}  x\\ y\\ z\\ 1\\  \end{bmatrix}  =  \begin{bmatrix}  cosθ\cdot x+sinθ\cdot z\\  y\\ -sinθ\cdot x+cosθ\cdot z\\ 1  \end{bmatrix}
其余轴类似
\\
透视： \begin{bmatrix}  1 & 0 & 0 & 0\\  0 & 1 & 0 & 0\\  0 & 0 & 1 & 0\\  0 & 0 & r & 1\\  \end{bmatrix}  \begin{bmatrix}  x\\ y\\ z\\ 1\\  \end{bmatrix}  =  \begin{bmatrix}  x\\  y\\ z\\ 1+zr  \end{bmatrix} \equiv \begin{bmatrix}  \frac{x}{1+zr}\\  \frac{y}{1+zr}\\  \frac{z}{1+zr}\\  1  \end{bmatrix}
$$
为什么r有值？我们的摄像机位于z轴上，即我们执行透视的轴是z轴（上述二维透视执行透视的轴是x轴），并且我们需要计算出r的值：

![](G:/Github Projects/TinyRenderer/TinyRenderer对应图像/摄像机r值确定.jpg)

可见，为了实现同一透视变换（对于所有图像的透视终点在位于摄像机时为无限小），对应的透视矩阵如下：
$$
透视矩阵： \begin{bmatrix}  1 & 0 & 0 & 0\\  0 & 1 & 0 & 0\\  0 & 0 & 1 & 0\\  0 & 0 & -\frac{1}{c} & 1\\  \end{bmatrix}
\\
满足投影平面为z=0，摄像机位于(0,0,c)
$$
依照新的齐次坐标变换、透视矩阵变换，对原有geometry.cpp和geometry.h进行更新，此处从略。

```c++
//4d-->3d
//除以最后一个分量（当最后一个分量为0时为向量，不为0时为坐标）
Vec3f m2v(Matrix m) {
	return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

//3d-->4d
//添加1以表示坐标
Matrix v2m(Vec3f v) {
	Matrix m(4, 1);
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1.f;
	return m;
}

//视角变换矩阵
//将物体x，y坐标转换到屏幕坐标
//zbuffer(-1,1)转换到0~255
Matrix viewport(int x, int y, int w, int h) {
	Matrix m = Matrix::identity(4);
	//第4列表示平移信息
	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	m[2][3] = depth / 2.f;
	//对角线表示缩放信息
	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	m[2][2] = depth / 2.f;
	return m;
}
```

```c++
//初始化透视矩阵
Matrix Projection = Matrix::identity(4);
//初始化视角矩阵
Matrix ViewPort   = viewport(width/8, height/8, width*3/4, height*3/4);
//投影矩阵[3][2]=-1/c，c为相机z坐标
Projection[3][2] = -1.f/camera.z;
```

除去透视矩阵外，新定义了视角矩阵，借助缩放和平移从而替代原先如(-1,1)映射到(0,width)的操作。

```c++
//视角矩阵 * 投影矩阵 * 坐标
screen_coords[j] = m2v(ViewPort * Projection * v2m(v));
```

<img src="G:/Github Projects/TinyRenderer/TinyRenderer对应图像/透视后的图像.jpg" style="zoom: 20%;" />

透视后的图像如图所示（相机z坐标为2）。

基于我们使用了视角矩阵，原先的triangle函数也可简化，不再使用世界坐标，并读取纹理：

```c++
void triangle(Vec3i* pts,Vec2i* uvs, TGAImage& img, float intensity,int* zBuffer) {
	//创建包围盒
	Vec2i bboxMin(pts[0].x, pts[0].y);
	Vec2i bboxMax(pts[0].x, pts[0].y);
	
	//计算包围盒
	for (int i = 1; i < 3; i++) {
		bboxMin.x = std::min(bboxMin.x, pts[i].x);
		bboxMin.y = std::min(bboxMin.y, pts[i].y);
		bboxMax.x = std::max(bboxMax.x, pts[i].x);
		bboxMax.y = std::max(bboxMax.y, pts[i].y);
	}


	//对包围盒内所有点进行遍历
	Vec2i P;
	for (P.x = bboxMin.x; P.x <= bboxMax.x; P.x++) {
		for (P.y = bboxMin.y; P.y <= bboxMax.y; P.y++) {
			
			Vec3f bc_Screen = Barycentric(pts, P);
			if (bc_Screen.x >= 0 && bc_Screen.y >= 0 && bc_Screen.z >= 0)
			{
				//使用重心计算当前点的深度并与深度图比较
				int zDepth = bc_Screen.x * pts[0].z + bc_Screen.y * pts[1].z + bc_Screen.z * pts[2].z;

				if (zDepth > zBuffer[P.x + P.y * width]) {
					zBuffer[P.x + P.y * width] = zDepth;

					//获取纹理坐标
					Vec2i uv = uvs[0] * bc_Screen.x + uvs[1] * bc_Screen.y + uvs[2] * bc_Screen.z;
					TGAColor color = model->diffuse(uv);
					//TGAColor color(255, 255, 255, 255);
					TGAColor bc_Color = TGAColor(color.r * intensity, color.g * intensity, color.b * intensity, 255);
					img.set(P.x, P.y, bc_Color);
				}
			}
		}
	}
}
```

<img src="G:/Github Projects/TinyRenderer/TinyRenderer对应图像/添加纹理.jpg" style="zoom:50%;" />

终于不是黑白图像了，可喜可贺。

## Lesson（5）移动视角

在经过透视中矩阵的学习应用后，移动视角就相对简单些了。

逆向思考：摄像机的移动，是不是相对世界绝对参考系而言的？那如果把摄像机变成绝对参考系，那所谓摄像机移动，不就是世界的相对反方向移动吗？

且在线性代数的学习中，我们能够知道，一个矩阵对应一个变换，而矩阵的逆对应着它的反变换。

![](G:/Github Projects/TinyRenderer/TinyRenderer对应图像/摄像机坐标变换.jpg)

旋转后，摄像机的坐标为(x,y,z)，又因为我们是考虑物体的相对移动，故而旋转后是x轴、y轴、z轴发生变化，且摄像机仍看向(0,0,0)，从而可知现在z轴的方向为：(x,y,z)-(0,0,0) = (x,y,z)。

但这并不能求解出剩下两个轴，为此，我们需引入一个up向量，其始终指向原坐标系的上方(0,1,0)。

容易确定的事情是，z轴方向的向量与up向量会处于同一平面，从而有：
$$
\textbf{up}\times \textbf{z} = \textbf{x}
\\
 \textbf{z}\times \textbf{x} = \textbf{y}
\\
前后顺序不可调换
$$
将x轴，y轴，z轴标准化后，得到旋转摄像机的逆矩阵：
$$
M_r^{-1}= \begin{bmatrix}  \hat{\textbf{x}} &\hat{\textbf{y}}&\hat{\textbf{z}} & \textbf{0}\\ 0 & 0 & 0 & 1 \end{bmatrix}^T= \begin{bmatrix}  \hat{\textbf{x}}^T & 0\\ \hat{\textbf{y}}^T & 0\\ \hat{\textbf{z}}^T & 0\\ \textbf{0}& 1 \end{bmatrix}
 \\
  \hat{\textbf{x}}，\hat{\textbf{y}}，\hat{\textbf{z}}
  为x，y，z的单位向量
  \\
  \\
  展开如下
  \\
  M_r^{-1} = 
\begin{bmatrix} 
\hat{\textbf{x}}^T & 0 \\ 
\hat{\textbf{y}}^T & 0 \\ 
\hat{\textbf{z}}^T & 0 \\ 
\textbf{0} & 1 
\end{bmatrix} 
=
\begin{bmatrix}
x_x & x_y & x_z & 0 \\
y_x & y_y & y_z & 0 \\
z_x & z_y & z_z & 0 \\
0 & 0 & 0 & 1
\end{bmatrix}
$$
此外还有平移的逆矩阵（由三维矩阵的二维平移推导得到）：
$$
M_t^{-1}= \begin{bmatrix}  1 & 0 & 0 & center_x\\ 0 & 1 & 0 & center_y\\ 0 & 0 & 1 & center_z\\ 0 & 0 & 0 & 1 \end{bmatrix}^{-1} = \begin{bmatrix}  1 & 0 & 0 & -center_x\\ 0 & 1 & 0 & -center_y\\ 0 & 0 & 1 &- center_z\\ 0 & 0 & 0 & 1 \end{bmatrix}
\\
(center_x,center_y,center_z)为观察坐标
$$
我们只需要将这两个逆矩阵（顺序很重要！摄像机先旋转后平移，则物体先平移后旋转）应用到物体（不是摄像机！）上，就实现了视角的移动。

矩阵实现代码如下：

```c++
//视角移动矩阵
//更改摄像机视角——实质为物体平移与旋转
//摄像机变换为先旋转后平移，故物体变换为先平移后旋转
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
	//计算出z，根据z和up计算出y，再计算出x
	//z = (eye-center).normalize
	//up x z = x
	//z x x = y
	Vec3f z = (eye - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();
	//旋转矩阵
	Matrix rotation = Matrix::identity(4);
	//平移矩阵
	Matrix translation = Matrix::identity(4);

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
	Matrix res = rotation * translation;
	return res;
}
```

现在我们将摄像机坐标设为(2,1,3)，观察中心设为(0,0,1)，得到图像：

<img src="G:/Github Projects/TinyRenderer/TinyRenderer对应图像/摄像机移动后的图像.jpg" style="zoom:50%;" />

非常成功！

现在我们来试试高洛德着色吧！

具体计算出每个三角形顶点的光照强度和距离，用插值来计算渲染时的光照强度（用距离的平方来模拟光的衰减，原因是光的扩散类似球形面积）。

```c++
//计算各顶点光照强度
intensity[j] = model->norm(i, j) * light_dir;

Vec3f new_v = Vec3f(m_v);

//计算各顶点与摄像机距离
distance[j] = std::pow((std::pow(new_v.x - camera.x, 2.0f) + std::pow(new_v.y - camera.y, 2.0f) + std::pow(new_v.z - camera.z, 2.0f)), 0.5f);



//获取顶点光照强度
float ity = intensity[0] * bc_Screen.x + intensity[1] * bc_Screen.y + intensity[2] * bc_Screen.z;
//避免过曝光
ity = std::min(1.f, std::abs(ity) + 0.01f);
//获取顶点距离
float dis = distance[0] * bc_Screen.x + distance[1] * bc_Screen.y + distance[2] * bc_Screen.z;

ity = ity * (25.f / std::pow(dis, 2.0f));
TGAColor color = model->diffuse(uv);
//TGAColor color(255, 255, 255, 255);
TGAColor bc_Color = TGAColor(color.r * ity, color.g * ity, color.b * ity,color.a * ity);
img.set(P.x, P.y, bc_Color);
```

获得图像：

<img src="G:/Github Projects/TinyRenderer/TinyRenderer对应图像/插值着色后的图像.jpg" style="zoom:50%;" />

完美！

马上就要进入收尾战斗了。

## Lesson（6）着色器

现在的代码实在过于臃肿，我们干脆把代码重新整理一下。

一部分为**顶点着色器**，另一部分为**片元着色器**。

```c++
//MVP矩阵
//模型变换、视口变换、透视变换
extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

void viewport(int x, int y, int w, int h);
void projection(float coeff = 0.f);//coeff = -1/c
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader
{
	virtual ~IShader();
	virtual Vec4f vertex(int iface, int nthvert) = 0;
	virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

void triangle(Vec4f* pts, IShader& shader, TGAImage& image, TGAImage& zBuffer);
```

```c++
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

//计算重心坐标
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
```

上述是着色器的公共定义。

下面是不同着色器的代码：

```c++
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
```

以及执行代码：

```c++
int main(int argc, char** argv){
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
	PhongShader shader;
	//实例化Toon着色
	//ToonShader shader;
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
```

挑选Phong着色器着色后模型如下：

<img src="G:/Github Projects/TinyRenderer/TinyRenderer对应图像/Phong着色图像.jpg" style="zoom:33%;" />

挑选Toon着色器着色后模型如下：

<img src="G:/Github Projects/TinyRenderer/TinyRenderer对应图像/Toon着色图像.jpg" style="zoom:33%;" />

至此，本次作业完成！