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
#include "library/autodj/autodjprocessor.h"
#include "library/autodj/autodjfeature.h"
#include "track/track.h"
#include "track/trackiterator.h"
#include "library/playlisttablemodel.h"

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
                         std::shared_ptr<Library> &library,
                         std::shared_ptr<PlayerManager> &ainf,
                         std::shared_ptr<DbConnectionPool> db,
                         QObject* parent=0) {

            httpServer.route("/", [settings] () {
                return QHttpServerResponse::fromFile(QString(settings->getResourcePath())+"/web/index.html" );
            });

            httpServer.route("/rcontrol",QHttpServerRequest::Method::Post,[this,parent,settings,collectionManager,library,ainf,db]
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
                        }else{
                            QJsonObject err;
                            err.insert("error","wrong password");
                            resproot.push_back(err);
                            jsonResponse.setArray(resproot);
                            responder.write(jsonResponse);
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
                                                            QStringList({"title","artist","album"}),
                                                            cur["searchtrack"].toString());


                        QSqlQuery query(QString("SELECT id,artist,title FROM library WHERE ")+search.toSql(),dbase);

                        QJsonArray tracklist;

                        if(query.exec() && query.first() ){
                            do{
                                QJsonObject jtrack;
                                jtrack.insert("id",query.value("id").toString());
                                jtrack.insert("artist",query.value("artist").toString());
                                jtrack.insert("title",query.value("title").toString());
                                tracklist.push_back(jtrack);
                            }while(query.next());
                            QJsonObject trackobj;
                            trackobj["tracklist"]=tracklist;
                            resproot.push_back(trackobj);
                        }
                    }
                    if(!cur["addautodj"].isNull()){
                        QJsonObject jautodj=cur["addautodj"].toObject();
                        if(!jautodj["trackid"].isNull() && !jautodj["position"].isNull() ){
                            QSqlDatabase dbase=DbConnectionPooled(db);
                            TrackId tid(jautodj["trackid"].toVariant());
                            PlaylistDAO playlist;
                            playlist.initialize(dbase);

                            int did=playlist.getPlaylistIdFromName("Auto DJ");
                            if(jautodj["position"].toString()=="begin"){
                                if(playlist.isHidden(did)==1){
                                    playlist.addTracksToAutoDJQueue(QList({tid}),PlaylistDAO::AutoDJSendLoc::TOP);
                                }
                            }else if(jautodj["position"].toString()=="end"){
                                if(playlist.isHidden(did)==1){
                                    playlist.addTracksToAutoDJQueue(QList({tid}),PlaylistDAO::AutoDJSendLoc::BOTTOM);
                                }
                            }
                        }
                    }

                    if(!cur["getautotracklist"].isNull()){
                            QSqlDatabase dbase=DbConnectionPooled(db);

                            PlaylistDAO playlist;
                            playlist.initialize(dbase);

                            int adjid=playlist.getPlaylistIdFromName("Auto DJ");
                            QJsonArray tracklist;

                            QList<TrackId> trackIds;

                            QSqlQuery query(dbase);
                            query.prepare(QStringLiteral("SELECT DISTINCT track_id FROM PlaylistTracks WHERE playlist_id = :id ORDER BY position ASC"));
                            query.bindValue(":id", adjid);
                            if (!query.exec()) {
                                return;
                            }
                            const int trackIdColumn = query.record().indexOf("track_id");

                            while (query.next()) {
                                    trackIds.append(TrackId(query.value(trackIdColumn)));
                            }

                            for(TrackIdList::Iterator it=trackIds.begin(); it!=trackIds.end(); ++it ){
                                TrackPointer tptrack=collectionManager->getTrackById(*it);
                                QJsonObject jtrack;
                                jtrack.insert("id",it->toString());
                                jtrack.insert("artist",tptrack->getArtist());
                                jtrack.insert("title",tptrack->getTitle());
                                tracklist.push_back(jtrack);
                            }

                            QJsonObject trackobj;
                            trackobj["tracklist"]=tracklist;
                            resproot.push_back(trackobj);
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
                                                                    std::shared_ptr<Library> &library,
                                                                    std::shared_ptr<DbConnectionPool> &database,
                                                                    std::shared_ptr<PlayerManager> &ainf,
                                    QObject* pParent) {
    kLogger.debug() << "Starting RemoteControl";
    if(QVariant(pConfig->get(ConfigKey("[RemoteControl]","actv")).value).toBool()){
        kLogger.debug() << "Starting RemoteControl Webserver";
        m_RemoteController = std::make_shared<RemoteController>(pConfig,trackscollmngr,library,ainf,database,pParent);
    }
}

mixxx::RemoteControl::~RemoteControl(){
        kLogger.debug() << "Shutdown RemoteControl";
}

