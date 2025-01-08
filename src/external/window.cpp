// Copyright (c) 2024 DavidDeadly
#include "window.h"

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <unordered_map>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "drawing.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

GLFWWindowManager::GLFWWindowManager() {
  std::cout << "Running GLFW: " << glfwGetVersionString() << std::endl;

  glfwSetErrorCallback(glfw_error_callback);
  bool hasFaild = !glfwInit();

  if (hasFaild) {
    std::cout << "Failed to initialize GLFW" << std::endl;
    exit(1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_ALPHA_BITS, 8);
  glfwWindowHint(GLFW_STENCIL_BITS, 0);
  glfwWindowHint(GLFW_DEPTH_BITS, 0);
}

void GLFWWindowManager::cleanUp() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

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
      glfwCreateWindow(this->width, this->height, this->title, monitor, NULL);
  if (!this->window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(this->window);
  glfwSetWindowUserPointer(this->window, pointer);

  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplOpenGL3_Init();

  GLFWimage icon[1];
  icon[0].pixels =
      stbi_load("resources/ipen.png", &icon[0].width, &icon[0].height, 0, 4);

  if (!icon[0].pixels) {
    std::cerr << "Failed to load icon: " << stbi_failure_reason() << std::endl;
    return;
  }

  glfwSetWindowIcon(window, 1, icon);

  stbi_image_free(icon[0].pixels);
}

static float pen_color[4] = {1, 1, 1, 1};
std::unordered_map<int, Color> keyToColor = {
    {GLFW_KEY_W, WHITE}, {GLFW_KEY_Q, BLACK}, {GLFW_KEY_R, RED},
    {GLFW_KEY_G, GREEN}, {GLFW_KEY_B, BLUE},  {GLFW_KEY_A, YELLOW},
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

  if (key == GLFW_KEY_R && mods == GLFW_MOD_CONTROL)
    return drawingManager->reset();

  if (key == GLFW_KEY_Z && mods == GLFW_MOD_CONTROL)
    return drawingManager->undo();

  bool redoYCombination = key == GLFW_KEY_Y && mods == GLFW_MOD_CONTROL;
  bool redoZCombination =
      key == GLFW_KEY_Z && mods == GLFW_MOD_CONTROL + GLFW_MOD_SHIFT;
  if (redoYCombination || redoZCombination)
    return drawingManager->redo();

  bool hasColor = keyToColor.contains(key);
  if (hasColor) {
    drawingManager->changeColor(pen_color, keyToColor[key]);
    return;
  }
}

static void cursorCallBack(GLFWwindow *window, double xpos, double ypos) {
  bool guiFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
  if (guiFocused)
    return;

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

  ImGui_ImplGlfw_InitForOpenGL(this->window, true);
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

  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

  while (!glfwWindowShouldClose(window)) {
    drawingManager->display();

    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    drawingManager->changeColor(pen_color);

    {
      ImGui::Begin("Toolbar");

      ImGui::ColorEdit4("Pen color", pen_color);

      if (ImGui::Button("Reset"))
        drawingManager->reset();

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / io.Framerate, io.Framerate);

      ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}
