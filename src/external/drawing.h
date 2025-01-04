// Copyright (c) 2024 DavidDeadly
#pragma once

#include <GLFW/glfw3.h>
#include <stack>
#include <unordered_map>
#include <vector>

#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
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

  virtual void undo() = 0;
  virtual void redo() = 0;
  virtual void reset() = 0;
  virtual void changeColor(float rgba[4]) = 0;
  virtual void changeColor(float rgba[4], Color color) = 0;
  virtual void drawLine(bool isDrawing, double xpos, double ypos) = 0;
  virtual void eraseStroke(double xpos, double ypos) = 0;
};

struct SkiaPath {
  SkPath *path;
  SkPaint paint;

  SkiaPath(SkPath *path, SkPaint paint) : path(path), paint(paint) {}

  ~SkiaPath() { delete path; }
};

class SkiaManager : public IDrawingManager {
private:
  SkSurface *surface;
  GrDirectContext *context;

  SkPaint *currentPaint;
  SkPath *currentPath;
  SkColor currentColor = SK_ColorRED;

  std::stack<SkiaPath *> redoStack;
  std::vector<SkiaPath *> iPaths;

  std::unordered_map<Color, std::array<float, 4>> colors = {
      {RED, {1, 0, 0, 1}},
      {GREEN, {0, 0, 1, 1}},
      {BLUE, {0, 1, 0, 1}},
      {YELLOW, {1, 1, 0, 1}},
  };

  void clearRedoStack();
  SkPaint *generatePaint();

public:
  void init(int width, int height);
  void cleanUp();
  void display();

  void reset();
  void undo();
  void redo();
  void changeColor(float rgba[4]);
  void changeColor(float rgba[4], Color color);
  void eraseStroke(double xpos, double ypos);
  void drawLine(bool isDrawing, double xpos, double ypost);
};
