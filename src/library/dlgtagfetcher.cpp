#include "library/dlgtagfetcher.h"

#include <QTreeWidget>
#include <QtDebug>

#include "track/track.h"
#include "track/tracknumbers.h"

namespace {

QStringList trackColumnValues(
        const Track& track) {
    mixxx::TrackMetadata trackMetadata;
    track.readTrackMetadata(&trackMetadata);
    const QString trackNumberAndTotal = TrackNumbers::joinAsString(
            trackMetadata.getTrackInfo().getTrackNumber(),
            trackMetadata.getTrackInfo().getTrackTotal());
    QStringList columnValues;
    columnValues.reserve(6);
    columnValues
            << trackMetadata.getTrackInfo().getYear()
            << trackMetadata.getAlbumInfo().getTitle()
            << trackMetadata.getAlbumInfo().getArtist()
            << trackNumberAndTotal
            << trackMetadata.getTrackInfo().getTitle()
            << trackMetadata.getTrackInfo().getArtist();
    return columnValues;
}

QStringList trackReleaseColumnValues(
        const mixxx::musicbrainz::TrackRelease& trackRelease) {
    const QString trackNumberAndTotal = TrackNumbers::joinAsString(
            trackRelease.trackNumber,
            trackRelease.trackTotal);
    QStringList columnValues;
    columnValues.reserve(6);
    columnValues
            << trackRelease.date
            << trackRelease.albumTitle
            << trackRelease.albumArtist
            << trackNumberAndTotal
            << trackRelease.title
            << trackRelease.artist;
    return columnValues;
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

DlgTagFetcher::DlgTagFetcher(
        const TrackModel* pTrackModel)
        // No parent because otherwise it inherits the style parent's
        // style which can make it unreadable. Bug #673411
        : QDialog(nullptr),
          m_pTrackModel(pTrackModel),
          m_tagFetcher(this),
          m_networkResult(NetworkResult::Ok) {
    init();
}

void DlgTagFetcher::init() {
    setupUi(this);

    if (m_pTrackModel) {
        connect(btnPrev, &QPushButton::clicked, this, &DlgTagFetcher::slotPrev);
        connect(btnNext, &QPushButton::clicked, this, &DlgTagFetcher::slotNext);
    } else {
        btnNext->hide();
        btnPrev->hide();
    }
    connect(btnApply, &QPushButton::clicked, this, &DlgTagFetcher::apply);
    connect(btnQuit, &QPushButton::clicked, this, &DlgTagFetcher::quit);
    connect(results, &QTreeWidget::currentItemChanged, this, &DlgTagFetcher::resultSelected);

    connect(&m_tagFetcher, &TagFetcher::resultAvailable, this, &DlgTagFetcher::fetchTagFinished);
    connect(&m_tagFetcher, &TagFetcher::fetchProgress, this, &DlgTagFetcher::fetchTagProgress);
    connect(&m_tagFetcher, &TagFetcher::networkError, this, &DlgTagFetcher::slotNetworkResult);

    // Resize columns, this can't be set in the ui file
    results->setColumnWidth(0, 50);  // Year column
    results->setColumnWidth(1, 160); // Album column
    results->setColumnWidth(2, 160); // Album artist column
    results->setColumnWidth(3, 50);  // Track (numbers) column
    results->setColumnWidth(4, 160); // Title column
    results->setColumnWidth(5, 160); // Artist column
}

void DlgTagFetcher::slotNext() {
    QModelIndex nextRow = m_currentTrackIndex.sibling(
            m_currentTrackIndex.row() + 1, m_currentTrackIndex.column());
    if (nextRow.isValid()) {
        loadTrack(nextRow);
        emit next();
    }
}

void DlgTagFetcher::slotPrev() {
    QModelIndex prevRow = m_currentTrackIndex.sibling(
            m_currentTrackIndex.row() - 1, m_currentTrackIndex.column());
    if (prevRow.isValid()) {
        loadTrack(prevRow);
        emit previous();
    }
}

void DlgTagFetcher::loadTrackInternal(const TrackPointer& track) {
    if (!track) {
        return;
    }
    results->clear();
    disconnect(m_track.get(),
            &Track::changed,
            this,
            &DlgTagFetcher::slotTrackChanged);

    m_track = track;
    m_data = Data();
    m_networkResult = NetworkResult::Ok;

    connect(m_track.get(),
            &Track::changed,
            this,
            &DlgTagFetcher::slotTrackChanged);

    m_tagFetcher.startFetch(m_track);

    updateStack();
}

void DlgTagFetcher::loadTrack(const TrackPointer& track) {
    VERIFY_OR_DEBUG_ASSERT(!m_pTrackModel) {
        return;
    }
    loadTrackInternal(track);
}

void DlgTagFetcher::loadTrack(const QModelIndex& index) {
    VERIFY_OR_DEBUG_ASSERT(m_pTrackModel) {
        return;
    }
    TrackPointer pTrack = m_pTrackModel->getTrack(index);
    m_currentTrackIndex = index;
    loadTrackInternal(pTrack);
}

void DlgTagFetcher::slotTrackChanged(TrackId trackId) {
    if (m_track && m_track->getId() == trackId) {
        updateStack();
    }
}

void DlgTagFetcher::apply() {
    int resultIndex = m_data.m_selectedResult;
    if (resultIndex < 0) {
        return;
    }
    DEBUG_ASSERT(resultIndex < m_data.m_results.size());
    const mixxx::musicbrainz::TrackRelease& trackRelease =
            m_data.m_results[resultIndex];
    mixxx::TrackMetadata trackMetadata;
    m_track->readTrackMetadata(&trackMetadata);
    if (!trackRelease.artist.isEmpty()) {
        trackMetadata.refTrackInfo().setArtist(
                trackRelease.artist);
    }
    if (!trackRelease.title.isEmpty()) {
        trackMetadata.refTrackInfo().setTitle(
                trackRelease.title);
    }
    if (!trackRelease.trackNumber.isEmpty()) {
        trackMetadata.refTrackInfo().setTrackNumber(
                trackRelease.trackNumber);
    }
    if (!trackRelease.trackTotal.isEmpty()) {
        trackMetadata.refTrackInfo().setTrackTotal(
                trackRelease.trackTotal);
    }
    if (!trackRelease.date.isEmpty()) {
        trackMetadata.refTrackInfo().setYear(
                trackRelease.date);
    }
    if (!trackRelease.albumArtist.isEmpty()) {
        trackMetadata.refAlbumInfo().setArtist(
                trackRelease.albumArtist);
    }
    if (!trackRelease.albumTitle.isEmpty()) {
        trackMetadata.refAlbumInfo().setTitle(
                trackRelease.albumTitle);
    }
#if defined(__EXTRA_METADATA__)
    if (!trackRelease.artistId.isNull()) {
        trackMetadata.refTrackInfo().setMusicBrainzArtistId(
                trackRelease.artistId);
    }
    if (!trackRelease.recordingId.isNull()) {
        trackMetadata.refTrackInfo().setMusicBrainzRecordingId(
                trackRelease.recordingId);
    }
    if (!trackRelease.trackReleaseId.isNull()) {
        trackMetadata.refTrackInfo().setMusicBrainzReleaseId(
                trackRelease.trackReleaseId);
    }
    if (!trackRelease.albumArtistId.isNull()) {
        trackMetadata.refAlbumInfo().setMusicBrainzArtistId(
                trackRelease.albumArtistId);
    }
    if (!trackRelease.albumReleaseId.isNull()) {
        trackMetadata.refAlbumInfo().setMusicBrainzReleaseId(
                trackRelease.albumReleaseId);
    }
    if (!trackRelease.releaseGroupId.isNull()) {
        trackMetadata.refAlbumInfo().setMusicBrainzReleaseGroupId(
                trackRelease.releaseGroupId);
    }
#endif // __EXTRA_METADATA__
    m_track->importMetadata(
            std::move(trackMetadata),
            // Prevent re-import of outdated metadata from file tags
            // by explicitly setting the synchronization time stamp
            // to the current time.
            QDateTime::currentDateTimeUtc());
}

void DlgTagFetcher::quit() {
    m_tagFetcher.cancel();
    accept();
}

void DlgTagFetcher::fetchTagProgress(QString text) {
    QString status = tr("Status: %1");
    loadingStatus->setText(status.arg(text));
}

void DlgTagFetcher::fetchTagFinished(
        TrackPointer pTrack,
        QList<mixxx::musicbrainz::TrackRelease> guessedTrackReleases) {
    VERIFY_OR_DEBUG_ASSERT(pTrack == m_track) {
        return;
    }
    m_data.m_pending = false;
    m_data.m_results = guessedTrackReleases;
    // qDebug() << "number of results = " << guessedTrackReleases.size();
    updateStack();
}

void DlgTagFetcher::slotNetworkResult(int httpError, QString app, QString message, int code) {
    m_networkResult = httpError == 0 ? NetworkResult::UnknownError : NetworkResult::HttpError;
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

    VERIFY_OR_DEBUG_ASSERT(m_track) {
        return;
    }

    addDivider(tr("Original tags"), results);
    addTrack(trackColumnValues(*m_track), -1, results);

    addDivider(tr("Suggested tags"), results);
    {
        int trackIndex = 0;
        QSet<QStringList> allColumnValues; // deduplication
        for (const auto& trackRelease : qAsConst(m_data.m_results)) {
            const auto columnValues = trackReleaseColumnValues(trackRelease);
            // Ignore duplicate results
            if (!allColumnValues.contains(columnValues)) {
                allColumnValues.insert(columnValues);
                addTrack(columnValues, trackIndex, results);
            }
            ++trackIndex;
        }
    }

    // Find the item that was selected last time
    for (int i = 0; i < results->model()->rowCount(); ++i) {
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
