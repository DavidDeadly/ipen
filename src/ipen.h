// Copyright (c) 2024 DavidDeadly
#pragma once

#include "external/drawing.h"
#include "external/window.h"

class Ipen {
private:
  IWindowManager *wm;
  IDrawingManager *dm;

public:
  Ipen(IWindowManager *wm, IDrawingManager *dm);

  void start();
  void end();
};
