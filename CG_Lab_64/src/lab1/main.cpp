#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Tool.h"

void bresenham(Tool tool, int x1, int y1, int x2, int y2) {
    if (x2 - x1 < 0) {
        int x0 = x2;
        int y0 = y2;
        x2 = x1;
        y2 = y1;
        x1 = x0;
        y1 = y0;
    }

    int dx = x2 - x1, dy = y2 - y1;
    int x0 = x1, y0 = y1;

    if (dy > 0) {
        if (dx > dy) {
            int pk = 2 * dy - dx;
            for (int i = 1; i <= dx; i++) {
                x0 += 1;
                if (pk < 0) {
                    pk += 2 * dy;
                } else {
                    y0 += 1;
                    pk += 2 * dy - 2 * dx;
                }
                const Color green{0.0f, 1.0f, 0.0f, 1.0f};
                tool.setPixel(x0, y0, green);
            }
        } else {
            int pk = 2 * dx - dy;
            for (int i = 1; i <= dy; i++) {
                y0 += 1;
                if (pk < 0) {
                    pk += 2 * dx;
                } else {
                    x0 += 1;
                    pk += 2 * dx - 2 * dy;
                }
                const Color green{0.0f, 1.0f, 0.0f, 1.0f};
                tool.setPixel(x0, y0, green);
            }
        }
    } else {
        if (dx > -dy) {
            int pk = 2 * dy + dx;
            for (int i = 1; i <= dx; i++) {
                x0 += 1;
                if (pk > 0) {
                    pk += 2 * dy;
                } else {
                    y0 -= 1;
                    pk += 2 * dy + 2 * dx;
                }
                const Color green{0.0f, 1.0f, 0.0f, 1.0f};
                tool.setPixel(x0, y0, green);
            }
        } else {
            int pk = -2 * dx - dy;
            for (int i = 1; i <= -dy; i++) {
                y0 -= 1;
                if (pk < 0) {
                    pk += 2 * dx;
                } else {
                    x0 += 1;
                    pk += 2 * dx + 2 * dy;
                }
                const Color green{0.0f, 1.0f, 0.0f, 1.0f};
                tool.setPixel(x0, y0, green);
            }
        }
    }
}

// 过取样反走样
void bresenham_antialiasing(Tool tool, int x1, int y1, int x2, int y2, int N=2) {
    if (x2 - x1 < 0) {
        int x0 = x2;
        int y0 = y2;
        x2 = x1;
        y2 = y1;
        x1 = x0;
        y1 = y0;
    }

    int dx = x2 - x1, dy = y2 - y1;
    int x0 = x1, y0 = y1;
    int lastx = x1, lasty = y1;
    int num = 0;

    if (dy > 0) {
        if (dx > dy) {
            int pk = 2 * dy * N - dx * N;
            for (int i = 1; i <= dx * N; i++) {
                num++;
                x0 += 1;
                if (pk < 0) {
                    pk += 2 * dy * N;
                } else {
                    y0 += 1;
                    pk += 2 * dy * N - 2 * dx * N;
                }
                if (((x0 - x1) / N != (lastx - x1) / N) || ((y0 - y1) / N != (lasty - y1) / N)) {
                    float value = num / (N * 1.0);
                    const Color green{0.0f, value, 0.0f, 1.0f};
                    tool.setPixel(x1 + (lastx - x1) / N, y1 + (lasty - y1) / N, green);
                    lastx = x0;
                    lasty = y0;
                    num = 0;
                }
            }
        } else {
            int pk = 2 * dx * N - dy * N;
            for (int i = 1; i <= dy * N; i++) {
                num++;
                y0 += 1;
                if (pk < 0) {
                    pk += 2 * dx * N;
                } else {
                    x0 += 1;
                    pk += 2 * dx * N - 2 * dy * N;
                }
                if (((x0 - x1) / N != (lastx - x1) / N) || ((y0 - y1) / N != (lasty - y1) / N)) {
                    float value = num / (N * 1.0);
                    const Color green{0.0f, value, 0.0f, 1.0f};
                    tool.setPixel(x1 + (lastx - x1) / N, y1 + (lasty - y1) / N, green);
                    lastx = x0;
                    lasty = y0;
                    num = 0;
                }
            }
        }
    } else {
        if (dx > -dy) {
            int pk = 2 * dy * N + dx * N;
            for (int i = 1; i <= dx * N; i++) {
                num++;
                x0 += 1;
                if (pk > 0) {
                    pk += 2 * dy * N;
                } else {
                    y0 -= 1;
                    pk += 2 * dy * N + 2 * dx * N;
                }
                if (((x0 - x1) / N != (lastx - x1) / N) || ((y0 - y1) / N != (lasty - y1) / N)) {
                    float value = num / (N * 1.0);
                    const Color green{0.0f, value, 0.0f, 1.0f};
                    tool.setPixel(x1 + (lastx - x1) / N, y1 + (lasty - y1) / N, green);
                    lastx = x0;
                    lasty = y0;
                    num = 0;
                }
            }
        } else {
            int pk = -2 * dx * N - dy * N;
            for (int i = 1; i <= -dy * N; i++) {
                num++;
                y0 -= 1;
                if (pk < 0) {
                    pk += 2 * dx * N;
                } else {
                    x0 += 1;
                    pk += 2 * dx * N + 2 * dy * N;
                }
                if (((x0 - x1) / N != (lastx - x1) / N) || ((y0 - y1) / N != (lasty - y1) / N)) {
                    float value = num / (N * 1.0);
                    const Color green{0.0f, value, 0.0f, 1.0f};
                    tool.setPixel(x1 + (lastx - x1) / N, y1 + (lasty - y1) / N, green);
                    lastx = x0;
                    lasty = y0;
                    num = 0;
                }
            }
        }
    }
}

int main() {
    // Window sizes
    const int width = 200;
    const int height = 150;
    Tool tool;
    if (!tool.init(width, height, "Tool"))
        return -1;

    const char *glsl_version = "#version 130";

    while (!tool.windowShouldClose()) {
        tool.pollEvents();
        tool.processInput();

        int x0 = 10, y0 = 30, x1 = 100, y1 = 150;
        bresenham_antialiasing(tool, x0, y0, x1, y1, 2);
		bresenham(tool, x0 + 100, y0, x1 + 100, y1);
        tool.flush(0.18f, 0.32f, 0.39f, 1.00f);
    }

    // Clean up
    tool.shutdown();
    return 0;
}