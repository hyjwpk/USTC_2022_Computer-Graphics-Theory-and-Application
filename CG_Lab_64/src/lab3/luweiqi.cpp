#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <iostream>

#define PATH "src/lab3/"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef long LONG;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float rotate = 0.0f;

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 摄像机移动
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // 切换绘制模式
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

    // 旋转
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        rotate += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        rotate -= 0.01f;
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
    float xpos = xposIn;
    float ypos = yposIn;

    // 鼠标位置初始化
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // 计算鼠标偏移量
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    // 更新lastX和lastY
    lastX = xpos;
    lastY = ypos;

    // 更新俯仰角和偏航角
    camera.ProcessMouseMovement(xoffset, yoffset);
}

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
    ~Submodel() {
        delete[] triganles_array;
    }
};

int texture_num;
int material_num;
int vertex_num;
int texture_coordinates_num;
int normal_num;
int model_group_num;

std::string *texture_array;
Material *material_array;
Vertex *vertex_array;
Texture_coordinates *coordinate_array;
Normal *normal_array;
Submodel *submodel_array;

int readfile(std::string name) {
    std::ifstream file;
    file.open(name);
    if (!file.is_open()) {
        return false;
    }
    std::string temp;
    // 读入纹理
    file >> texture_num;
    texture_array = new std::string[texture_num];
    for (int i = 0; i < texture_num; i++) {
        file >> texture_array[i];
    }
    // 读入材质
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
    // 读入顶点
    file >> vertex_num;
    vertex_array = new Vertex[vertex_num];
    for (int i = 0; i < vertex_num; i++) {
        file >> vertex_array[i].x;
        file >> vertex_array[i].y;
        file >> vertex_array[i].z;
    }
    // 读入贴图坐标
    file >> texture_coordinates_num;
    coordinate_array = new Texture_coordinates[texture_coordinates_num];
    for (int i = 0; i < texture_coordinates_num; i++) {
        file >> coordinate_array[i].value[0];
        file >> coordinate_array[i].value[1];
    }
    // 读入法线
    file >> normal_num;
    normal_array = new Normal[normal_num];
    for (int i = 0; i < normal_num; i++) {
        for (int j = 0; j < 3; j++) {
            file >> normal_array[i].value[j];
        }
    }
    // 读入模型分组
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

void readtextures(std::string name, GLuint *texture) {
    std::string absoulte_path = PATH;
    name = absoulte_path + name;
    BITMAPINFOHEADER header; // 位图信息头
    std::ifstream file;
    file.open(name, std::ios::binary);
    if (!file.is_open()) {
        exit(-1);
    }
    file.seekg(14, std::ios::beg);                        // 文件头偏移
    file.read((char *)&header, sizeof(BITMAPINFOHEADER)); // 读入位图信息头
    int bytes = header.biBitCount / 8;                    // 每个像素含有的字节数
    // 读取长宽
    int width = std::abs(header.biWidth);
    int height = std::abs(header.biHeight);
    BYTE *pixels = new BYTE[width * height * bytes];
    file.read((char *)pixels, width * height * bytes * sizeof(char)); // 读取位图

    // 将图片转换为RGB格式
    BYTE *data = new BYTE[width * height * 3];
    for (int i = 0; i < width * height; i++) {
        data[i * 3] = pixels[i * bytes + 2];
        data[i * 3 + 1] = pixels[i * bytes + 1];
        data[i * 3 + 2] = pixels[i * bytes];
    }

    // 设置纹理环绕
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    // 设置纹理过滤
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 设置多级渐远纹理
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 生成纹理
    glGenTextures(1, texture);
    // 绑定纹理
    glBindTexture(GL_TEXTURE_2D, *texture);

    // 生成纹理
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    free(pixels);
    free(data);
    file.close();
}

int main() {
    // 读取模型文件
    if (!readfile(PATH "luweiqi.txt")) {
        exit(-1);
    }

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
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 启用深度缓冲
    glEnable(GL_DEPTH_TEST);

    // glEnable(GL_PROGRAM_POINT_SIZE);

    // 光源立方体顶点数据
    float cube[] = {
        -0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f,  
         0.5f,  0.5f, -0.5f,  
         0.5f,  0.5f, -0.5f,  
        -0.5f,  0.5f, -0.5f, 
        -0.5f, -0.5f, -0.5f, 

        -0.5f, -0.5f,  0.5f, 
         0.5f, -0.5f,  0.5f,  
         0.5f,  0.5f,  0.5f,  
         0.5f,  0.5f,  0.5f,  
        -0.5f,  0.5f,  0.5f, 
        -0.5f, -0.5f,  0.5f, 

        -0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f, -0.5f, 
        -0.5f, -0.5f, -0.5f, 
        -0.5f, -0.5f, -0.5f, 
        -0.5f, -0.5f,  0.5f, 
        -0.5f,  0.5f,  0.5f, 

         0.5f,  0.5f,  0.5f,  
         0.5f,  0.5f, -0.5f,  
         0.5f, -0.5f, -0.5f,  
         0.5f, -0.5f, -0.5f,  
         0.5f, -0.5f,  0.5f,  
         0.5f,  0.5f,  0.5f,  

        -0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f,  
         0.5f, -0.5f,  0.5f,  
         0.5f, -0.5f,  0.5f,  
        -0.5f, -0.5f,  0.5f, 
        -0.5f, -0.5f, -0.5f, 

        -0.5f,  0.5f, -0.5f, 
         0.5f,  0.5f, -0.5f,  
         0.5f,  0.5f,  0.5f,  
         0.5f,  0.5f,  0.5f,  
        -0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f, -0.5f, 
    };

    // 定义顶点数组
    float **vertices = new float *[model_group_num];
    for (int i = 0; i < model_group_num; i++) {
        vertices[i] = new float[submodel_array[i].triangles_num * 3 * 8];
        for (int j = 0; j < submodel_array[i].triangles_num; j++) {
            vertices[i][j * 3 * 8 + 0] = vertex_array[submodel_array[i].triganles_array[j].vi1 - 1].x;
            vertices[i][j * 3 * 8 + 1] = vertex_array[submodel_array[i].triganles_array[j].vi1 - 1].y;
            vertices[i][j * 3 * 8 + 2] = vertex_array[submodel_array[i].triganles_array[j].vi1 - 1].z;
            vertices[i][j * 3 * 8 + 3] = coordinate_array[submodel_array[i].triganles_array[j].ti1 - 1].value[0];
            vertices[i][j * 3 * 8 + 4] = coordinate_array[submodel_array[i].triganles_array[j].ti1 - 1].value[1];
            vertices[i][j * 3 * 8 + 5] = normal_array[submodel_array[i].triganles_array[j].ni1 - 1].value[0];
            vertices[i][j * 3 * 8 + 6] = normal_array[submodel_array[i].triganles_array[j].ni1 - 1].value[1];
            vertices[i][j * 3 * 8 + 7] = normal_array[submodel_array[i].triganles_array[j].ni1 - 1].value[2];
            vertices[i][j * 3 * 8 + 8] = vertex_array[submodel_array[i].triganles_array[j].vi2 - 1].x;
            vertices[i][j * 3 * 8 + 9] = vertex_array[submodel_array[i].triganles_array[j].vi2 - 1].y;
            vertices[i][j * 3 * 8 + 10] = vertex_array[submodel_array[i].triganles_array[j].vi2 - 1].z;
            vertices[i][j * 3 * 8 + 11] = coordinate_array[submodel_array[i].triganles_array[j].ti2 - 1].value[0];
            vertices[i][j * 3 * 8 + 12] = coordinate_array[submodel_array[i].triganles_array[j].ti2 - 1].value[1];
            vertices[i][j * 3 * 8 + 13] = normal_array[submodel_array[i].triganles_array[j].ni2 - 1].value[0];
            vertices[i][j * 3 * 8 + 14] = normal_array[submodel_array[i].triganles_array[j].ni2 - 1].value[1];
            vertices[i][j * 3 * 8 + 15] = normal_array[submodel_array[i].triganles_array[j].ni2 - 1].value[2];
            vertices[i][j * 3 * 8 + 16] = vertex_array[submodel_array[i].triganles_array[j].vi3 - 1].x;
            vertices[i][j * 3 * 8 + 17] = vertex_array[submodel_array[i].triganles_array[j].vi3 - 1].y;
            vertices[i][j * 3 * 8 + 18] = vertex_array[submodel_array[i].triganles_array[j].vi3 - 1].z;
            vertices[i][j * 3 * 8 + 19] = coordinate_array[submodel_array[i].triganles_array[j].ti3 - 1].value[0];
            vertices[i][j * 3 * 8 + 20] = coordinate_array[submodel_array[i].triganles_array[j].ti3 - 1].value[1];
            vertices[i][j * 3 * 8 + 21] = normal_array[submodel_array[i].triganles_array[j].ni3 - 1].value[0];
            vertices[i][j * 3 * 8 + 22] = normal_array[submodel_array[i].triganles_array[j].ni3 - 1].value[1];
            vertices[i][j * 3 * 8 + 23] = normal_array[submodel_array[i].triganles_array[j].ni3 - 1].value[2];
        }
    }

    // 读取纹理
    unsigned int *texture = new unsigned int[texture_num];
    for (int i = 0; i < texture_num; i++) {
        readtextures(texture_array[i], &texture[i]);
    }

    // 创建缓冲对象
    unsigned int VBO[model_group_num], VAO[model_group_num];

    for (int i = 0; i < model_group_num; i++) {
        // 创建VAO缓冲对象
        glGenVertexArrays(1, &VAO[i]);
        // 绑定VAO缓冲对象
        glBindVertexArray(VAO[i]);
        // 创建VBO缓冲对象
        glGenBuffers(1, &VBO[i]);
        // 绑定VBO缓冲对象
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        // 填充VBO数据
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * submodel_array[i].triangles_num * 3 * 8, vertices[i], GL_STATIC_DRAW);
        // 设置顶点属性指针
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }

    unsigned int lightCubeVAO, lightCubeVBO;
    // 创建VAO缓冲对象
    glGenVertexArrays(1, &lightCubeVAO);
    // 绑定VAO缓冲对象
    glBindVertexArray(lightCubeVAO);
    // 创建VBO缓冲对象
    glGenBuffers(1, &lightCubeVBO);
    // 绑定VBO缓冲对象
    glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
    // 填充VBO数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // 创建着色器程序
    Shader ourShader(PATH "luweiqi.vs", PATH "luweiqi.fs");
    Shader cubeShader(PATH "light_cube.vs", PATH "light_cube.fs");

    // 设置线框绘制模式
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window)) {
        // 记录两帧之间的时间差
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 处理输入
        processInput(window);

        // 清空缓冲区
        glClearColor(25.0 / 255.0, 25.0 / 255.0, 25.0 / 255.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 设置矩阵
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, rotate * glm::radians(50.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // 设置光源位置
        glm::vec3 lightPos(-1.0f + 5 * sin((float)glfwGetTime()), -3.0f + 5 * cos((float)glfwGetTime()), 1.0f);

        // 启用着色器程序
        ourShader.use();
        ourShader.setMat4("model", model);
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        ourShader.setVec3("lightPos", lightPos);
        ourShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        ourShader.setVec3("viewPos", camera.Position);

        // 绘制模型
        for (int i = 0; i < model_group_num; i++) {
            ourShader.setVec3("material.ambient", material_array[i].ambient[0], material_array[i].ambient[1], material_array[i].ambient[2]);
            ourShader.setVec3("material.diffuse", material_array[i].diffuse[0], material_array[i].diffuse[1], material_array[i].diffuse[2]);
            ourShader.setVec3("material.specular", material_array[i].specular[0], material_array[i].specular[1], material_array[i].specular[2]);
            ourShader.setVec3("material.emission", material_array[i].emission[0], material_array[i].emission[1], material_array[i].emission[2]);
            ourShader.setFloat("material.shininess", material_array[i].shininess);
            glBindTexture(GL_TEXTURE_2D, texture[i]);
            glBindVertexArray(VAO[i]);
            glDrawArrays(GL_TRIANGLES, 0, submodel_array[i].triangles_num * 3);
        }

        // 启用着色器程序
        cubeShader.use();
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));
        cubeShader.setMat4("model", model);
        cubeShader.setMat4("view", view);
        cubeShader.setMat4("projection", projection);

        // 绘制光源
        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 交换缓冲区
        glfwSwapBuffers(window);

        // 检查事件
        glfwPollEvents();
    }

    // 释放内存
    for (int i = 0; i < model_group_num; i++) {
        delete[] vertices[i];
    }
    delete[] vertices;
    delete[] texture;
    delete[] texture_array;
    delete[] material_array;
    delete[] vertex_array;
    delete[] coordinate_array;
    delete[] normal_array;
    delete[] submodel_array;
    for (int i = 0; i < model_group_num; i++) {
        glDeleteBuffers(1, &VBO[i]);
        glDeleteVertexArrays(1, &VAO[i]);
    }
    glDeleteBuffers(1, &lightCubeVBO);
    glDeleteVertexArrays(1, &lightCubeVAO);

    glfwTerminate();
    return 0;
}
