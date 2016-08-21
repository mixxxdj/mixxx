#include "skin/skincontext.h"
#include "waveform/renderers/waveformsignalcolors.h"
#include "widget/wskincolor.h"

#include "waveform/renderers/waveformmarkproperties.h"

WaveformMarkProperties::WaveformMarkProperties()
    : m_iHotCue(-1) {
}

WaveformMarkProperties::WaveformMarkProperties(const QDomNode& node,
                                               const SkinContext& context,
                                               const WaveformSignalColors& signalColors)
    : WaveformMarkProperties() {
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
