#include "library/parserpls.h"

#include <QDir>
#include <QFile>
#include <QtDebug>

#include "errordialoghandler.h"

namespace {

QString getFilePath(QTextStream* pStream) {
    QString textline = pStream->readLine();
    while (!textline.isEmpty()) {
        if (textline.isNull()) {
            break;
        }

        if (textline.contains("File")) {
            int iPos = textline.indexOf("=", 0);
            ++iPos;
            return textline.right(textline.length() - iPos);
        }
        textline = pStream->readLine();
    }
    // Signal we reached the end
    return QString();
}

} // namespace

// static
bool ParserPls::isPlaylistFilenameSupported(const QString& playlistFile) {
    return playlistFile.endsWith(".pls", Qt::CaseInsensitive);
}

// static
QList<QString> ParserPls::parseAllLocations(const QString& playlistFile) {
    QFile file(playlistFile);

    QList<QString> locations;
    if (file.open(QIODevice::ReadOnly)) {
        /* Unfortunately, QT 4.7 does not handle <CR> (=\r or asci value 13) line breaks.
         * This is important on OS X where iTunes, e.g., exports M3U playlists using <CR>
         * rather that <LF>
         *
         * Using QFile::readAll() we obtain the complete content of the playlist as a ByteArray.
         * We replace any '\r' with '\n' if applicable
         * This ensures that playlists from iTunes on OS X can be parsed
         */
        QByteArray byteArray = file.readAll();
        //detect encoding
        bool isCRLF_encoded = byteArray.contains("\r\n");
        bool isCR_encoded = byteArray.contains("\r");
        if (isCR_encoded && !isCRLF_encoded) {
            byteArray.replace('\r', '\n');
        }
        QTextStream textstream(byteArray.constData());

        while(!textstream.atEnd()) {
            QString psLine = getFilePath(&textstream);
            if(psLine.isNull()) {
                break;
            } else {
                locations.append(psLine);
            }

        }

        file.close();
    }

    qDebug() << "ParserPls::parse() failed"
             << playlistFile
             << file.errorString();

    return locations;
}

bool ParserPls::writePLSFile(const QString &file_str, const QList<QString> &items, bool useRelativePath)
{
    QFile file(file_str);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        ErrorDialogHandler* pDialogHandler = ErrorDialogHandler::instance();
        ErrorDialogProperties* props = pDialogHandler->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle(QObject::tr("Playlist Export Failed"));
        props->setText(QObject::tr("Could not create file") + " " + file_str);
        props->setDetails(file.errorString());
        pDialogHandler->requestErrorDialog(props);
        return false;
    }
    //Base folder of file
    QString base = file_str.section('/', 0, -2);
    QDir base_dir(base);

    QTextStream out(&file);
    out << "[playlist]\n";
    out << "NumberOfEntries=" << items.size() << "\n";
    for (int i = 0; i < items.size(); ++i) {
        //Write relative path if possible
        if (useRelativePath) {
            //QDir::relativePath() will return the absolutePath if it cannot compute the
            //relative Path
            out << "File" << i << "=" << base_dir.relativeFilePath(items.at(i)) << "\n";
        } else {
            out << "File" << i << "=" << items.at(i) << "\n";
        }
    }

    return true;
}
