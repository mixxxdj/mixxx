#include "widget/wtimermenu.h"

WTimerMenu::WTimerMenu(QWidget *parent)
        : QMenu(parent) {
    createActions();
}

WTimerMenu::~WTimerMenu() {
    delete m_pStart;
    delete m_pStop;
    delete m_pReset;
}

void WTimerMenu::createActions() {
    m_pStart = new QAction(tr("Start",
            "Start the Timer manually"), this);
    connect(m_pStart, &QAction::triggered, this, &WTimerMenu::startTimer);
    addAction(m_pStart);

    m_pStop = new QAction(tr("Stop",
            "Stop/Pause the Timer manually"), this);
    connect(m_pStop, &QAction::triggered, this, &WTimerMenu::stopTimer);
    addAction(m_pStop);

    m_pReset = new QAction(tr("Reset",
            "Stop/Pause the Timer manually"), this);
    connect(m_pReset, &QAction::triggered, this, &WTimerMenu::resetTimer);
    addAction(m_pReset);
}

void WTimerMenu::startTimer() {
    //TODO
}

void WTimerMenu::startTimer() {
    //TODO
}

void WTimerMenu::startTimer() {
    //TODO
}
