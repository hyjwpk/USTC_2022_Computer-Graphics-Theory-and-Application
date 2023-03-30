#include <GL\glut.h>
#include <cmath>
#include <fstream>
#include <iostream>

using namespace std;

struct Material {
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float emission[4];
    float shininess;
    int texture_index;
};

struct Vertex {
    float x;
    float y;
    float z;
};

struct Texture_coordinates {
    float value[2];
};

struct Normal {
    float value[3];
};

struct Triangle {
    unsigned int vi1, ti1, ni1, vi2, ti2, ni2, vi3, ti3, ni3;
};

struct Submodel {
    int triangles_num;
    int material_index;
    Triangle *triganles_array;
};

float angel;

int texture_num;
int material_num;
int vertex_num;
int texture_coordinates_num;
int normal_num;
int model_group_num;

string *textures_array;
Material *material_array;
Vertex *vertex_array;
Texture_coordinates *coordinates_array;
Normal *normal_array;
Submodel *submodel_array;

void readtextures(string name, GLuint *texture) {
    BITMAPINFOHEADER header; //位图信息头
    ifstream file;
    file.open(name, ios::binary);
    if (!file.is_open()) {
        exit(-1);
    }
    file.seekg(14, ios::beg);                             //文件头偏移
    file.read((char *)&header, sizeof(BITMAPINFOHEADER)); //读入位图信息头
    int bytes = header.biBitCount / 8;                    //每个像素含有的字节数
    //读取长宽
    int width = abs(header.biWidth);
    int height = abs(header.biHeight);
    BYTE *pixels = new BYTE[width * height * bytes];
    file.read((char *)pixels, width * height * bytes * sizeof(char));                          //读取位图
    glGenTextures(1, texture);                                                                 //生成纹理
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);                                //设定仅使用纹理
    glBindTexture(GL_TEXTURE_2D, *texture);                                                    //纹理绑定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);                          //使用线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);                          //使用线性插值
    GLenum format = bytes == 3 ? GL_BGR_EXT : GL_BGRA_EXT;                                     //设置图片颜色格式
    glTexImage2D(GL_TEXTURE_2D, 0, bytes, width, height, 0, format, GL_UNSIGNED_BYTE, pixels); //生成纹理
    free(pixels);
    file.close();
}

int readfile(string name) {
    ifstream file;
    file.open(name);
    if (!file.is_open()) {
        return false;
    }
    string temp;
    //读入纹理
    file >> texture_num;
    textures_array = new string[texture_num];
    for (int i = 0; i < texture_num; i++) {
        file >> textures_array[i];
    }
    //读入材质
    file >> material_num;
    material_array = new Material[material_num];
    for (int i = 0; i < material_num; i++) {
        for (int j = 0; j < 4; j++) {
            file >> material_array[i].ambient[j];
        }
        for (int j = 0; j < 4; j++) {
            file >> material_array[i].diffuse[j];
        }
        for (int j = 0; j < 4; j++) {
            file >> material_array[i].specular[j];
        }
        for (int j = 0; j < 4; j++) {
            file >> material_array[i].emission[j];
        }
        file >> material_array[i].shininess;
        file >> material_array[i].texture_index;
    }
    //读入顶点
    file >> vertex_num;
    vertex_array = new Vertex[vertex_num];
    for (int i = 0; i < vertex_num; i++) {
        file >> vertex_array[i].x;
        file >> vertex_array[i].y;
        file >> vertex_array[i].z;
    }
    //读入贴图坐标
    file >> texture_coordinates_num;
    coordinates_array = new Texture_coordinates[texture_coordinates_num];
    for (int i = 0; i < texture_coordinates_num; i++) {
        file >> coordinates_array[i].value[0];
        file >> coordinates_array[i].value[1];
    }
    //读入法线
    file >> normal_num;
    normal_array = new Normal[normal_num];
    for (int i = 0; i < normal_num; i++) {
        for (int j = 0; j < 3; j++) {
            file >> normal_array[i].value[j];
        }
    }
    //读入模型分组
    file >> model_group_num;
    file >> temp;
    file >> temp;
    file >> temp;
    submodel_array = new Submodel[model_group_num];
    for (int i = 0; i < model_group_num; i++) {
        file >> submodel_array[i].triangles_num;
        file >> submodel_array[i].material_index;
        submodel_array[i].triganles_array = new Triangle[submodel_array[i].triangles_num];
        for (int j = 0; j < submodel_array[i].triangles_num; j++) {
            file >> submodel_array[i].triganles_array[j].vi1;
            file >> submodel_array[i].triganles_array[j].ti1;
            file >> submodel_array[i].triganles_array[j].ni1;
            file >> submodel_array[i].triganles_array[j].vi2;
            file >> submodel_array[i].triganles_array[j].ti2;
            file >> submodel_array[i].triganles_array[j].ni2;
            file >> submodel_array[i].triganles_array[j].vi3;
            file >> submodel_array[i].triganles_array[j].ti3;
            file >> submodel_array[i].triganles_array[j].ni3;
        }
    }
    file.close();
    return true;
}

