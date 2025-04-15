#include <memory>
#include <vector>

#include <QHttpServer>
#include <QHttpServerResponse>
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpServer>
#include <QSqlResult>

#include "remote.h"

#include "util/db/dbconnectionpooled.h"

#include "library/library.h"
#include "library/trackcollection.h"
#include "library/trackcollectioniterator.h"
#include "library/searchquery.h"
#include "library/searchqueryparser.h"

const mixxx::Logger kLogger("RemoteControl");

namespace mixxx {
    class Session {
    private:
        QTime    loginTime;
        QUuid    sessionid;
        friend RemoteController;
    };

    ::std::vector<Session> m_Session;

    class RemoteController {
    public:
        RemoteController(UserSettingsPointer settings,
                         std::shared_ptr<TrackCollectionManager> &collectionManager,
                         std::shared_ptr<DbConnectionPool> db,
                         QObject* parent=0) {

            httpServer.route("/", [settings] () {
                return QHttpServerResponse::fromFile(QString(settings->getResourcePath())+"/web/index.html" );
            });

            httpServer.route("/rcontrol",QHttpServerRequest::Method::Post,[this,settings,collectionManager,db]
                (const QHttpServerRequest &request, QHttpServerResponder &responder) {

                QJsonDocument jsonRequest = QJsonDocument::fromJson(request.body());
                QJsonDocument jsonResponse;
                QJsonArray requroot = jsonRequest.array();
                QJsonArray resproot = {};
                for(QJsonArray::Iterator i = requroot.begin(); i<requroot.end(); ++i){
                    QJsonObject cur=i->toObject();
                    if(!cur["login"].isNull()){
                        if(QString::compare(cur["login"].toObject()["password"].toString(),
                            settings->get(ConfigKey("[RemoteControl]","pass")).value)==0
                        ){
                            
                            QJsonObject sessid;                           
                            Session session;
                            session.sessionid = QUuid::createUuid();
                            session.loginTime = QTime::currentTime();
                            m_Session.push_back(session);
                            sessid.insert("sessionid",session.sessionid.toString());
                            resproot.push_back(sessid);
                            jsonResponse.setArray(resproot);
                            responder.write(jsonResponse);
                            return;
                        }else{
                            QJsonObject err;
                            err.insert("error","wrong password");
                            resproot.push_back(err);
                            jsonResponse.setArray(resproot);
                            responder.write(jsonResponse);
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
                    responder.write(jsonResponse);
                    return;
                }
                
                for(QJsonArray::Iterator i =requroot.begin(); i<requroot.end(); ++i){
                    QJsonObject cur=i->toObject();
                    if(!cur["searchtrack"].isNull()){
                        QJsonArray list;

                        QSqlDatabase dbase=DbConnectionPooled(db);

                        TextFilterNode search(dbase,
                                                            QStringList({"id","title","artist"}),
                                                            cur["searchtrack"].toString());

                        QSqlQuery query(search.toSql(),dbase);

                        if(query.exec() && query.first() ){
                            do{
                                list.push_back(query.value(0).toString());
                                kLogger.info() << query.value("title").toString();
                            }while(query.next());
                            resproot.push_back(list);
                        }
                    }
                }

                jsonResponse.setArray(resproot);
                responder.write(jsonResponse);
                return;
            });

            httpServer.route("/<arg>",QHttpServerRequest::Method::Get, [settings] (const QString &url) {
                return QHttpServerResponse::fromFile(QString(settings->getResourcePath())+"/web/"+url);
            });

            auto tcpserver = new QTcpServer();

            if (!tcpserver->listen(QHostAddress(settings->get(ConfigKey("[RemoteControl]","host")).value),
                                               quint16(settings->get(ConfigKey("[RemoteControl]","port")).value.toUInt())) ||
                                              !httpServer.bind(tcpserver)
                ) {
                qCritical()  << "Cannot listen at Port" << settings->get(ConfigKey("[RemoteControl]","port")).value.toInt();
                delete tcpserver;
            }
        };

        virtual ~RemoteController(){

        };
    private:
        QObject*                 m_Parent;
        QHttpServer           httpServer;
    };
};

mixxx::RemoteControl::RemoteControl(UserSettingsPointer pConfig,
                                                                    std::shared_ptr<TrackCollectionManager> &trackscollmngr,
                                                                    std::shared_ptr<DbConnectionPool> &database,
                                    QObject* pParent) {
    kLogger.debug() << "Starting RemoteControl";
    if(QVariant(pConfig->get(ConfigKey("[RemoteControl]","actv")).value).toBool()){
        kLogger.debug() << "Starting RemoteControl Webserver";
        m_RemoteController = std::make_shared<RemoteController>(pConfig,trackscollmngr,database,pParent);
    }
}

mixxx::RemoteControl::~RemoteControl(){
        kLogger.debug() << "Shutdown RemoteControl";
}

