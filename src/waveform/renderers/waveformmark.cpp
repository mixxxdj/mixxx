#include <QtDebug>

#include "skin/skincontext.h"
#include "waveform/renderers/waveformsignalcolors.h"
#include "widget/wskincolor.h"

#include "waveformmark.h"

namespace {
Qt::Alignment decodeAlignmentFlags(const QString& alignString, Qt::Alignment defaultFlags) {
    QStringList stringFlags = alignString.toLower()
                                      .split('|',
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                                              Qt::SkipEmptyParts);
#else
                                              QString::SkipEmptyParts);
#endif

    Qt::Alignment hflags;
    Qt::Alignment vflags;

    for (const auto& stringFlag : stringFlags) {
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
    QString positionControl;
    QString endPositionControl;
    if (hotCue != Cue::kNoHotCue) {
        positionControl = "hotcue_" + QString::number(hotCue + 1) + "_position";
        endPositionControl = "hotcue_" + QString::number(hotCue + 1) + "_endposition";
    } else {
        positionControl = context.selectString(node, "Control");
    }

    if (!positionControl.isEmpty()) {
        m_pPositionCO = std::make_unique<ControlProxy>(group, positionControl);
    }
    if (!endPositionControl.isEmpty()) {
        m_pEndPositionCO = std::make_unique<ControlProxy>(group, endPositionControl);
    }

    QString visibilityControl = context.selectString(node, "VisibilityControl");
    if (!visibilityControl.isEmpty()) {
        ConfigKey key = ConfigKey::parseCommaSeparated(visibilityControl);
        m_pVisibleCO = std::make_unique<ControlProxy>(key);
    }

    QColor color(context.selectString(node, "Color"));
    if (!color.isValid()) {
        // As a fallback, grab the color from the parent's AxesColor
        color = signalColors.getAxesColor();
        qDebug() << "Didn't get mark <Color>, using parent's <AxesColor>:" << color;
    } else {
        color = WSkinColor::getCorrectColor(color);
    }
    int dimBrightThreshold = signalColors.getDimBrightThreshold();
    setBaseColor(color, dimBrightThreshold);

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

void WaveformMark::setBaseColor(QColor baseColor, int dimBrightThreshold) {
    m_image = QImage();
    m_fillColor = baseColor;
    m_borderColor = Color::chooseContrastColor(baseColor, dimBrightThreshold);
    m_labelColor = Color::chooseColorByBrightness(baseColor,
            QColor(255, 255, 255, 255),
            QColor(0, 0, 0, 255),
            dimBrightThreshold);
};

bool WaveformMark::contains(QPoint point, Qt::Orientation orientation) const {
    // Without some padding, the user would only have a single pixel width that
    // would count as hovering over the WaveformMark.
    float lineHoverPadding = 5.0;
    int position;
    if (orientation == Qt::Horizontal) {
        position = point.x();
    } else {
        position = point.y();
    }
    bool lineHovered = m_linePosition >= position - lineHoverPadding &&
            m_linePosition <= position + lineHoverPadding;

    return m_label.area().contains(point) || lineHovered;
}
