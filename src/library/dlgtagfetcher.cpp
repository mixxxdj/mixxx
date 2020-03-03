#include <QTreeWidget>
#include <QtDebug>

#include "library/dlgtagfetcher.h"
#include "track/tracknumbers.h"

namespace {

QStringList track2row(const Track& track) {
    const QString trackNumberAndTotal = TrackNumbers::joinAsString(
            track.getTrackNumber(),
            track.getTrackTotal());
    QStringList row;
    row.reserve(6);
    row
            << track.getYear()
            << track.getAlbum()
            << track.getAlbumArtist()
            << trackNumberAndTotal
            << track.getTitle()
            << track.getArtist();
    return row;
}

void addTrack(
        const QStringList& trackRow,
        int resultIndex,
        QTreeWidget* parent) {
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, trackRow);
    item->setData(0, Qt::UserRole, resultIndex);
    item->setData(0, Qt::TextAlignmentRole, Qt::AlignRight);
}

} // anonymous namespace

DlgTagFetcher::DlgTagFetcher(QWidget *parent)
        : QDialog(parent),
          m_tagFetcher(parent),
          m_networkResult(NetworkResult::Ok) {
    init();
}

void DlgTagFetcher::init() {
    setupUi(this);

    connect(btnApply, &QPushButton::clicked,
            this, &DlgTagFetcher::apply);
    connect(btnQuit, &QPushButton::clicked,
            this, &DlgTagFetcher::quit);
    connect(btnPrev, &QPushButton::clicked,
            this, &DlgTagFetcher::previous);
    connect(btnNext, &QPushButton::clicked,
            this, &DlgTagFetcher::next);
    connect(results, &QTreeWidget::currentItemChanged,
            this, &DlgTagFetcher::resultSelected);

    connect(&m_tagFetcher, &TagFetcher::resultAvailable,
            this, &DlgTagFetcher::fetchTagFinished);
    connect(&m_tagFetcher, &TagFetcher::fetchProgress,
            this, &DlgTagFetcher::fetchTagProgress);
    connect(&m_tagFetcher, &TagFetcher::networkError,
            this, &DlgTagFetcher::slotNetworkResult);

    // Resize columns, this can't be set in the ui file
    results->setColumnWidth(0, 50);  // Year column
    results->setColumnWidth(1, 160); // Album column
    results->setColumnWidth(2, 160); // Album artist column
    results->setColumnWidth(3, 50);  // Track (numbers) column
    results->setColumnWidth(4, 160); // Title column
    results->setColumnWidth(5, 160); // Artist column
}

void DlgTagFetcher::loadTrack(const TrackPointer& track) {
    if (track == NULL) {
        return;
    }
    results->clear();
    disconnect(track.get(),
            &Track::changed,
            this,
            &DlgTagFetcher::slotTrackChanged);

    m_track = track;
    m_data = Data();
    m_networkResult = NetworkResult::Ok;
    m_tagFetcher.startFetch(m_track);

    connect(track.get(),
            &Track::changed,
            this,
            &DlgTagFetcher::slotTrackChanged);

    updateStack();
}

void DlgTagFetcher::slotTrackChanged(TrackId trackId) {
    if (m_track && m_track->getId() == trackId) {
        updateStack();
    }
}

void DlgTagFetcher::apply() {
    int resultIndex = m_data.m_selectedResult;
    if (resultIndex > -1) {
        mixxx::TrackMetadata importedMetadata;
        m_data.m_results[resultIndex]->readTrackMetadata(&importedMetadata);
        mixxx::TrackMetadata updatedMetadata;
        m_track->readTrackMetadata(&updatedMetadata);
        if (!importedMetadata.getTrackInfo().getArtist().isEmpty()) {
            updatedMetadata.refTrackInfo().setArtist(
                    importedMetadata.getTrackInfo().getArtist());
        }
        if (!importedMetadata.getTrackInfo().getTitle().isEmpty()) {
            updatedMetadata.refTrackInfo().setTitle(
                    importedMetadata.getTrackInfo().getTitle());
        }
        if (!importedMetadata.getTrackInfo().getTrackNumber().isEmpty()) {
            updatedMetadata.refTrackInfo().setTrackNumber(
                    importedMetadata.getTrackInfo().getTrackNumber()
            );
        }
        if (!importedMetadata.getTrackInfo().getTrackTotal().isEmpty()) {
            updatedMetadata.refTrackInfo().setTrackTotal(
                    importedMetadata.getTrackInfo().getTrackTotal()
            );
        }
        if (!importedMetadata.getTrackInfo().getYear().isEmpty()) {
            updatedMetadata.refTrackInfo().setYear(
                    importedMetadata.getTrackInfo().getYear());
        }
        if (!importedMetadata.getAlbumInfo().getArtist().isEmpty()) {
            updatedMetadata.refAlbumInfo().setArtist(
                    importedMetadata.getAlbumInfo().getArtist());
        }
        if (!importedMetadata.getAlbumInfo().getTitle().isEmpty()) {
            updatedMetadata.refAlbumInfo().setTitle(
                    importedMetadata.getAlbumInfo().getTitle());
        }
#if defined(__EXTRA_METADATA__)
        if (!importedMetadata.getTrackInfo().getMusicBrainzArtistId().isNull()) {
            updatedMetadata.refTrackInfo().setMusicBrainzArtistId(
                    importedMetadata.getTrackInfo().getMusicBrainzArtistId()
            );
        }
        if (!importedMetadata.getTrackInfo().getMusicBrainzRecordingId().isNull()) {
            updatedMetadata.refTrackInfo().setMusicBrainzRecordingId(
                    importedMetadata.getTrackInfo().getMusicBrainzRecordingId()
            );
        }
        if (!importedMetadata.getTrackInfo().getMusicBrainzReleaseId().isNull()) {
            updatedMetadata.refTrackInfo().setMusicBrainzReleaseId(
                    importedMetadata.getTrackInfo().getMusicBrainzReleaseId()
            );
        }
        if (!importedMetadata.getAlbumInfo().getMusicBrainzArtistId().isNull()) {
            updatedMetadata.refAlbumInfo().setMusicBrainzArtistId(
                    importedMetadata.getAlbumInfo().getMusicBrainzArtistId()
            );
        }
        if (!importedMetadata.getAlbumInfo().getMusicBrainzReleaseId().isNull()) {
            updatedMetadata.refAlbumInfo().setMusicBrainzReleaseId(
                    importedMetadata.getAlbumInfo().getMusicBrainzReleaseId());
        }
        if (!importedMetadata.getAlbumInfo().getMusicBrainzReleaseGroupId().isNull()) {
            updatedMetadata.refAlbumInfo().setMusicBrainzReleaseGroupId(
                    importedMetadata.getAlbumInfo().getMusicBrainzReleaseGroupId());
        }
#endif // __EXTRA_METADATA__
        m_track->importMetadata(std::move(updatedMetadata));
    }
}

