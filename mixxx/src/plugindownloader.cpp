#include <QtCore>
#include <QtGui>
#include "defs_version.h"
#include "plugindownloader.h"

#define TEMP_EXTENSION ".tmp"

PluginDownloader::PluginDownloader(QObject* parent) : QObject(parent)
{
    qDebug() << "PluginDownloader constructed";
    
    m_pNetwork = new QNetworkAccessManager();
    connect(m_pNetwork, SIGNAL(finished(QNetworkReply*)),
         this, SLOT(finishedSlot(QNetworkReply*)));
    
    QString pluginDir;
#ifdef __WINDOWS__
    pluginDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/";
#endif

#ifdef __APPLE__
    QString frameworksDir;
    pluginDir = QCoreApplication::applicationDirPath(); //blah/Mixxx.app/Contents/MacOS
    pluginDir.remove("MacOS");
    frameworksDir = pluginDir;
    pluginDir.append("PlugIns/"); //blah/Mixxx.app/Contents/PlugIns
    frameworksDir.append("Frameworks/"); //blah/Mixxx.app/Contents/Frameworks
#endif

#ifdef __LINUX__
    pluginDir = QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + "/.mixxx/";
#endif
    qDebug() << "PluginDownloader: Plugin directory is" << pluginDir;
    
    //Make sure the directory exists...
    QDir dir;
    dir.mkpath(pluginDir);

#ifdef __WINDOWS__
    m_mp4PluginFiles.insert(QUrl("http://downloads.mixxx.org/plugins/win32/mp4v2.dll"),
                           pluginDir + "mp4v2.dll");
    m_mp4PluginFiles.insert(QUrl("http://downloads.mixxx.org/plugins/win32/faad2.dll"),
                           pluginDir + "faad2.dll");
    m_mp4PluginFiles.insert(QUrl("http://downloads.mixxx.org/plugins/win32/libsoundsourcem4a.dll"),
                           pluginDir + "libsoundsourcem4a.dll");
#endif

#ifdef __APPLE__
    m_mp4PluginFiles.insert(QUrl("http://downloads.mixxx.org/plugins/osx/mp4v2.dylib"),
                           frameworksDir + "mp4v2.dylib");
    m_mp4PluginFiles.insert(QUrl("http://downloads.mixxx.org/plugins/osx/faad2.dylib"),
                           frameworksDir + "faad2.dylib");
    m_mp4PluginFiles.insert(QUrl("http://downloads.mixxx.org/plugins/osx/libsoundsourcem4a.dylib"),
                           pluginDir + "libsoundsourcem4a.dylib");
#endif

#ifdef __LINUX__

    //64-bit Ubuntu
    if (QDir::exists("/usr/lib64"))
    {
        m_mp4PluginFiles.insert(QUrl("http://downloads.mixxx.org/plugins/ubuntu-amd64/mixxx-m4a_1.8.0-ubuntu-amd64.deb"),
                                pluginDir + "mixxx-m4a_1.8.0-ubuntu-amd64.deb");
    }
    else
    {
        m_mp4PluginFiles.insert(QUrl("http://downloads.mixxx.org/plugins/ubuntu-i386/mixxx-m4a_1.8.0-ubuntu-i386.deb"),
                                pluginDir + "mixxx-m4a_1.8.0-ubuntu-i386.deb");
    }

    //Make a dummy entry so we detect when the deb package is installed
    m_mp4PluginFiles.insert(QUrl("DUMMY"),
                            "/usr/lib/libsoundsourcem4a.so");
#endif

}

PluginDownloader::~PluginDownloader()
{
    delete m_pNetwork;
}

bool PluginDownloader::checkForM4APlugin()
{
    bool ret = true;

    QList<QString> pluginFiles = m_mp4PluginFiles.values();

    //Check to make sure all the plugin files exist
    for (int i = 0; i < pluginFiles.count(); i++)
    {
        ret = ret && QFile::exists(pluginFiles[i]);
    }
    
    return ret;
}

bool PluginDownloader::downloadM4APlugin()
{
    qDebug() << "PluginDownloader::downloadM4APlugin()";

    QMapIterator<QUrl, QString> it(m_mp4PluginFiles);
    while (it.hasNext())
    {
        it.next();
        if (it.key() != QUrl("DUMMY"))
            m_downloadQueue.enqueue(qMakePair(it.key(), it.value()));
    }
    downloadFromQueue();

    return true;
}


bool PluginDownloader::downloadFromQueue()
{
    QPair<QUrl, QString> download = m_downloadQueue.dequeue();
    QUrl downloadUrl = download.first;
    QString filename = download.second;

    m_pDownloadedFile = new QFile(filename + TEMP_EXTENSION);
    if (!m_pDownloadedFile->open(QIODevice::WriteOnly))
    {
        //TODO: Error
        qDebug() << "Failed to open" << m_pDownloadedFile->fileName();
        return false;
    }
   
    qDebug() << "PluginDownloader: setting up download stuff";
    m_pRequest = new QNetworkRequest(downloadUrl);

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
    m_pDownloadedFile->remove();
}

void PluginDownloader::slotProgress( qint64 bytesReceived, qint64 bytesTotal )
{
    qDebug() << bytesReceived << "/" << bytesTotal;
}

void PluginDownloader::downloadFinished()
{
    qDebug() << "PluginDownloader: Download finished!";
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
    else
    {
#ifdef __LINUX__
        if (filenameWithoutTmp.endsWith(".deb"))
        {
            //Launch the gdebi graphical Debian package installer...
            QProcess::startDetached("gdebi-gtk " + filenameWithoutTmp);
        }
#endif
    }
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
