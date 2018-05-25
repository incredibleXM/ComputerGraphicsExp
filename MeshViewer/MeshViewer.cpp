#include <string.h>
#include <stdlib.h>
#include <vector>
#include <GL/glui.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "common.h"


#ifndef OBJ_CLASS
#define OBJ_CLASS

struct Vector3;
Vector3 operator + (const Vector3& one, const Vector3& two);
Vector3 operator - (const Vector3& one, const Vector3& two);
Vector3 operator * (const Vector3& one, double scale);
Vector3 operator / (const Vector3& one, double scale);
Vector3 Cross(Vector3& one, Vector3& two);

struct Vector3
{
	double fX;
	double fY;
	double fZ;
	Vector3(double x = 0.0, double y = 0.0, double z = 0.0) : fX(x), fY(y), fZ(z) {}
	Vector3 operator +=(const Vector3& v) { return *this = *this + v; }
	double Length() { return sqrt(fX * fX + fY * fY + fZ * fZ); }
	void Normalize()//归一化
	{
		double fLen = Length();
		if (fabs(fLen) > 1e-6)
		{
			fX /= fLen;
			fY /= fLen;
			fZ /= fLen;
		}
	}
};

struct Point
{
	Vector3 pos;
	Vector3 normal;
};

struct Face
{
	int pts[3];
	Vector3 normal;
};

class CObj
{
public:
	CObj(void);
	~CObj(void);

	std::vector<Point> m_pts; //顶点
	std::vector<Face> m_faces;//面

public:
	bool ReadObjFile(const char* pcszFileName);//读入模型文件

private:
	void UnifyModel();//单位化模型
	void ComputeFaceNormal(Face& f);//计算面的法线
};

using std::min;
using std::max;

Vector3 operator + (const Vector3& one, const Vector3& two) //两个向量相加
{
	return Vector3(one.fX + two.fX, one.fY + two.fY, one.fZ + two.fZ);
}

Vector3 operator - (const Vector3& one, const Vector3& two) //两个向量相减
{
	return Vector3(one.fX - two.fX, one.fY - two.fY, one.fZ - two.fZ);
}

Vector3 operator * (const Vector3& one, double scale) //向量与数的乘操作
{
	return Vector3(one.fX * scale, one.fY * scale, one.fZ * scale);
}

Vector3 operator / (const Vector3& one, double scale) //向量与数的除操作
{
	return one * (1.0 / scale);
}

Vector3 Cross(Vector3& one, Vector3& two)
{//计算两个向量的叉积
	Vector3 vCross;

	vCross.fX = ((one.fY * two.fZ) - (one.fZ * two.fY));
	vCross.fY = ((one.fZ * two.fX) - (one.fX * two.fZ));
	vCross.fZ = ((one.fX * two.fY) - (one.fY * two.fX));

	return vCross;
}

CObj::CObj(void)
{
}


CObj::~CObj(void)
{
}

bool CObj::ReadObjFile(const char* pcszFileName)
{//读取模型文件

	FILE* fpFile = fopen(pcszFileName, "r"); //以只读方式打开文件
	if (fpFile == NULL)
	{
		return false;
	}

	m_pts.clear();
	m_faces.clear();

	//TODO：将模型文件中的点和面数据分别存入m_pts和m_faces中
	const int strLength = 256;
	char dat[strLength];
	Point v;
	Face f;
	while(fgets(dat, strLength, fpFile))
	{
		std::stringstream input(dat);
		char type[3];

		input >> type;
		if(type[0] == 'v')
		{
			input >> v.pos.fX >> v.pos.fY >> v.pos.fZ;
			m_pts.push_back(v);
		}
		else if(type[0] == 'f')
		{
			input >> f.pts[0] >> f.pts[1] >> f.pts[2];
			for(int i=0; i<3; i++) f.pts[i]--;
			m_faces.push_back(f);
		}

	}

	fclose(fpFile);

	UnifyModel(); //将模型归一化
	for(auto& x: m_faces) ComputeFaceNormal(x);
	for(auto& x: m_pts) x.normal.Normalize();
	return true;
}

