#include <QtDebug>

#include "waveformmark.h"

#include "waveformwidgetrenderer.h"
#include "controlobject.h"
#include "controlobjectthread.h"
#include "widget/wskincolor.h"

WaveformMark::WaveformMark()
    : m_pointControl(NULL) {
}

WaveformMark::~WaveformMark() {
    if (m_pointControl) {
        delete m_pointControl;
    }
}

void WaveformMark::setup(const QString& group, const QDomNode& node,
                         const SkinContext& context,
                         const WaveformSignalColors& signalColors) {
    QString item = context.selectString(node, "Control");
    if (!item.isEmpty()) {
        m_pointControl = new ControlObjectThread(group, item);
    }

    m_color = context.selectString(node, "Color");
    if (!m_color.isValid()) {
        // As a fallback, grab the color from the parent's AxesColor
        m_color = signalColors.getAxesColor();
        qDebug() << "Didn't get mark <Color>, using parent's <AxesColor>:" << m_color;
    } else {
        m_color = WSkinColor::getCorrectColor(m_color);
    }

    m_textColor = context.selectString(node, "TextColor");
    if (!m_textColor.isValid()) {
        // Read the text color, otherwise use the parent's BgColor.
        m_textColor = signalColors.getBgColor();
        qDebug() << "Didn't get mark <TextColor>, using parent's <BgColor>:" << m_textColor;
    }

    QString markAlign = context.selectString(node, "Align");
    if (markAlign.contains("bottom", Qt::CaseInsensitive)) {
        m_align = Qt::AlignBottom;
    } else {
        m_align = Qt::AlignTop; // Default
    }

    m_text = context.selectString(node, "Text");
    m_pixmapPath = context.selectString(node, "Pixmap");
    if (!m_pixmapPath.isEmpty()) {
        m_pixmapPath = context.getSkinPath(m_pixmapPath);
    }
}


void WaveformMark::setKeyAndIndex(const ConfigKey& key, int i) {
    m_pointControl = new ControlObjectThread(key);
    m_text = m_text.arg(i);
}
