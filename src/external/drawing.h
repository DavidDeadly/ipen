// Copyright (c) 2024 DavidDeadly
#pragma once

#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include <GLFW/glfw3.h>

class IDrawingManager {
public:
  virtual void init(int width, int height) = 0;
  virtual void cleanUp() = 0;

  virtual void display() = 0;
  virtual void drawLine(bool isDrawing, double xpos, double ypos) = 0;
};

struct SkiaLine {
  double prevX, prevY, currX, currY;
};

class SkiaManager : public IDrawingManager {
private:
  SkPaint *paint;
  SkSurface *surface;
  GrDirectContext *context;

  std::vector<SkiaLine> lines;
  double prevX, prevY;

  void setDefaults();

public:
  void init(int width, int height);
  void cleanUp();

  void display();
  void drawLine(bool isDrawing, double xpos, double ypost);
};
