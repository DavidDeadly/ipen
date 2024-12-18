// Copyright (c) 2024 DavidDeadly
#include "drawing.h"

#include <GLFW/glfw3.h>
#include <iostream>

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLAssembleInterface.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"

void SkiaManager::init(int width, int height) {
  auto interface = GrGLMakeNativeInterface();
  if (interface == nullptr) {
    // backup plan. see
    // https://gist.github.com/ad8e/dd150b775ae6aa4d5cf1a092e4713add?permalink_comment_id=4680136#gistcomment-4680136
    interface = GrGLMakeAssembledInterface(
        nullptr, (GrGLGetProc) * [](void *, const char *p) -> void * {
          return (void *)glfwGetProcAddress(p);
        });
  }

  this->context = GrDirectContexts::MakeGL(interface).release();

  GrGLFramebufferInfo framebufferInfo;
  framebufferInfo.fFBOID = 0; // assume default framebuffer

  // We are always using OpenGL and we use RGBA8 internal format for both RGBA
  // and BGRA configs in OpenGL.
  //(replace line below with this one to enable correct color spaces)
  // framebufferInfo.fFormat = GL_SRGB8_ALPHA8;
  framebufferInfo.fFormat = GL_RGBA8;

  SkColorType colorType = kRGBA_8888_SkColorType;
  GrBackendRenderTarget backendRenderTarget =
      GrBackendRenderTargets::MakeGL(width, height,
                                     0, // sample count
                                     0, // stencil bits
                                     framebufferInfo);

  //(replace line below with this one to enable correct color spaces)
  // sSurface= SkSurfaces::WrapBackendRenderTarget(sContext,
  // backendRenderTarget, kBottomLeft_GrSurfaceOrigin, colorType,
  // SkColorSpace::MakeSRGB(), nullptr).release();
  this->surface = SkSurfaces::WrapBackendRenderTarget(
                      context, backendRenderTarget, kBottomLeft_GrSurfaceOrigin,
                      colorType, nullptr, nullptr)
                      .release();
  if (this->surface == nullptr)
    abort();

  this->setDefaults();
}

void SkiaManager::cleanUp() {
  delete surface;
  delete context;
}

void SkiaManager::setDefaults() {
  this->paint = new SkPaint();
  paint->setColor(SK_ColorRED);
  paint->setAntiAlias(true);
  paint->setStrokeWidth(2.0f);
  paint->setStyle(SkPaint::kStroke_Style);
}

void SkiaManager::display() {
  SkCanvas *canvas = this->surface->getCanvas();
  canvas->clear(SK_ColorTRANSPARENT);

  for (const auto line : this->lines)
    canvas->drawLine(line.prevX, line.prevY, line.currX, line.currY,
                     *this->paint);

  this->context->flush();
}

void SkiaManager::drawLine(bool isDrawing, double xpos, double ypos) {
  bool isNotDrawing = !isDrawing;

  if (isNotDrawing) {
    this->prevX = -1;
    this->prevY = -1;
    return;
  }

  bool isValidLine = this->prevX >= 0 && this->prevY >= 0;
  if (isValidLine) {
    this->lines.push_back({this->prevX, this->prevY, xpos, ypos});
    std::cout << "Drawing with cursor at: " << xpos << ", " << ypos
              << std::endl;
  }

  this->prevX = xpos;
  this->prevY = ypos;
}
