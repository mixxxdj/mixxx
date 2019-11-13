#include <QtDebug>

#include "skin/skincontext.h"
#include "waveform/renderers/waveformsignalcolors.h"
#include "widget/wskincolor.h"

#include "waveformmark.h"

namespace {
Qt::Alignment decodeAlignmentFlags(QString alignString, Qt::Alignment defaultFlags) {
    QStringList stringFlags = alignString.toLower()
            .split("|", QString::SkipEmptyParts);

    Qt::Alignment hflags = 0L;
    Qt::Alignment vflags = 0L;

    for (auto stringFlag : stringFlags) {
        if (stringFlag == "center") {
            hflags |= Qt::AlignHCenter;
            vflags |= Qt::AlignVCenter;
        } else if (stringFlag == "left") {
            hflags |= Qt::AlignLeft;
        } else if (stringFlag == "hcenter") {
            hflags |= Qt::AlignHCenter;
        } else if (stringFlag == "right") {
            hflags |= Qt::AlignRight;
        } else if (stringFlag == "top") {
            vflags |= Qt::AlignTop;
        } else if (stringFlag == "vcenter") {
            vflags |= Qt::AlignVCenter;
        } else if (stringFlag == "bottom") {
            vflags |= Qt::AlignBottom;
        }
    }

    if (hflags != Qt::AlignLeft && hflags != Qt::AlignHCenter && hflags != Qt::AlignRight) {
        hflags = defaultFlags & Qt::AlignHorizontal_Mask;
    }

    if (vflags != Qt::AlignTop && vflags != Qt::AlignVCenter && vflags != Qt::AlignBottom) {
        vflags = defaultFlags & Qt::AlignVertical_Mask;
    }

    return hflags | vflags;
}
} // anonymous namespace

WaveformMark::WaveformMark(const QString& group,
                           const QDomNode& node,
                           const SkinContext& context,
                           const WaveformSignalColors& signalColors,
                           int hotCue)
        : m_iHotCue(hotCue) {
    QString control;
    if (hotCue != Cue::kNoHotCue) {
        control = "hotcue_" + QString::number(hotCue + 1) + "_position";
    } else {
        control = context.selectString(node, "Control");
    }
    if (!control.isEmpty()) {
        m_pPointCos = std::make_unique<ControlProxy>(group, control);
    }

    QString visibilityControl = context.selectString(node, "VisibilityControl");
    if (!visibilityControl.isEmpty()) {
        ConfigKey key = ConfigKey::parseCommaSeparated(visibilityControl);
        m_pVisibleCos = std::make_unique<ControlProxy>(key);
    }

    QColor color(context.selectString(node, "Color"));
    if (!color.isValid()) {
        // As a fallback, grab the color from the parent's AxesColor
        color = signalColors.getAxesColor();
        qDebug() << "Didn't get mark <Color>, using parent's <AxesColor>:" << color;
    } else {
        color = WSkinColor::getCorrectColor(color);
    }
    setBaseColor(color);

    m_textColor = context.selectString(node, "TextColor");
    if (!m_textColor.isValid()) {
        // Read the text color, otherwise use the parent's BgColor.
        m_textColor = signalColors.getBgColor();
        qDebug() << "Didn't get mark <TextColor>, using parent's <BgColor>:" << m_textColor;
    }

    QString markAlign = context.selectString(node, "Align");
    m_align = decodeAlignmentFlags(markAlign, Qt::AlignBottom | Qt::AlignHCenter);

    // Hotcue text is set by the cue's label in the database, not by the skin.
    if (hotCue == Cue::kNoHotCue) {
        m_text = context.selectString(node, "Text");
    }

    m_pixmapPath = context.selectString(node, "Pixmap");
    if (!m_pixmapPath.isEmpty()) {
        m_pixmapPath = context.makeSkinPath(m_pixmapPath);
    }
}

void WaveformMark::setBaseColor(QColor baseColor) {
    m_fillColor = baseColor;
    m_borderColor = Color::chooseContrastColor(baseColor);
    m_labelColor = Color::chooseColorByBrightness(baseColor, QColor(255,255,255,255), QColor(0,0,0,255));
};
