// Copyright (c) 2024 DavidDeadly
#include "window.h"
#include "drawing.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <unordered_map>

GLFWWindowManager::GLFWWindowManager() {
  std::cout << "Running GLFW: " << glfwGetVersionString() << std::endl;

  bool hasFaild = !glfwInit();
  if (hasFaild) {
    std::cout << "Failed to initialize GLFW" << std::endl;
    exit(1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //(uncomment to enable correct color spaces)
  // glfwWindowHint(GLFW_SRGB_CAPABLE,
  // GL_TRUE);
  glfwWindowHint(GLFW_STENCIL_BITS, 0);
  // glfwWindowHint(GLFW_ALPHA_BITS, 0);
  glfwWindowHint(GLFW_DEPTH_BITS, 0);
}

void GLFWWindowManager::cleanUp() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void GLFWWindowManager::createWindow(IDrawingManager *pointer) {
  if (this->window)
    return;

  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
  glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);

  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  glfwGetMonitorWorkarea(monitor, NULL, NULL, &this->width, &this->height);

  this->window =
      glfwCreateWindow(this->width, this->height, this->title, NULL, NULL);
  if (!this->window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(this->window);
  glfwSetWindowUserPointer(this->window, pointer);
}

std::unordered_map<int, Color> keyToColor = {
    {GLFW_KEY_R, RED},
    {GLFW_KEY_G, GREEN},
    {GLFW_KEY_B, BLUE},
    {GLFW_KEY_A, YELLOW},
};

static void keyboardCallback(GLFWwindow *window, int key, int scancode,
                             int action, int mods) {
  if (action != GLFW_PRESS)
    return;

  if (key == GLFW_KEY_ESCAPE) {
    glfwSetWindowShouldClose(window, GL_TRUE);
    return;
  }

  IDrawingManager *drawingManager =
      static_cast<IDrawingManager *>(glfwGetWindowUserPointer(window));

  if (key == GLFW_KEY_R && mods == GLFW_MOD_SHIFT) {
    return drawingManager->reset();
  }

  bool hasColor = keyToColor.contains(key);
  if (hasColor) {
    drawingManager->changeColor(keyToColor[key]);
    return;
  }
}

static void cursorCallBack(GLFWwindow *window, double xpos, double ypos) {
  IDrawingManager *drawingManager =
      static_cast<IDrawingManager *>(glfwGetWindowUserPointer(window));

  if (!drawingManager) {
    std::cerr << "No drawing manager found to process cursor movement"
              << std::endl;
    return;
  }

  bool isErasing =
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
  if (isErasing) {
    drawingManager->eraseStroke(xpos, ypos);
    return;
  }

  bool isDrawing =
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
  drawingManager->drawLine(isDrawing, xpos, ypos);
}

void GLFWWindowManager::setUpListeners() {
  glfwSetKeyCallback(window, keyboardCallback);
  glfwSetCursorPosCallback(window, cursorCallBack);
}

void GLFWWindowManager::render() {
  glfwSwapInterval(1);
  IDrawingManager *drawingManager =
      static_cast<IDrawingManager *>(glfwGetWindowUserPointer(window));

  if (!drawingManager) {
    std::cerr << "No drawing manager found to start rendering cycle"
              << std::endl;
    return;
  }

  while (!glfwWindowShouldClose(window)) {
    drawingManager->display();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}
