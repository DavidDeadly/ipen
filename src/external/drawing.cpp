// Copyright (c) 2024 DavidDeadly
#include "drawing.h"
#include "include/core/SkPaint.h"
#include <cstddef>

#define SK_GANESH
#define SK_GL // For GrContext::MakeGL
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>

#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMaskFilter.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLAssembleInterface.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"

void SkiaManager::init(int width, int height) {
  this->width = width;
  this->height = height;

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
  paint->setStrokeWidth(4);
  paint->setStyle(SkPaint::kStroke_Style);
  paint->setStrokeCap(SkPaint::kRound_Cap);
  paint->setStrokeJoin(SkPaint::kRound_Join);

  paint->setMaskFilter(
      SkMaskFilter::MakeBlur(SkBlurStyle::kSolid_SkBlurStyle, 1));

  delete this->currentPaint;
  return paint;
}

void SkiaManager::display() {
  SkCanvas *canvas = this->surface->getCanvas();
  canvas->clear(SK_ColorTRANSPARENT);

  for (const auto iPath : this->iPaths)
    canvas->drawPath(*iPath->path, iPath->paint);

  this->context->flush();
}

void SkiaManager::drawLine(bool isDrawing, double xpos, double ypos) {
  bool isNotDrawing = !isDrawing;
  if (isNotDrawing) {
    this->currentPath = NULL;
    return;
  }

  double clampedX = std::clamp(xpos, 0.0, (double)this->width);
  double clampedY = std::clamp(ypos, 0.0, (double)this->height);

  bool hasPath = std::find_if(this->iPaths.begin(), this->iPaths.end(),
                              [this](const auto &iPath) {
                                return iPath->path == this->currentPath;
                              }) != this->iPaths.end();

  if (!hasPath) {
    this->clearRedoStack();

    this->currentPaint = this->generatePaint();
    this->currentPath = new SkPath();
    this->currentPath->moveTo(clampedX, clampedY);

    auto path = new SkiaPath(this->currentPath, *this->currentPaint);
    this->iPaths.push_back(path);
  }

  this->currentPath->lineTo(clampedX, clampedY);
  std::cout << "Drawing with cursor at: " << clampedX << ", " << clampedY
            << std::endl;
}

SkColor rbgaToSkColor(float rgba[4]) {
  float red = rgba[0] * 255;
  float green = rgba[1] * 255;
  float blue = rgba[2] * 255;
  float alpha = rgba[3] * 255;

  return SkColorSetARGB(alpha, red, green, blue);
}

void SkiaManager::changeColor(float rgba[4]) {
  SkColor skColor = rbgaToSkColor(rgba);

  if (this->currentColor == skColor)
    return;

  this->currentColor = skColor;
  std::cout << "SkiaManager - Changing color from UI to: " << &rgba
            << std::endl;
}

void SkiaManager::changeColor(float rgba[4], Color color) {
  std::cout << "SkiaManager - Changing color to: " << color << std::endl;

  std::array<float, 4> rgbaColor = this->colors[color];
  std::cout << "rgbaColor: " << rgbaColor.data() << std::endl;
  if (rgbaColor.size() != 4) {
    std::cerr << "Invalid color: " << color << std::endl;
    return;
  }

  float *nextColor = rgbaColor.data();
  SkColor skColor = rbgaToSkColor(nextColor);

  rgba[0] = nextColor[0];
  rgba[1] = nextColor[1];
  rgba[2] = nextColor[2];
  rgba[3] = nextColor[3];

  std::cout << "SkiaManager - Color changed to: " << rgbaColor.data()
            << std::endl;
  this->currentColor = skColor;
}

void SkiaManager::reset() {
  this->surface->getCanvas()->clear(SK_ColorTRANSPARENT);

  for (auto iPath : this->iPaths) {
    delete iPath;
  }

  this->iPaths.clear();
}

// Utility function to calculate the distance from a point to a line segment
static float distanceToSegment(const SkPoint &lineStart, const SkPoint &lineEnd,
                               const SkPoint &point) {
  // Compute the vector from the line's start to end point
  SkVector lineDirection = lineEnd - lineStart;
  SkVector pointToStart = point - lineStart;

  // Compute the projection of the point onto the line segment
  float scaleFactor =
      pointToStart.dot(lineDirection) / lineDirection.dot(lineDirection);

  // Clamp the projection to the segment range [0, 1]
  float MIN = 0;
  float MAX = 1;
  scaleFactor = std::max(MIN, std::min(MAX, scaleFactor));

  // Calculate the closest point on the segment using the correct operation
  float closestPointX = lineStart.fX + scaleFactor * lineDirection.fX;
  float closestPointY = lineStart.fY + scaleFactor * lineDirection.fY;
  SkPoint closestPoint = SkPoint::Make(closestPointX, closestPointY);

  // Manually calculate the distance between the point and the projection
  float dx = closestPoint.fX - point.fX;
  float dy = closestPoint.fY - point.fY;

  // Euclidean distance
  float distance = std::pow(dx, 2) + std::pow(dy, 2);
  return std::sqrt(distance);
}

static bool isPointNearPath(SkiaPath *iPath, const SkPoint &point) {
  SkPath::Verb verb;
  SkPath::Iter iter(*iPath->path, false);
  SkPoint currentPoint, lastPoint;

  float strokeWidth = iPath->paint.getStrokeWidth();
  float simetricalStrokeWidth = strokeWidth / 2.0f;
  const float ERASER_HIT_BOX =
      simetricalStrokeWidth + 2.0f; // TODO: make it configurable

  bool firstSegment = true;
  while ((verb = iter.next(&currentPoint)) != SkPath::kDone_Verb) {
    if (verb != SkPath::kLine_Verb)
      continue;

    if (firstSegment) {
      firstSegment = false;
      continue;
    }

    float distance = distanceToSegment(lastPoint, currentPoint, point);
    if (distance <= ERASER_HIT_BOX)
      return true;

    lastPoint = currentPoint;
  }

  return false;
}

void SkiaManager::eraseStroke(double xpos, double ypos) {
  auto pathToEraseIter =
      std::find_if(this->iPaths.begin(), this->iPaths.end(),
                   [xpos, ypos](const auto &iPath) {
                     SkPoint clickedPoint = SkPoint::Make(xpos, ypos);

                     return isPointNearPath(iPath, clickedPoint);
                   });

  if (pathToEraseIter == this->iPaths.end())
    return;

  delete *pathToEraseIter;
  this->iPaths.erase(pathToEraseIter);

  std::cout << "SkiaManager - Erasing stroke at: " << xpos << ", " << ypos
            << std::endl;
}

void SkiaManager::undo() {
  if (this->iPaths.empty()) {
    std::cerr << "Nothing to undo!" << std::endl;
    return;
  }

  SkiaPath *lastPath = this->iPaths.back();

  redoStack.push(lastPath);
  this->iPaths.pop_back();

  std::cout << "Undo performed!" << std::endl;
}

void SkiaManager::redo() {
  if (this->redoStack.empty()) {
    std::cerr << "Nothing to redo!" << std::endl;
    return;
  }

  SkiaPath *lastPath = this->redoStack.top();
  this->iPaths.push_back(lastPath);
  this->redoStack.pop();

  std::cout << "Redo performed!" << std::endl;
}

void SkiaManager::clearRedoStack() {
  while (!this->redoStack.empty()) {
    SkiaPath *lastPath = this->redoStack.top();
    delete lastPath;

    this->redoStack.pop();
  }
}
