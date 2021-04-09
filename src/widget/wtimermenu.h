#pragma once

#include <QAction>
#include <QMenu>
#include <QWidget>

class WTimerMenu : public QWidget {
    Q_OBJECT
  public:
    WTimerMenu(QWidget* parent = nullptr);

    WTimerMenu::~WTimerMenu() {
        delete m_pStart;
        delete m_pStop;
        delete m_pReset;
    }

  private:
    void createActions();

    QAction* m_pStart;
    QAction* m_pStop;
    QAction* m_pReset;
};
