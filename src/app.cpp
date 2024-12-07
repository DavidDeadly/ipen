// Copyright (c) 2024 DavidDeadly
#include "include/core/SkPaint.h"
#include <cstddef>
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <libinput.h>
#include <linux/input.h>
#include <string>
#include <thread>
#include <unistd.h>

#define SK_GANESH
#define SK_GL // For GrContext::MakeGL
#include "GLFW/glfw3.h"

#include "app.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLAssembleInterface.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"

static GLFWwindow *createWindow(App *app) {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //(uncomment to enable correct color spaces)
  // glfwWindowHint(GLFW_SRGB_CAPABLE,
  // GL_TRUE);
  glfwWindowHint(GLFW_STENCIL_BITS, 0);
  // glfwWindowHint(GLFW_ALPHA_BITS, 0);
  glfwWindowHint(GLFW_DEPTH_BITS, 0);

  // window style
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
  glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);

  GLFWwindow *window = glfwCreateWindow(700, 700, "Ipen", NULL, NULL);
  if (!window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return NULL;
  }

  glfwGetWindowSize(window, &app->width, &app->height);

  glfwMakeContextCurrent(window);
  return window;
}

App::App(std::string name) {
  this->name = name;
  this->prevX = -1;
  this->prevY = -1;
  this->isNotDrawing = true;
}

App::~App() {
  delete surface;
  delete context;
  delete worker;

  libinput_unref(input);
  libinput_device_unref(this->device);
}

void App::start() {
  int failedInit = !glfwInit();
  if (failedInit) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return;
  }

  window = createWindow(this);
  initSkia(width, height);
  glfwSetWindowUserPointer(window, this);

  glfwSwapInterval(1);

  auto handler = initLibinput(window);
  if (handler)
    this->worker = new std::thread(handler);

  glfwSetKeyCallback(window, App::key_callback);
  glfwSetCursorPosCallback(window, App::cursor_position_callback);

  canvas = surface->getCanvas();
  SkPaint paint;
  paint.setColor(SK_ColorRED);
  paint.setAntiAlias(true);
  paint.setStrokeWidth(2.0f);
  paint.setStyle(SkPaint::kStroke_Style);

  auto threadId = std::this_thread::get_id();
  std::cout << "drawing on thread: " << threadId << std::endl;

  while (!glfwWindowShouldClose(window)) {
    canvas->clear(SK_ColorTRANSPARENT);

    for (const auto line : lines)
      canvas->drawLine(line.prevX, line.prevY, line.currX, line.currY, paint);

    context->flush();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  if (this->worker && this->worker->joinable())
    this->worker->join();

  glfwDestroyWindow(window);
  glfwTerminate();
}

void App::initSkia(int w, int h) {
  auto interface = GrGLMakeNativeInterface();
  if (interface == nullptr) {
    // backup plan. see
    // https://gist.github.com/ad8e/dd150b775ae6aa4d5cf1a092e4713add?permalink_comment_id=4680136#gistcomment-4680136
    interface = GrGLMakeAssembledInterface(
        nullptr, (GrGLGetProc) * [](void *, const char *p) -> void * {
          return (void *)glfwGetProcAddress(p);
        });
  }

  context = GrDirectContexts::MakeGL(interface).release();

  GrGLFramebufferInfo framebufferInfo;
  framebufferInfo.fFBOID = 0; // assume default framebuffer

  // We are always using OpenGL and we use RGBA8 internal format for both RGBA
  // and BGRA configs in OpenGL.
  //(replace line below with this one to enable correct color spaces)
  // framebufferInfo.fFormat = GL_SRGB8_ALPHA8;
  framebufferInfo.fFormat = GL_RGBA8;

  SkColorType colorType = kRGBA_8888_SkColorType;
  GrBackendRenderTarget backendRenderTarget =
      GrBackendRenderTargets::MakeGL(w, h,
                                     0, // sample count
                                     0, // stencil bits
                                     framebufferInfo);

  //(replace line below with this one to enable correct color spaces)
  // sSurface= SkSurfaces::WrapBackendRenderTarget(sContext,
  // backendRenderTarget, kBottomLeft_GrSurfaceOrigin, colorType,
  // SkColorSpace::MakeSRGB(), nullptr).release();
  surface = SkSurfaces::WrapBackendRenderTarget(context, backendRenderTarget,
                                                kBottomLeft_GrSurfaceOrigin,
                                                colorType, nullptr, nullptr)
                .release();
  if (surface == nullptr)
    abort();
}

