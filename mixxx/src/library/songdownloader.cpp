#include <QtCore>
#include <QtGui>
#include "defs_version.h"
#include "songdownloader.h"

#define TEMP_EXTENSION ".tmp"

SongDownloader::SongDownloader(QObject* parent) : QObject(parent)
{
    qDebug() << "SongDownloader constructed";
    
    m_pNetwork = new QNetworkAccessManager();
    m_pDownloadedFile = NULL;
    m_pRequest = NULL;
    //connect(m_pNetwork, SIGNAL(finished(QNetworkReply*)),
    //     this, SLOT(finishedSlot(QNetworkReply*)));

}

SongDownloader::~SongDownloader()
{
    delete m_pNetwork;
    
    delete m_pDownloadedFile;
    m_pDownloadedFile = NULL;
    delete m_pRequest;
    m_pRequest = NULL;
}

bool SongDownloader::downloadSongFromURL(QUrl& url)
{
    qDebug() << "SongDownloader::downloadSongFromURL()";

    m_downloadQueue.enqueue(url);
    downloadFromQueue();

    return true;
}


bool SongDownloader::downloadFromQueue()
{
    QUrl downloadUrl = m_downloadQueue.dequeue();
    //Extract the filename from the URL path
    QString filename = downloadUrl.path();
    QFileInfo fileInfo(filename);
    filename = fileInfo.fileName();

    m_pDownloadedFile = new QFile(filename + TEMP_EXTENSION);
    if (!m_pDownloadedFile->open(QIODevice::WriteOnly))
    {
        //TODO: Error
        qDebug() << "Failed to open" << m_pDownloadedFile->fileName();
        return false;
    }
   
    qDebug() << "SongDownloader: setting up download stuff";
    m_pRequest = new QNetworkRequest(downloadUrl);

    //Set up user agent for great justice
    QString mixxxUA = QString("%1 %2").arg(QApplication::applicationName(), VERSION);
    QByteArray mixxxUABA = mixxxUA.toAscii();
    m_pRequest->setRawHeader("User-Agent", mixxxUABA);
    m_pReply = m_pNetwork->get(*m_pRequest);

    connect(m_pReply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(m_pReply, SIGNAL(error(QNetworkReply::NetworkError)),
         this, SLOT(slotError(QNetworkReply::NetworkError)));   
    connect(m_pReply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(slotProgress(qint64, qint64)));
    connect(m_pReply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SIGNAL(downloadProgress(qint64, qint64)));
    connect(m_pReply, SIGNAL(finished()),
            this, SLOT(slotDownloadFinished()));    

    return true;
}

void SongDownloader::slotReadyRead()
{
    //Magic. Isn't this how C++ is supposed to work?
    //m_pDownloadedFile << m_pReply;
    //Update: :( 
    
    //QByteArray buffer;
    while (m_pReply->bytesAvailable() > 0)
    {
        qDebug() << "downloading...";
        m_pDownloadedFile->write(m_pReply->read(512));
    }
}

void SongDownloader::slotError(QNetworkReply::NetworkError error)
{
    qDebug() << "SongDownloader: Network error while trying to download a plugin.";
    
    //Delete partial file
    m_pDownloadedFile->remove();

    emit(downloadError());
}

void SongDownloader::slotProgress( qint64 bytesReceived, qint64 bytesTotal )
{
    qDebug() << bytesReceived << "/" << bytesTotal;
    emit(downloadProgress(bytesReceived, bytesTotal));
}

void SongDownloader::slotDownloadFinished()
{
    qDebug() << "SongDownloader: Download finished!";
    //Finish up with the reply and close the file handle
    m_pReply->deleteLater();
    m_pDownloadedFile->close();

    //Chop off the .tmp from the filename 
    QFileInfo info(*m_pDownloadedFile);
    QString filenameWithoutTmp = info.absoluteFilePath();
    filenameWithoutTmp.chop(QString(TEMP_EXTENSION).length());
    m_pDownloadedFile->rename(filenameWithoutTmp);
    delete m_pDownloadedFile;
    m_pDownloadedFile = NULL;
    delete m_pRequest;
    m_pRequest = NULL;

    if (m_downloadQueue.count() > 0) {
        downloadFromQueue();
    }

    {
        //XXX: Add the song to the My Downloads crate


        //Emit this signal when all the files have been downloaded.
        emit(downloadFinished());
    }
}

/*
void SongDownloader::finishedSlot(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        qDebug() << "SongDownloader: finishedSlot, no error";
    }
    else
        qDebug() << "SongDownloader: NAM error :-/";
}*/
