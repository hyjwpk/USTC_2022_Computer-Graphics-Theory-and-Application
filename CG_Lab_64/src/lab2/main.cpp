#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <learnopengl/shader_m.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

#define PATH "src/lab2/"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

#define eps 1e-7

class Point {
public:
    double x, y;
    int type = 0;
};

class Line {
public:
    Point p1, p2;
};

double multiplication_cross(Line l, Point p) {
    return (l.p2.x - l.p1.x) * (p.y - l.p1.y) - (l.p2.y - l.p1.y) * (p.x - l.p1.x);
}

double distance(Point p1, Point p2) {
    return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

double distance(Line l, Point p) {
    return fabs(multiplication_cross(l, p)) / distance(l.p1, l.p2);
}

Point intersect_point(Line l1, Line l2) {
    Point p;
    if (multiplication_cross(l1, l2.p1) * multiplication_cross(l1, l2.p2) <= 0 && multiplication_cross(l2, l1.p1) * multiplication_cross(l2, l1.p2) <= 0) {
        double u = distance(l1, l2.p1) / (distance(l1, l2.p1) + distance(l1, l2.p2));
        p.x = l2.p1.x + u * (l2.p2.x - l2.p1.x);
        p.y = l2.p1.y + u * (l2.p2.y - l2.p1.y);
    } else {
        p.x = -1;
        p.y = -1;
    }
    return p;
}

// 射线法判断点是否在多边形内
bool is_in_polygon(Point p, std::vector<Point> polygon) {
    Line l;
    l.p1 = p;
    l.p2.x = 1;
    l.p2.y = p.y;

    for (int i = 0; i < 4; i++) {
        int count = 0;

        // 考虑到可能在边界上，加入微小扰动
        if (i % 2 == 0) {
            l.p1.x += eps;
        } else {
            l.p1.x -= eps;
        }
        if (i <= 1) {
            l.p1.y += eps;
        } else {
            l.p1.y -= eps;
        }

        for (size_t i = 0; i < polygon.size(); i++) {
            Line l1;
            l1.p1 = polygon[i];
            l1.p2 = polygon[(i + 1) % polygon.size()];
            Point p1 = intersect_point(l, l1);
            if (p1.x != -1 && p1.y != -1) {
                count++;
            }
        }

        if (count % 2 == 1) {
            return true;
        }
    }
    return false;
}

std::vector<Point> polygon_intersect(std::vector<Point> polygon1, std::vector<Point> polygon2) {
    std::vector<Point> polygon;
    for (size_t i = 0; i < polygon1.size(); i++) {
        std::vector<Point> temp;
        polygon.push_back(polygon1[i]);
        Line l1;
        l1.p1 = polygon1[i];
        l1.p2 = polygon1[(i + 1) % polygon1.size()];

        for (size_t j = 0; j < polygon2.size(); j++) {
            Line l2;
            l2.p1 = polygon2[j];
            l2.p2 = polygon2[(j + 1) % polygon2.size()];
            Point p = intersect_point(l1, l2);
            p.type = 1;
            if (p.x != -1 && p.y != -1 && (p.x != l1.p1.x || p.y != l1.p1.y) && (p.x != l1.p2.x || p.y != l1.p2.y)) {
                bool repeat = false;
                for (auto &point : temp) {
                    if (point.x == p.x && point.y == p.y) {
                        repeat = true;
                        break;
                    }
                }
                if (!repeat) {
                    temp.push_back(p);
                }
            }
        }

        // 保证点按照逆时针方向排序
        sort(temp.begin(), temp.end(), [=](Point p1, Point p2) {
            return distance(p1, l1.p1) < distance(p2, l1.p1);
        });

        for (size_t j = 0; j < temp.size(); j++) {
            polygon.push_back(temp[j]);
        }
    }
    return polygon;
}

// 多边形裁剪
std::vector<std::vector<Point>> Weiler_Atherton(std::vector<Point> &polygon1, std::vector<Point> &polygon2) {
    std::vector<std::vector<Point>> result;
    // 计算多边形的交点
    std::vector<Point> polygon1_new = polygon_intersect(polygon1, polygon2);
    std::vector<Point> polygon2_new = polygon_intersect(polygon2, polygon1);

    // 计算对应点
    int *flag1 = new int[polygon1_new.size()]();
    int *flag2 = new int[polygon2_new.size()]();
    for (size_t i = 0; i < polygon1_new.size(); i++) {
        for (size_t j = 0; j < polygon2_new.size(); j++) {
            if (fabs(polygon1_new[i].x - polygon2_new[j].x) < eps && fabs(polygon1_new[i].y - polygon2_new[j].y) < eps) {
                flag1[i] = j;
                flag2[j] = i;
                if (polygon1_new[i].type == 1 && polygon2_new[j].type == 0) {
                    polygon2_new[j].type = 1;
                }
                if (polygon1_new[i].type == 0 && polygon2_new[j].type == 1) {
                    polygon1_new[i].type = 1;
                }
                if (polygon1_new[i].type == 0 && polygon2_new[j].type == 0) {
                    polygon2_new[j].type = 2;
                    polygon1_new[i].type = 2;
                }
            }
        }
    }

    // 访问记录
    int *visit1 = new int[polygon1_new.size()]();
    int *visit2 = new int[polygon2_new.size()]();

    // 遍历多边形的全部点
    for (size_t i = 0; i < polygon1_new.size(); i++) {
        // 如果当前点未遍历过
        if (visit1[i] == 0) {
            std::vector<Point> polygon; // 需要绘制的多边形
            int cur = 1;                // 当前所处的多边形
            int j = i;                  // 当前遍历的点
            bool in = false;
            while (1) {
                if (cur == 1) {
                    if (visit1[j] == 1) {
                        result.push_back(polygon);
                        break;
                    }

                    // 将当前点加入到需要绘制的多边形中
                    if (in || is_in_polygon(polygon1_new[j], polygon2)) {
                        polygon.push_back(polygon1_new[j]);
                    }

                    // 标记当前点已遍历
                    visit1[j] = 1;
                    if (polygon1_new[j].type != 0) {
                        visit2[flag1[j]] = 1;
                    }

                    // 当前点在多边形内，下一点在多边形外
                    if (polygon1_new[j].type == 1 && (is_in_polygon(polygon1_new[j], polygon2) && !is_in_polygon(polygon1_new[(j + 1) % polygon1_new.size()], polygon2))) {
                        cur = 2;
                        // 找到另一多边形对应点下一点
                        j = (flag1[j] + 1) % polygon2_new.size();
                        in = true;
                    } else {
                        j = (j + 1) % polygon1_new.size();
                    }
                } else {
                    if (visit2[j] == 1) {
                        result.push_back(polygon);
                        break;
                    }

                    // 将当前点加入到需要绘制的多边形中
                    polygon.push_back(polygon2_new[j]);

                    // 标记当前点已遍历
                    visit2[j] = 1;
                    if (polygon2_new[j].type != 0) {
                        visit1[flag2[j]] = 1;
                    }

                    // 当前点在多边形内，下一点在多边形外
                    if (polygon2_new[j].type == 1) {
                        cur = 1;
                        // 找到另一多边形对应点下一点
                        j = (flag2[j] + 1) % polygon1_new.size();
                    } else {
                        j = (j + 1) % polygon2_new.size();
                    }
                }
            }
        }
    }

    delete[] flag1;
    delete[] flag2;
    delete[] visit1;
    delete[] visit2;

    return result;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    // 初始化glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口对象
    GLFWwindow *window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 初始化glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 设置视口
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // 注册窗口变化监听
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_PROGRAM_POINT_SIZE);

    // 定义顶点数组
    float window_vertices[] = {
        -0.5,-0.5,0.0,
        0.5,-0.5,0.0,
        0.5,0.5,0.0,
        -0.5,0.5,0.0
    };

    float item_vertices[] = {
        -0.7f, -0.3f, 0.0f,
        -0.3f, -0.3f, 0.0f,
        -0.6f, 0.1f, 0.0f,
        -0.6f, 0.6f, 0.0f,
        -0.3f, 0.6f, 0.0f,
        0.3f, 0.0f, 0.0f,
        0.3f, 0.7f, 0.0f,
        -0.7f, 0.7f, 0.0f};

    // float item_vertices[] = {
    // -0.7,0.0,0.0,
    // 0.5,-0.5,0.0,
    // 0.0,0.7,0.0};

    // float item_vertices[] = {
    // -0.7,0.0,0.0,
    // -0.5,-0.5,0.0,
    // 0.5,-0.5,0.0,
    // 0.0,0.7,0.0};

    // float item_vertices[] = {
    //     0.0,0.7,0.0,
    //     -0.3,-0.7,0.0,
    //     0.0,0.0,0.0,
    //     0.3,-0.7,0.0
    // };

    // float item_vertices[] = {
    //     0.3,0.0,0.0,
    //     0.7,0.7,0.0,
    //     0.3,0.3,0.0,
    //     -0.5,0.5,0.0};

    // 创建缓冲对象
    unsigned int VBO_window, VAO_window;
    glGenVertexArrays(1, &VAO_window);
    glGenBuffers(1, &VBO_window);

    // 绑定VAO缓冲对象
    glBindVertexArray(VAO_window);

    // 绑定VBO缓对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO_window);
    // 填充VBO数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(window_vertices), window_vertices, GL_STATIC_DRAW);

    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // 创建缓冲对象
    unsigned int VBO_item, VAO_item;
    glGenVertexArrays(1, &VAO_item);
    glGenBuffers(1, &VBO_item);

    // 绑定VAO缓冲对象
    glBindVertexArray(VAO_item);

    // 绑定VBO缓对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO_item);
    // 填充VBO数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(item_vertices), item_vertices, GL_STATIC_DRAW);

    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // 多边形裁剪
    std::vector<Point> polygon1;
    std::vector<Point> polygon2;
    for (size_t i = 0; i < sizeof(item_vertices) / sizeof(float) / 3; i++) {
        Point p;
        p.x = item_vertices[i * 3];
        p.y = item_vertices[i * 3 + 1];
        polygon1.push_back(p);
    }
    for (size_t i = 0; i < sizeof(window_vertices) / sizeof(float) / 3; i++) {
        Point p;
        p.x = window_vertices[i * 3];
        p.y = window_vertices[i * 3 + 1];
        polygon2.push_back(p);
    }
    std::vector<std::vector<Point>> result = Weiler_Atherton(polygon1, polygon2);
    int num = result.size();

    // 创建顶点数组
    float **vertices = new float *[num]();
    for (int i = 0; i < num; i++) {
        vertices[i] = new float[result[i].size() * 3]();
        for (size_t j = 0; j < result[i].size(); j++) {
            vertices[i][j * 3] = result[i][j].x;
            vertices[i][j * 3 + 1] = result[i][j].y;
            vertices[i][j * 3 + 2] = 0.0f;
        }
    }

    // 创建缓冲对象
    unsigned int *VBO_result = new unsigned int[num]();
    unsigned int *VAO_result = new unsigned int[num]();
    for (int i = 0; i < num; i++) {
        glGenVertexArrays(1, &VAO_result[i]);
        glGenBuffers(1, &VBO_result[i]);

        // 绑定VAO缓冲对象
        glBindVertexArray(VAO_result[i]);

        // 绑定VBO缓对象
        glBindBuffer(GL_ARRAY_BUFFER, VBO_result[i]);
        // 填充VBO数据
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * result[i].size() * 3, vertices[i], GL_STATIC_DRAW);

        // 设置顶点属性指针
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    // 创建着色器程序
    Shader ourShader(PATH "vertexShader.vs", PATH "fragmentShader.fs");

    // // 设置线框绘制模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    while (!glfwWindowShouldClose(window)) {
        // 处理输入
        processInput(window);

        // 清空缓冲区
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        // 启用着色器程序
        ourShader.use();

        // 绘制
        ourShader.setVec3("color", 1.0 / 255.0, 75.0 / 255.0, 150.0 / 255.0);
        glBindVertexArray(VAO_window);
        glDrawArrays(GL_LINE_LOOP, 0, 4);

        ourShader.setVec3("color", 255.0 / 255.0, 0.0 / 255.0, 0.0 / 255.0);
        glBindVertexArray(VAO_item);
        glDrawArrays(GL_LINE_LOOP, 0, sizeof(item_vertices) / sizeof(float) / 3);

        ourShader.setVec3("color", 0.0 / 255.0, 255.0 / 255.0, 0.0 / 255.0);
        for (int i = 0; i < num; i++) {
            glBindVertexArray(VAO_result[i]);
            glDrawArrays(GL_LINE_LOOP, 0, result[i].size());
        }

        // 交换缓冲区
        glfwSwapBuffers(window);

        // 检查事件
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO_window);
    glDeleteBuffers(1, &VBO_window);
    glDeleteVertexArrays(1, &VAO_item);
    glDeleteBuffers(1, &VBO_item);
    for (int i = 0; i < num; i++) {
        glDeleteVertexArrays(1, &VAO_result[i]);
        glDeleteBuffers(1, &VBO_result[i]);
    }
    delete[] VAO_result;
    delete[] VBO_result;
    for (int i = 0; i < num; i++) {
        delete[] vertices[i];
    }
    delete[] vertices;
    glfwTerminate();
    return 0;
}
