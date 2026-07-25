#include <QWidget>
#include <QUrl>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QStringList>

#include "moc_dlgprefremotecontrol.cpp"

#include "preferences/dialog/dlgprefremotecontrol.h"

#include "remote/remote.h"

namespace {
QStringList currentNonLoopbackAddresses() {
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
    return addresses;
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
    updateCurrentIpDisplay();
    connect(this->remoteport, &QLineEdit::textChanged, this, &DlgPrefRemoteControl::updateCurrentIpDisplay);
};

DlgPrefRemoteControl::~DlgPrefRemoteControl(){
};

QUrl DlgPrefRemoteControl::helpUrl() const {
    return QUrl();
}

void DlgPrefRemoteControl::slotUpdate(){
    updateCurrentIpDisplay();
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

void DlgPrefRemoteControl::updateCurrentIpDisplay(){
    const QStringList addresses = currentNonLoopbackAddresses();
    if(addresses.isEmpty()){
        this->remotecurrentip->setText(tr("none found"));
        return;
    }
    const QString port = this->remoteport->text();
    QStringList urls;
    for(const QString& address : addresses){
        urls << QStringLiteral("http://%1:%2").arg(address, port);
    }
    this->remotecurrentip->setText(urls.join(", "));
}

void DlgPrefRemoteControl::slotResetToDefaults(){
    this->remoteactv->setChecked(false);
    this->remoteaddr->setText("0.0.0.0");
    this->remoteport->setText("8080");
    this->remotepass->setText("mixxx");      
}
