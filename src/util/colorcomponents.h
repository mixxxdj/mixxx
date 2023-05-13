#pragma once

#include <QColor>

void getHsvF(const QColor& color, float* pH, float* pS, float* pV, float* pA = nullptr);
void getHslF(const QColor& color, float* pH, float* pS, float* pL, float* pA = nullptr);
void getRgbF(const QColor& color, float* pR, float* pG, float* pB, float* pA = nullptr);
