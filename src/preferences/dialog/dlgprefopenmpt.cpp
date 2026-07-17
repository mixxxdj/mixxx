#include "preferences/dialog/dlgprefopenmpt.h"

#include <iterator>

#include "moc_dlgprefopenmpt.cpp"
#include "preferences/dialog/ui_dlgprefopenmptdlg.h"
#include "preferences/usersettings.h"
#include "sources/soundsourceopenmpt.h"

namespace {
const char* kConfigGroup = "[OpenMPT]";

// Interpolation filter taps as understood by libopenmpt, in the same order as
// the combo box entries (None, Linear, Cubic, Sinc).
constexpr int kInterpolationFilters[] = {1, 2, 4, 8};
constexpr int kDefaultInterpolationFilter = 8; // windowed sinc (best)
constexpr int kDefaultStereoSeparation = 100;  // native

int interpolationFilterToIndex(int filterLength) {
    for (int i = 0; i < static_cast<int>(std::size(kInterpolationFilters)); ++i) {
        if (kInterpolationFilters[i] == filterLength) {
            return i;
        }
    }
    return 3; // default to sinc if the stored value is unexpected
}
} // namespace

DlgPrefOpenMPT::DlgPrefOpenMPT(QWidget* parent, UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_pUi(new Ui::DlgPrefOpenMPT),
          m_pConfig(pConfig) {
    m_pUi->setupUi(this);

    // Keep the stereo-separation slider and spin box in sync.
    connect(m_pUi->stereoSeparation,
            &QAbstractSlider::valueChanged,
            m_pUi->stereoSeparationSpin,
            &QSpinBox::setValue);
    connect(m_pUi->stereoSeparationSpin,
            QOverload<int>::of(&QSpinBox::valueChanged),
            m_pUi->stereoSeparation,
            &QAbstractSlider::setValue);

    createLinkColor();
    m_pUi->openmptSettingsHint->setText(
            tr("All settings take effect on next track load. Currently loaded "
               "tracks are not affected. For an explanation of these settings, "
               "see the %1")
                    .arg(coloredLinkString(
                            m_pLinkColor,
                            "OpenMPT manual",
                            "https://wiki.openmpt.org/Manual:_Setup/Player")));

    setScrollSafeGuardForAllInputWidgets(this);
}

DlgPrefOpenMPT::~DlgPrefOpenMPT() {
    delete m_pUi;
}

void DlgPrefOpenMPT::slotApply() {
    applySettings();
    saveSettings();
}

void DlgPrefOpenMPT::slotUpdate() {
    loadSettings();
}

void DlgPrefOpenMPT::slotResetToDefaults() {
    m_pUi->interpolationFilter->setCurrentIndex(
            interpolationFilterToIndex(kDefaultInterpolationFilter));
    m_pUi->stereoSeparation->setValue(kDefaultStereoSeparation);
}

void DlgPrefOpenMPT::loadSettings() {
    const int filterLength = m_pConfig->getValue(
            ConfigKey(kConfigGroup, "InterpolationFilterLength"),
            kDefaultInterpolationFilter);
    m_pUi->interpolationFilter->setCurrentIndex(
            interpolationFilterToIndex(filterLength));
    m_pUi->stereoSeparation->setValue(m_pConfig->getValue(
            ConfigKey(kConfigGroup, "StereoSeparationPercent"),
            kDefaultStereoSeparation));
}

void DlgPrefOpenMPT::saveSettings() {
    m_pConfig->set(ConfigKey(kConfigGroup, "InterpolationFilterLength"),
            ConfigValue(kInterpolationFilters[m_pUi->interpolationFilter->currentIndex()]));
    m_pConfig->set(ConfigKey(kConfigGroup, "StereoSeparationPercent"),
            ConfigValue(m_pUi->stereoSeparation->value()));
}

void DlgPrefOpenMPT::applySettings() {
    mixxx::SoundSourceOpenMPT::Settings settings;
    settings.interpolationFilterLength =
            kInterpolationFilters[m_pUi->interpolationFilter->currentIndex()];
    settings.stereoSeparationPercent = m_pUi->stereoSeparation->value();
    mixxx::SoundSourceOpenMPT::configure(settings);
}
