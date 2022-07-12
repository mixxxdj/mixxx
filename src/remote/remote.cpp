#include "remote.h"

#include "httpserver/httplistener.h"

const mixxx::Logger kLogger("RemoteControl");

class RemoteController : public ::stefanfrings::HttpRequestHandler {
    public:
        RemoteController(QObject* parent=0) : ::stefanfrings::HttpRequestHandler(parent){
        };

        void service(::stefanfrings::HttpRequest& request, 
                     ::stefanfrings::HttpResponse& response){
            response.write("Hello World",true);
        };
    private:
        QCoreApplication *app;
};

mixxx::RemoteControl::RemoteControl(UserSettingsPointer pConfig, QObject* pParent){
    m_pSettings=pConfig;
    if(QVariant(m_pSettings->get(ConfigKey("[RemoteControl]","actv")).value).toBool()){
        kLogger.debug() << "Starting RemoteControl";
        QSettings settings;
        settings.setValue("host",m_pSettings->get(ConfigKey("[RemoteControl]","host")).value);
        settings.setValue("port",m_pSettings->get(ConfigKey("[RemoteControl]","port")).value);
        RemoteController rControl;
        ::stefanfrings::HttpListener(&settings, &rControl, pParent);
    }
        
}

mixxx::RemoteControl::~RemoteControl(){
        kLogger.debug() << "Shutdown RemoteControl";
}

