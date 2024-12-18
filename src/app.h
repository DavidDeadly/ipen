// Copyright (c) 2024 DavidDeadly
#pragma once

#include <functional>
#include <libinput.h>
#include <thread>
#include <vector>

#include "GLFW/glfw3.h"

struct Line {
  double prevX, prevY, currX, currY;
};

class App {
private:
  std::vector<Line> lines;
  double prevX, prevY;
  bool isNotDrawing;

  libinput_device *device;
  libinput *input;
  std::thread *worker;

public:
  ~App();

  void start();

private:
  void initSkia(int w, int h);
  std::function<void()> initLibinput(GLFWwindow *window);

  static std::function<void()> handle_input_events(GLFWwindow *window);
  static void
  handle_tablet_event(App *app,
                      struct libinput_event_tablet_tool *tablet_event);
};
