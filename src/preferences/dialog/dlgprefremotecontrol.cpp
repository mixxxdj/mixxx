#include <QWidget>
#include <QUrl>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QStringList>

#include "moc_dlgprefremotecontrol.cpp"

#include "preferences/dialog/dlgprefremotecontrol.h"

#include "remote/remote.h"

namespace {
QString currentNonLoopbackAddresses() {
    QStringList addresses;
    const auto allAddresses = QNetworkInterface::allAddresses();
    for (const QHostAddress& address : allAddresses) {
        if (address.isLoopback()) {
            continue;
        }
        if (address.protocol() != QAbstractSocket::IPv4Protocol) {
            continue;
        }
        addresses << address.toString();
    }
    if (addresses.isEmpty()) {
        return QObject::tr("none found");
    }
    return addresses.join(", ");
}
} // namespace

DlgPrefRemoteControl::DlgPrefRemoteControl(QWidget *pParent,UserSettingsPointer  pSettings,
        std::shared_ptr<mixxx::RemoteControl> pRemoteControl) : DlgPreferencePage(pParent){
    m_pSettings=pSettings;
    m_pRemoteControl=pRemoteControl;
    setupUi(this);
    if(QVariant(m_pSettings->get(ConfigKey("[RemoteControl]","actv")).value).toBool()){
        this->remoteactv->setChecked(true);
    }
    this->remoteaddr->setText(m_pSettings->get(ConfigKey("[RemoteControl]","host")).value);
    this->remoteport->setText(m_pSettings->get(ConfigKey("[RemoteControl]","port")).value);
    this->remotepass->setText(m_pSettings->get(ConfigKey("[RemoteControl]","pass")).value);
    this->remotecurrentip->setText(currentNonLoopbackAddresses());
};

DlgPrefRemoteControl::~DlgPrefRemoteControl(){
};

QUrl DlgPrefRemoteControl::helpUrl() const {
    return QUrl();
}

void DlgPrefRemoteControl::slotUpdate(){
    this->remotecurrentip->setText(currentNonLoopbackAddresses());
}
      
void DlgPrefRemoteControl::slotApply(){
    if(this->remoteactv->isChecked()){
        m_pSettings->set(ConfigKey("[RemoteControl]","actv"),ConfigValue(true));
    }else{
        m_pSettings->set(ConfigKey("[RemoteControl]","actv"),ConfigValue(false));
    }
    
    m_pSettings->set(ConfigKey("[RemoteControl]","host"),ConfigValue(this->remoteaddr->text()));
    m_pSettings->set(ConfigKey("[RemoteControl]","port"),ConfigValue(this->remoteport->text()));
    m_pSettings->set(ConfigKey("[RemoteControl]","pass"),ConfigValue(this->remotepass->text()));
    m_pSettings->save();

    if(m_pRemoteControl){
        m_pRemoteControl->reload();
    }
}

void DlgPrefRemoteControl::slotResetToDefaults(){
    this->remoteactv->setChecked(false);
    this->remoteaddr->setText("0.0.0.0");
    this->remoteport->setText("8080");
    this->remotepass->setText("mixxx");      
}
