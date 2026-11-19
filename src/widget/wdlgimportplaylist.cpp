#include "widget/wdlgimportplaylist.h"

#include <QCheckBox>
#include <QFile>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringLiteral>
#include <QTableWidget>
#include <QTextStream>
#include <QVBoxLayout>

#include "library/dao/playlistdao.h"
#include "library/parsercsv.h"
#include "moc_wdlgimportplaylist.cpp"

namespace {
constexpr int kColumnCount = 10;
constexpr int kColTitle = 0;
constexpr int kColArtist = 1;
constexpr int kColAlbum = 2;
constexpr int kColAlbumArtist = 3;
constexpr int kColId = 4;
constexpr int kColDuration = 5;
constexpr int kColBitrate = 6;
constexpr int kColRating = 7;
constexpr int kColSize = 8;
constexpr int kColLocation = 9;

QString escapeLikePattern(const QString& text) {
    QString out = text;
    out.replace('\\', "\\\\");
    out.replace('%', "\\%");
    out.replace('_', "\\_");
    return out;
}

QStringList extractWords(const QString& text) {
    const QStringList words = text.split(' ', Qt::SkipEmptyParts);
    QStringList result;
    for (const QString& w : words) {
        if (w.size() > 1) {
            result << w;
        }
    }
    return result;
}

QTableWidgetItem* makeItem(const QString& text) {
    auto* item = new QTableWidgetItem(text);
    item->setToolTip(text);
    return item;
}
} // namespace

