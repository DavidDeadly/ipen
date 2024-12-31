// Copyright (c) 2024 DavidDeadly
#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>

#include "include/core/SkColor.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"

enum Color {
  RED,
  GREEN,
  BLUE,
  YELLOW,
};

class IDrawingManager {
public:
  virtual void init(int width, int height) = 0;
  virtual void cleanUp() = 0;

  virtual void display() = 0;
  virtual void drawLine(bool isDrawing, double xpos, double ypos) = 0;
  virtual void reset() = 0;
  virtual void changeColor(Color color) = 0;
};

struct SkiaLine {
  double prevX, prevY, currX, currY;
  SkPaint *paint;
};

class SkiaManager : public IDrawingManager {
private:
  SkPaint *currentPaint;
  SkSurface *surface;
  GrDirectContext *context;

  std::vector<SkiaLine> lines;
  double prevX, prevY;

  SkPaint *generatePaint();
  SkColor currentColor = SK_ColorRED;

  std::unordered_map<Color, SkColor> colors = {
      {RED, SK_ColorRED},
      {GREEN, SK_ColorGREEN},
      {BLUE, SK_ColorBLUE},
      {YELLOW, SK_ColorYELLOW},
  };

public:
  void init(int width, int height);
  void cleanUp();

  void display();
  void reset();
  void drawLine(bool isDrawing, double xpos, double ypost);
  void changeColor(Color color);
};
