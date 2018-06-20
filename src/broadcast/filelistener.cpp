
#include <QtCore/QTextCodec>
#include "broadcast/filelistener.h"

#include "moc_filelistener.cpp"
#include "preferences/dialog/dlgprefbroadcast.h"

FileListener::FileListener(UserSettingsPointer pConfig)
        : m_COsettingsChanged(kSettingsChanged),
          m_pConfig(pConfig),
          m_latestSettings(DlgPrefMetadata::getPersistedSettings(pConfig)) {

    connect(&m_COsettingsChanged,SIGNAL(valueChanged(double)),
            this,SLOT(slotFileSettingsChanged(double)));
    updateStateFromSettings();
}

FileListener::~FileListener() {
    m_file.resize(0);
}

void FileListener::slotBroadcastCurrentTrack(TrackPointer pTrack) {
    if (!pTrack)
        return;
    QTextStream stream(&m_file);
    //Clear file
    m_file.resize(0);
    QTextCodec *codec = QTextCodec::codecForName(m_latestSettings.fileEncoding);
    DEBUG_ASSERT(codec);
    stream.setCodec(codec);
    QString writtenString =
            m_latestSettings.fileFormatString.replace("author",pTrack->getArtist()).
            replace("title",pTrack->getTitle());
    stream << writtenString << '\n';
}

void FileListener::slotScrobbleTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
}

void FileListener::slotAllTracksPaused() {
    m_file.resize(0);
}

void FileListener::slotFileSettingsChanged(double value) {
    if (value) {
        m_latestSettings = DlgPrefMetadata::getLatestSettings();
        updateStateFromSettings();
    }
}

void FileListener::updateStateFromSettings() {
    if (m_latestSettings.enabled) {
        updateFile();
    }
    else  {
        m_file.setFileName(m_latestSettings.filePath);
        if (m_file.exists()) {
            m_file.remove();
        }
    }
}

void FileListener::updateFile() {
    if (m_file.isOpen()) {
        m_file.seek(0);
        QByteArray fileContents = m_file.readAll();
        QTextCodec *codec = QTextCodec::codecForName(m_latestSettings.fileEncoding);
        DEBUG_ASSERT(codec);
        QByteArray newFileContents = codec->fromUnicode(fileContents);
        m_file.remove();
        m_file.setFileName(m_latestSettings.filePath);
        m_file.open(QIODevice::ReadWrite |
                    QIODevice::Truncate |
                    QIODevice::Text |
                    QIODevice::Unbuffered);
        m_file.write(newFileContents);
    }
    else {
        m_file.setFileName(m_latestSettings.filePath);
        m_file.open(QIODevice::ReadWrite |
                    QIODevice::Truncate |
                    QIODevice::Text |
                    QIODevice::Unbuffered);
    }
}