WDlgImportPlaylist::WDlgImportPlaylist(const QList<ImportEntry>& entries,
        const QSqlDatabase& database,
        int playlistId,
        QTextStream* reportStream,
        QWidget* parent)
        : QDialog(parent),
          m_entries(entries),
          m_database(database),
          m_playlistId(playlistId),
          m_reportStream(reportStream) {
    setWindowTitle(tr("Inspect Import File Entries"));
    resize(900, 600);

    auto* layout = new QVBoxLayout(this);

    m_labelCurrentEntry = new QLabel(this);
    m_labelCurrentEntry->setTextFormat(Qt::RichText);
    layout->addWidget(m_labelCurrentEntry);

    auto* editLayout = new QHBoxLayout();
    m_lineEditTitle = new QLineEdit(this);
    m_lineEditArtist = new QLineEdit(this);
    m_lineEditTitle->setPlaceholderText(tr("Title"));
    m_lineEditArtist->setPlaceholderText(tr("Artist"));
    m_searchButton = new QPushButton(tr("Search"), this);

    editLayout->addWidget(new QLabel(tr("Title:"), this));
    editLayout->addWidget(m_lineEditTitle);
    editLayout->addWidget(new QLabel(tr("Artist:"), this));
    editLayout->addWidget(m_lineEditArtist);
    editLayout->addWidget(m_searchButton);
    layout->addLayout(editLayout);

    m_checkMatchBoth = new QCheckBox(tr("Match title AND artist"), this);
    m_checkMatchBoth->setToolTip(tr(
            "If checked, results must match both title and artist. "
            "Otherwise, either field may match."));
    m_checkMatchBoth->setChecked(true);
    layout->addWidget(m_checkMatchBoth);

    m_tableCandidates = new QTableWidget(this);
    m_tableCandidates->setColumnCount(kColumnCount);
    m_tableCandidates->setHorizontalHeaderLabels(
            {tr("Title"),
                    tr("Artist"),
                    tr("Album"),
                    tr("Album Artist"),
                    tr("Id"),
                    tr("Duration"),
                    tr("Bitrate (type)"),
                    tr("Rating"),
                    tr("Size"),
                    tr("Location")});
    m_tableCandidates->setColumnHidden(kColId, true);
    m_tableCandidates->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableCandidates->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableCandidates->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableCandidates->horizontalHeader()->setSectionResizeMode(kColTitle, QHeaderView::Stretch);
    m_tableCandidates->horizontalHeader()->setSectionResizeMode(kColArtist, QHeaderView::Stretch);
    layout->addWidget(m_tableCandidates);

    auto* buttonLayout = new QHBoxLayout();
    m_addSelectedButton = new QPushButton(tr("Add Selected"), this);
    m_nextButton = new QPushButton(tr("Next"), this);
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    buttonLayout->addWidget(m_addSelectedButton);
    buttonLayout->addWidget(m_nextButton);
    buttonLayout->addWidget(m_cancelButton);
    layout->addLayout(buttonLayout);

    connect(m_searchButton, &QPushButton::clicked, this, [this]() {
        m_tableCandidates->setSortingEnabled(false);
        runSearch(m_lineEditTitle->text(), m_lineEditArtist->text());
        m_tableCandidates->setSortingEnabled(true);
    });

    connect(m_checkMatchBoth, &QCheckBox::toggled, this, [this]() {
        if (m_currentIndex < m_entries.size()) {
            m_tableCandidates->setSortingEnabled(false);
            runSearch(m_lineEditTitle->text(), m_lineEditArtist->text());
            m_tableCandidates->setSortingEnabled(true);
        }
    });

    connect(m_addSelectedButton, &QPushButton::clicked, this, [this]() {
        addSelectedTracks();
    });

    connect(m_nextButton, &QPushButton::clicked, this, [this]() {
        if (m_currentIndex > 0 && m_currentIndex <= m_entries.size()) {
            const auto& entry = m_entries[m_currentIndex - 1];
            if (m_reportStream && m_reportStream->device()) {
                *m_reportStream << entry.artist.trimmed() << " - "
                                << entry.title.trimmed() << " - "
                                << (m_importedThisEntry ? "imported" : "not imported")
                                << "\n";
                m_reportStream->flush();
            }
        }
        m_importedThisEntry = false;
        advanceToNext();
    });

    connect(m_cancelButton, &QPushButton::clicked, this, [this]() {
        for (int i = m_currentIndex - 1; i >= 0 && i < m_entries.size(); ++i) {
            const auto& entry = m_entries[i];
            if (m_reportStream && m_reportStream->device()) {
                *m_reportStream << entry.artist.trimmed() << " - "
                                << entry.title.trimmed() << " - not imported\n";
            }
        }
        if (m_reportStream) {
            m_reportStream->flush();
        }
        reject();
    });

    connect(m_tableCandidates,
            &QTableWidget::cellDoubleClicked,
            this,
            [this](int, int) {
                addSelectedTracks();
                if (m_currentIndex > 0 && m_currentIndex <= m_entries.size()) {
                    const auto& entry = m_entries[m_currentIndex - 1];
                    if (m_reportStream && m_reportStream->device()) {
                        *m_reportStream << entry.artist.trimmed() << " - "
                                        << entry.title.trimmed() << " - "
                                        << (m_importedThisEntry ? "imported" : "not imported")
                                        << "\n";
                        m_reportStream->flush();
                    }
                }
                m_importedThisEntry = false;
                advanceToNext();
            });

    advanceToNext();
}

void WDlgImportPlaylist::advanceToNext() {
    if (m_currentIndex >= m_entries.size()) {
        accept();
        return;
    }

    const auto& entry = m_entries[m_currentIndex];
    m_labelCurrentEntry->setText(
            tr("<span style='font-size:12pt;'>Select a corresponding track "
               "for importfile-entry (%1/%2):</span><br>"
               "<b><span style='font-size:14pt;'>%3 - %4%5%6</span></b>")
                    .arg(m_currentIndex + 1)
                    .arg(m_entries.size())
                    .arg(entry.title,
                            entry.artist,
                            entry.duration.isEmpty()
                                    ? QString()
                                    : tr(" [%1]").arg(entry.duration),
                            entry.album.isEmpty()
                                    ? QString()
                                    : tr("<br><span style='font-size:12pt;'>"
                                         "Album: %1</span>")
                                              .arg(entry.album)));

    m_lineEditTitle->setText(entry.title);
    m_lineEditArtist->setText(entry.artist);

    m_tableCandidates->setSortingEnabled(false);
    runSearch(entry.title, entry.artist);
    m_tableCandidates->setSortingEnabled(true);
    ++m_currentIndex;
}

