#include "library/dlgtagfetcher.h"

#include <QTreeWidget>
#include <QtDebug>

#include "defs_urls.h"
#include "moc_dlgtagfetcher.cpp"
#include "track/track.h"
#include "track/tracknumbers.h"

namespace {

// Constant percentages for QProgressBar.

// There are 3 constant steps while fetching metadata from musicbrainz.
// 1. -> "Fingerprinting track"
// 2. -> "Identifying track through Acoustid"
// 3. -> "Retrieving metadata from MusicBrainz"
// These three steps never change. They are triggered once at all times.
// After each step progress bar value increases by 15%.

constexpr int kPercentOfConstantTask = 15;

// After these steps are passed, last step remains.
// 4. -> "Fetching track data from the Musicbrainz database"
// Due to various recordings fetched from AcoustID, the number of triggers for this step may vary.
// This can cause different scaling.
// In order to have a better scaling, the remaining 55% are divided by the number of recordings found.

constexpr int kPercentLeftForRecordingsFound = 55;

constexpr int kMaximumValueOfQProgressBar = 100;

constexpr int kMinimumValueOfQProgressBar = 0;

// Original Index of the track tag, listed all the time below 'Original Tags'.
constexpr int kOriginalTrackIndex = -1;

QStringList trackColumnValues(
        const Track& track) {
    const mixxx::TrackMetadata trackMetadata = track.getMetadata();
    const QString trackNumberAndTotal = TrackNumbers::joinAsString(
            trackMetadata.getTrackInfo().getTrackNumber(),
            trackMetadata.getTrackInfo().getTrackTotal());
    QStringList columnValues;
    columnValues.reserve(6);
    columnValues
            << trackMetadata.getTrackInfo().getTitle()
            << trackMetadata.getTrackInfo().getArtist()
            << trackMetadata.getAlbumInfo().getTitle()
            << trackMetadata.getTrackInfo().getYear()
            << trackNumberAndTotal
            << trackMetadata.getAlbumInfo().getArtist();
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
            << trackRelease.title
            << trackRelease.artist
            << trackRelease.albumTitle
            << trackRelease.date
            << trackNumberAndTotal
            << trackRelease.albumArtist;
    return columnValues;
}

void addTrack(
        const QStringList& trackRow,
        int tagIndex,
        QTreeWidget* parent) {
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, trackRow);
    item->setData(0, Qt::UserRole, tagIndex);
    item->setData(0, Qt::TextAlignmentRole, Qt::AlignLeft);
}

void updateOriginalTag(const Track& track, QTreeWidget* parent) {
    const mixxx::TrackMetadata trackMetadata = (track).getMetadata();
    const QString trackNumberAndTotal = TrackNumbers::joinAsString(
            trackMetadata.getTrackInfo().getTrackNumber(),
            trackMetadata.getTrackInfo().getTrackTotal());

    parent->topLevelItem(1)->setText(0, trackMetadata.getTrackInfo().getTitle());
    parent->topLevelItem(1)->setText(1, trackMetadata.getTrackInfo().getArtist());
    parent->topLevelItem(1)->setText(2, trackMetadata.getAlbumInfo().getTitle());
    parent->topLevelItem(1)->setText(3, trackMetadata.getTrackInfo().getYear());
    parent->topLevelItem(1)->setText(4, trackNumberAndTotal);
    parent->topLevelItem(1)->setText(5, trackMetadata.getAlbumInfo().getArtist());
}

} // anonymous namespace

DlgTagFetcher::DlgTagFetcher(
        const TrackModel* pTrackModel)
        // No parent because otherwise it inherits the style parent's
        // style which can make it unreadable. Bug #673411
        : QDialog(nullptr),
          m_pTrackModel(pTrackModel),
          m_tagFetcher(this) {
    init();
}

void DlgTagFetcher::init() {
    setupUi(this);
    setWindowIcon(QIcon(MIXXX_ICON_PATH));

    if (m_pTrackModel) {
        connect(btnPrev, &QPushButton::clicked, this, &DlgTagFetcher::slotPrev);
        connect(btnNext, &QPushButton::clicked, this, &DlgTagFetcher::slotNext);
    } else {
        btnNext->hide();
        btnPrev->hide();
    }
    connect(btnApply, &QPushButton::clicked, this, &DlgTagFetcher::apply);
    connect(btnQuit, &QPushButton::clicked, this, &DlgTagFetcher::quit);
    connect(btnRetry, &QPushButton::clicked, this, &DlgTagFetcher::retry);
    connect(tags, &QTreeWidget::currentItemChanged, this, &DlgTagFetcher::tagSelected);

    connect(&m_tagFetcher, &TagFetcher::resultAvailable, this, &DlgTagFetcher::fetchTagFinished);
    connect(&m_tagFetcher,
            &TagFetcher::fetchProgress,
            this,
            &DlgTagFetcher::showProgressOfConstantTask);
    connect(&m_tagFetcher,
            &TagFetcher::currentRecordingFetchedFromMusicBrainz,
            this,
            &DlgTagFetcher::showProgressOfRecordingTask);
    connect(&m_tagFetcher,
            &TagFetcher::numberOfRecordingsFoundFromAcoustId,
            this,
            &DlgTagFetcher::setPercentOfEachRecordings);
    connect(&m_tagFetcher, &TagFetcher::networkError, this, &DlgTagFetcher::slotNetworkResult);

    loadingProgressBar->setMaximum(kMaximumValueOfQProgressBar);
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
    tags->clear();
    disconnect(m_track.get(),
            &Track::changed,
            this,
            &DlgTagFetcher::slotTrackChanged);

    m_track = track;
    m_data = Data();

    btnRetry->setDisabled(true);
    btnApply->setDisabled(true);
    successMessage->setVisible(false);
    loadingProgressBar->setVisible(true);
    loadingProgressBar->setValue(kMinimumValueOfQProgressBar);
    tags->clear();
    addDivider(tr("Original tags"), tags);
    addTrack(trackColumnValues(*m_track), kOriginalTrackIndex, tags);

    connect(m_track.get(),
            &Track::changed,
            this,
            &DlgTagFetcher::slotTrackChanged);

    m_tagFetcher.startFetch(m_track);
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
        updateOriginalTag(*m_track, tags);
    }
}

