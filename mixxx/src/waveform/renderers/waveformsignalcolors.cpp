#include "waveformsignalcolors.h"

#include "util/colorcomponents.h"
#include "widget/wskincolor.h"

class QDomNode;

namespace {
constexpr int kDefaultDimBrightThreshold = 127;
} // namespace

WaveformSignalColors::WaveformSignalColors()
        : m_dimBrightThreshold(kDefaultDimBrightThreshold) {
}

bool WaveformSignalColors::setup(const QDomNode &node, const SkinContext& context) {
    // NOTE(rryan): It is critical that every color is converted to RGB with
    // toRgb(). Otherwise Mixxx will waste 3% of its CPU time while rendering
    // the filtered waveform doing RGB color space conversions!

    m_signalColor = QColor(context.selectString(node, "SignalColor"));
    m_signalColor = WSkinColor::getCorrectColor(m_signalColor).toRgb();

    m_lowColor = QColor(context.selectString(node, "SignalLowColor"));
    m_lowColor = WSkinColor::getCorrectColor(m_lowColor).toRgb();

    m_midColor = QColor(context.selectString(node, "SignalMidColor"));
    m_midColor = WSkinColor::getCorrectColor(m_midColor).toRgb();

    m_highColor = QColor(context.selectString(node, "SignalHighColor"));
    m_highColor = WSkinColor::getCorrectColor(m_highColor).toRgb();

    m_rgbLowColor = QColor(context.selectString(node, "SignalRGBLowColor"));
    if (!m_rgbLowColor.isValid()) {
        m_rgbLowColor = Qt::red;
    }
    m_rgbLowColor = WSkinColor::getCorrectColor(m_rgbLowColor).toRgb();

    m_rgbMidColor = QColor(context.selectString(node, "SignalRGBMidColor"));
    if (!m_rgbMidColor.isValid()) {
        m_rgbMidColor = Qt::green;
    }
    m_rgbMidColor = WSkinColor::getCorrectColor(m_rgbMidColor).toRgb();

    m_rgbHighColor = QColor(context.selectString(node, "SignalRGBHighColor"));
    if (!m_rgbHighColor.isValid()) {
        m_rgbHighColor = Qt::blue;
    }
    m_rgbHighColor = WSkinColor::getCorrectColor(m_rgbHighColor).toRgb();

    // filtered colors
    m_rgbLowFilteredColor = QColor(context.selectString(node, "SignalRGBLowFilteredColor"));
    if (!m_rgbLowFilteredColor.isValid()) {
        m_rgbLowFilteredColor = m_rgbLowColor.darker(300);
    }
    m_rgbLowFilteredColor = WSkinColor::getCorrectColor(m_rgbLowFilteredColor).toRgb();

    m_rgbMidFilteredColor = QColor(context.selectString(node, "SignalRGBMidFilteredColor"));
    if (!m_rgbMidFilteredColor.isValid()) {
        m_rgbMidFilteredColor = m_rgbMidColor.darker(300);
        ;
    }
    m_rgbMidFilteredColor = WSkinColor::getCorrectColor(m_rgbMidFilteredColor).toRgb();

    m_rgbHighFilteredColor = QColor(context.selectString(node, "SignalRGBHighFilteredColor"));
    if (!m_rgbHighFilteredColor.isValid()) {
        m_rgbHighFilteredColor = m_rgbHighColor.darker(300);
        ;
    }
    m_rgbHighFilteredColor = WSkinColor::getCorrectColor(m_rgbHighFilteredColor).toRgb();

    m_axesColor = context.selectColor(node, "AxesColor");
    if (!m_axesColor.isValid()) {
        m_axesColor = QColor(245,245,245);
    }
    m_axesColor = WSkinColor::getCorrectColor(m_axesColor).toRgb();

    m_playPosColor = context.selectColor(node, "PlayPosColor");
    m_playPosColor = WSkinColor::getCorrectColor(m_playPosColor).toRgb();
    if (!m_playPosColor.isValid()) {
        m_playPosColor = m_axesColor;
    }

    // This color is used to draw an overlay over the played part the overview-waveforms
    m_playedOverlayColor = context.selectColor(node, "PlayedOverlayColor");
    m_playedOverlayColor = WSkinColor::getCorrectColor(m_playedOverlayColor).toRgb();
    if (!m_playedOverlayColor.isValid()) {
        m_playedOverlayColor = Qt::transparent;
    }

    // This color is used to draw an overlay over the entire overview waveforms
    // if vinyl passthrough is enabled. Default is black with 73% opacity.
    m_passthroughOverlayColor = context.selectColor(node, "PassthroughOverlayColor");
    m_passthroughOverlayColor = WSkinColor::getCorrectColor(m_passthroughOverlayColor).toRgb();
    if (!m_passthroughOverlayColor.isValid()) {
        m_passthroughOverlayColor = WSkinColor::getCorrectColor(QColor(0, 0, 0, 187)).toRgb();
    }
    // This color is for the "Passthrough" label on scrolling waveforms. Default is orange.
    m_passthroughLabelColor = context.selectColor(node, "PassthroughLabelColor");
    m_passthroughLabelColor = WSkinColor::getCorrectColor(m_passthroughLabelColor).toRgb();
    if (!m_passthroughLabelColor.isValid()) {
        m_passthroughLabelColor = WSkinColor::getCorrectColor(QColor(100, 53, 0, 255)).toRgb();
    }

    m_bgColor = context.selectColor(node, "BgColor");
    if (!m_bgColor.isValid()) {
        m_bgColor = Qt::transparent;
    }
    m_bgColor = WSkinColor::getCorrectColor(m_bgColor).toRgb();

    bool okay;
    m_dimBrightThreshold = context.selectInt(node, QStringLiteral("DimBrightThreshold"), &okay);
    if (!okay) {
        m_dimBrightThreshold = kDefaultDimBrightThreshold;
    }

    bool filteredColorValid = m_lowColor.isValid() && m_midColor.isValid() && m_highColor.isValid();

    if (m_signalColor.isValid() && filteredColorValid) {
        return true; //default
    }

    if (m_signalColor.isValid() && !filteredColorValid) {
        fallBackFromSignalColor(); //pre waveform-2.0 skins
        return false;
    }

    fallBackDefaultColor();
    return false;
}

