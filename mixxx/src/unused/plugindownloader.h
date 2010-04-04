#include <QtCore>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QQueue>

class PluginDownloader : public QObject
{
    Q_OBJECT
    public:
        PluginDownloader(QObject* parent);
        ~PluginDownloader();
        
        QString getPluginDir();
        bool checkForM4APlugin();
        bool downloadM4APlugin();
       
    public slots:
        void slotReadyRead();
        void slotError(QNetworkReply::NetworkError error);
        void slotProgress( qint64 bytesReceived, qint64 bytesTotal );
        void slotDownloadFinished();
        //void finishedSlot(QNetworkReply* reply);
    signals:
        void downloadProgress(qint64, qint64);
        void downloadFinished();
        void downloadError();

    private:
        bool downloadFromQueue();
        
        QNetworkAccessManager* m_pNetwork;
        QQueue<QPair<QUrl, QString> > m_downloadQueue;
        QFile* m_pDownloadedFile;
        //QString m_mp4libPath;
        QMap<QUrl, QString> m_mp4PluginFiles;
        QNetworkReply* m_pReply;
        QNetworkRequest* m_pRequest;
};

