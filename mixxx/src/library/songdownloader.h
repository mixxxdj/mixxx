#ifndef SONGDOWNLOADER_H
#define SONGDOWNLOADER_H

#include <QtCore>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QQueue>

class SongDownloader : public QObject
{
    Q_OBJECT
    public:
        SongDownloader(QObject* parent);
        ~SongDownloader();

        bool downloadSongFromURL(QUrl& url);

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
        QQueue<QUrl> m_downloadQueue;
        QFile* m_pDownloadedFile;
        QNetworkReply* m_pReply;
        QNetworkRequest* m_pRequest;
};

#endif // SONGDOWNLOADER_H
