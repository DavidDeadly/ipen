// Copyright (c) 2024 DavidDeadly
#include <cstddef>
#include <iostream>

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
}

static void draw_with_skia(SkCanvas *canvas, GrDirectContext *context) {
  // Clear the canvas
  canvas->clear(SK_AlphaTRANSPARENT);

  // Draw a red circle
  SkPaint paint;
  paint.setColor(SK_ColorRED);
  canvas->drawCircle(200, 200, 100, paint);

  // Draw a blue rectangle
  paint.setColor(SK_ColorBLUE);
  canvas->drawRect(SkRect::MakeXYWH(300, 150, 200, 100), paint);

  auto text = SkTextBlob::MakeFromString("Hello", SkFont(nullptr, 20));
  canvas->drawTextBlob(text, 50, 25, paint);

  paint.setColor(SK_ColorRED);
  paint.setAntiAlias(true);
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setStrokeWidth(10);

  canvas->drawLine(20, 20, 100, 100, paint);
  context->flush();
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

  GLFWwindow *window =
      glfwCreateWindow(app->width, app->height, "Ipen", NULL, NULL);
  if (!window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return NULL;
  }

  glfwMakeContextCurrent(window);
  return window;
}

App::App(std::string name, int width, int height) {
  this->width = width;
  this->height = height;
  this->name = name;
}

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

  SkCanvas *canvas = surface->getCanvas();

  while (!glfwWindowShouldClose(window)) {
    glfwWaitEvents();

    draw_with_skia(canvas, context);

    glfwSwapBuffers(window);
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
