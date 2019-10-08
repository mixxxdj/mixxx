#ifndef COLORJSPROXY_H
#define COLORJSPROXY_H

#include <QObject>
#include <QJSEngine>
#include <QJSValue>

#include "util/color/color.h"

class ColorJSProxy: public QObject {
    Q_OBJECT
  public:
    ColorJSProxy(QJSEngine* pScriptEngine);

    virtual ~ColorJSProxy();

    Q_INVOKABLE QJSValue predefinedColorFromId(int iId);
    Q_INVOKABLE QJSValue predefinedColorsList();

  private:
    QJSValue jsColorFrom(PredefinedColorPointer predefinedColor);
    QJSValue makePredefinedColorsList(QJSEngine* pScriptEngine);
    QJSEngine* m_pScriptEngine;
    QJSValue m_predefinedColorsList;
};

#endif /* COLORJSPROXY_H */