void changeangel() {
    angel = fmod(angel + 0.5, 360);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); //载入默认矩阵
    gluLookAt(3 * cos(angel * 3.14 / 180) + 0, 3 * sin(angel * 3.14 / 180) + 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    glutPostRedisplay();
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST); //开启深度缓冲

    //设置视景体
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); //载入默认矩阵
    gluPerspective(50.0f, (GLfloat)(1000) / (GLfloat)(700), 0.1f, 100.0f);
    //设置观察矩阵
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); //载入默认矩阵
    gluLookAt(3 * cos(angel * 3.14 / 180) + 0, 3 * sin(angel * 3.14 / 180) + 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

    for (int i = 0; i < model_group_num; i++) {
        //设置材质
        glMaterialfv(GL_FRONT, GL_AMBIENT, material_array[i].ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, material_array[i].diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, material_array[i].specular);
        glMaterialfv(GL_FRONT, GL_EMISSION, material_array[i].emission);
        glMaterialf(GL_FRONT, GL_SHININESS, material_array[i].shininess);
        GLuint *texture = new GLuint;
        readtextures(textures_array[i], texture);
        //绘制各分组三角形
        for (int j = 0; j < submodel_array[i].triangles_num; j++) {
            glBegin(GL_TRIANGLES);
            int vi1 = submodel_array[i].triganles_array[j].vi1 - 1;
            int vi2 = submodel_array[i].triganles_array[j].vi2 - 1;
            int vi3 = submodel_array[i].triganles_array[j].vi3 - 1;
            int ni1 = submodel_array[i].triganles_array[j].ni1 - 1;
            int ni2 = submodel_array[i].triganles_array[j].ni2 - 1;
            int ni3 = submodel_array[i].triganles_array[j].ni3 - 1;
            int ti1 = submodel_array[i].triganles_array[j].ti1 - 1;
            int ti2 = submodel_array[i].triganles_array[j].ti2 - 1;
            int ti3 = submodel_array[i].triganles_array[j].ti3 - 1;
            glTexCoord2fv(coordinates_array[ti1].value);
            glNormal3fv(normal_array[ni1].value);
            glVertex3f(vertex_array[vi1].x, vertex_array[vi1].y, vertex_array[vi1].z);
            glTexCoord2fv(coordinates_array[ti2].value);
            glNormal3fv(normal_array[ni2].value);
            glVertex3f(vertex_array[vi2].x, vertex_array[vi2].y, vertex_array[vi2].z);
            glTexCoord2fv(coordinates_array[ti3].value);
            glNormal3fv(normal_array[ni3].value);
            glVertex3f(vertex_array[vi3].x, vertex_array[vi3].y, vertex_array[vi3].z);
            glEnd();
        }
        glDeleteTextures(1, texture);
        free(texture);
    }
    glFlush();
}

int main(int argc, char **argv) {
    if (!readfile("luweiqi.txt")) { //读取模型文件
        exit(-1);
    }
    glutInit(&argc, argv);                                    //初始化glut
    glutInitDisplayMode(GLUT_DEPTH | GLUT_SINGLE | GLUT_RGB); //设置窗口的模式－深度缓存，单缓存，颜色模型
    int cx = glutGet(GLUT_SCREEN_WIDTH);
    int cy = glutGet(GLUT_SCREEN_HEIGHT);
    int high = 700;
    int width = 1000;
    glutInitWindowPosition((cx - width) / 2, (cy - high) / 2 - 50); //设置窗口的位置
    glutInitWindowSize(width, high);                                //设置窗口的大小
    glutCreateWindow("liuweiqi");                                   //创建窗口并赋予title
    glutDisplayFunc(renderScene);                                   //调用renderScene把绘制传送到窗口
    glutIdleFunc(changeangel);                                      //生成动画
    glutMainLoop();                                                 //进入循环等待
    return 0;
}