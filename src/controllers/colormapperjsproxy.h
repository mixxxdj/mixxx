#pragma once

#include <QScriptContext>
#include <QScriptEngine>

#include "controllers/colormapper.h"

class ColorMapperJSProxy final : public QObject {
    Q_OBJECT
  public:
    ColorMapperJSProxy() = delete;
    ColorMapperJSProxy(QScriptEngine* pScriptEngine, QMap<QRgb, QVariant> availableColors);

    ~ColorMapperJSProxy() override {
        delete m_colorMapper;
    };

  public slots:
    QScriptValue getNearestColor(uint ColorCode);
    QScriptValue getNearestValue(uint ColorCode);

  private:
    QScriptEngine* m_pScriptEngine;
    ColorMapper* m_colorMapper;
};

QScriptValue ColorMapperJSProxyConstructor(QScriptContext* context, QScriptEngine* engine);
