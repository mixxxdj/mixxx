#include <QDebug>

#include "waveformmark.h"

#include "waveformwidgetrenderer.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "xmlparse.h"

WaveformMark::WaveformMark()
    : m_pointControl(NULL) {
}

void WaveformMark::setup(const QString& group, const QDomNode& node, const WaveformSignalColors& signalColors) {
    QString item = XmlParse::selectNodeQString(node, "Control");
    ControlObject* pPointControl = ControlObject::getControl(ConfigKey(group, item));
    if (pPointControl) {
        m_pointControl = new ControlObjectThreadMain(pPointControl);
    }

    m_color = XmlParse::selectNodeQString(node, "Color");
    if (m_color == "") {
        // As a fallback, grab the color from the parent's AxesColor
        m_color = signalColors.getAxesColor();
        qDebug() << "Didn't get mark <Color>, using parent's <AxesColor>:" << m_color;
    }

    m_textColor = XmlParse::selectNodeQString(node, "TextColor");
    if (m_textColor == "") {
        // Read the text color, otherwise use the parent's BgColor.
        m_textColor = signalColors.getBgColor();
        qDebug() << "Didn't get mark <TextColor>, using parent's <BgColor>:" << m_textColor;
    }

    QString markAlign = XmlParse::selectNodeQString(node, "Align");
    if (markAlign.contains("bottom", Qt::CaseInsensitive)) {
        m_align = Qt::AlignBottom;
    } else {
        m_align = Qt::AlignTop; // Default
    }

    m_text = XmlParse::selectNodeQString(node, "Text");
    m_pixmapPath = XmlParse::selectNodeQString(node,"Pixmap");
}

