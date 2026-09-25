#include "widget/wtime.h"

#include <QLocale>
#include <QTime>
#include <QTimer>

#include "moc_wtime.cpp"
#include "skin/legacy/skincontext.h"

namespace {
static constexpr short s_iSecondInterval = 100;
static constexpr short s_iMinuteInterval = 1000;
} // namespace

WTime::WTime(QWidget* parent)
        : WLabel(parent),
          m_sTimeFormat("h:mm AP"),
          m_interval(s_iMinuteInterval) {
    m_pTimer = new QTimer(this);
}

WTime::~WTime() {
    delete m_pTimer;
}

void WTime::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
    setTimeFormat(node, context);
    m_pTimer->start(m_interval);
    connect(m_pTimer, &QTimer::timeout, this, &WTime::refreshTime);
    refreshTime();
}

void WTime::setTimeFormat(const QDomNode& node, const SkinContext& context) {
    // if a custom format is defined, all other formatting flags are ignored
    QString customFormat;
    if (context.hasNodeSelectString(node, "CustomFormat", &customFormat)) {
        // set the time format to the custom format
        m_sTimeFormat = customFormat;
        // if seconds are to be displayed, use the seconds refresh interval explicitly
        if (customFormat.contains(QStringLiteral("ss"), Qt::CaseInsensitive)) {
            m_interval = s_iSecondInterval;
        }
    } else {
        // check if seconds should be shown
        QString secondsFormat = context.selectString(node, "ShowSeconds");
        if(secondsFormat == "true" || secondsFormat == "yes") {
            // Note: don't use QLocale::LongFormat since that would not only show
            // seconds but also append other locale-dependent info, eg. time zone
            // or AM/PM.
            // Use 'h' (not 'hh') to omit leading zeros.
            m_interval = s_iSecondInterval;
            m_sTimeFormat = QStringLiteral("h:mm:ss");
        } else {
            m_interval = s_iMinuteInterval;
            m_sTimeFormat = QLocale().timeFormat(QLocale::ShortFormat);
        }
    }
}

void WTime::refreshTime() {
    QTime time = QTime::currentTime();
    QString timeString = time.toString(m_sTimeFormat);
    if (text() != timeString) {
        setText(timeString);
        //if (CmdlineArgs::Instance().getDeveloper()) {
        //    qDebug() << "WTime::refreshTime" << timeString << font().family();
        //}
    }
}
