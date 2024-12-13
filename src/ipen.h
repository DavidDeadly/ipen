// Copyright (c) 2024 DavidDeadly
#pragma once

#include "external/window.h"

class Ipen {
private:
  IWindowManager *wm;

public:
  Ipen(IWindowManager *wm);

  void start();
  void end();
};
