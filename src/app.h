// Copyright (c) 2024 DavidDeadly
#pragma once

#include "GLFW/glfw3.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"

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

public:
  // Initializes GLFW and SKIA
  App(std::string name);

  // Destructor to clean up resources used for GLFW and SKIA
  ~App();

  void start();

private:
  void callback(GLFWwindow *window, double xpos, double ypos);

  void initSkia(int w, int h);
};
