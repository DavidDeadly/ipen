// Copyright (c) 2024 DavidDeadly
#include "window.h"

#include <GLFW/glfw3.h>
#include <iostream>

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

void GLFWWindowManager::createWindow() {
  if (this->window)
    return;

  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
  glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);

  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode = glfwGetVideoMode(monitor);

  glfwWindowHint(GLFW_RED_BITS, mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

  this->window =
      glfwCreateWindow(mode->width, mode->height, title, monitor, NULL);
  if (!this->window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(window);
}

void keyboardCallback(GLFWwindow *window, int key, int scancode, int action,
                      int mods) {
  if (action != GLFW_PRESS)
    return;

  if (key == GLFW_KEY_ESCAPE)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

void cursorCallBack(GLFWwindow *window, double xpos, double ypos) {
  std::cout << "NEW Mouse position: " << xpos << ", " << ypos << std::endl;
}

void GLFWWindowManager::setUpListeners() {
  glfwSetKeyCallback(window, keyboardCallback);
  glfwSetCursorPosCallback(window, cursorCallBack);

  glfwSwapInterval(1);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}
