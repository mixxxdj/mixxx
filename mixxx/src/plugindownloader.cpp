#include <QtCore>
#include <QtGui>
#include "defs_version.h"
#include "plugindownloader.h"

PluginDownloader::PluginDownloader(QObject* parent) : QObject(parent)
{
    qDebug() << "PluginDownloader constructed";
/*
    if (!checkForM4APlugin())
    {
        //TODO: Show M4A Plugin download dialog
        
        downloadM4APlugin();
    }
    */
}

PluginDownloader::~PluginDownloader()
{

}

bool PluginDownloader::checkForM4APlugin()
{
#ifdef __WINDOWS__
    m_pluginDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    m_mp4libPath = m_pluginDir + "/mp4v2.dll";
#endif

#ifdef __MACOSX__
    m_pluginDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    m_mp4libPath = m_pluginDir + "/mixxx/plugins/mp4v2.dylib";
#endif

#ifdef __LINUX__
    m_pluginDir = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
    m_mp4libPath = m_pluginDir + "/.mixxx/plugins/libmp4v2.so";
#endif
    qDebug() << "PluginDownloader: Checking for plugins in" << m_mp4libPath;
    
    return QFile::exists(m_mp4libPath);
}

bool PluginDownloader::downloadM4APlugin()
{
    qDebug() << "PluginDownloader::downloadM4APlugin()";
    
    QDir dir;
    dir.mkpath(m_pluginDir);
    m_tempMp4libPath = m_mp4libPath + ".tmp";

    m_pDownloadedFile = new QFile(m_tempMp4libPath);
    if (!m_pDownloadedFile->open(QIODevice::WriteOnly))
    {
        //TODO: Error
        qDebug() << "Failed to open" << m_pDownloadedFile->fileName();
        return false;
    }
   
    qDebug() << "PluginDownloader: setting up download stuff";
    m_pNetwork = new QNetworkAccessManager();
    connect(m_pNetwork, SIGNAL(finished(QNetworkReply*)),
         this, SLOT(finishedSlot(QNetworkReply*)));
    m_pRequest = new QNetworkRequest(QUrl("http://downloads.mixxx.org/beats/Peer%20Control%20Demo%202.ogg"));

    //Set up user agent for great justice
    QString mixxxUA = QString("%1 %2").arg(QApplication::applicationName()).arg(VERSION);
    QByteArray mixxxUABA = mixxxUA.toAscii();
    m_pRequest->setRawHeader("User-Agent", mixxxUABA);
    m_pReply = m_pNetwork->get(*m_pRequest);

    connect(m_pReply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(m_pReply, SIGNAL(error(QNetworkReply::NetworkError)),
         this, SLOT(slotError(QNetworkReply::NetworkError)));   
    connect(m_pReply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(slotProgress(qint64, qint64)));
    connect(m_pReply, SIGNAL(finished()),
            this, SLOT(downloadFinished()));    
    
    return true;
}

void PluginDownloader::slotReadyRead()
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

void PluginDownloader::slotError(QNetworkReply::NetworkError error)
{
    qDebug() << "PluginDownloader: Network error while trying to download a plugin.";
    
    //Delete partial file
    QFile::remove(m_tempMp4libPath);
}

void PluginDownloader::slotProgress( qint64 bytesReceived, qint64 bytesTotal )
{
    qDebug() << bytesReceived << "/" << bytesTotal;
}

void PluginDownloader::downloadFinished()
{
    qDebug() << "PluginDownloader: Download finished!";
    m_pReply->deleteLater();
    m_pDownloadedFile->close();
    QFile::rename(m_tempMp4libPath, m_mp4libPath);
}

void PluginDownloader::finishedSlot(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        qDebug() << "PluginDownloader: finishedSlot, no error";
    }
    else
        qDebug() << "PluginDownloader: NAM error :-/";
}
