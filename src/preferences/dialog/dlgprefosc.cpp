#include "preferences/dialog/dlgprefosc.h"

#include "moc_dlgprefosc.cpp"

namespace {
const QString kConfigGroup = QStringLiteral("[OSC]");
const QString kEnabledKey = QStringLiteral("Enabled");
const QString kInputPortKey = QStringLiteral("InputPort");
const QString kOutputPortKey = QStringLiteral("OutputPort");
const QString kOutputHostKey = QStringLiteral("OutputHost");
const QString kBindAddressKey = QStringLiteral("BindAddress");

constexpr int kDefaultInputPort = 11119;
constexpr int kDefaultOutputPort = 11118;
} // namespace

DlgPrefOsc::DlgPrefOsc(QWidget* pParent, UserSettingsPointer pConfig)
        : DlgPreferencePage(pParent),
          m_pConfig(pConfig) {
    setupUi(this);

    connect(OscEnabledCheckBox,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefOsc::slotToggleOsc);

    setScrollSafeGuardForAllInputWidgets(this);
}

void DlgPrefOsc::slotUpdate() {
    const bool enabled = m_pConfig->getValue(
            ConfigKey(kConfigGroup, kEnabledKey), false);
    OscEnabledCheckBox->setChecked(enabled);

    OscInputPortSpinBox->setValue(m_pConfig->getValue(
            ConfigKey(kConfigGroup, kInputPortKey), kDefaultInputPort));
    OscOutputPortSpinBox->setValue(m_pConfig->getValue(
            ConfigKey(kConfigGroup, kOutputPortKey), kDefaultOutputPort));
    QString outputHost = m_pConfig->getValueString(ConfigKey(kConfigGroup, kOutputHostKey));
    if (outputHost.isEmpty()) {
        outputHost = QStringLiteral("127.0.0.1");
    }
    OscOutputHostLineEdit->setText(outputHost);

    QString bindAddress = m_pConfig->getValueString(ConfigKey(kConfigGroup, kBindAddressKey));
    if (bindAddress.isEmpty()) {
        bindAddress = QStringLiteral("127.0.0.1");
    }
    OscBindAddressLineEdit->setText(bindAddress);

    slotToggleOsc(enabled ? Qt::Checked : Qt::Unchecked);
}

void DlgPrefOsc::slotApply() {
    m_pConfig->setValue(ConfigKey(kConfigGroup, kEnabledKey),
            OscEnabledCheckBox->isChecked());
    m_pConfig->setValue(ConfigKey(kConfigGroup, kInputPortKey),
            OscInputPortSpinBox->value());
    m_pConfig->setValue(ConfigKey(kConfigGroup, kOutputPortKey),
            OscOutputPortSpinBox->value());
    m_pConfig->setValue(ConfigKey(kConfigGroup, kOutputHostKey),
            OscOutputHostLineEdit->text().trimmed());
    m_pConfig->setValue(ConfigKey(kConfigGroup, kBindAddressKey),
            OscBindAddressLineEdit->text().trimmed());
}

void DlgPrefOsc::slotResetToDefaults() {
    OscEnabledCheckBox->setChecked(false);
    OscInputPortSpinBox->setValue(kDefaultInputPort);
    OscOutputPortSpinBox->setValue(kDefaultOutputPort);
    OscOutputHostLineEdit->setText(QStringLiteral("127.0.0.1"));
    OscBindAddressLineEdit->setText(QStringLiteral("127.0.0.1"));
    slotToggleOsc(Qt::Unchecked);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefOsc::slotToggleOsc(Qt::CheckState buttonState) {
#else
void DlgPrefOsc::slotToggleOsc(int buttonState) {
#endif
    const bool enabled = buttonState == Qt::Checked;
    OscInputPortSpinBox->setEnabled(enabled);
    OscOutputPortSpinBox->setEnabled(enabled);
    OscOutputHostLineEdit->setEnabled(enabled);
    OscBindAddressLineEdit->setEnabled(enabled);
}
