// Copyright (c) 2024 DavidDeadly
#include "ipen.h"

Ipen::Ipen(IWindowManager *wm) { this->wm = wm; }

void Ipen::start() {
  this->wm->createWindow();
  this->wm->setUpListeners();
}
void Ipen::end() { this->wm->cleanUp(); }
