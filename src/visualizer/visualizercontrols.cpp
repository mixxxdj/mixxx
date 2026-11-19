#include "visualizer/visualizercontrols.h"

#include <QMetaEnum>
#include <QString>
#include <QVariant>

#include "moc_visualizercontrols.cpp"

namespace mixxx {

const QString kVisualizerConfigGroup = QStringLiteral("[Visualizer]");

const ConfigKey kVisualizerShowWindowConfigKey =
        ConfigKey(kVisualizerConfigGroup, QStringLiteral("show_window"));
const ConfigKey kVisualizerStyleConfigKey =
        ConfigKey(kVisualizerConfigGroup, QStringLiteral("style"));
const ConfigKey kVisualizerSelectImageConfigKey =
        ConfigKey(kVisualizerConfigGroup, QStringLiteral("select_image"));
const ConfigKey kVisualizerBassIntensityConfigKey =
        ConfigKey(kVisualizerConfigGroup, QStringLiteral("bass_intensity"));
const ConfigKey kVisualizerMidIntensityConfigKey =
        ConfigKey(kVisualizerConfigGroup, QStringLiteral("mid_intensity"));
const ConfigKey kVisualizerHighIntensityConfigKey =
        ConfigKey(kVisualizerConfigGroup, QStringLiteral("high_intensity"));
const ConfigKey kVisualizerImagePathConfigKey =
        ConfigKey(kVisualizerConfigGroup, QStringLiteral("image_path"));

namespace {

constexpr double kIntensityMin = 0.0;
constexpr double kIntensityMax = 1.0;
constexpr double kIntensityDefault = 0.5;

constexpr VisualizerControls::Style kDefaultStyle =
        VisualizerControls::Style::CoverDistortion;

} // namespace

VisualizerControls::VisualizerControls()
        : m_bassIntensity(kVisualizerBassIntensityConfigKey,
                  kIntensityMin,
                  kIntensityMax,
                  /*allowOutOfBounds*/ false,
                  /*ignoreNops*/ true,
                  /*track*/ false,
                  /*persist*/ true,
                  kIntensityDefault),
          m_midIntensity(kVisualizerMidIntensityConfigKey,
                  kIntensityMin,
                  kIntensityMax,
                  /*allowOutOfBounds*/ false,
                  /*ignoreNops*/ true,
                  /*track*/ false,
                  /*persist*/ true,
                  kIntensityDefault),
          m_highIntensity(kVisualizerHighIntensityConfigKey,
                  kIntensityMin,
                  kIntensityMax,
                  /*allowOutOfBounds*/ false,
                  /*ignoreNops*/ true,
                  /*track*/ false,
                  /*persist*/ true,
                  kIntensityDefault),
          m_style(kVisualizerStyleConfigKey,
                  /*persist*/ true,
                  controlValueFromStyle(kDefaultStyle)),
          m_showWindow(kVisualizerShowWindowConfigKey),
          m_selectImage(kVisualizerSelectImageConfigKey) {
    m_style.setBehavior(control::ButtonMode::Toggle,
            QMetaEnum::fromType<Style>().keyCount());
    m_showWindow.setButtonMode(control::ButtonMode::Toggle);
}

// static
VisualizerControls::Style VisualizerControls::styleFromControlValue(double value) {
    bool isNumber = false;
    const int number = QVariant(value).toInt(&isNumber);
    if (!isNumber || QMetaEnum::fromType<Style>().valueToKey(number) == nullptr) {
        return kDefaultStyle;
    }
    return QVariant(number).value<Style>();
}

// static
double VisualizerControls::controlValueFromStyle(Style style) {
    return QVariant::fromValue(style).toInt();
}

} // namespace mixxx
