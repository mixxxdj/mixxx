#pragma once

#include <util/widgethelper.h>

#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QWidget>

#include "track/beats.h"
#include "util/parented_ptr.h"

class WTempoMenu : public QWidget {
    Q_OBJECT
  public:
    explicit WTempoMenu(QWidget* parent = nullptr);
    ~WTempoMenu() override;
    void setBeatsPointer(mixxx::BeatsPointer pBeats) {
        m_pBeats = pBeats;
    }
    void setBeat(std::optional<mixxx::Beat> beat);
    void popup(const QPoint& p);

  private slots:
    void slotTextInput(const QString& bpm);
    void slotSlightDecrease();
    void slotSlightIncrease();
    void slotBigDecrease();
    void slotBigIncrease();

  private:
    void setBpm(mixxx::Bpm bpm);

  private:
    parented_ptr<QLineEdit> m_pBpmEditBox;
    parented_ptr<QPushButton> m_pBpmSlightDecreaseButton;
    parented_ptr<QPushButton> m_pBpmSlightIncreaseButton;
    parented_ptr<QPushButton> m_pBpmBigDecreaseButton;
    parented_ptr<QPushButton> m_pBpmBigIncreaseButton;
    mixxx::BeatsPointer m_pBeats;
    std::optional<mixxx::Beat> m_beat;
};