void WDlgImportPlaylist::runSearch(const QString& title, const QString& artist) {
    m_tableCandidates->setRowCount(0);

    const QStringList titleWords = extractWords(title);
    const QStringList artistWords = extractWords(artist);

    QStringList titleConds(titleWords.size(),
            QStringLiteral("lower(title) LIKE lower(?) ESCAPE '\\'"));
    const QString titleWhere = titleConds.join(QStringLiteral(" AND "));

    QStringList artistConds(artistWords.size(),
            QStringLiteral("lower(artist) LIKE lower(?) ESCAPE '\\'"));
    const QString artistWhere = artistConds.join(QStringLiteral(" AND "));

    QStringList conditions;
    if (!titleWhere.isEmpty()) {
        conditions << "(" + titleWhere + ")";
    }
    if (!artistWhere.isEmpty()) {
        conditions << "(" + artistWhere + ")";
    }

    QString whereClause;
    if (!conditions.isEmpty()) {
        if (!title.isEmpty() && !artist.isEmpty()) {
            const QString conjunction = m_checkMatchBoth->isChecked()
                    ? QStringLiteral(" AND ")
                    : QStringLiteral(" OR ");
            whereClause = QStringLiteral("WHERE ") + conditions.join(conjunction);
        } else if (title.isEmpty()) {
            whereClause = QStringLiteral("WHERE ") + artistWhere;
        } else {
            whereClause = QStringLiteral("WHERE ") + titleWhere;
        }
    } else {
        whereClause = QStringLiteral("WHERE library.id = -1");
    }

    const QString queryString = QStringLiteral(
            "SELECT library.id AS id, "
            "       library.title AS title, "
            "       library.artist AS artist, "
            "       library.album AS album, "
            "       library.album_artist AS album_artist, "
            "       printf('%d:%02d', "
            "              CAST(library.duration AS INT) / 60, "
            "              CAST(library.duration AS INT) % 60) AS duration_mss, "
            "       library.bitrate || ' (' || library.filetype || ')' AS filebitrate, "
            "       library.rating AS rating, "
            "       track_locations.directory || '/' || track_locations.filename "
            "           AS tracklocation, "
            "       printf('%.2f', track_locations.filesize / (1024.0 * 1024.0)) "
            "           AS filesize_mb "
            "FROM library "
            "JOIN track_locations ON library.id = track_locations.id %1")
                                        .arg(whereClause);

    QSqlQuery query(m_database);
    query.prepare(queryString);

    auto likePattern = [](const QString& word) {
        return QVariant(QStringLiteral("%") + escapeLikePattern(word) + QStringLiteral("%"));
    };

    for (const QString& w : titleWords) {
        query.addBindValue(likePattern(w));
    }
    for (const QString& w : artistWords) {
        query.addBindValue(likePattern(w));
    }

    if (!query.exec()) {
        qWarning() << "[WDlgImportPlaylist] query failed:"
                   << query.lastError().text();
        return;
    }

    while (query.next()) {
        const int row = m_tableCandidates->rowCount();
        m_tableCandidates->insertRow(row);
        m_tableCandidates->setItem(row, kColTitle, makeItem(query.value("title").toString()));
        m_tableCandidates->setItem(row, kColArtist, makeItem(query.value("artist").toString()));
        m_tableCandidates->setItem(row, kColAlbum, makeItem(query.value("album").toString()));
        m_tableCandidates->setItem(row,
                kColAlbumArtist,
                makeItem(query.value("album_artist").toString()));
        m_tableCandidates->setItem(row, kColId, new QTableWidgetItem(query.value("id").toString()));
        m_tableCandidates->setItem(row,
                kColDuration,
                makeItem(query.value("duration_mss").toString()));
        m_tableCandidates->setItem(row,
                kColBitrate,
                makeItem(query.value("filebitrate").toString()));
        m_tableCandidates->setItem(row, kColRating, makeItem(query.value("rating").toString()));
        m_tableCandidates->setItem(row, kColSize, makeItem(query.value("filesize_mb").toString()));
        m_tableCandidates->setItem(row,
                kColLocation,
                makeItem(query.value("tracklocation").toString()));
    }
}

