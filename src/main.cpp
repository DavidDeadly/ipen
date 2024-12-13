// Copyright (c) 2024 DavidDeadly
#include "external/window.h"

#include "app.h"
#include "ipen.h"

int main() {
  // App *app = new App("Ipen");
  //
  // app->start();

  IWindowManager *wm = new GLFWWindowManager();

  Ipen *ipen = new Ipen(wm);

  ipen->start();
  ipen->end();

  return 0;
}
