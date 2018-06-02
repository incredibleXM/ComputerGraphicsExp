#include <bits/stdc++.h>
#include <GL/glui.h>
#include <GL/glx.h>

#define CRTL_OPEN           0x00
#define CRTL_INPUT          0x01
#define CRTL_CLEAR          0x02
#define CRTL_DENSE          0x03
#define CRTL_POLY           0x04

#define SHAPE_MODEL			0x00
#define SHAPE_INPUT         0x01
#define SHAPE_CLEAR         0x02

int g_main_window;
int g_draw_content;
int degree, cnt_num;
int drawPolygon = 1;
GLUI_Checkbox *checkbox;
GLUI_Spinner *spinner;
float Points[4][3];
float uv[8];

const double eps = 1e-8;
int N = 50;

struct Point {
    float x, y, z;
    Point(float x=0.0, float y=0.0, float z=0.0):x(x), y(y), z(z) {}
};

std::vector<Point> pvec;

double getRatio(double t,double a,double b,double c,double d)
{
    return a * pow(t, 3) + b * pow(t, 2) + c * t + d;
}

double calc(int k, int d, double u)
{
    if(d == 0) {
        if(u>=uv[k] && u<uv[k+1]) return 1;
        else return 0;
    } else {
        double res = 0;
        if(fabs(uv[k+d]-uv[k]) > eps) res += (u-uv[k])/(uv[k+d]-uv[k])*calc(k, d-1, u);
        if(fabs(uv[k+d+1]-uv[k+1]) > eps) res += (uv[k+d+1]-u)/(uv[k+d+1]-uv[k+1])*calc(k+1, d-1, u);
        return res;
    }
}

Point calcPt(int n, double u)
{
    float x = 0, y = 0, z = 0;
    for(int k=0; k<=n; k++) {
        float tmp = calc(k, degree, u);
        x += Points[k][0] * tmp;
        y += Points[k][1] * tmp;
        z += Points[k][2] * tmp;
    }
    return Point(x, y, z);
}

void Curve(Point a, Point b, Point c, Point d)
{
    double delta = 1.0 / N;
    glPointSize(5.0f);
    glColor3f(0, 1.0f, 0);
    glBegin(GL_LINE_STRIP);
        for(int i=0; i<=N; i++) {
            double t = delta * i;
            double ratio[4];
            ratio[0] = getRatio(t, -1, 3, -3, 1);
            ratio[1] = getRatio(t, 3, -6, 0, 4);
            ratio[2] = getRatio(t, -3, 3, 3, 1);
            ratio[3] = getRatio(t, 1, 0, 0, 0);
            double x = 0.0, y = 0.0;
            x += ratio[0] * a.x + ratio[1] * b.x + ratio[2] * c.x + ratio[3] * d.x;
            y += ratio[0] * a.y + ratio[1] * b.y + ratio[2] * c.y + ratio[3] * d.y;
            x /= 6.0, y /= 6.0;
            glVertex3f(x, y, 0);
        }
    glEnd();
}

void myGlutDisplay()
{
    glClear(GL_COLOR_BUFFER_BIT);

    if(g_draw_content == SHAPE_MODEL)
    {
        glLoadIdentity();

        if(drawPolygon)
        {
            glPointSize(5.0f);
            glColor3f(1.0f, 0, 0);
            glBegin(GL_LINE_STRIP);
                for(int i=0; i<cnt_num; i++) {
                    glVertex3fv(Points[i]);
                }
            glEnd();
        }

        glPointSize(5.0f);
        /*
        glColor3f(0, 0, 0);
        GLUnurbsObj *BSplineCurve;
        BSplineCurve = gluNewNurbsRenderer();
        gluBeginCurve(BSplineCurve);
            gluNurbsCurve(BSplineCurve, 8, uv, 3, &Points[0][0],
                            4, GL_MAP1_VERTEX_3);
        gluEndCurve(BSplineCurve);
        */
        glColor3f(0, 1.0f, 0);
        glBegin(GL_LINE_STRIP);
            for(int i=0; i<=N; i++) {
                float p = uv[degree] + (float)i / N * (uv[cnt_num]-uv[degree]);
                Point t = calcPt(3, p);
                glVertex3f(t.x, t.y, 0);
            }
        glEnd();

    }
    else if(g_draw_content == SHAPE_INPUT)
    {
        glLoadIdentity();

        glPointSize(5.0f);
        glColor3f(0, 0, 1.0f);
        glBegin(GL_POINTS);
            for(auto x: pvec) glVertex3f(x.x, x.y, x.z);
        glEnd();

        int sz = pvec.size();
        if(sz >= 4) {
            for(int i=0; i<sz-3; i++) {
                Curve(pvec[i], pvec[i+1], pvec[i+2], pvec[i+3]);
            }
        }
    }
    else if(g_draw_content == SHAPE_CLEAR)
    {
        glLoadIdentity();
    }
    glutSwapBuffers();
}

