#include "visualizer/visualizercontrols.h"

#include <QString>

namespace mixxx {

namespace {

const QString kVisualizerGroup = QStringLiteral("[Visualizer]");

constexpr double kIntensityMin = 0.0;
constexpr double kIntensityMax = 1.0;
constexpr double kIntensityDefault = 0.5;

} // namespace

VisualizerControls::VisualizerControls()
        : m_bassIntensity(ConfigKey(kVisualizerGroup, QStringLiteral("bass_intensity")),
                  kIntensityMin,
                  kIntensityMax,
                  false,
                  true,
                  false,
                  true,
                  kIntensityDefault),
          m_midIntensity(ConfigKey(kVisualizerGroup, QStringLiteral("mid_intensity")),
                  kIntensityMin,
                  kIntensityMax,
                  false,
                  true,
                  false,
                  true,
                  kIntensityDefault),
          m_highIntensity(ConfigKey(kVisualizerGroup, QStringLiteral("high_intensity")),
                  kIntensityMin,
                  kIntensityMax,
                  false,
                  true,
                  false,
                  true,
                  kIntensityDefault),
          m_style(ConfigKey(kVisualizerGroup, QStringLiteral("style")),
                  true,
                  static_cast<double>(VisualizerStyle::CoverDistortion)),
          m_showWindow(ConfigKey(kVisualizerGroup, QStringLiteral("show_window"))),
          m_selectImage(ConfigKey(kVisualizerGroup, QStringLiteral("select_image"))) {
    m_style.setBehavior(control::ButtonMode::Toggle,
            static_cast<int>(VisualizerStyle::Count));
    m_showWindow.setButtonMode(control::ButtonMode::Toggle);
}

} // namespace mixxx