void WaveformSignalColors::fallBackFromSignalColor() {
    // qWarning() << "WaveformSignalColors::fallBackFromSignalColor - "
    //           << "skin do not provide low/mid/high signal colors";

    float h, s, l, a;
    getHslF(m_signalColor, &h, &s, &l, &a);

    constexpr double analogousAngle = 1.0 / 12.0;

    if (s < 0.1) { // gray
        const float sMax = 1.0f - h;
        m_lowColor.setHslF(h,s,l);
        m_midColor.setHslF(h, s + sMax * 0.2f, l);
        m_highColor.setHslF(h, s + sMax * 0.4f, l);
    } else {
        if (l < 0.1) { // ~white
            const float lMax = 1.0f - l;
            m_lowColor.setHslF(h,s,l);
            m_midColor.setHslF(h, s, l + lMax * 0.2f);
            m_highColor.setHslF(h, s, l + lMax * 0.4f);
        } else if (l < 0.5) {
            const float lMax = 1.0f - l;
            m_lowColor.setHslF(h,s,l);
            m_midColor.setHslF(
                    static_cast<float>(stableHue(h - analogousAngle * 0.3)),
                    s,
                    l + lMax * 0.1f);
            m_highColor.setHslF(
                    static_cast<float>(stableHue(h + analogousAngle * 0.3)),
                    s,
                    l + lMax * 0.4f);
        } else if (l < 0.9) {
            const float lMin = l;
            m_lowColor.setHslF(h,s,l);
            m_midColor.setHslF(
                    static_cast<float>(stableHue(h - analogousAngle * 0.3)),
                    s,
                    l - lMin * 0.1f);
            m_highColor.setHslF(
                    static_cast<float>(stableHue(h + analogousAngle * 0.3)),
                    s,
                    l - lMin * 0.4f);
        } else { // ~black
            const float lMin = l;
            m_lowColor.setHslF(h,s,l);
            m_midColor.setHslF(h, s, l - lMin * 0.2f);
            m_highColor.setHslF(h, s, l - lMin * 0.4f);
        }
    }


    // NOTE(rryan): It is critical that every color is converted to RGB with
    // toRgb(). Otherwise Mixxx will waste 3% of its CPU time while rendering
    // the filtered waveform doing RGB color space conversions!
    m_lowColor = m_lowColor.toRgb();
    m_midColor = m_midColor.toRgb();
    m_highColor = m_highColor.toRgb();
}

void WaveformSignalColors::fallBackDefaultColor() {
    qWarning() << "WaveformSignalColors::fallBackDefaultColor - "
                  "Skin does not provide valid signal colors, using default color...";

    m_signalColor = Qt::green;
    m_signalColor = m_signalColor.toRgb();
    fallBackFromSignalColor();
}

//NOTE(vRince) this sabilise hue between -1.0 and 2.0 but not more !
double WaveformSignalColors::stableHue(double hue) const {
    return hue < 0.0 ? hue + 1.0 : hue > 1.0 ? hue - 1.0 : hue;
}
