// Copyright (c) 2024 DavidDeadly
#include "external/drawing.h"
#include "external/window.h"

#include "app.h"
#include "ipen.h"

int main() {
  // App *app = new App("Ipen");
  //
  // app->start();
  IWindowManager *windowService = new GLFWWindowManager();
  IDrawingManager *drawingService = new SkiaManager();

  Ipen *ipen = new Ipen(windowService, drawingService);

  ipen->start();
  ipen->end();
}
