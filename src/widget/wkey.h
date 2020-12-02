#ifndef WKEY_H
#define WKEY_H

#include <QByteArrayData>
#include <QDomNode>
#include <QLabel>
#include <QString>

#include "control/controlproxy.h"
#include "skin/skincontext.h"
#include "widget/wlabel.h"

class QObject;
class QWidget;

class WKey : public WLabel  {
    Q_OBJECT
  public:
    explicit WKey(const QString& group, QWidget* pParent = nullptr);

    void onConnectedControlChanged(double dParameter, double dValue) override;
    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void setValue(double dValue);
    void keyNotationChanged(double dValue);
    void setCents();

  private:
    double m_dOldValue;
    bool m_displayCents;
    bool m_displayKey;
    ControlProxy m_keyNotation;
    ControlProxy m_engineKeyDistance;
};

#endif /* WKEY_H */
