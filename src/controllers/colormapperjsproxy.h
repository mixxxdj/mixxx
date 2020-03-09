#pragma once

#include <QScriptContext>
#include <QScriptEngine>

#include "controllers/colormapper.h"

// This is a wrapper class that exposes ColorMapper via the QScriptEngine and
// makes it possible to create and use ColorMapper object from JavaScript
// controller mappings.
class ColorMapperJSProxy final : public QObject {
    Q_OBJECT
  public:
    ColorMapperJSProxy() = delete;
    ColorMapperJSProxy(QScriptEngine* pScriptEngine, QMap<QRgb, QVariant> availableColors);

    ~ColorMapperJSProxy() override {
        delete m_colorMapper;
    };

  public slots:
    // These slots have a return value, because they are callable from
    // controller scripts
    QScriptValue getNearestColor(uint ColorCode);
    QScriptValue getNearestValue(uint ColorCode);

  private:
    QScriptEngine* m_pScriptEngine;
    ColorMapper* m_colorMapper;
};

QScriptValue ColorMapperJSProxyConstructor(QScriptContext* context, QScriptEngine* engine);