void CObj::UnifyModel()
{//为统一显示不同尺寸的模型，将模型归一化，将模型尺寸缩放到0.0-1.0之间
//原理：找出模型的边界最大和最小值，进而找出模型的中心
//以模型的中心点为基准对模型顶点进行缩放
//TODO:添加模型归一化代码
	Vector3 Max, Min;
	Max = Min = m_pts[0].pos;
	for(int i=1; i<m_pts.size(); i++)
	{
		Min.fX = min(Min.fX, m_pts[i].pos.fX);
		Min.fY = min(Min.fY, m_pts[i].pos.fY);
		Min.fZ = min(Min.fZ, m_pts[i].pos.fZ);

		Max.fX = max(Max.fX, m_pts[i].pos.fX);
		Max.fY = max(Max.fY, m_pts[i].pos.fY);
		Max.fZ = max(Max.fZ, m_pts[i].pos.fZ);
	}

	Vector3 center = (Max + Min) * 0.5;

	Vector3 tmp = Max - Min;
	double scale = tmp.Length();
	scale = 10.0 / scale;
	for(int i=0; i<m_pts.size(); i++)
	{
		Vector3& pos = m_pts[i].pos;
		pos = (pos - center) * scale;
	}
}

void CObj::ComputeFaceNormal(Face& f)
{//TODO:计算面f的法向量，并保存
	Vector3 v1 = m_pts[f.pts[1]].pos - m_pts[f.pts[0]].pos;
	Vector3 v2 = m_pts[f.pts[2]].pos - m_pts[f.pts[1]].pos;
	Vector3 vn = Cross(v1, v2);
	for(int i=0; i<3; i++) m_pts[f.pts[i]].normal += vn;
	f.normal = vn;
	f.normal.Normalize();
}

#endif


int g_xform_mode = TRANSFORM_NONE;
int   g_main_window;
double g_windows_width, g_windows_height;

//定义两个纹理对象编号
GLuint texGround;
#define BMP_Header_Length 54  //图像数据在内存块中的偏移量

CObj g_obj;
Display *dpy;
//the lighting
static GLfloat g_light0_ambient[] =  {0.0f, 0.0f, 0.0f, 1.0f};//环境光
static GLfloat g_light0_diffuse[] =  {1.0f, 1.0f, 1.0f, 1.0f};//散射光
static GLfloat g_light0_specular[] = {1.0f,1.0f,1.0f,1.0f}; //镜面光
static GLfloat g_light0_position[] = {0.0f, 0.0f, 100.0f, 0.0f};//光源的位置。第4个参数为1，表示点光源；第4个参数量为0，表示平行光束{0.0f, 0.0f, 10.0f, 0.0f}

static GLfloat g_material[] = {0.96f, 0.8f, 0.69f, 1.0f};//材质
static GLfloat g_rquad = 0;

static float g_x_offset   = 0.0;
static float g_y_offset   = 0.0;
static float g_z_offset   = 0.0;
static float g_scale_size = 1;
static int  g_press_x; //鼠标按下时的x坐标
static int  g_press_y; //鼠标按下时的y坐标
static float g_x_angle = 0.0;
static float g_y_angle = 0.0;

const int n = 1000;
const GLfloat R = 0.5f;
const GLfloat Pi = 3.1415926536f;
int g_view_type = VIEW_FLAT;
int g_draw_content = SHAPE_TRIANGLE;

// 函数power_of_two用于判断一个整数是不是2的整数次幂
int power_of_two(int n)
{
    if( n <= 0 )
        return 0;
    return (n & (n-1)) == 0;
}

