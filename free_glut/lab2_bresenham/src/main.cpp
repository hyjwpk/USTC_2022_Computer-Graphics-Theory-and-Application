// 实现直线段的反走样算法

#include <GL\glut.h>

void bresenham(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1, dy = y2 - y1;
    int pk = 2 * dy - dx;
    int x0 = x1, y0 = y1;
    glBegin(GL_POINTS);
    glVertex2f(x1, y1);
    for (int i = 1; i <= dx; i++) {
        x0 += 1;
        if (pk < 0) {
            pk += 2 * dy;
        } else {
            y0 += 1;
            pk += 2 * dy - 2 * dx;
        }
        glVertex2f(x0, y0);
    }
    glEnd();
}

// 过取样反走样
void bresenham_antialiasing(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1, dy = y2 - y1;
    int pk = 2 * dy * 4 - dx * 4;
    int x0 = x1, y0 = y1;
    int lastx = x1, lasty = y1;
	int num = 0;
    glBegin(GL_POINTS);
    for (int i = 1; i <= dx * 4; i++) {
        num++;
        x0 += 1;
        if (pk < 0) {
            pk += 2 * dy * 4;
        } else {
            y0 += 1;
            pk += 2 * dy * 4 - 2 * dx * 4;
        }
        if (((x0 - x1) / 4 != (lastx - x1) / 4) || ((y0 - y1) / 4 != (lasty - y1) / 4)) {
            float value = num / 16.0 + 0.7;
            glColor3f(value, value, value);
            glVertex2i(x1 + (lastx - x1) / 4, y1 + (lasty - y1) / 4);
            lastx = x0;
            lasty = y0;
            num = 0;
        }
    }
    glEnd();
}

void renderScene(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    gluOrtho2D(0, 800, 0, 600); //设置坐标范围
    bresenham(100, 100, 600, 400);
    bresenham_antialiasing(100, 200, 600, 500);
    glFlush();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); //设置显示模式 单缓冲 RGB模式
    glutInitWindowPosition(50, 100);
    glutInitWindowSize(800, 600);
    glutCreateWindow("antialiasing");
    glutDisplayFunc(renderScene); //设置显示函数
    glutMainLoop();
    return 0;
}