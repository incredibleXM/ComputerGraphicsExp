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
	void Normalize()//��һ��
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

	std::vector<Point> m_pts; //����
	std::vector<Face> m_faces;//��

public:
	bool ReadObjFile(const char* pcszFileName);//����ģ���ļ�

private:
	void UnifyModel();//��λ��ģ��
	void ComputeFaceNormal(Face& f);//������ķ���
};

using std::min;
using std::max;

Vector3 operator + (const Vector3& one, const Vector3& two) //�����������
{
	return Vector3(one.fX + two.fX, one.fY + two.fY, one.fZ + two.fZ);
}

Vector3 operator - (const Vector3& one, const Vector3& two) //�����������
{
	return Vector3(one.fX - two.fX, one.fY - two.fY, one.fZ - two.fZ);
}

Vector3 operator * (const Vector3& one, double scale) //���������ĳ˲���
{
	return Vector3(one.fX * scale, one.fY * scale, one.fZ * scale);
}

Vector3 operator / (const Vector3& one, double scale) //���������ĳ�����
{
	return one * (1.0 / scale);
}

Vector3 Cross(Vector3& one, Vector3& two)
{//�������������Ĳ��
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
{//��ȡģ���ļ�

	FILE* fpFile = fopen(pcszFileName, "r"); //��ֻ����ʽ���ļ�
	if (fpFile == NULL)
	{
		return false;
	}

	m_pts.clear();
	m_faces.clear();

	//TODO����ģ���ļ��еĵ�������ݷֱ����m_pts��m_faces��
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

	UnifyModel(); //��ģ�͹�һ��
	for(auto& x: m_faces) ComputeFaceNormal(x);
	for(auto& x: m_pts) x.normal.Normalize();
	return true;
}

void CObj::UnifyModel()
{//Ϊͳһ��ʾ��ͬ�ߴ��ģ�ͣ���ģ�͹�һ������ģ�ͳߴ����ŵ�0.0-1.0֮��
//ԭ���ҳ�ģ�͵ı߽�������Сֵ�������ҳ�ģ�͵�����
//��ģ�͵����ĵ�Ϊ��׼��ģ�Ͷ����������
//TODO:���ģ�͹�һ������
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
{//TODO:������f�ķ�������������
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

//�����������������
GLuint texGround;
#define BMP_Header_Length 54  //ͼ���������ڴ���е�ƫ����

CObj g_obj;
Display *dpy;
//the lighting
static GLfloat g_light0_ambient[] =  {0.0f, 0.0f, 0.0f, 1.0f};//������
static GLfloat g_light0_diffuse[] =  {1.0f, 1.0f, 1.0f, 1.0f};//ɢ���
static GLfloat g_light0_specular[] = {1.0f,1.0f,1.0f,1.0f}; //�����
static GLfloat g_light0_position[] = {0.0f, 0.0f, 100.0f, 0.0f};//��Դ��λ�á���4������Ϊ1����ʾ���Դ����4��������Ϊ0����ʾƽ�й���{0.0f, 0.0f, 10.0f, 0.0f}

static GLfloat g_material[] = {0.96f, 0.8f, 0.69f, 1.0f};//����
static GLfloat g_rquad = 0;

static float g_x_offset   = 0.0;
static float g_y_offset   = 0.0;
static float g_z_offset   = 0.0;
static float g_scale_size = 1;
static int  g_press_x; //��갴��ʱ��x����
static int  g_press_y; //��갴��ʱ��y����
static float g_x_angle = 0.0;
static float g_y_angle = 0.0;

const int n = 1000;
const GLfloat R = 0.5f;
const GLfloat Pi = 3.1415926536f;
int g_view_type = VIEW_FLAT;
int g_draw_content = SHAPE_TRIANGLE;

// ����power_of_two�����ж�һ�������ǲ���2����������
int power_of_two(int n)
{
    if( n <= 0 )
        return 0;
    return (n & (n-1)) == 0;
}

/* ����load_texture
* ��ȡһ��BMP�ļ���Ϊ����
* ���ʧ�ܣ�����0������ɹ�������������
*/
GLuint load_texture(const char* file_name)
{
    GLint width, height, total_bytes;
    GLubyte* pixels = 0;
    GLuint last_texture_ID=0, texture_ID = 0;

    // ���ļ������ʧ�ܣ�����
    FILE* pFile = fopen(file_name, "rb");
    if( pFile == 0 )
        return 0;

    // ��ȡ�ļ���ͼ��Ŀ�Ⱥ͸߶�
    fseek(pFile, 0x0012, SEEK_SET);
    fread(&width, 4, 1, pFile);
    fread(&height, 4, 1, pFile);
    fseek(pFile, BMP_Header_Length, SEEK_SET);

    // ����ÿ��������ռ�ֽ����������ݴ����ݼ����������ֽ���
    {
        GLint line_bytes = width * 3;
        while( line_bytes % 4 != 0 )
            ++line_bytes;
        total_bytes = line_bytes * height;
    }

    // �����������ֽ��������ڴ�
    pixels = (GLubyte*)malloc(total_bytes);
    if( pixels == 0 )
    {
        fclose(pFile);
        return 0;
    }

    // ��ȡ��������
    if( fread(pixels, total_bytes, 1, pFile) <= 0 )
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    // �Ծ;ɰ汾�ļ��ݣ����ͼ��Ŀ�Ⱥ͸߶Ȳ��ǵ������η�������Ҫ��������
    // ��ͼ���߳�����OpenGL�涨�����ֵ��Ҳ����
    {
        GLint max;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
        if( !power_of_two(width)
            || !power_of_two(height)
            || width > max
            || height > max )
        {
            const GLint new_width = 256;
            const GLint new_height = 256; // �涨���ź��µĴ�СΪ�߳���������
            GLint new_line_bytes, new_total_bytes;
            GLubyte* new_pixels = 0;

            // ����ÿ����Ҫ���ֽ��������ֽ���
            new_line_bytes = new_width * 3;
            while( new_line_bytes % 4 != 0 )
                ++new_line_bytes;
            new_total_bytes = new_line_bytes * new_height;

            // �����ڴ�
            new_pixels = (GLubyte*)malloc(new_total_bytes);
            if( new_pixels == 0 )
            {
                free(pixels);
                fclose(pFile);
                return 0;
            }

            // ������������
            gluScaleImage(GL_RGB,
                width, height, GL_UNSIGNED_BYTE, pixels,
                new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

            // �ͷ�ԭ�����������ݣ���pixelsָ���µ��������ݣ�����������width��height
            free(pixels);
            pixels = new_pixels;
            width = new_width;
            height = new_height;
        }
    }

    // ����һ���µ�������
    glGenTextures(1, &texture_ID);
    if( texture_ID == 0 )
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    // ���µ������������������������
    // �ڰ�ǰ���Ȼ��ԭ���󶨵������ţ��Ա��������лָ�
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
    glBindTexture(GL_TEXTURE_2D, lastTextureID);  //�ָ�֮ǰ�������
    free(pixels);
    return texture_ID;
}


void DrawTriangle()
{//����������
	glBegin(GL_TRIANGLES);
		glNormal3f(0.0f, 0.0f, 1.0f);  //ָ���淨��
		glVertex3f( 0.0f, 1.0f, 0.0f);                    // �϶���
		glVertex3f(-1.0f,-1.0f, 0.0f);                    // ����
		glVertex3f( 1.0f,-1.0f, 0.0f);                    // ����
	glEnd();
}

void DrawCube()
{//����������

	glBegin(GL_QUADS);
		glNormal3f( 0.0f, 0.0f, 1.0f);  //ָ���淨��
		glVertex3f( 1.0f, 1.0f,1.0f);   //�о��涥�����ݣ���ʱ��˳��
		glVertex3f(-1.0f, 1.0f, 1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
	//ǰ----------------------------
		glNormal3f( 0.0f, 0.0f,-1.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);
	//��----------------------------
		glNormal3f( 0.0f, 1.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);
	//��----------------------------
		glNormal3f( 0.0f,-1.0f, 0.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
	//��----------------------------
		glNormal3f( 1.0f, 0.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);
	//��----------------------------
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);
	//��----------------------------*/
	glEnd();
}

void DrawCircle()
{//����Բ
	glBegin(GL_POLYGON);
		glNormal3f(0.0f, 0.0f, 1.0f);
		for(int i=0; i<n; ++i)
			glVertex2f(R*cos(2*Pi/n*i), R*sin(2*Pi/n*i));
	glEnd();
}

void DrawCylinder()
{
	glBegin(GL_TRIANGLES); // �����Բ
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

	glBegin(GL_TRIANGLES); // �����Բ
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
{//TODO: ����ģ��
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
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);//�ð�ɫ����

	glLightfv(GL_LIGHT0, GL_AMBIENT, g_light0_ambient);//���ó����Ļ�����
	glLightfv(GL_LIGHT0, GL_DIFFUSE, g_light0_diffuse);//���ó�����ɢ���
	glLightfv(GL_LIGHT0, GL_POSITION, g_light0_position);//���ó�����λ��

	glMaterialfv(GL_FRONT, GL_DIFFUSE, g_material);//ָ�����ڹ��ռ���ĵ�ǰ��������


	glEnable(GL_LIGHTING);//�����ƹ�
	glEnable(GL_LIGHT0);//��������0
	glShadeModel(GL_SMOOTH); //������ɫģʽΪ�⻬��ɫ
	glEnable(GL_DEPTH_TEST);//������Ȳ���

	glMatrixMode(GL_MODELVIEW); //ָ����ǰ����Ϊģ���Ӿ�����
	glLoadIdentity(); //����ǰ���û�����ϵ��ԭ���Ƶ�����Ļ���ģ�������һ����λ����
	gluLookAt(0.0, 0.0, 8.0, 0, 0, 0, 0, 1.0, 0);//�ú�������һ����ͼ���󣬲��뵱ǰ�������.
	//��һ��eyex, eyey,eyez ��������������λ��;�ڶ���centerx,centery,centerz �����ͷ��׼�����������������λ��;������upx,upy,upz ������ϵķ��������������еķ���
}

void loadObjFile(void)
{//����ģ��

	//����ϵͳ�Ի���
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

	g_obj.ReadObjFile(path.c_str()); //����ģ���ļ�
}

void myGlutDisplay() //��ͼ������ ����ϵͳ�ڱ�Ҫʱ�̾ͻ�Դ���������»��Ʋ���
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); //�����ɫ�����Լ���Ȼ���
	glEnable(GL_NORMALIZE); //�򿪷���������һ����ȷ���˷��ߵĳ���Ϊ1

	glScalef(g_scale_size, g_scale_size, g_scale_size);//����
	g_scale_size = 1.0;
	glTranslatef(g_x_offset, g_y_offset, g_z_offset);
	g_x_offset = 0, g_y_offset = 0, g_z_offset = 0;
	glRotatef(g_x_angle, 0, 1, 0);
	glRotatef(g_y_angle, 1, 0, 0);
	g_x_angle = 0.0, g_y_angle = 0.0;


	glMatrixMode(GL_MODELVIEW);//ģ����ͼ����
	glPushMatrix(); //ѹ�뵱ǰ�����ջ


	if (g_draw_content == SHAPE_MODEL)
	{//����ģ��
		DrawModel(g_obj);
	}
	else if (g_draw_content == SHAPE_TRIANGLE)  //��������
	{
		glLoadIdentity();
		glTranslatef(0.0f, 0.0f, -6.0f);
		DrawTriangle();
	}
	else if(g_draw_content == SHAPE_CUBE)  //��������
	{
		glLoadIdentity();
		glTranslatef(0.0f, 0.0f, -6.0f);
		glRotatef(g_rquad, g_rquad, g_rquad, 1.0f);	// ��XYZ������ת������
		DrawCube();
		g_rquad+=0.2f;// ������ת����
	}
	else if (g_draw_content == SHAPE_CIRCLE) // ��Բ
	{
		glLoadIdentity();
		glTranslatef(0.0f, 0.0f, -6.0f);
		DrawCircle();
	}
	else if (g_draw_content == SHAPE_CYLINDER)
	{//TODO: ��ӻ�Բ���Ĵ���
		DrawCylinder();
	}
	else if (g_draw_content == SHAPE_CONE)
	{//TODO����ӻ�Բ׶�Ĵ���
		DrawCone();
	}
	glPopMatrix();
	glutSwapBuffers(); //˫����
}

void myGlutReshape(int x,int y) //���ı䴰�ڴ�Сʱ�Ļص�����
{
	if (y == 0)
	{
		y = 1;
	}

	g_windows_width = x;
	g_windows_height = y;
	double xy_aspect = (float)x / (float)y;
	GLUI_Master.auto_set_viewport(); //�Զ������ӿڴ�С

	glMatrixMode( GL_PROJECTION );//��ǰ����ΪͶӰ����
	glLoadIdentity();
	gluPerspective(60.0, xy_aspect, 0.01, 1000.0);//�Ӿ���

	glutPostRedisplay(); //��ǵ�ǰ������Ҫ���»���
}

void myGlutKeyboard(unsigned char Key, int x, int y)
{//����ʱ��ص�����

}

void myGlutMouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) //����״̬Ϊ����
	{
		g_press_x = x;
		g_press_y = y;
		if (button == GLUT_LEFT_BUTTON)
		{//�������������ʾ��ģ�ͽ�����ת����
			g_xform_mode = TRANSFORM_ROTATE;
		}
		else if (button == GLUT_MIDDLE_BUTTON)
		{//�������Ļ��ֱ�ʾ��ģ�ͽ���ƽ�Ʋ���
			g_xform_mode = TRANSFORM_TRANSLATE;
		}
		else if (button ==  GLUT_RIGHT_BUTTON)
		{//���������Ҽ���ʾ��ģ�ͽ������Ų���
			g_xform_mode = TRANSFORM_SCALE;
		}
	}
	else if (state == GLUT_UP)
	{//���û�а���꣬�򲻶�ģ�ͽ����κβ���
		g_xform_mode = TRANSFORM_NONE;
	}
}

void myGlutMotion(int x, int y) //������������ʱ,����϶����¼�
{
	if (g_xform_mode == TRANSFORM_ROTATE) //��ת
	{//TODO:�������ƶ�����ģ����ת�����Ĵ���
		g_x_angle = x - g_press_x;
		g_y_angle = y - g_press_y;
		g_press_x = x;
		g_press_y = y;
	}
	else if(g_xform_mode == TRANSFORM_SCALE) //����
	{//TODO:�������ƶ�����ģ�����Ų����Ĵ���
		g_scale_size *= (1 + (y - g_press_y) / 500.0);
		if(g_scale_size < 0.0001)
		{
			g_scale_size = 0.0001;
		}
		g_press_y = y;
	}
	else if(g_xform_mode == TRANSFORM_TRANSLATE) //ƽ��
	{//TODO:�������ƶ�����ģ��ƽ�Ʋ���y�Ĵ���
		g_x_offset += (x - g_press_x) / 100.0;
		g_y_offset -= (y - g_press_y) / 100.0;
		g_press_x = x;
		g_press_y = y;
	}

	// force the redraw function
	glutPostRedisplay();
}

void myGlutIdle(void) //���лص�����
{
	if ( glutGetWindow() != g_main_window )
		glutSetWindow(g_main_window);

	glutPostRedisplay();
}

void glui_control(int control ) //����ؼ��ķ���ֵ
{
	switch(control)
	{
	case CRTL_LOAD://ѡ��open���ؼ�
		loadObjFile();
		g_draw_content = SHAPE_MODEL;
		break;
	case CRTL_CHANGE://ѡ��Type���
		if (g_view_type == VIEW_POINT)
		{
			glDisable(GL_TEXTURE_2D);
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); // ���������Ϊ������Ʒ�ʽ
		}
		else if (g_view_type == VIEW_WIRE)
		{
			glDisable(GL_TEXTURE_2D);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // ���������Ϊ�߶λ��Ʒ�ʽ
		}
		else if (g_view_type == VIEW_TEXTURE)
		{
			glEnable(GL_TEXTURE_2D);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // ��������Ϊ��䷽ʽ
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
	GLUI_Master.set_glutDisplayFunc( myGlutDisplay ); //ע����Ⱦ�¼��ص������� ϵͳ����Ҫ�Դ���������»��Ʋ���ʱ����
	GLUI_Master.set_glutReshapeFunc( myGlutReshape );  //ע�ᴰ�ڴ�С�ı��¼��ص�����
	GLUI_Master.set_glutKeyboardFunc( myGlutKeyboard );//ע����������¼��ص�����
	glutMotionFunc( myGlutMotion);//ע������ƶ��¼��ص�����
	GLUI_Master.set_glutMouseFunc( myGlutMouse );//ע��������¼��ص�����
	GLUI_Master.set_glutIdleFunc(myGlutIdle); //ΪGLUIע��һ����׼��GLUT���лص���������ϵͳ���ڿ���ʱ,�ͻ���ø�ע��ĺ���

	//GLUI
	GLUI *glui = GLUI_Master.create_glui_subwindow( g_main_window, GLUI_SUBWINDOW_RIGHT); //�½��Ӵ��壬λ����������Ҳ�
	new GLUI_StaticText(glui, "GLUI" ); //��GLUI���½�һ����̬�ı����������Ϊ��GLUI��
	new GLUI_Separator(glui); //�½��ָ���
	new GLUI_Button(glui,"Open", CRTL_LOAD, glui_control); //�½���ť�ؼ��������ֱ�Ϊ���������塢���֡�ID���ص�����������ť������ʱ,���ᱻ����.
	new GLUI_Button(glui, "Quit", 0,(GLUI_Update_CB)exit );//�½��˳���ť������ť������ʱ,�˳�����

	GLUI_Panel *type_panel = glui->add_panel("Type" ); //���Ӵ���glui���½���壬����Ϊ��Type��
	GLUI_RadioGroup *radio = glui->add_radiogroup_to_panel(type_panel, &g_view_type, CRTL_CHANGE, glui_control); //��Type��������һ�鵥ѡ��ť
	glui->add_radiobutton_to_group(radio, "points");
	glui->add_radiobutton_to_group(radio, "wire");
	glui->add_radiobutton_to_group(radio, "flat");
	glui->add_radiobutton_to_group(radio, "texture");

	GLUI_Panel *draw_panel = glui->add_panel("Draw" ); //���Ӵ���glui���½���壬����Ϊ��Draw��
	new GLUI_Button(draw_panel,"Triangle",CRTL_TRIANGLE,glui_control);
	new GLUI_Button(draw_panel,"Cube",CRTL_CUBE,glui_control);
	new GLUI_Button(draw_panel,"Circle",CRTL_CIRCLE,glui_control);
	new GLUI_Button(draw_panel,"Cylinder",CRTL_CYLINDER,glui_control);
	new GLUI_Button(draw_panel,"Cone",CRTL_CONE,glui_control);
	new GLUI_Button(draw_panel,"Model",CRTL_MODEL,glui_control);

	glui->set_main_gfx_window(g_main_window ); //���Ӵ���glui��������main_window�󶨣�������glui�еĿؼ���ֵ�������ı䣬���glui���ڱ��ػ�
	GLUI_Master.set_glutIdleFunc( myGlutIdle );
}

int main(int argc, char* argv[]) //�������
{
  /****************************************/
  /*   Initialize GLUT and create window  */
  /****************************************/

  freopen("log.txt", "w", stdout);//�ض�λ�����������log.txt�ļ���
  glutInit(&argc, argv);//��ʼ��glut
  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );//��ʼ����Ⱦģʽ
  glutInitWindowPosition(200, 200 ); //��ʼ������λ��
  glutInitWindowSize(800, 600 ); //��ʼ�����ڴ�С

  g_main_window = glutCreateWindow("Model Viewer" ); //����������Model Viewer

  myGlui();
  myInit();
  texGround = load_texture("trump.bmp");
  glutMainLoop();//����glut��Ϣѭ��

  return EXIT_SUCCESS;
}
