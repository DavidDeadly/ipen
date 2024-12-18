// Copyright (c) 2024 DavidDeadly
#include "ipen.h"

Ipen::Ipen(IWindowManager *wm, IDrawingManager *dm) {
  this->wm = wm;
  this->dm = dm;
}

void Ipen::start() {
  this->wm->createWindow(this->dm);
  this->wm->setUpListeners();

  this->dm->init(this->wm->width, this->wm->height);

  this->wm->render();
}

void Ipen::end() {
  this->wm->cleanUp();
  this->dm->cleanUp();
}