/* 函数load_texture
* 读取一个BMP文件作为纹理
* 如果失败，返回0，如果成功，返回纹理编号
*/
GLuint load_texture(const char* file_name)
{
    GLint width, height, total_bytes;
    GLubyte* pixels = 0;
    GLuint last_texture_ID=0, texture_ID = 0;

    // 打开文件，如果失败，返回
    FILE* pFile = fopen(file_name, "rb");
    if( pFile == 0 )
        return 0;

    // 读取文件中图象的宽度和高度
    fseek(pFile, 0x0012, SEEK_SET);
    fread(&width, 4, 1, pFile);
    fread(&height, 4, 1, pFile);
    fseek(pFile, BMP_Header_Length, SEEK_SET);

    // 计算每行像素所占字节数，并根据此数据计算总像素字节数
    {
        GLint line_bytes = width * 3;
        while( line_bytes % 4 != 0 )
            ++line_bytes;
        total_bytes = line_bytes * height;
    }

    // 根据总像素字节数分配内存
    pixels = (GLubyte*)malloc(total_bytes);
    if( pixels == 0 )
    {
        fclose(pFile);
        return 0;
    }

    // 读取像素数据
    if( fread(pixels, total_bytes, 1, pFile) <= 0 )
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    // 对就旧版本的兼容，如果图象的宽度和高度不是的整数次方，则需要进行缩放
    // 若图像宽高超过了OpenGL规定的最大值，也缩放
    {
        GLint max;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
        if( !power_of_two(width)
            || !power_of_two(height)
            || width > max
            || height > max )
        {
            const GLint new_width = 256;
            const GLint new_height = 256; // 规定缩放后新的大小为边长的正方形
            GLint new_line_bytes, new_total_bytes;
            GLubyte* new_pixels = 0;

            // 计算每行需要的字节数和总字节数
            new_line_bytes = new_width * 3;
            while( new_line_bytes % 4 != 0 )
                ++new_line_bytes;
            new_total_bytes = new_line_bytes * new_height;

            // 分配内存
            new_pixels = (GLubyte*)malloc(new_total_bytes);
            if( new_pixels == 0 )
            {
                free(pixels);
                fclose(pFile);
                return 0;
            }

            // 进行像素缩放
            gluScaleImage(GL_RGB,
                width, height, GL_UNSIGNED_BYTE, pixels,
                new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

            // 释放原来的像素数据，把pixels指向新的像素数据，并重新设置width和height
            free(pixels);
            pixels = new_pixels;
            width = new_width;
            height = new_height;
        }
    }

    // 分配一个新的纹理编号
    glGenTextures(1, &texture_ID);
    if( texture_ID == 0 )
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    // 绑定新的纹理，载入纹理并设置纹理参数
    // 在绑定前，先获得原来绑定的纹理编号，以便在最后进行恢复
    GLint lastTextureID=last_texture_ID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureID);
    glBindTexture(GL_TEXTURE_2D, texture_ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
        GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, lastTextureID);  //恢复之前的纹理绑定
    free(pixels);
    return texture_ID;
}


void DrawTriangle()
{//绘制三角形
	glBegin(GL_TRIANGLES);
		glNormal3f(0.0f, 0.0f, 1.0f);  //指定面法向
		glVertex3f( 0.0f, 1.0f, 0.0f);                    // 上顶点
		glVertex3f(-1.0f,-1.0f, 0.0f);                    // 左下
		glVertex3f( 1.0f,-1.0f, 0.0f);                    // 右下
	glEnd();
}