void DlgTagFetcher::quit() {
    m_tagFetcher.cancel();
    accept();
}

void DlgTagFetcher::fetchTagProgress(QString text) {
    QString status = tr("Status: %1");
    loadingStatus->setText(status.arg(text));
}

void DlgTagFetcher::fetchTagFinished(const TrackPointer track,
                                     const QList<TrackPointer>& tracks) {
    // check if the answer is for this track
    if (m_track->getLocation() != track->getLocation()) {
        return;
    }

    m_data.m_pending = false;
    m_data.m_results = tracks;
    // qDebug() << "number of results = " << tracks.size();
    updateStack();
}

void DlgTagFetcher::slotNetworkResult(int httpError, QString app, QString message, int code) {
    m_networkResult = httpError == 0 ?  NetworkResult::UnknownError : NetworkResult::HttpError;
    m_data.m_pending = false;
    QString strError = tr("HTTP Status: %1");
    QString strCode = tr("Code: %1");
    httpStatus->setText(strError.arg(httpError) + "\n" + strCode.arg(code) + "\n" + message);
    QString unknownError = tr("Mixxx can't connect to %1 for an unknown reason.");
    cantConnectMessage->setText(unknownError.arg(app));
    QString cantConnect = tr("Mixxx can't connect to %1.");
    cantConnectHttp->setText(cantConnect.arg(app));
    updateStack();
}

void DlgTagFetcher::updateStack() {
    if (m_data.m_pending) {
        stack->setCurrentWidget(loading_page);
        return;
    } else if (m_networkResult == NetworkResult::HttpError) {
        stack->setCurrentWidget(networkError_page);
        return;
    } else if (m_networkResult == NetworkResult::UnknownError) {
        stack->setCurrentWidget(generalnetworkError_page);
        return;
    } else if (m_data.m_results.isEmpty()) {
        stack->setCurrentWidget(error_page);
        return;
    }
    btnApply->setEnabled(true);
    stack->setCurrentWidget(results_page);

    results->clear();

    addDivider(tr("Original tags"), results);
    addTrack(track2row(*m_track), -1, results);

    addDivider(tr("Suggested tags"), results);
    {
        int trackIndex = 0;
        QSet<QStringList> trackRows;
        foreach (const TrackPointer track, m_data.m_results) {
            const auto trackRow = track2row(*track);
            // Ignore duplicate results
            if (!trackRows.contains(trackRow)) {
                trackRows.insert(trackRow);
                addTrack(trackRow, trackIndex, results);
            }
            ++trackIndex;
        }
    }

    // Find the item that was selected last time
    for (int i=0 ; i<results->model()->rowCount() ; ++i) {
        const QModelIndex index = results->model()->index(i, 0);
        const QVariant id = index.data(Qt::UserRole);
        if (!id.isNull() && id.toInt() == m_data.m_selectedResult) {
            results->setCurrentIndex(index);
            break;
        }
    }
}

void DlgTagFetcher::addDivider(const QString& text, QTreeWidget* parent) const {
    QTreeWidgetItem* item = new QTreeWidgetItem(parent);
    item->setFirstColumnSpanned(true);
    item->setText(0, text);
    item->setFlags(Qt::NoItemFlags);
    item->setForeground(0, palette().color(QPalette::Disabled, QPalette::Text));

    QFont bold_font(font());
    bold_font.setBold(true);
    item->setFont(0, bold_font);
}

void DlgTagFetcher::resultSelected() {
  if (!results->currentItem())
    return;

  const int resultIndex = results->currentItem()->data(0, Qt::UserRole).toInt();
  m_data.m_selectedResult = resultIndex;
}
