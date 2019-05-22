#include <QDesktopServices>
#include <QFileDialog>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "dlgprefosc.h"
#include "oscclient/defs_oscclient.h"
#include "oscserver/defs_oscserver.h"
#include "util/sandbox.h"

DlgPrefOsc::DlgPrefOsc(QWidget *parent, UserSettingsPointer &pConfig)
    : DlgPreferencePage(parent), m_pConfig(pConfig) {
  m_pUpdateCO = std::make_unique<ControlObject>(
      ConfigKey(OSC_SERVER_PREF_KEY, "NeedsUpdate"));
  m_pErrorCO =
      std::make_unique<ControlObject>(ConfigKey(OSC_SERVER_PREF_KEY, "Error"));
  m_pErrorCO->connectValueChangeRequest(this, &DlgPrefOsc::slotError);

  setupUi(this);

  LineEditPortServer->setPlaceholderText(
      QString("e.g. 7777, default: %1").arg(OSC_SERVER_DEFAULT_PORT));

  // I think it might be better to wait for the user to select "Apply" or "OK"
  // since this is how most applications behave and they can reach those buttons
  // via the "Tab" key
  /*
  connect(LineEditServer, SIGNAL(returnPressed()),
          this, SLOT(slotApply()));
  connect(LineEditPort, SIGNAL(returnPressed()),
          this, SLOT(slotApply()));
  connect(LineEditPortServer, SIGNAL(returnPressed()),
          this, SLOT(slotApply()));
  */

  // slotApply();
}

DlgPrefOsc::~DlgPrefOsc() {}

void DlgPrefOsc::slotResetToDefaults() {
  LineEditServer->setText("");
  LineEditPort->setText("");
  CheckServerEnabled->setChecked(false);
  LineEditPortServer->setText("");
}

// This function updates/refreshes the contents of this dialog.
void DlgPrefOsc::slotUpdate() {

  QString serverUrl =
      m_pConfig->getValueString(ConfigKey(OSC_CLIENT_PREF_KEY, "Server"));
  LineEditServer->setText(serverUrl);
  QString port =
      m_pConfig->getValueString(ConfigKey(OSC_CLIENT_PREF_KEY, "Port"));
  LineEditPort->setText(port);
  bool serverEnabled =
      m_pConfig->getValue<bool>(ConfigKey(OSC_SERVER_PREF_KEY, "Enabled"));
  CheckServerEnabled->setChecked(serverEnabled);
  QString serverPort =
      m_pConfig->getValueString(ConfigKey(OSC_SERVER_PREF_KEY, "Port"));
  LineEditPortServer->setText(serverPort);
}

void DlgPrefOsc::slotApply() {
  m_pConfig->set(ConfigKey(OSC_CLIENT_PREF_KEY, "Server"),
                 ConfigValue(LineEditServer->text()));
  m_pConfig->set(ConfigKey(OSC_CLIENT_PREF_KEY, "Port"),
                 ConfigValue(LineEditPort->text()));

  bool serverNeedsUpdate = false;

  bool prevEnabled =
      m_pConfig->getValue<bool>(ConfigKey(OSC_SERVER_PREF_KEY, "Enabled"));
  if (CheckServerEnabled->isChecked() != prevEnabled) {
    serverNeedsUpdate = true;
  }

  QString prevPort =
      m_pConfig->getValueString(ConfigKey(OSC_SERVER_PREF_KEY, "Port"));
  if (LineEditServer->text().compare(prevPort) != 0) {
    serverNeedsUpdate = true;
  }

  m_pConfig->set(ConfigKey(OSC_SERVER_PREF_KEY, "Enabled"),
                 ConfigValue(CheckServerEnabled->isChecked()));
  m_pConfig->set(ConfigKey(OSC_SERVER_PREF_KEY, "Port"),
                 ConfigValue(LineEditPortServer->text()));

  // Wait until after all m_pConfig values have been set to trigger the update
  if (serverNeedsUpdate) {
    m_pUpdateCO->set(1.0);
  }
}

void DlgPrefOsc::slotError(double error) {
  if (error > 0.0) {
    LabelStatus->setText(
        "Failed to initialize OSC server. "
        "Please check that the port you entered is not in use and that no "
        "firewall is blocking Mixxx.");
  } else {
    if (m_pConfig->getValue<bool>(ConfigKey(OSC_SERVER_PREF_KEY, "Enabled"))) {
      LabelStatus->setText("OSC server enabled");
    } else {
      LabelStatus->setText("OSC server disabled");
    }
  }
}
