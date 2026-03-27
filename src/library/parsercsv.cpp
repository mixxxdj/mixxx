#include "library/parsercsv.h"

#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QtDebug>

#include "errordialoghandler.h"
#include "library/basesqltablemodel.h"
#include "library/parser.h"

namespace {

bool isColumnExported(BaseSqlTableModel* pPlaylistTableModel, int column) {
    if (pPlaylistTableModel->isColumnInternal(column)) {
        return false;
    }
    if (pPlaylistTableModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW) == column) {
        return false;
    }
    if (pPlaylistTableModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART) == column) {
        // This is the bas64 encoded image which may hit the maximum line length of spreadsheet applications
        return false;
    }
    return true;
}

} // namespace

// static
bool ParserCsv::isPlaylistFilenameSupported(const QString& playlistFile) {
    return playlistFile.endsWith(".csv", Qt::CaseInsensitive);
}

// static
QList<QString> ParserCsv::parseAllLocations(const QString& playlistFile) {
    QFile file(playlistFile);

    QList<QString> locations;
    //qDebug() << "ParserCsv: Starting to parse.";
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray bytes = file.readAll();

        QList<QList<QString>> tokens = tokenize(bytes, ',');

        const auto detect_location_column =
                [&](const auto& tokens_list,
                        auto predicate) -> std::optional<std::size_t> {
            const auto it = std::find_if(std::begin(tokens_list), std::end(tokens_list), predicate);
            return (it != std::end(tokens_list))
                    ? static_cast<std::size_t>(std::distance(std::begin(tokens_list), it))
                    : std::optional<std::size_t>{};
        };
        if (!tokens.isEmpty()) {
            std::optional<std::size_t> locationColumnIndex = detect_location_column(
                    tokens[0],
                    [&](auto i) { return i == QObject::tr("Location"); });
            if ((!locationColumnIndex.has_value()) && tokens.size() > 1) {
                // Last resort, find column with valid path in first row
                // This happens in case of csv files in a different language,
                // where the column name is not "Location" If the first row
                // contains a valid path separator, we assume this is the location column.
                // - Linux & macOS: Only / is a valid path separator
                // - Windows: / and \ are valid path separators
                // This is independent of the existence of the file referred in
                // the first row, as it's only used for the column detection,
                // and other rows might contain paths to existing files
                locationColumnIndex = detect_location_column(tokens[1],
                        [&](auto i) {
#ifdef Q_OS_WIN
                            return (i.contains(QChar('\\')) || i.contains(QChar('/')));
                        });
#else
                            return i.contains(QChar('/'));
                        });
#endif
            }
            if (locationColumnIndex.has_value()) {
                for (int row = 1; row < tokens.size(); ++row) {
                    if (locationColumnIndex.has_value() &&
                            locationColumnIndex.value() <
                                    static_cast<std::size_t>(
                                            tokens[row].size())) {
                        locations.append(tokens[row][static_cast<int>(
                                locationColumnIndex.value())]);
                    }
                }
            } else {
                qInfo() << "No location column found in"
                        << playlistFile;
            }
        }
        file.close();
    }

    qDebug() << "ParserCsv::parse() failed"
             << playlistFile
             << file.errorString();

    return locations;
}

// Code was posted at http://www.qtcentre.org/threads/35511-Parsing-CSV-data
// by "adzajac" and adapted to use QT Classes
QList<QList<QString>> ParserCsv::tokenize(const QByteArray& str, char delimiter) {
    QList<QList<QString>> tokens;

    unsigned int row = 0;
    bool quotes = false;
    QByteArray field = "";

    tokens.append(QList<QString>());

    for (int pos = 0; pos < str.length(); ++pos) {
        char c = str[pos];
        if (!quotes && c == '"') {
            quotes = true;
        } else if (quotes && c== '"' ) {
            if (pos + 1 < str.length() && str[pos + 1] == '"') {
                field.append(c);
                pos++;
            } else {
                quotes = false;
            }
        } else if (!quotes && c == delimiter) {
            if (Parser::isUtf8(field.constData())) {
                tokens[row].append(QString::fromUtf8(field));
            } else {
                tokens[row].append(QString::fromLatin1(field));
            }
            field.clear();
        } else if (!quotes && c == '\r' && str[pos + 1] == '\n') {
            // skip \r in \r\n
        } else if (!quotes && (c == '\r' || c == '\n')) {
            if (Parser::isUtf8(field.constData())) {
                tokens[row].append(QString::fromUtf8(field));
            } else {
                tokens[row].append(QString::fromLatin1(field));
            }
            field.clear();
            tokens.append(QList<QString>());
            row++;
        } else {
            field.push_back(c);
        }
    }
    return tokens;
}

