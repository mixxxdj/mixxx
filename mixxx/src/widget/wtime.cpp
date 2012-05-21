#include "widget/wtime.h"

WTime::WTime(QWidget *parent)
        : WLabel(parent),
          m_sTimeFormat("h:mm AP"),
          m_iInterval(s_iMinuteInterval) {
    m_pTimer = new QTimer(this);
}

WTime::~WTime() {
    delete m_pTimer;
}

void WTime::setup(QDomNode node) {
    WLabel::setup(node);
    setTimeFormat(node);
    m_pTimer->start(m_iInterval);
    connect(m_pTimer, SIGNAL(timeout()),
            this, SLOT(refreshTime()));
    refreshTime();
}

void WTime::setTimeFormat(QDomNode node) {
    // if a custom format is defined, all other formatting flags are ignored
    if (selectNode(node, "CustomFormat").isNull()) {
        // check if seconds should be shown
        QString secondsFormat = selectNodeQString(node, "ShowSeconds");
       if(secondsFormat == "true" || secondsFormat == "yes") {
           m_sTimeFormat = "h:mm:ss";
           m_iInterval = s_iSecondInterval;
       } else {
           m_sTimeFormat = "h:mm";
           m_iInterval = s_iMinuteInterval;
       }
       // check if 24 hour format or 12 hour format is selected
       QString clockFormat = selectNodeQString(node, "ClockFormat");
       if (clockFormat == "24" || clockFormat == "24hrs") {
       } else if (clockFormat == "12" ||
                  clockFormat == "12hrs" ||
                  clockFormat == "12ap") {
           m_sTimeFormat += " ap";
       } else if (clockFormat == "12AP") {
           m_sTimeFormat += " AP";
       } else {
           qDebug() << "WTime: Unknown clock format: " << clockFormat;
       }
    } else {
        // set the time format to the custom format
        m_sTimeFormat = selectNodeQString(node, "CustomFormat");
    }
}

void WTime::refreshTime() {
    QTime time = QTime::currentTime();
    QString timeString = time.toString(m_sTimeFormat);
    if (m_pLabel->text() != timeString) {
        m_pLabel->setText(timeString);
    }
}
