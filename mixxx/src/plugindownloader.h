#include <QtCore>
#include <QtNetwork>
#include <QNetworkAccessManager>

class PluginDownloader : public QObject
{
    Q_OBJECT
    public:
        PluginDownloader(QObject* parent);
        ~PluginDownloader();
        
        bool checkForM4APlugin();
        bool downloadM4APlugin();
       
    public slots:
        void slotReadyRead();
        void slotError(QNetworkReply::NetworkError error);
        void slotProgress( qint64 bytesReceived, qint64 bytesTotal );
        void downloadFinished();
        void finishedSlot(QNetworkReply* reply);
    private:
        
        QNetworkAccessManager* m_pNetwork;
        QFile* m_pDownloadedFile;
        QString m_pluginDir;
        QString m_mp4libPath;
        QString m_tempMp4libPath;
        QNetworkReply* m_pReply;
        QNetworkRequest* m_pRequest;
};