void DrawCube()
{//绘制立方体

	glBegin(GL_QUADS);
		glNormal3f( 0.0f, 0.0f, 1.0f);  //指定面法向
		glVertex3f( 1.0f, 1.0f,1.0f);   //列举面顶点数据，逆时针顺序
		glVertex3f(-1.0f, 1.0f, 1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
	//前----------------------------
		glNormal3f( 0.0f, 0.0f,-1.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);
	//后----------------------------
		glNormal3f( 0.0f, 1.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);
	//上----------------------------
		glNormal3f( 0.0f,-1.0f, 0.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
	//下----------------------------
		glNormal3f( 1.0f, 0.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);
	//右----------------------------
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);
	//左----------------------------*/
	glEnd();
}

void DrawCircle()
{//绘制圆
	glBegin(GL_POLYGON);
		glNormal3f(0.0f, 0.0f, 1.0f);
		for(int i=0; i<n; ++i)
			glVertex2f(R*cos(2*Pi/n*i), R*sin(2*Pi/n*i));
	glEnd();
}

void DrawCylinder()
{
	glBegin(GL_TRIANGLES); // 上面的圆
	    glNormal3f(0.0f, 0.0f, 1.0f);

	    for(int i=0; i<n; i++)
	    {
	        float p = 2*Pi/n*i;
	        glVertex3f(R*cos(p), R*sin(p), 1.0f);
			glVertex3f(0, 0, 1.0f);
			p = 2*Pi/n*(i+1);
	        glVertex3f(R*cos(p), R*sin(p), 1.0f);
	    }
    glEnd();

	glBegin(GL_TRIANGLES); // 下面的圆
	    glNormal3f(0.0f, 0.0f, -1.0f);
	    for(int i=0; i<n; i++)
	    {
	        float p = 2*Pi/n*i;
	        glVertex3f(R*cos(p), R*sin(p), -1.0f);
			p = 2*Pi/n*(i+1);
	        glVertex3f(R*cos(p), R*sin(p), -1.0f);
			glVertex3f(0, 0, 0);
	    }
    glEnd();

	glBindTexture(GL_TEXTURE_2D, texGround);
	glBegin(GL_QUADS);
		for(int i=0; i<n; i++)
		{
			float p = 2*Pi/n*i;
			glNormal3f(R*cos(p), R*sin(p), 0.0f);
			glTexCoord2f((float)i/n, 0.0);
			glVertex3f(R*cos(p), R*sin(p), -1.0f);
			glTexCoord2f((float)i/n, 1.0);
			glVertex3f(R*cos(p), R*sin(p), 1.0f);
			p = 2*Pi/n*(i+1);
			glTexCoord2f((float)(i+1)/n, 1.0);
			glVertex3f(R*cos(p), R*sin(p), 1.0f);
			glTexCoord2f((float)(i+1)/n, 0.0);
			glVertex3f(R*cos(p), R*sin(p), -1.0f);
		}
	glEnd();
}

void DrawCone()
{
	float h = 2.0f;

	glBegin(GL_TRIANGLES);
	    glNormal3f(0.0f, 0.0f, -1.0f);
	    for(int i=0; i<n; i++)
	    {
	        float p = 2*Pi/n*i;
	        glVertex3f(R*cos(p), R*sin(p), 0);
			p = 2*Pi/n*(i+1);
	        glVertex3f(R*cos(p), R*sin(p), 0);
			glVertex3f(0, 0, 0);
	    }
    glEnd();

	glBindTexture(GL_TEXTURE_2D, texGround);
	float tmp = R*sqrt(2)/2;
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(-tmp, -tmp, 0.0f);
		glTexCoord2f(1.0, 0.0); glVertex3f(tmp, -tmp, 0.0f);
		glTexCoord2f(1.0, 1.0); glVertex3f(tmp, tmp, 0.0f);
		glTexCoord2f(0.0, 1.0); glVertex3f(-tmp, tmp, 0.0f);
	glEnd();

	glBegin(GL_TRIANGLES);
		for(int i=0; i<n; i++)
		{
			float p = 2*Pi/n*i;
			Vector3 v1(R*cos(p), R*sin(p), 0);
			Vector3 v2(0, 0, h);
			p = 2*Pi/n*(i+1);
			Vector3 v3(R*cos(p), R*sin(p), 0);
			Vector3 c1 = v3 - v2;
			Vector3 c2 = v2 - v1;
			Vector3 vn = Cross(c1, c2);

			glNormal3f(vn.fX, vn.fY, vn.fZ);

			p = 2*Pi/n*i;
			glVertex3f(R*cos(p), R*sin(p), 0);
			glVertex3f(0, 0, h);
			p = 2*Pi/n*(i+1);
			glVertex3f(R*cos(p), R*sin(p), 0);
		}
	glEnd();
}

void DrawModel(CObj &model)
{//TODO: 绘制模型
	glBegin(GL_TRIANGLES);
		for(int i=0; i<model.m_faces.size(); i++)
		{
			// Vector3 vn = model.m_faces[i].normal;
			// glNormal3d(vn.fX, vn.fY, vn.fZ);
			for(int j=0; j<3; j++)
			{
				Vector3 vn = model.m_pts[model.m_faces[i].pts[j]].normal;
				glNormal3d(vn.fX, vn.fY, vn.fZ);
				Vector3 tmp = model.m_pts[model.m_faces[i].pts[j]].pos;
				glVertex3d(tmp.fX, tmp.fY, tmp.fZ);
			}
		}
	glEnd();

}

void myInit()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);//用白色清屏

	glLightfv(GL_LIGHT0, GL_AMBIENT, g_light0_ambient);//设置场景的环境光
	glLightfv(GL_LIGHT0, GL_DIFFUSE, g_light0_diffuse);//设置场景的散射光
	glLightfv(GL_LIGHT0, GL_POSITION, g_light0_position);//设置场景的位置

	glMaterialfv(GL_FRONT, GL_DIFFUSE, g_material);//指定用于光照计算的当前材质属性


	glEnable(GL_LIGHTING);//开启灯光
	glEnable(GL_LIGHT0);//开启光照0
	glShadeModel(GL_SMOOTH); //设置着色模式为光滑着色
	glEnable(GL_DEPTH_TEST);//启用深度测试

	glMatrixMode(GL_MODELVIEW); //指定当前矩阵为模型视景矩阵
	glLoadIdentity(); //将当前的用户坐标系的原点移到了屏幕中心：类似于一个复位操作
	gluLookAt(0.0, 0.0, 8.0, 0, 0, 0, 0, 1.0, 0);//该函数定义一个视图矩阵，并与当前矩阵相乘.
	//第一组eyex, eyey,eyez 相机在世界坐标的位置;第二组centerx,centery,centerz 相机镜头对准的物体在世界坐标的位置;第三组upx,upy,upz 相机向上的方向在世界坐标中的方向
}