void DlgTagFetcher::apply() {
    int tagIndex = m_data.m_selectedTag;
    if (tagIndex < 0) {
        return;
    }
    DEBUG_ASSERT(tagIndex < m_data.m_tags.size());
    const mixxx::musicbrainz::TrackRelease& trackRelease =
            m_data.m_tags[tagIndex];
    mixxx::TrackMetadata trackMetadata = m_track->getMetadata();
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
    m_track->replaceMetadataFromSource(
            std::move(trackMetadata),
            // Prevent re-import of outdated metadata from file tags
            // by explicitly setting the synchronization time stamp
            // to the current time.
            QDateTime::currentDateTimeUtc());
}

void DlgTagFetcher::retry() {
    btnRetry->setDisabled(true);
    btnApply->setDisabled(true);
    loadingProgressBar->setValue(kMinimumValueOfQProgressBar);
    m_tagFetcher.startFetch(m_track);
}

void DlgTagFetcher::quit() {
    m_tagFetcher.cancel();
    accept();
}

void DlgTagFetcher::reject() {
    m_tagFetcher.cancel();
    accept();
}

void DlgTagFetcher::showProgressOfConstantTask(const QString& text) {
    QString status = tr("%1");
    loadingProgressBar->setFormat(status.arg(text));
    loadingProgressBar->setValue(loadingProgressBar->value() + kPercentOfConstantTask);
}

void DlgTagFetcher::showProgressOfRecordingTask() {
    QString status = tr("Fetching track data from the MusicBrainz database");
    loadingProgressBar->setFormat(status);
    loadingProgressBar->setValue(loadingProgressBar->value() + m_percentForOneRecording);
}

// TODO(fatihemreyildiz): display the task tags one by one.
void DlgTagFetcher::setPercentOfEachRecordings(int totalRecordingsFound) {
    m_percentForOneRecording = kPercentLeftForRecordingsFound / totalRecordingsFound;
}

void DlgTagFetcher::fetchTagFinished(
        TrackPointer pTrack,
        const QList<mixxx::musicbrainz::TrackRelease>& guessedTrackReleases) {
    VERIFY_OR_DEBUG_ASSERT(pTrack == m_track) {
        return;
    }
    m_data.m_tags = guessedTrackReleases;
    if (guessedTrackReleases.size() == 0) {
        loadingProgressBar->setValue(kMaximumValueOfQProgressBar);
        QString emptyMessage = tr("Could not find this track in the MusicBrainz database.");
        loadingProgressBar->setFormat(emptyMessage);
        return;
    } else {
        btnApply->setEnabled(true);
        btnRetry->setEnabled(false);

        loadingProgressBar->setValue(kMaximumValueOfQProgressBar);
        loadingProgressBar->setVisible(false);
        successMessage->setVisible(true);

        VERIFY_OR_DEBUG_ASSERT(m_track) {
            return;
        }

        addDivider(tr("Suggested tags"), tags);
        {
            int trackIndex = 0;
            QSet<QStringList> allColumnValues; // deduplication
            for (const auto& trackRelease : qAsConst(m_data.m_tags)) {
                const auto columnValues = trackReleaseColumnValues(trackRelease);
                // Ignore duplicate tags
                if (!allColumnValues.contains(columnValues)) {
                    allColumnValues.insert(columnValues);
                    addTrack(columnValues, trackIndex, tags);
                }
                ++trackIndex;
            }
        }

        for (int i = 0; i < tags->model()->columnCount(); i++) {
            tags->resizeColumnToContents(i);
            int sectionSize = (tags->columnWidth(i) + 10);
            tags->header()->resizeSection(i, sectionSize);
        }
    }

    // qDebug() << "number of tags = " << guessedTrackReleases.size();
}

void DlgTagFetcher::slotNetworkResult(
        int httpError,
        const QString& app,
        const QString& message,
        int code) {
    QString cantConnect = tr("Can't connect to %1: %2");
    loadingProgressBar->setFormat(cantConnect.arg(app, message));
    qWarning() << "Error while getting track metadata!"
               << "Service:" << app
               << "HTTP Status:" << httpError
               << "Code:" << code
               << "Server message:" << message;
    btnRetry->setEnabled(true);
    loadingProgressBar->setValue(kMaximumValueOfQProgressBar);
    return;
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

void DlgTagFetcher::tagSelected() {
    if (!tags->currentItem()) {
        return;
    }

    if (tags->currentItem()->data(0, Qt::UserRole).toInt() == kOriginalTrackIndex) {
        tags->currentItem()->setFlags(Qt::ItemIsEnabled);
        return;
    }
    const int tagIndex = tags->currentItem()->data(0, Qt::UserRole).toInt();
    m_data.m_selectedTag = tagIndex;
}
