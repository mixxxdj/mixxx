#include <vector>

#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonObject>

#include "remote.h"

#include "library/library.h"
#include "library/trackcollection.h"
#include "library/trackcollectioniterator.h"

#include <httpserver/httplistener.h>
#include <httpserver/staticfilecontroller.h>
#include <httpserver/httpsessionstore.h>

const mixxx::Logger kLogger("RemoteControl");

#define FSWEBPATH "/home/jan.koester/projects/mixxx/res/web/"

std::shared_ptr<::stefanfrings::StaticFileController> m_staticFileController;

namespace mixxx {
    class Session {
    private:
        QTime    loginTime;
        QUuid    sessionid;
        friend RemoteController;
    };
    
    ::std::vector<Session> m_Session;
    
    class RemoteController : public ::stefanfrings::HttpRequestHandler {
    public:
        RemoteController(UserSettingsPointer settings,TrackCollectionManager *collectionManager,QObject* parent=0) : 
                    ::stefanfrings::HttpRequestHandler(parent){
            m_pSettings=settings;
            m_Parent=parent;
            m_TrackCollectionManager=collectionManager;
        };
        
        virtual ~RemoteController(){
        };
        
        void service(::stefanfrings::HttpRequest& request, 
                     ::stefanfrings::HttpResponse& response){
            QByteArray path=request.getPath();
            if (path==("/")) {
                response.redirect("/index.html");
                response.flush();
            }else if(path=="/rcontrol"){
                QJsonDocument jsonRequest = QJsonDocument::fromJson(request.getBody());
                QJsonDocument jsonResponse;
                QJsonArray requroot = jsonRequest.array();
                QJsonArray resproot = {};
                for(QJsonArray::Iterator i = requroot.begin(); i<requroot.end(); ++i){
                    QJsonObject cur=i->toObject();
                    if(!cur["login"].isNull()){
                        if(QString::compare(cur["login"].toObject()["password"].toString(),
                            m_pSettings->get(ConfigKey("[RemoteControl]","pass")).value)==0
                        ){
                            
                            QJsonObject sessid;                           
                            Session session;
                            session.sessionid = QUuid::createUuid();
                            session.loginTime = QTime::currentTime();
                            m_Session.push_back(session);
                            sessid.insert("sessionid",session.sessionid.toString());
                            resproot.push_back(sessid);
                            jsonResponse.setArray(resproot);
                            response.write(jsonResponse.toJson());
                            response.flush();
                            kLogger.debug() << "successfuly login";
                            return;
                        }else{
                            QJsonObject err;
                            err.insert("error","wrong password");
                            resproot.push_back(err);
                            jsonResponse.setArray(resproot);
                            response.write(jsonResponse.toJson());
                            response.flush();
                            return;
                        };
                    }
                }
                
                bool auth = false;
                
                for(QJsonArray::Iterator i =requroot.begin(); i<requroot.end(); ++i){
                    QJsonObject cur=i->toObject();
                    if(!cur["sessionid"].isNull()){
                        std::vector<Session>::iterator it;
                        for (it = m_Session.begin(); it < m_Session.end(); it++){
                            if(QString::compare(cur["sessionid"].toString(),it->sessionid.toString())==0){
                                auth=true;
                                QJsonObject auth;
                                auth.insert("logintime",it->loginTime.toString());
                                resproot.push_back(auth);
                            }
                        }
                    }
                }
                
                if(!auth){
                    QJsonObject err;
                    err.insert("error","wrong sessionid");
                    resproot.push_back(err);
                    jsonResponse.setArray(resproot);
                    response.write(jsonResponse.toJson());
                    response.flush();
                    return;
                }
                
                for(QJsonArray::Iterator i =requroot.begin(); i<requroot.end(); ++i){
                    QJsonObject cur=i->toObject();
                    if(!cur["searchtrack"].isNull()){
                        QJsonArray list;
                        TrackIdList tracklist;
                        TrackByIdCollectionIterator curcoll(m_TrackCollectionManager,tracklist);
                        do{
                            for(TrackIdList::Iterator curtrack=tracklist.begin(); curtrack<tracklist.end(); curtrack++){
                                list.push_back(curtrack->toString());
                                kLogger.info() << curtrack->toString();
                            };
                        }while(curcoll.nextItem());
                        resproot.push_back(list);                   
                    }
                }
                
                jsonResponse.setArray(resproot);
                response.write(jsonResponse.toJson());
                response.flush();
            }else{
                m_staticFileController->service(request,response);
            }
        };
    private:
        TrackCollectionManager *m_TrackCollectionManager;
        UserSettingsPointer     m_pSettings;
        QObject*                m_Parent;
    };
};

mixxx::RemoteControl::RemoteControl(UserSettingsPointer pConfig,TrackCollectionManager *trackscollmngr,
                                    QObject* pParent){
    kLogger.debug() << "Starting RemoteControl";
    m_pSettings=pConfig;
    m_pTrackCollectionManager=trackscollmngr;
    if(QVariant(m_pSettings->get(
                        ConfigKey("[RemoteControl]","actv")
                    ).value).toBool()){
        kLogger.debug() << "Starting RemoteControl Webserver";
    
        m_HttpSettings = std::make_shared<QSettings>();
        
        m_HttpSettings->setValue("host",
                          m_pSettings->get(
                              ConfigKey("[RemoteControl]","host")
                          ).value);
        m_HttpSettings->setValue("port",
                          m_pSettings->get(
                              ConfigKey("[RemoteControl]","port")
                          ).value);
        
        m_FileSettings = std::make_shared<QSettings>();
        
        m_FileSettings->setValue("path",FSWEBPATH);
        
        m_FileSettings->setValue("encoding","UTF-8");
        
        m_FileSettings->setValue("maxAge",90000);
        
        m_FileSettings->setValue("cacheTime",0);
        
        m_FileSettings->setValue("cacheSize",0);
        
        m_FileSettings->setValue("maxCachedFileSize",0);
        
        m_RemoteController = std::make_shared<RemoteController>(pConfig,m_pTrackCollectionManager,pParent);
        
        m_staticFileController=std::make_shared<::stefanfrings::StaticFileController>
                                (m_FileSettings.get(),pParent);
        
        m_SessionSettings = std::make_shared<QSettings>();
        
        m_FileSettings->setValue("expirationTime","3600000");
        m_FileSettings->setValue("cookieName","sessionid");
        m_FileSettings->setValue("cookiePath","/");
        m_FileSettings->setValue("cookieComment","Identifies the mixxx session");
        m_HttpListener=std::make_shared<::stefanfrings::HttpListener>(
            m_HttpSettings.get(),
            m_RemoteController.get(), pParent);
    }
        
}

mixxx::RemoteControl::~RemoteControl(){
        kLogger.debug() << "Shutdown RemoteControl";
}

