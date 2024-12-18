// Copyright (c) 2024 DavidDeadly
#include "external/drawing.h"
#include "external/window.h"

#include "ipen.h"

int main() {
  IWindowManager *windowService = new GLFWWindowManager();
  IDrawingManager *drawingService = new SkiaManager();

  Ipen *ipen = new Ipen(windowService, drawingService);

  ipen->start();
  ipen->end();
}
