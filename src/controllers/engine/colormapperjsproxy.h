#pragma once

#include <QJSEngine>

#include "controllers/colormapper.h"

// This is a wrapper class that exposes ColorMapper via the QScriptEngine and
// makes it possible to create and use ColorMapper object from JavaScript
// controller mappings.
class ColorMapperJSProxy final : public QObject {
    Q_OBJECT
  public:
    ColorMapperJSProxy() = delete;
    ColorMapperJSProxy(QJSEngine* pScriptEngine, QMap<QRgb, QVariant> availableColors);

    ~ColorMapperJSProxy() override {
        delete m_colorMapper;
    };

    // Q_INVOKABLE is need here because these methods callable from controller
    // scripts

    // For a given RGB color code (e.g. 0xFF0000), this finds the nearest
    // available color and returns a JS object with properties "red", "green",
    // "blue" (each with value range 0-255).
    Q_INVOKABLE QJSValue getNearestColor(uint ColorCode);

    // For a given RGB color code (e.g. 0xFF0000), this finds the nearest
    // available color, then returns the value associated with that color
    // (which could be a MIDI byte value for example).
    Q_INVOKABLE QJSValue getValueForNearestColor(uint ColorCode);

  private:
    QJSEngine* m_pScriptEngine;
    ColorMapper* m_colorMapper;
};