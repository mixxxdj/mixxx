#pragma once

#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>

#include "control/pollingcontrolproxy.h"
#include "util/parented_ptr.h"
#include "widget/wwidget.h"

class QPushButton;
class QDoubleSpinBox;
class QStackedLayout;
class SkinContext;

// Custom PushButton which emits a custom signal when right-clicked
class TapPushButton : public QPushButton {
    Q_OBJECT
  public:
    explicit TapPushButton(QWidget* parent = 0)
            : QPushButton(parent) {
    }

  protected:
    void mousePressEvent(QMouseEvent* e) override;

  signals:
    void rightClicked();
};

class BpmSpinBox : public QDoubleSpinBox {
    Q_OBJECT
  public:
    explicit BpmSpinBox(QWidget* pParent = 0)
            : QDoubleSpinBox(pParent) {
    }

    void setLineeditAlignment(QFlags<enum Qt::AlignmentFlag> flags) {
        lineEdit()->setAlignment(flags);
    }
  signals:
    void stepped(int steps);

  private:
    void stepBy(int steps) override;
};

class WBpmEditor : public WWidget {
    Q_OBJECT
  public:
    explicit WBpmEditor(const QString& group, QWidget* pParent = nullptr);

    void setup(const QDomNode& node, const SkinContext& context);

    enum class Mode {
        Listen,
        Select,
        Tap,
        Edit,
    };

    void setTapButtonTooltip(const QString& tooltip);
    void setEditButtonTooltip(const QString& tooltip);

  private slots:
    void switchMode(Mode mode);

  private:
    bool eventFilter(QObject* pObj, QEvent* pEvent) override;
    void startEditMode();
    void applySpinboxSteps(int steps);
    void applySpinboxValueAndQuit();

    PollingControlProxy m_tempoTapCO;
    PollingControlProxy m_bpmTapCO;
    PollingControlProxy m_trackLoadedCO;
    PollingControlProxy m_bpmCO;
    PollingControlProxy m_fileBpmCO;
    PollingControlProxy m_rateRatioCO;

    parented_ptr<QStackedLayout> m_pModeLayout;
    parented_ptr<QPushButton> m_pClickOverlay;
    parented_ptr<QWidget> m_pSelectWidget;
    parented_ptr<QPushButton> m_pTapSelectButton;
    parented_ptr<QPushButton> m_pEditSelectButton;
    parented_ptr<TapPushButton> m_pTapButton;
    parented_ptr<BpmSpinBox> m_pEditBox;

    QTimer m_hideTimer;

    int m_spinboxDecimals;
};
