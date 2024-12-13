// Copyright (c) 2024 DavidDeadly
#pragma once

#include <GLFW/glfw3.h>

class IWindowManager {
public:
  virtual void createWindow() = 0;
  virtual void cleanUp() = 0;
  virtual void setUpListeners() = 0;
};

class GLFWWindowManager : public IWindowManager {
private:
  GLFWwindow *window;
  const char *title = "Ipen";

public:
  GLFWWindowManager();

  void createWindow();
  void setUpListeners();
  void cleanUp();
};
