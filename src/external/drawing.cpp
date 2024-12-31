// Copyright (c) 2024 DavidDeadly
#include "drawing.h"

#define SK_GANESH
#define SK_GL // For GrContext::MakeGL
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

  this->currentPaint = new SkPaint();
  this->currentPaint->setColor(this->currentColor);
  this->currentPaint->setAntiAlias(true);
  this->currentPaint->setStrokeWidth(2.0f);
  this->currentPaint->setStyle(SkPaint::kStroke_Style);
}

void SkiaManager::cleanUp() {
  delete surface;
  delete context;
}

SkPaint *SkiaManager::generatePaint() {
  bool samePaint = this->currentPaint->getColor() == this->currentColor;
  if (samePaint)
    return this->currentPaint;

  SkPaint *paint = new SkPaint();
  paint->setColor(this->currentColor);
  paint->setAntiAlias(true);
  paint->setStrokeWidth(2.0f);
  paint->setStyle(SkPaint::kStroke_Style);

  return paint;
}

void SkiaManager::display() {
  SkCanvas *canvas = this->surface->getCanvas();
  canvas->clear(SK_ColorTRANSPARENT);

  for (const auto path : this->paths)
    canvas->drawPath(*path.path, *path.paint);

  this->context->flush();
}

void SkiaManager::drawLine(bool isDrawing, double xpos, double ypos) {
  bool isNotDrawing = !isDrawing;
  if (isNotDrawing) {
    this->currentPath = NULL;
    return;
  }

  bool hasPath = std::find_if(this->paths.begin(), this->paths.end(),
                              [this](const auto &skiaPath) {
                                return skiaPath.path == this->currentPath;
                              }) != this->paths.end();

  if (!hasPath) {
    this->currentPaint = this->generatePaint();
    this->currentPath = new SkPath();
    this->currentPath->moveTo(xpos, ypos);

    this->paths.push_back({this->currentPath, this->currentPaint});
  }

  bool isValidLine = this->prevX >= 0 && this->prevY >= 0;
  if (isValidLine) {
    this->currentPath->lineTo(xpos, ypos);

    std::cout << "Drawing with cursor at: " << xpos << ", " << ypos
              << std::endl;
  }
}

void SkiaManager::changeColor(Color color) {
  std::cout << "SkiaManager - Changing color to: " << color << std::endl;

  SkColor skColor = this->colors[color];

  if (!skColor) {
    std::cerr << "Invalid color: " << color << std::endl;
    return;
  }

  std::cout << "SkiaManager - Color changed to: " << skColor << std::endl;
  this->currentColor = skColor;
}

void SkiaManager::reset() {
  this->paths.clear();
  this->prevX = -1;
  this->prevY = -1;
}
