#pragma once

#include <QObject>

#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "preferences/configobject.h"

namespace mixxx {

/// The control group of the visualizer window.
extern const QString kVisualizerConfigGroup;

extern const ConfigKey kVisualizerShowWindowConfigKey;
extern const ConfigKey kVisualizerStyleConfigKey;
extern const ConfigKey kVisualizerSelectImageConfigKey;
extern const ConfigKey kVisualizerBassIntensityConfigKey;
extern const ConfigKey kVisualizerMidIntensityConfigKey;
extern const ConfigKey kVisualizerHighIntensityConfigKey;
/// Not a control: the user setting that holds the path of the image for
/// the style CustomImage.
extern const ConfigKey kVisualizerImagePathConfigKey;

/// The controls of the visualizer window. They are created at startup,
/// independent of the skin and of the window itself, so that skins and
/// controller mappings can use them.
class VisualizerControls {
    Q_GADGET
  public:
    /// The selectable styles of the visualizer window. The values are the
    /// values of the control [Visualizer],style and must not be changed,
    /// because they are saved in the user settings.
    enum class Style {
        /// The covers of the audible decks.
        CoverDistortion = 0,
        /// An image that the user has selected, shown in place of the
        /// covers.
        CustomImage = 1,
    };
    Q_ENUM(Style)

    VisualizerControls();
    virtual ~VisualizerControls() = default;

    /// Returns the style for a value of the control [Visualizer],style. A
    /// value that is not a style gives the default style.
    static Style styleFromControlValue(double value);

    /// Returns the value of the control [Visualizer],style for a style.
    static double controlValueFromStyle(Style style);

  private:
    ControlPotmeter m_bassIntensity;
    ControlPotmeter m_midIntensity;
    ControlPotmeter m_highIntensity;
    ControlPushButton m_style;
    ControlPushButton m_showWindow;
    ControlPushButton m_selectImage;
};

} // namespace mixxx
