#include "widget/wtime.h"

#include <QTime>

WTime::WTime(QWidget *parent)
        : WLabel(parent),
          m_sTimeFormat("h:mm AP"),
          m_iInterval(s_iMinuteInterval) {
    m_pTimer = new QTimer(this);
}

WTime::~WTime() {
    delete m_pTimer;
}

void WTime::setup(QDomNode node, const SkinContext& context) {
    WLabel::setup(node, context);
    setTimeFormat(node, context);
    m_pTimer->start(m_iInterval);
    connect(m_pTimer, SIGNAL(timeout()),
            this, SLOT(refreshTime()));
    refreshTime();
}

void WTime::setTimeFormat(QDomNode node, const SkinContext& context) {
    // if a custom format is defined, all other formatting flags are ignored
    if (!context.hasNode(node, "CustomFormat")) {
        // check if seconds should be shown
        QString secondsFormat = context.selectString(node, "ShowSeconds");
       if(secondsFormat == "true" || secondsFormat == "yes") {
           m_sTimeFormat = "h:mm:ss";
           m_iInterval = s_iSecondInterval;
       } else {
           m_sTimeFormat = "h:mm";
           m_iInterval = s_iMinuteInterval;
       }
       // check if 24 hour format or 12 hour format is selected
       QString clockFormat = context.selectString(node, "ClockFormat");
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
        m_sTimeFormat = context.selectString(node, "CustomFormat");
    }
}

void WTime::refreshTime() {
    QTime time = QTime::currentTime();
    QString timeString = time.toString(m_sTimeFormat);
    if (text() != timeString) {
        setText(timeString);
    }
}
