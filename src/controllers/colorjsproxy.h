#ifndef COLORJSPROXY_H
#define COLORJSPROXY_H

#include <QObject>
#include <QScriptEngine>
#include <QScriptValue>

#include "util/color/color.h"

class ColorJSProxy: public QObject {
    Q_OBJECT
  public:
    ColorJSProxy(QScriptEngine* pScriptEngine);

    virtual ~ColorJSProxy();

    Q_INVOKABLE QScriptValue predefinedColorFromId(int iId);
    Q_INVOKABLE QScriptValue predefinedColorsList();

  private:
    QScriptValue jsColorFrom(PredefinedColorPointer predefinedColor);
    QScriptValue makePredefinedColorsList(QScriptEngine* pScriptEngine);
    QScriptEngine* m_pScriptEngine;
    QScriptValue m_predefinedColorsList;
};

#endif /* COLORJSPROXY_H */
