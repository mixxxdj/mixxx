#include <QDebug>

#include "waveformmark.h"

#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "xmlparse.h"

WaveformMark::WaveformMark()
    : m_pointControl(NULL) {
}

void WaveformMark::setup(const QString& group, const QDomNode& node) {
    QString item = XmlParse::selectNodeQString(node, "Control");
    ControlObject* pPointControl = ControlObject::getControl(ConfigKey(group, item));
    if (pPointControl) {
        m_pointControl = new ControlObjectThreadMain(pPointControl);
    }

    m_color = XmlParse::selectNodeQString(node, "Color");
    if (m_color == "") {
        // As a fallback, grab the mark color from the parent's MarkerColor
        m_color = XmlParse::selectNodeQString(node.parentNode(), "MarkerColor");
        qDebug() << "Didn't get mark 'Color', using parent's 'MarkerColor':" << m_color;
    }

    m_textColor = XmlParse::selectNodeQString(node, "TextColor");
    if (m_textColor == "") {
        // Read the text color, otherwise use the parent's BgColor.
        m_textColor = XmlParse::selectNodeQString(node.parentNode(), "BgColor");
        qDebug() << "Didn't get mark TextColor, using parent's BgColor:" << m_textColor;
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