void WDlgImportPlaylist::addSelectedTracks() {
    QList<int> trackIds;
    const auto selected = m_tableCandidates->selectionModel()->selectedRows(kColId);
    for (const auto& index : selected) {
        trackIds.append(index.data().toInt());
    }

    if (trackIds.isEmpty()) {
        QMessageBox::information(this,
                tr("No selection"),
                tr("Please select at least one track."));
        return;
    }

    QSqlQuery query(m_database);
    for (int trackId : trackIds) {
        ++m_position;
        query.prepare(
                "INSERT INTO PlaylistTracks (playlist_id, track_id, position) "
                "VALUES (?, ?, ?)");
        query.addBindValue(m_playlistId);
        query.addBindValue(trackId);
        query.addBindValue(m_position);
        if (!query.exec()) {
            qWarning() << "[WDlgImportPlaylist] insert failed:"
                       << query.lastError().text();
        }
    }
    m_importedThisEntry = true;
}

std::optional<QList<ImportEntry>> WDlgImportPlaylist::parseImportFile(
        const QString& path, QString* errorMessage) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = tr("Cannot open %1: %2")
                                    .arg(path, file.errorString());
        }
        return std::nullopt;
    }

    const QByteArray bytes = file.readAll();

    // 1st comma -> semicolon -> tab
    QList<QList<QString>> rows = ParserCsv::tokenize(bytes, ',');
    if (rows.isEmpty() || rows.first().size() < 2) {
        const auto alt = ParserCsv::tokenize(bytes, ';');
        if (!alt.isEmpty() && alt.first().size() >= 2) {
            rows = alt;
        } else {
            const auto tabs = ParserCsv::tokenize(bytes, '\t');
            if (!tabs.isEmpty() && tabs.first().size() >= 2) {
                rows = tabs;
            }
        }
    }

    if (rows.isEmpty()) {
        if (errorMessage) {
            *errorMessage = ("The file is empty.");
        }
        return std::nullopt;
    }

    const QStringList header = rows.first();
    int titleIndex = -1;
    int artistIndex = -1;
    int albumIndex = -1;
    int durationIndex = -1;
    for (int i = 0; i < header.size(); ++i) {
        const QString c = header.at(i).trimmed().toLower();
        if (c == QLatin1String("title") || c == QLatin1String("song")) {
            titleIndex = i;
        } else if (c == QLatin1String("artist")) {
            artistIndex = i;
        } else if (c == QLatin1String("album")) {
            albumIndex = i;
        } else if (c == QLatin1String("duration") ||
                c == QLatin1String("time")) {
            durationIndex = i;
        }
    }

    if (titleIndex < 0 || artistIndex < 0) {
        if (errorMessage) {
            *errorMessage = tr(
                    "Could not find Title and Artist columns "
                    "in the header. Found: %1")
                                    .arg(header.join(QStringLiteral(", ")));
        }
        return std::nullopt;
    }

    QList<ImportEntry> entries;
    entries.reserve(rows.size() - 1);
    for (int r = 1; r < rows.size(); ++r) {
        const auto& row = rows.at(r);
        if (row.size() <= qMax(titleIndex, artistIndex)) {
            continue;
        }
        ImportEntry entry;
        entry.title = row.at(titleIndex);
        entry.artist = row.at(artistIndex);
        if (albumIndex >= 0 && albumIndex < row.size()) {
            entry.album = row.at(albumIndex);
        }
        if (durationIndex >= 0 && durationIndex < row.size()) {
            entry.duration = row.at(durationIndex);
        }
        entries.append(entry);
    }

    if (entries.isEmpty()) {
        if (errorMessage) {
            *errorMessage = ("The file contains no usable entries.");
        }
        return std::nullopt;
    }
    return entries;
}
