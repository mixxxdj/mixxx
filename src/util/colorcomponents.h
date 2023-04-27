#pragma once

#include <QColor>

void getHsvF(const QColor& color, float* h, float* s, float* v, float* a = nullptr);
void getHslF(const QColor& color, float* h, float* s, float* l, float* a = nullptr);
void getRgbF(const QColor& color, float* r, float* g, float* b, float* a = nullptr);
