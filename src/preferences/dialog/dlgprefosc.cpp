#include "preferences/dialog/dlgprefosc.h"

#include "moc_dlgprefosc.cpp"

DlgPrefOsc::DlgPrefOsc(QWidget* pParent,
        UserSettingsPointer pConfig)
        : DlgPreferencePage(pParent),
          m_pConfig(pConfig) {
    setupUi(this);

    // If OSC Receiver X is active -> OSC messages from Mixxx will be send to this receiver

    setScrollSafeGuardForAllInputWidgets(this);
}

void DlgPrefOsc::slotUpdate() {
    // Enable OSC-functions in Mixxx
    OscEnabledCheckBox->setChecked(m_pConfig->getValue(ConfigKey("[OSC]", "OscEnabled"), false));

    OscPortIn->setValue(m_pConfig->getValue(ConfigKey("[OSC]", "OscPortIn"), 9000));
    OscPortOut->setValue(
            m_pConfig->getValue(ConfigKey("[OSC]", "OscPortOut"), 9001));
    //    OscOutputBufferSize->setValue(m_pConfig->getValue(ConfigKey("[OSC]",
    //    "OscOutputBufferSize"), 1024));
    //    OscIpMtuSize->setValue(m_pConfig->getValue(ConfigKey("[OSC]",
    //    "OscIpMtuSize"), 1536));

    OscReceiver1ActiveCheckBox->setChecked(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver1Active"), false));
    OscReceiver1IpByte1->setValue(
            m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver1IpByte1"), 1));
    OscReceiver1IpByte2->setValue(
            m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver1IpByte2"), 1));
    OscReceiver1IpByte3->setValue(
            m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver1IpByte3"), 1));
    OscReceiver1IpByte4->setValue(
            m_pConfig->getValue(ConfigKey("[OSC]", "OscReceiver1IpByte4"), 1));

    OscReceiver2ActiveCheckBox->setChecked(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver2Active"), false));
    OscReceiver2IpByte1->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver2IpByte1"), 1));
    OscReceiver2IpByte2->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver2IpByte2"), 1));
    OscReceiver2IpByte3->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver2IpByte3"), 1));
    OscReceiver2IpByte4->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver2IpByte4"), 1));

    OscReceiver3ActiveCheckBox->setChecked(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver3Active"), false));
    OscReceiver3IpByte1->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver3IpByte1"), 1));
    OscReceiver3IpByte2->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver3IpByte2"), 1));
    OscReceiver3IpByte3->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver3IpByte3"), 1));
    OscReceiver3IpByte4->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver3IpByte4"), 1));

    OscReceiver4ActiveCheckBox->setChecked(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver4Active"), false));
    OscReceiver4IpByte1->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver4IpByte1"), 1));
    OscReceiver4IpByte2->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver4IpByte2"), 1));
    OscReceiver4IpByte3->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver4IpByte3"), 1));
    OscReceiver4IpByte4->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver4IpByte4"), 1));

    OscReceiver5ActiveCheckBox->setChecked(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver5Active"), false));
    OscReceiver5IpByte1->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver5IpByte1"), 1));
    OscReceiver5IpByte2->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver5IpByte2"), 1));
    OscReceiver5IpByte3->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver5IpByte3"), 1));
    OscReceiver5IpByte4->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscReceiver5IpByte4"), 1));

    OscSendSyncTriggers->setChecked(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscSendSyncTriggers"), false));
    OscSendSyncTriggersInterval->setValue(m_pConfig->getValue(
            ConfigKey("[OSC]", "OscSendSyncTriggersInterval"), 5000));
}

void DlgPrefOsc::slotApply() {
    m_pConfig->setValue(ConfigKey("[OSC]", "OscEnabled"),
            OscEnabledCheckBox->isChecked());

    m_pConfig->setValue(ConfigKey("[OSC]", "OscPortIn"),
            OscPortIn->value());

    m_pConfig->setValue(ConfigKey("[OSC]", "OscPortOut"),
            OscPortOut->value());

    //    m_pConfig->setValue(ConfigKey("[OSC]", "OscOutputBufferSize"),
    //            OscOutputBufferSize->value());

    //    m_pConfig->setValue(ConfigKey("[OSC]", "OscIpMtuSize"),
    //            OscIpMtuSize->value());

    QString OscReceiverIp, OscReceiverIpByte1, OscReceiverIpByte2,
            OscReceiverIpByte3, OscReceiverIpByte4;

    // Receiver 1
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver1Active"),
            OscReceiver1ActiveCheckBox->isChecked());
    OscReceiverIpByte1 = QString("%1").arg(OscReceiver1IpByte1->value());
    OscReceiverIpByte2 = QString("%1").arg(OscReceiver1IpByte2->value());
    OscReceiverIpByte3 = QString("%1").arg(OscReceiver1IpByte3->value());
    OscReceiverIpByte4 = QString("%1").arg(OscReceiver1IpByte4->value());
    OscReceiverIp = QString("%1.%2.%3.%4")
                            .arg(OscReceiverIpByte1,
                                    OscReceiverIpByte2,
                                    OscReceiverIpByte3,
                                    OscReceiverIpByte4);
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver1Ip"), OscReceiverIp);
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver1IpByte1"),
            OscReceiver1IpByte1->value());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver1IpByte2"),
            OscReceiver1IpByte2->value());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver1IpByte3"),
            OscReceiver1IpByte3->value());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver1IpByte4"),
            OscReceiver1IpByte4->value());

    // Receiver 2
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver2Active"),
            OscReceiver2ActiveCheckBox->isChecked());
    OscReceiverIpByte1 = QString("%1").arg(OscReceiver2IpByte1->value());
    OscReceiverIpByte2 = QString("%1").arg(OscReceiver2IpByte2->value());
    OscReceiverIpByte3 = QString("%1").arg(OscReceiver2IpByte3->value());
    OscReceiverIpByte4 = QString("%1").arg(OscReceiver2IpByte4->value());
    OscReceiverIp = QString("%1.%2.%3.%4")
                            .arg(OscReceiverIpByte1,
                                    OscReceiverIpByte2,
                                    OscReceiverIpByte3,
                                    OscReceiverIpByte4);
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver2Ip"), OscReceiverIp);
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver2IpByte1"),
            OscReceiver2IpByte1->value());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver2IpByte2"),
            OscReceiver2IpByte2->value());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver2IpByte3"),
            OscReceiver2IpByte3->value());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver2IpByte4"),
            OscReceiver2IpByte4->value());

    // Receiver 3
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver3Active"),
            OscReceiver3ActiveCheckBox->isChecked());
    OscReceiverIpByte1 = QString("%1").arg(OscReceiver3IpByte1->value());
    OscReceiverIpByte2 = QString("%1").arg(OscReceiver3IpByte2->value());
    OscReceiverIpByte3 = QString("%1").arg(OscReceiver3IpByte3->value());
    OscReceiverIpByte4 = QString("%1").arg(OscReceiver3IpByte4->value());
    OscReceiverIp = QString("%1.%2.%3.%4")
                            .arg(OscReceiverIpByte1,
                                    OscReceiverIpByte2,
                                    OscReceiverIpByte3,
                                    OscReceiverIpByte4);
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver3Ip"), OscReceiverIp);
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver3IpByte1"),
            OscReceiver3IpByte1->value());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver3IpByte2"),
            OscReceiver3IpByte2->value());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver3IpByte3"),
            OscReceiver3IpByte3->value());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver3IpByte4"),
            OscReceiver3IpByte4->value());

    // Receiver 4
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver4Active"),
            OscReceiver4ActiveCheckBox->isChecked());
    OscReceiverIpByte1 = QString("%1").arg(OscReceiver4IpByte1->value());
    OscReceiverIpByte2 = QString("%1").arg(OscReceiver4IpByte2->value());
    OscReceiverIpByte3 = QString("%1").arg(OscReceiver4IpByte3->value());
    OscReceiverIpByte4 = QString("%1").arg(OscReceiver4IpByte4->value());
    OscReceiverIp = QString("%1.%2.%3.%4")
                            .arg(OscReceiverIpByte1,
                                    OscReceiverIpByte2,
                                    OscReceiverIpByte3,
                                    OscReceiverIpByte4);
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver4Ip"), OscReceiverIp);
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver4IpByte1"),
            OscReceiver4IpByte1->value());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver4IpByte2"),
            OscReceiver4IpByte2->value());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver4IpByte3"),
            OscReceiver4IpByte3->value());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver4IpByte4"),
            OscReceiver4IpByte4->value());

    // Receiver 5
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver5Active"),
            OscReceiver5ActiveCheckBox->isChecked());
    OscReceiverIpByte1 = QString("%1").arg(OscReceiver5IpByte1->value());
    OscReceiverIpByte2 = QString("%1").arg(OscReceiver5IpByte2->value());
    OscReceiverIpByte3 = QString("%1").arg(OscReceiver5IpByte3->value());
    OscReceiverIpByte4 = QString("%1").arg(OscReceiver5IpByte4->value());
    OscReceiverIp = QString("%1.%2.%3.%4")
                            .arg(OscReceiverIpByte1,
                                    OscReceiverIpByte2,
                                    OscReceiverIpByte3,
                                    OscReceiverIpByte4);
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver5Ip"), OscReceiverIp);
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver5IpByte1"),
            OscReceiver5IpByte1->value());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver5IpByte2"),
            OscReceiver5IpByte2->value());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver5IpByte3"),
            OscReceiver5IpByte3->value());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscReceiver5IpByte4"),
            OscReceiver5IpByte4->value());

    // Sync Triggers
    m_pConfig->setValue(ConfigKey("[OSC]", "OscSendSyncTriggers"),
            OscSendSyncTriggers->isChecked());
    m_pConfig->setValue(ConfigKey("[OSC]", "OscSendSyncTriggersInterval"),
            OscSendSyncTriggersInterval->value());
}

void DlgPrefOsc::slotResetToDefaults() {
    OscEnabledCheckBox->setChecked(false);
    OscReceiver1ActiveCheckBox->setChecked(false);
    OscReceiver2ActiveCheckBox->setChecked(false);
    OscReceiver3ActiveCheckBox->setChecked(false);
    OscReceiver4ActiveCheckBox->setChecked(false);
    OscReceiver5ActiveCheckBox->setChecked(false);
    OscSendSyncTriggers->setChecked(false);
}
