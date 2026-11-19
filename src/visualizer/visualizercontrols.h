#pragma once

#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"

namespace mixxx {

/// The selectable styles of the visualizer window. The values
/// are the values of the control [Visualizer],style and must not be changed,
/// because they are saved in the user settings.
enum class VisualizerStyle {
    /// The covers of the audible decks.
    CoverDistortion = 0,
    /// An image that the user has selected, shown in place
    /// of the covers.
    CustomImage = 1,
    Count,
};

/// The controls of the visualizer window. They are created at
/// startup, independent of the skin and of the window itself, so that skins
/// and controller mappings can use them.
class VisualizerControls {
  public:
    VisualizerControls();
    virtual ~VisualizerControls() = default;

  private:
    ControlPotmeter m_bassIntensity;
    ControlPotmeter m_midIntensity;
    ControlPotmeter m_highIntensity;
    ControlPushButton m_style;
    ControlPushButton m_showWindow;
    ControlPushButton m_selectImage;
};

} // namespace mixxx