bool ParserCsv::writeCSVFile(const QString &file_str, BaseSqlTableModel* pPlaylistTableModel, bool useRelativePath)
{
    /*
     * Important note:
     * On Windows \n will produce a <CR><CL> (=\r\n)
     * On Linux and OS X \n is <CR> (which remains \n)
     */

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

    qDebug() << "Basepath: " << base;
    QTextStream out(&file);
    // rfc4180: Common usage of CSV is US-ASCII ...
    // Using UTF-8 to get around codepage issues
    // and it's the default encoding in Ooo Calc
    DEBUG_ASSERT(out.encoding() == QStringConverter::Utf8);

    // writing header section
    bool first = true;
    int columns = pPlaylistTableModel->columnCount();
    for (int i = 0; i < columns; ++i) {
        if (!isColumnExported(pPlaylistTableModel, i)) {
            continue;
        }
        if (!first) {
            out << ",";
        } else {
            first = false;
        }
        out << "\"";
        QString field = pPlaylistTableModel->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
        out << field.replace('\"', "\"\"");  // escape "
        out << "\"";
    }
    out << "\r\n"; // CRLF according to rfc4180


    int rows = pPlaylistTableModel->rowCount();
    for (int j = 0; j < rows; j++) {
        // writing fields section
        first = true;
        for (int i = 0; i < columns; ++i) {
            if (!isColumnExported(pPlaylistTableModel, i)) {
                continue;
            }
            if (!first) {
                out << ",";
            } else {
                first = false;
            }
            out << "\"";
            QString field;
            if (i ==
                    pPlaylistTableModel->fieldIndex(
                            ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION)) {
                field = pPlaylistTableModel
                                ->data(pPlaylistTableModel->index(j, i),
                                        BaseTrackTableModel::kDataExportRole)
                                .toString();
                if (useRelativePath) {
                    field = base_dir.relativeFilePath(field);
                }
            } else {
                field = pPlaylistTableModel
                                ->data(pPlaylistTableModel->index(j, i),
                                        BaseTrackTableModel::kDataExportRole)
                                .toString();
            }
            out << field.replace('\"', "\"\"");  // escape "
            out << "\"";
        }
        out << "\r\n"; // CRLF according to rfc4180
    }
    return true;
}

bool ParserCsv::writeReadableTextFile(const QString &file_str, BaseSqlTableModel* pPlaylistTableModel, bool writeTimestamp)
{
    /*
     * Important note:
     * On Windows \n will produce a <CR><CL> (=\r\n)
     * On Linux and OS X \n is <CR> (which remains \n)
     */

    QFile file(file_str);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr,
                QObject::tr("Readable text Export Failed"),
                QObject::tr("Could not create file") + " " + file_str + "\n" + file.errorString());
        return false;
    }

    QTextStream out(&file);

    // export each row as "01. 0:00:00 Artist - Title"

    int msecsFromStartToMidnight = 0;
    int i; // fieldIndex
    int rows = pPlaylistTableModel->rowCount();
    for (int j = 0; j < rows; j++) {
        // writing fields section
        i = pPlaylistTableModel->fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);
        if (i >= 0) {
            int nr = pPlaylistTableModel
                             ->data(pPlaylistTableModel->index(j, i),
                                     BaseSqlTableModel::kDataExportRole)
                             .toInt();
            out << QString("%1.").arg(nr,2,10,QLatin1Char('0'));
        }

        if (writeTimestamp) {
            i = pPlaylistTableModel->fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_DATETIMEADDED);
            if (i >= 0) {
                QTime time =
                        pPlaylistTableModel
                                ->data(pPlaylistTableModel->index(j, i),
                                        BaseTrackTableModel::kDataExportRole)
                                .toTime();
                if (j == 0) {
                    msecsFromStartToMidnight = time.msecsTo(QTime(0,0,0,0));
                }
                QTime time2 = time.addMSecs(msecsFromStartToMidnight);
                out << " ";
                out << time2.toString("H:mm:ss");
            }
        }

        i = pPlaylistTableModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST);
        if (i >= 0) {
            out << " ";
            out << pPlaylistTableModel
                            ->data(pPlaylistTableModel->index(j, i),
                                    BaseTrackTableModel::kDataExportRole)
                            .toString();
        }
        i = pPlaylistTableModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TITLE);
        if (i >= 0) {
            out << " - ";
            out << pPlaylistTableModel->data(pPlaylistTableModel->index(j,i)).toString();;
        }
        out << "\n";
    }
    return true;
}
