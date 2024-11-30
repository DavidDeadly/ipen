// Copyright (c) 2024 DavidDeadly
#include <iostream>

#include "GLFW/glfw3.h"
#include "app.h"

const char *name = "Ipen";

int main() {
  App *app = new App(name);

  app->start();
}