void myGlutReshape(int w, int h)
{
    // prevents division by zero when minimising window
    if(h == 0) h = 1;

    // set the drawable region of the window
    glViewport(0, 0, w, h);
    // set up the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // just use a perspective projection
    //gluPerspective(45,(float)w/h,0.1,100);
    gluOrtho2D(0, w, h, 0);
    // go back to modelview matrix so we can move the objects about
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void myGlutIdle(void)
{
	if ( glutGetWindow() != g_main_window )
		glutSetWindow(g_main_window);

	glutPostRedisplay();
}

void myGlutMouse(int button, int state, int x, int y)
{
    if(state == GLUT_DOWN && button == GLUT_LEFT_BUTTON
        && g_draw_content == SHAPE_INPUT)
    {
        pvec.push_back(Point(x, y, 0));
    }
    else if(state == GLUT_UP)
    {

    }

    glutPostRedisplay();
}


void glui_control(int control)
{
    switch(control) {
        case CRTL_OPEN:
            g_draw_content = SHAPE_MODEL;
            break;
        case CRTL_INPUT:
            N = 50;
            g_draw_content = SHAPE_INPUT;
            break;
        case CRTL_CLEAR:
            pvec.clear();
            g_draw_content = SHAPE_CLEAR;
            break;
        default:
            break;
    }
}

void myGlui()
{
    GLUI_Master.set_glutDisplayFunc(myGlutDisplay);
    GLUI_Master.set_glutReshapeFunc(myGlutReshape);
    GLUI_Master.set_glutMouseFunc(myGlutMouse);
    GLUI_Master.set_glutIdleFunc(myGlutIdle);

    GLUI *glui = GLUI_Master.create_glui_subwindow( g_main_window, GLUI_SUBWINDOW_RIGHT);
    new GLUI_StaticText(glui, "Xia Meng" );
    new GLUI_Separator( glui );
	new GLUI_Button(glui, "Open", CRTL_OPEN, glui_control);
    checkbox = new GLUI_Checkbox( glui, "Polygon", &drawPolygon, CRTL_POLY, glui_control);
    spinner  = new GLUI_Spinner( glui, "Density:", &N, CRTL_DENSE, glui_control);
    spinner->set_int_limits(3, 100);
    new GLUI_Button(glui,"Input", CRTL_INPUT, glui_control);
    new GLUI_Button(glui,"Clear", CRTL_CLEAR, glui_control);
	new GLUI_Button(glui, "Quit", 0, (GLUI_Update_CB)exit);

    glui->set_main_gfx_window(g_main_window);
    GLUI_Master.set_glutIdleFunc( myGlutIdle );
}


int main(int argc, char *argv[])
{
    freopen("cubic.txt", "r", stdin);
    // freopen("log.txt", "w", stdout);

    std::cin >> degree >> cnt_num;
    for(int i=0; i <= degree+cnt_num; i++) std::cin >> uv[i];
    for(int i=0; i<cnt_num; i++) {
        std::cin >> Points[i][0] >> Points[i][1];
        Points[i][0] *= 5, Points[i][0] += 50;
        Points[i][1] *= 5, Points[i][1] += 50;
        Points[i][1] = 600 - Points[i][1];
        Points[i][2] = 0.0;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(800, 600);

    g_main_window = glutCreateWindow("Curve Producer by XM");

    myGlui();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glShadeModel(GL_FLAT);
    // myInit();

    glutMainLoop();
    return EXIT_SUCCESS;
}