void loadObjFile(void)
{//加载模型

	//调用系统对话框
	dpy = XOpenDisplay(NULL);

	GLXDrawable gld = glXGetCurrentDrawable();
	GLXContext glc = glXGetCurrentContext();

	glXMakeCurrent(dpy, gld, glc);

	int select;
	std::cin >> select;
	std::string path;
	if(select == 1) path = "Model/maneki.obj";
	else if(select == 2) path = "Model/skull.obj";
	else if(select == 3) path = "Model/woman.obj";

	g_obj.ReadObjFile(path.c_str()); //读入模型文件
}

void myGlutDisplay() //绘图函数， 操作系统在必要时刻就会对窗体进行重新绘制操作
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); //清除颜色缓冲以及深度缓冲
	glEnable(GL_NORMALIZE); //打开法线向量归一化，确保了法线的长度为1

	glScalef(g_scale_size, g_scale_size, g_scale_size);//缩放
	g_scale_size = 1.0;
	glTranslatef(g_x_offset, g_y_offset, g_z_offset);
	g_x_offset = 0, g_y_offset = 0, g_z_offset = 0;
	glRotatef(g_x_angle, 0, 1, 0);
	glRotatef(g_y_angle, 1, 0, 0);
	g_x_angle = 0.0, g_y_angle = 0.0;


	glMatrixMode(GL_MODELVIEW);//模型视图矩阵
	glPushMatrix(); //压入当前矩阵堆栈


	if (g_draw_content == SHAPE_MODEL)
	{//绘制模型
		DrawModel(g_obj);
	}
	else if (g_draw_content == SHAPE_TRIANGLE)  //画三角形
	{
		glLoadIdentity();
		glTranslatef(0.0f, 0.0f, -6.0f);
		DrawTriangle();
	}
	else if(g_draw_content == SHAPE_CUBE)  //画立方体
	{
		glLoadIdentity();
		glTranslatef(0.0f, 0.0f, -6.0f);
		glRotatef(g_rquad, g_rquad, g_rquad, 1.0f);	// 在XYZ轴上旋转立方体
		DrawCube();
		g_rquad+=0.2f;// 增加旋转变量
	}
	else if (g_draw_content == SHAPE_CIRCLE) // 画圆
	{
		glLoadIdentity();
		glTranslatef(0.0f, 0.0f, -6.0f);
		DrawCircle();
	}
	else if (g_draw_content == SHAPE_CYLINDER)
	{//TODO: 添加画圆柱的代码
		DrawCylinder();
	}
	else if (g_draw_content == SHAPE_CONE)
	{//TODO：添加画圆锥的代码
		DrawCone();
	}
	glPopMatrix();
	glutSwapBuffers(); //双缓冲
}

