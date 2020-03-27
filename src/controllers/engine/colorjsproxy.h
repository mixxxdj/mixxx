#pragma once

#include <QJSValue>
#include <QObject>

#include "util/color/color.h"

class ControllerEngine;

class ColorJSProxy : public QObject {
    Q_OBJECT
  public:
    explicit ColorJSProxy(ControllerEngine* pControllerEngine);

    ~ColorJSProxy() override;

    Q_INVOKABLE QJSValue colorMapper(const QMap<QRgb, QVariant>& colorMap);

  private:
    ControllerEngine* m_pControllerEngine;
};