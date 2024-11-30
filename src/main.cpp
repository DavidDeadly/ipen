// Copyright (c) 2024 DavidDeadly
#include <iostream>

#include "app.h"

const char *name = "Ipen";

int main() {
  App *app = new App(name, 800, 600);

  app->start();
}
