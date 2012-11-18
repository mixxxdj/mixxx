#include "waveformmark.h"

#include "controlobject.h"
#include "widget/wwidget.h"
#include <QDomNode>

#include <QDebug>

WaveformMark::WaveformMark()
    : m_pointControl(NULL) {
}

void WaveformMark::setup(const QString& group, const QDomNode& node) {
    QString item = WWidget::selectNodeQString( node, "Control");
    m_pointControl = ControlObject::getControl( ConfigKey(group, item));

    m_color = WWidget::selectNodeQString( node, "Color");
    if( m_color == "") {

        // As a fallback, grab the mark color from the parent's MarkerColor
        m_color = WWidget::selectNodeQString(node.parentNode(), "MarkerColor");
        qDebug() << "Didn't get mark 'Color', using parent's 'MarkerColor':" << m_color;
    }

    m_textColor = WWidget::selectNodeQString(node, "TextColor");
    if( m_textColor == "") {
        // Read the text color, otherwise use the parent's BgColor.
        m_textColor = WWidget::selectNodeQString(node.parentNode(), "BgColor");
        qDebug() << "Didn't get mark TextColor, using parent's BgColor:" << m_textColor;
    }

    QString markAlign = WWidget::selectNodeQString(node, "Align");
    if (markAlign.contains("bottom", Qt::CaseInsensitive)) {
        m_align = Qt::AlignBottom;
    } else {
        m_align = Qt::AlignTop; // Default
    }

    m_text = WWidget::selectNodeQString(node, "Text");
    m_pixmapPath = WWidget::selectNodeQString(node,"Pixmap");
}

