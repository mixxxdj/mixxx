#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class WCueMenuPopup : public QWidget {
    Q_OBJECT
  public:
    WCueMenuPopup(UserSettingsPointer pConfig, QWidget* parent = nullptr);

    ~WCueMenuPopup() {
        delete m_pTimerHeading;
        delete m_pStartButton;
        delete m_pStopButton;
    }

    QLabel* m_pTimerHeading;
    QPushButton* m_pStartButton;
    QPushButton* m_pStopButton;
};
