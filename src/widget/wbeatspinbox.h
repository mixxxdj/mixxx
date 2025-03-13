#pragma once

#include <QDoubleSpinBox>
#include <QLineEdit>

#include "control/controlproxy.h"
#include "widget/wbasewidget.h"

class QDomNode;
class SkinContext;

class WBeatSpinBox : public QDoubleSpinBox, public WBaseWidget {
    Q_OBJECT
  public:
    WBeatSpinBox(QWidget* parent, const ConfigKey& configKey, int decimals = 5,
            double minimum = 0.03125, double maximum = 512.00);

    void setup(const QDomNode& node, const SkinContext& context);

  private slots:
    void slotSpinboxValueChanged(double newValue);
    void slotControlValueChanged(double newValue);

  private:
    QString textFromValue(double value) const override;
    double valueFromText(const QString& text) const override;
    QValidator::State validate(QString& input, int& pos) const override;

    void stepBy(int steps) override;
    QString fractionString(int numerator, int denominator) const;

    ControlProxy m_valueControl;

    // for L/R button BeatJump/LoopMove
    ControlProxy beatJumpForward;
    ControlProxy beatJumpBackward;
    ControlProxy beatJumpSize;
    ControlProxy beatJumpOneForward;
    ControlProxy beatJumpOneBackward;
    ControlProxy beatJumpEightForward;
    ControlProxy beatJumpEightBackward;
    ControlProxy beatLoopEnabled;
    ControlProxy beatLoopSize;
    QSize sizeSpinBox;
    QSize sizeLoopBox;

    // for font scaling
    bool event(QEvent* pEvent) override;
    void keyPressEvent(QKeyEvent* pEvent) override;
    double m_scaleFactor;
};

// This is an inherited class that supports font scaling
class WBeatLineEdit : public QLineEdit {
    Q_OBJECT
  public:
    explicit WBeatLineEdit(QWidget* parent=0)
        : QLineEdit(parent),
          m_scaleFactor(1.0) {
    }

    void setScaleFactor(double scaleFactor) {
        m_scaleFactor = scaleFactor;
    }

  private:
    bool event(QEvent* pEvent) override;
    double m_scaleFactor;
};