// STACIC METHODS

void App::handle_tablet_event(App *app,
                              struct libinput_event_tablet_tool *tablet_event) {
  double xpos = libinput_event_tablet_tool_get_x(tablet_event);
  double ypos = libinput_event_tablet_tool_get_y(tablet_event);
  // double pressure = libinput_event_tablet_tool_get_pressure(tablet_event);

  // std::cout << "Pen movement: X=" << xpos << ", Y=" << ypos
  //           << ", Pressure=" << pressure << std::endl;

  if (app->isNotDrawing) {
    app->prevX = -1;
    app->prevY = -1;
    return;
  }

  bool isValidLine = app->prevX >= 0 && app->prevY >= 0;
  if (isValidLine) {
    app->lines.push_back({app->prevX, app->prevY, xpos, ypos});
    std::cout << "Drawing with pen at: " << xpos << ", " << ypos << std::endl;
  }

  app->prevX = xpos;
  app->prevY = ypos;
}

std::function<void()> App::handle_input_events(GLFWwindow *window) {
  return [window]() {
    auto threadId = std::this_thread::get_id();
    std::cout << "Listening for pen input on thread..." << threadId
              << std::endl;

    App *app = static_cast<App *>(glfwGetWindowUserPointer(window));
    std::cout << "app: " << &app << std::endl;

    while (libinput_dispatch(app->input) == 0) {
      struct libinput_event *event;

      while ((event = libinput_get_event(app->input)) != NULL) {
        auto type = libinput_event_get_type(event);

        if (type == LIBINPUT_EVENT_TABLET_TOOL_AXIS) {
          auto *tablet_event = libinput_event_get_tablet_tool_event(event);
          app->handle_tablet_event(app, tablet_event);
        }

        if (type == LIBINPUT_EVENT_TABLET_TOOL_TIP) {
          auto *tablet_event = libinput_event_get_tablet_tool_event(event);
          auto tip_state =
              libinput_event_tablet_tool_get_tip_state(tablet_event);

          app->isNotDrawing = tip_state == LIBINPUT_TABLET_TOOL_TIP_UP;
        }

        libinput_event_destroy(event);
      }
    }
  };
}

std::function<void()> App::initLibinput(GLFWwindow *window) {
  static struct libinput_interface interface = {
      [](const char *path, int flags, void *user_data) {
        return open(path, flags);
      },
      [](int fd, void *user_data) { close(fd); }};

  struct libinput *input = libinput_path_create_context(&interface, NULL);

  if (!input) {
    std::cerr << "Failed to initialize libinput context" << std::endl;
    return NULL;
  }

  // TODO: get pen path programmatically
  const char *path = "/dev/input/event4";
  struct libinput_device *device = libinput_path_add_device(input, path);
  if (!device) {
    std::cerr << "Failed to initialize input device" << std::endl;
    libinput_unref(input);
    return NULL;
  }

  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));
  app->input = input;
  app->device = device;

  auto handler = handle_input_events(window);

  return handler;
}

void App::key_callback(GLFWwindow *window, int key, int scancode, int action,
                       int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  if (key == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
    std::cout << "Mouse press" << std::endl;
  }
}

void App::cursor_position_callback(GLFWwindow *window, double xpos,
                                   double ypos) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  app->isNotDrawing =
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS;

  if (app->isNotDrawing) {
    app->prevX = -1;
    app->prevY = -1;
    return;
  }

  bool isValidLine = app->prevX >= 0 && app->prevY >= 0;
  if (isValidLine) {
    app->lines.push_back({app->prevX, app->prevY, xpos, ypos});
    std::cout << "Drawing with cursor at: " << xpos << ", " << ypos
              << std::endl;
  }

  app->prevX = xpos;
  app->prevY = ypos;
}
