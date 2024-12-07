// Copyright (c) 2024 DavidDeadly
#pragma once

#include <libinput.h>
#include <thread>
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

  libinput_device *device;
  libinput *input;
  std::thread *worker;

public:
  // Initializes GLFW and SKIA
  App(std::string name);

  // Destructor to clean up resources used for GLFW and SKIA
  ~App();

  void start();

private:
  void initSkia(int w, int h);
  std::function<void()> initLibinput(GLFWwindow *window);

  static std::function<void()> handle_input_events(GLFWwindow *window);
  static void
  handle_tablet_event(App *app,
                      struct libinput_event_tablet_tool *tablet_event);

  static void key_callback(GLFWwindow *window, int key, int scancode,
                           int action, int mods);

  static void cursor_position_callback(GLFWwindow *window, double xpos,
                                       double ypos);
};
