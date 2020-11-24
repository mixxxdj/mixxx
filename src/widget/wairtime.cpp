#include <QtDebug>
#include <QTime>
#include <QLocale>

#include "widget/wairtime.h"
#include "util/cmdlineargs.h"

WAirTime::WAirTime(QWidget *parent)
        : WLabel(parent),
          m_sTimeFormat("h:mm:ss AP"),
          m_iInterval(s_iMinuteInterval) {
    m_pTimer = new QTimer(this);
}

WAirTime::~WAirTime() {
    delete m_pTimer;
}

void WAirTime::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
    setTimeFormat(node, context);
    m_pTimer->start(m_iInterval);
    connect(m_pTimer, &QTimer::timeout, this, &WTime::refreshTime);
    refreshTime();
}

void WAirTime::setTimeFormat(const QDomNode& node, const SkinContext& context) {
    // if a custom format is defined, all other formatting flags are ignored
    QString customFormat;
    if (context.hasNodeSelectString(node, "CustomFormat", &customFormat)) {
        // set the time format to the custom format
        m_sTimeFormat = customFormat;
    } else {
        // check if seconds should be shown
        QString secondsFormat = context.selectString(node, "ShowSeconds");
        // long format is equivalent to showing seconds
        QLocale::FormatType format;
        if(secondsFormat == "true" || secondsFormat == "yes") {
            format = QLocale::LongFormat;
            m_iInterval = s_iSecondInterval;
        } else {
            format = QLocale::ShortFormat;
            m_iInterval = s_iMinuteInterval;
        }
        m_sTimeFormat = QLocale().timeFormat(format);
    }
}

void WAirTime::refreshTime() {
    QTime time = QTime::currentTime();
    QString timeString = time.toString(m_sTimeFormat);
    if (text() != timeString) {
        setText(timeString);
        //if (CmdlineArgs::Instance().getDeveloper()) {
        //    qDebug() << "WTime::refreshTime" << timeString << font().family();
        //}
    }
}
