// Copyright (c) 2024 DavidDeadly
#include "include/core/SkPaint.h"
#include <cstddef>
#include <iostream>
#include <vector>

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

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  if (key == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
    std::cout << "Mouse press" << std::endl;
  }
}

struct Line {
  double prevX, prevY, currX, currY;
};

std::vector<Line> lines;
static double prevX = -1, prevY = -1;
static void cursor_position_callback(GLFWwindow *window, double xpos,
                                     double ypos) {
  bool isNotDrawing =
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS;

  if (isNotDrawing) {
    prevX = -1;
    prevY = -1;
    return;
  }

  bool isValidLine = prevX >= 0 && prevY >= 0;
  if (isValidLine)
    lines.push_back({prevX, prevY, xpos, ypos});

  prevX = xpos;
  prevY = ypos;
}

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

App::App(std::string name) { this->name = name; }

void App::start() {
  int failedInit = !glfwInit();
  if (failedInit) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return;
  }

  window = createWindow(this);
  init_skia(width, height);

  glfwSwapInterval(1);
  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);

  canvas = surface->getCanvas();
  SkPaint paint;
  paint.setColor(SK_ColorRED);
  paint.setAntiAlias(true);
  paint.setStrokeWidth(2.0f);
  paint.setStyle(SkPaint::kStroke_Style);

  while (!glfwWindowShouldClose(window)) {
    canvas->clear(SK_ColorTRANSPARENT);

    for (const auto line : lines)
      canvas->drawLine(line.prevX, line.prevY, line.currX, line.currY, paint);

    context->flush();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
}

void App::init_skia(int w, int h) {
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

App::~App() {
  delete surface;
  delete context;
}
