#include <QFileDialog>
#include <QDesktopServices>

//#include "dlgprefosc.h"
#include "oscclient/defs_oscclient.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "util/sandbox.h"

DlgPrefOsc::DlgPrefOsc(QWidget* parent, UserSettingsPointer& pConfig)
        : DlgPreferencePage(parent),
          m_pConfig(pConfig) {
    setupUi(this);

    connect(LineEditServer, SIGNAL(returnPressed()),
            this, SLOT(slotApply()));
    connect(LineEditPort, SIGNAL(returnPressed()),
            this, SLOT(slotApply()));

    //slotApply();
}


DlgPrefOsc::~DlgPrefOsc() {

}


void DlgPrefOsc::slotResetToDefaults() {
    LineEditServer->setText("");
}

// This function updates/refreshes the contents of this dialog.
void DlgPrefOsc::slotUpdate() {

    QString serverUrl = m_pConfig->getValueString(ConfigKey(OSC_CLIENT_PREF_KEY, "Server"));
    LineEditServer->setText(serverUrl);
    QString port = m_pConfig->getValueString(ConfigKey(OSC_CLIENT_PREF_KEY, "Port"));
    LineEditPort->setText(port);
    qDebug() << "update";

}

void DlgPrefOsc::slotApply() {
    m_pConfig->set(ConfigKey(OSC_CLIENT_PREF_KEY, "Server"), ConfigValue(LineEditServer->text()));
    m_pConfig->set(ConfigKey(OSC_CLIENT_PREF_KEY, "Port"), ConfigValue(LineEditPort->text()));
    qDebug() << "apply";
}