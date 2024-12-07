// Copyright (c) 2024 DavidDeadly
#pragma once

#include <vector>

#include "GLFW/glfw3.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"

struct Line {
  double prevX, prevY, currX, currY;
};

class App {
public:
  std::string name;
  int width;
  int height;

private:
  SkCanvas *canvas;
  SkSurface *surface;
  GrDirectContext *context;
  GLFWwindow *window;

  std::vector<Line> lines;
  double prevX, prevY;
  bool isNotDrawing;

public:
  // Initializes GLFW and SKIA
  App(std::string name);

  // Destructor to clean up resources used for GLFW and SKIA
  ~App();

  void start();

  static void cursor_position_callback(GLFWwindow *window, double xpos,
                                       double ypos);

private:
  void initSkia(int w, int h);

  static void key_callback(GLFWwindow *window, int key, int scancode,
                           int action, int mods);
};
