#include <QtDebug>
#include <QTime>
#include <QLocale>

#include "util/cmdlineargs.h"
#include "widget/wtime.h"

WTime::WTime(ConfigObject<ConfigValue>* pConfig, QWidget *parent)
        : WLabel(parent),
          m_pConfig(pConfig),
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

        // long format is equivalent to showing seconds
        QLocale::FormatType format;
        if(secondsFormat == "true" || secondsFormat == "yes") {
            format = QLocale::LongFormat;
        } else {
            format = QLocale::ShortFormat;
        }

        // choose right locale
        QString userLocale = CmdlineArgs::Instance().getLocale();
        if (userLocale.isEmpty()) {
            userLocale = m_pConfig->getValueString(ConfigKey("[Config]","Locale"));
        }

        // if no locale was provided by the cmdline or the config it is likely
        // that we are using the system locale.
        QLocale* locale;
        if (userLocale.isEmpty()) {
            locale = new QLocale();
        } else {
            locale = new QLocale(userLocale);
        }
        m_sTimeFormat = locale->timeFormat(format);

        delete locale;
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