void myGlutReshape(int x,int y) //当改变窗口大小时的回调函数
{
	if (y == 0)
	{
		y = 1;
	}

	g_windows_width = x;
	g_windows_height = y;
	double xy_aspect = (float)x / (float)y;
	GLUI_Master.auto_set_viewport(); //自动设置视口大小

	glMatrixMode( GL_PROJECTION );//当前矩阵为投影矩阵
	glLoadIdentity();
	gluPerspective(60.0, xy_aspect, 0.01, 1000.0);//视景体

	glutPostRedisplay(); //标记当前窗口需要重新绘制
}

void myGlutKeyboard(unsigned char Key, int x, int y)
{//键盘时间回调函数

}

void myGlutMouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) //鼠标的状态为按下
	{
		g_press_x = x;
		g_press_y = y;
		if (button == GLUT_LEFT_BUTTON)
		{//按下鼠标的左键表示对模型进行旋转操作
			g_xform_mode = TRANSFORM_ROTATE;
		}
		else if (button == GLUT_MIDDLE_BUTTON)
		{//按下鼠标的滑轮表示对模型进行平移操作
			g_xform_mode = TRANSFORM_TRANSLATE;
		}
		else if (button ==  GLUT_RIGHT_BUTTON)
		{//按下鼠标的右键表示对模型进行缩放操作
			g_xform_mode = TRANSFORM_SCALE;
		}
	}
	else if (state == GLUT_UP)
	{//如果没有按鼠标，则不对模型进行任何操作
		g_xform_mode = TRANSFORM_NONE;
	}
}

void myGlutMotion(int x, int y) //处理当鼠标键摁下时,鼠标拖动的事件
{
	if (g_xform_mode == TRANSFORM_ROTATE) //旋转
	{//TODO:添加鼠标移动控制模型旋转参数的代码
		g_x_angle = x - g_press_x;
		g_y_angle = y - g_press_y;
		g_press_x = x;
		g_press_y = y;
	}
	else if(g_xform_mode == TRANSFORM_SCALE) //缩放
	{//TODO:添加鼠标移动控制模型缩放参数的代码
		g_scale_size *= (1 + (y - g_press_y) / 500.0);
		if(g_scale_size < 0.0001)
		{
			g_scale_size = 0.0001;
		}
		g_press_y = y;
	}
	else if(g_xform_mode == TRANSFORM_TRANSLATE) //平移
	{//TODO:添加鼠标移动控制模型平移参数y的代码
		g_x_offset += (x - g_press_x) / 100.0;
		g_y_offset -= (y - g_press_y) / 100.0;
		g_press_x = x;
		g_press_y = y;
	}

	// force the redraw function
	glutPostRedisplay();
}

void myGlutIdle(void) //空闲回调函数
{
	if ( glutGetWindow() != g_main_window )
		glutSetWindow(g_main_window);

	glutPostRedisplay();
}

void glui_control(int control ) //处理控件的返回值
{
	switch(control)
	{
	case CRTL_LOAD://选择“open”控件
		loadObjFile();
		g_draw_content = SHAPE_MODEL;
		break;
	case CRTL_CHANGE://选择Type面板
		if (g_view_type == VIEW_POINT)
		{
			glDisable(GL_TEXTURE_2D);
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); // 设置两面均为顶点绘制方式
		}
		else if (g_view_type == VIEW_WIRE)
		{
			glDisable(GL_TEXTURE_2D);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 设置两面均为线段绘制方式
		}
		else if (g_view_type == VIEW_TEXTURE)
		{
			glEnable(GL_TEXTURE_2D);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // 设置两面为填充方式
		}
		break;
	case CRTL_TRIANGLE:
		g_draw_content = SHAPE_TRIANGLE;
        break;
	case CRTL_CUBE:
		g_draw_content = SHAPE_CUBE;
		break;
	case CRTL_CIRCLE:
		g_draw_content = SHAPE_CIRCLE;
		break;
	case CRTL_CYLINDER:
		g_draw_content = SHAPE_CYLINDER;
		break;
	case CRTL_CONE:
		g_draw_content = SHAPE_CONE;
		break;
	case CRTL_MODEL:
		g_draw_content = SHAPE_MODEL;
		break;
	default:
		break;
	}
}

