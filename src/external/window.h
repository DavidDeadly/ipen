// Copyright (c) 2024 DavidDeadly
#pragma once

#include "drawing.h"
#include <GLFW/glfw3.h>
#include <functional>

class IWindowManager {
public:
  virtual void createWindow(IDrawingManager *pointer) = 0;
  virtual void cleanUp() = 0;
  virtual void setUpListeners() = 0;
  virtual void render() = 0;

  int width;
  int height;
};

class GLFWWindowManager : public IWindowManager {
private:
  GLFWwindow *window;
  const char *title = "Ipen";

public:
  GLFWWindowManager();

  void createWindow(IDrawingManager *pointer);
  void setUpListeners();
  void render();
  void cleanUp();
};
