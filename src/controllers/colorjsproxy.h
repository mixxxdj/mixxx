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

    void setDefaultColor(const QColor& defaultColor);

  private:
    QScriptValue jsColorFrom(PredefinedColorPointer predefinedColor);
    // Initialization order is important: m_predefinedColorsList is initialized
    // with makePredefinedColorsList, which uses m_pScriptEngine.
    QScriptEngine* m_pScriptEngine;
    QScriptValue makePredefinedColorsList(QColor defaultColorRgba = Color::kPredefinedColorsSet.noColor->m_defaultRgba);
    QScriptValue m_predefinedColorsList;
};

#endif /* COLORJSPROXY_H */