void myGlui()
{
	GLUI_Master.set_glutDisplayFunc( myGlutDisplay ); //注册渲染事件回调函数， 系统在需要对窗体进行重新绘制操作时调用
	GLUI_Master.set_glutReshapeFunc( myGlutReshape );  //注册窗口大小改变事件回调函数
	GLUI_Master.set_glutKeyboardFunc( myGlutKeyboard );//注册键盘输入事件回调函数
	glutMotionFunc( myGlutMotion);//注册鼠标移动事件回调函数
	GLUI_Master.set_glutMouseFunc( myGlutMouse );//注册鼠标点击事件回调函数
	GLUI_Master.set_glutIdleFunc(myGlutIdle); //为GLUI注册一个标准的GLUT空闲回调函数，当系统处于空闲时,就会调用该注册的函数

	//GLUI
	GLUI *glui = GLUI_Master.create_glui_subwindow( g_main_window, GLUI_SUBWINDOW_RIGHT); //新建子窗体，位于主窗体的右部
	new GLUI_StaticText(glui, "GLUI" ); //在GLUI下新建一个静态文本框，输出内容为“GLUI”
	new GLUI_Separator(glui); //新建分隔符
	new GLUI_Button(glui,"Open", CRTL_LOAD, glui_control); //新建按钮控件，参数分别为：所属窗体、名字、ID、回调函数，当按钮被触发时,它会被调用.
	new GLUI_Button(glui, "Quit", 0,(GLUI_Update_CB)exit );//新建退出按钮，当按钮被触发时,退出程序

	GLUI_Panel *type_panel = glui->add_panel("Type" ); //在子窗体glui中新建面板，名字为“Type”
	GLUI_RadioGroup *radio = glui->add_radiogroup_to_panel(type_panel, &g_view_type, CRTL_CHANGE, glui_control); //在Type面板中添加一组单选按钮
	glui->add_radiobutton_to_group(radio, "points");
	glui->add_radiobutton_to_group(radio, "wire");
	glui->add_radiobutton_to_group(radio, "flat");
	glui->add_radiobutton_to_group(radio, "texture");

	GLUI_Panel *draw_panel = glui->add_panel("Draw" ); //在子窗体glui中新建面板，名字为“Draw”
	new GLUI_Button(draw_panel,"Triangle",CRTL_TRIANGLE,glui_control);
	new GLUI_Button(draw_panel,"Cube",CRTL_CUBE,glui_control);
	new GLUI_Button(draw_panel,"Circle",CRTL_CIRCLE,glui_control);
	new GLUI_Button(draw_panel,"Cylinder",CRTL_CYLINDER,glui_control);
	new GLUI_Button(draw_panel,"Cone",CRTL_CONE,glui_control);
	new GLUI_Button(draw_panel,"Model",CRTL_MODEL,glui_control);

	glui->set_main_gfx_window(g_main_window ); //将子窗体glui与主窗体main_window绑定，当窗体glui中的控件的值发生过改变，则该glui窗口被重绘
	GLUI_Master.set_glutIdleFunc( myGlutIdle );
}

int main(int argc, char* argv[]) //程序入口
{
  /****************************************/
  /*   Initialize GLUT and create window  */
  /****************************************/

  freopen("log.txt", "w", stdout);//重定位，将输出放入log.txt文件中
  glutInit(&argc, argv);//初始化glut
  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );//初始化渲染模式
  glutInitWindowPosition(200, 200 ); //初始化窗口位置
  glutInitWindowSize(800, 600 ); //初始化窗口大小

  g_main_window = glutCreateWindow("Model Viewer" ); //创建主窗体Model Viewer

  myGlui();
  myInit();
  texGround = load_texture("trump.bmp");
  glutMainLoop();//进入glut消息循环

  return EXIT_SUCCESS;
}
