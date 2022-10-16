#include "library/dlgtagfetcher.h"

#include <QTreeWidget>
#include <QtDebug>

#include "defs_urls.h"
#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "library/library_prefs.h"
#include "moc_dlgtagfetcher.cpp"
#include "preferences/dialog/dlgpreflibrary.h"
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

// There are 2 following steps for cover art fetching.
// 1. -> "Cover Art Links Task"
// 2. -> "Cover Art Image Task"
// When a tag is selected, first step starts.
// First step looks for available cover art links for selected tag.
// Some tags might not have any available cover arts.
// If no links found, user is informed and cover art fetching is finished.
// If links are found, second step starts instantly.
// Second step gets actual cover art with according to user's chosen size.

constexpr int kPercentForCoverArtLinksTask = 35;

constexpr int kPercentForCoverArtImageTask = 70;

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
        QTreeWidget* pParent) {
    QTreeWidgetItem* item = new QTreeWidgetItem(pParent, trackRow);
    item->setData(0, Qt::UserRole, tagIndex);
    item->setData(0, Qt::TextAlignmentRole, Qt::AlignLeft);
}

void updateOriginalTag(const Track& track, QTreeWidget* pParent) {
    const mixxx::TrackMetadata trackMetadata = track.getMetadata();
    const QString trackNumberAndTotal = TrackNumbers::joinAsString(
            trackMetadata.getTrackInfo().getTrackNumber(),
            trackMetadata.getTrackInfo().getTrackTotal());

    pParent->topLevelItem(1)->setText(0, trackMetadata.getTrackInfo().getTitle());
    pParent->topLevelItem(1)->setText(1, trackMetadata.getTrackInfo().getArtist());
    pParent->topLevelItem(1)->setText(2, trackMetadata.getAlbumInfo().getTitle());
    pParent->topLevelItem(1)->setText(3, trackMetadata.getTrackInfo().getYear());
    pParent->topLevelItem(1)->setText(4, trackNumberAndTotal);
    pParent->topLevelItem(1)->setText(5, trackMetadata.getAlbumInfo().getArtist());
}

} // anonymous namespace

DlgTagFetcher::DlgTagFetcher(UserSettingsPointer pConfig, const TrackModel* pTrackModel)
        // No parent because otherwise it inherits the style parent's
        // style which can make it unreadable. Bug #673411
        : QDialog(nullptr),
          m_pConfig(pConfig),
          m_pTrackModel(pTrackModel),
          m_tagFetcher(this),
          m_pWCurrentCoverArtLabel(make_parented<WCoverArtLabel>(this)),
          m_pWFetchedCoverArtLabel(make_parented<WCoverArtLabel>(this)) {
    init();
}

void DlgTagFetcher::init() {
    setupUi(this);
    setWindowIcon(QIcon(MIXXX_ICON_PATH));

    currentCoverArtLayout->setAlignment(Qt::AlignRight | Qt::AlignTop | Qt::AlignCenter);
    currentCoverArtLayout->setSpacing(0);
    currentCoverArtLayout->setContentsMargins(0, 0, 0, 0);
    currentCoverArtLayout->insertWidget(0, m_pWCurrentCoverArtLabel.get());

    fetchedCoverArtLayout->setAlignment(Qt::AlignRight | Qt::AlignBottom | Qt::AlignCenter);
    fetchedCoverArtLayout->setSpacing(0);
    fetchedCoverArtLayout->setContentsMargins(0, 0, 0, 0);
    fetchedCoverArtLayout->insertWidget(0, m_pWFetchedCoverArtLabel.get());

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

    btnRetry->setDisabled(true);

    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &DlgTagFetcher::slotCoverFound);
    }

    connect(&m_tagFetcher,
            &TagFetcher::coverArtArchiveLinksAvailable,
            this,
            &DlgTagFetcher::slotStartFetchCoverArt);

    connect(&m_tagFetcher,
            &TagFetcher::coverArtImageFetchAvailable,
            this,
            &DlgTagFetcher::slotLoadBytesToLabel);

    connect(&m_tagFetcher,
            &TagFetcher::coverArtLinkNotFound,
            this,
            &DlgTagFetcher::slotCoverArtLinkNotFound);
}

void DlgTagFetcher::slotNext() {
    if (isSignalConnected(QMetaMethod::fromSignal(&DlgTagFetcher::next))) {
        emit next();
    } else {
        QModelIndex nextRow = m_currentTrackIndex.sibling(
                m_currentTrackIndex.row() + 1, m_currentTrackIndex.column());
        if (nextRow.isValid()) {
            loadTrack(nextRow);
        }
    }
}

void DlgTagFetcher::slotPrev() {
    if (isSignalConnected(QMetaMethod::fromSignal(&DlgTagFetcher::previous))) {
        emit previous();
    } else {
        QModelIndex prevRow = m_currentTrackIndex.sibling(
                m_currentTrackIndex.row() - 1, m_currentTrackIndex.column());
        if (prevRow.isValid()) {
            loadTrack(prevRow);
        }
    }
}

void DlgTagFetcher::loadTrack(const TrackPointer& pTrack) {
    if (m_track) {
        tags->clear();
        disconnect(m_track.get(),
                &Track::changed,
                this,
                &DlgTagFetcher::slotTrackChanged);
        m_data = Data();
    }
    tags->clear();

    CoverInfo coverInfo;
    QPixmap pixmap;
    m_pWFetchedCoverArtLabel->setCoverArt(coverInfo, pixmap);

    disconnect(m_track.get(),
            &Track::changed,
            this,
            &DlgTagFetcher::slotTrackChanged);

    m_track = pTrack;
    if (!m_track) {
        return;
    }

    btnRetry->setDisabled(true);
    btnApply->setDisabled(true);
    statusMessage->setVisible(false);
    loadingProgressBar->setVisible(true);
    loadingProgressBar->setValue(kMinimumValueOfQProgressBar);
    addDivider(tr("Original tags"), tags);
    addTrack(trackColumnValues(*m_track), kOriginalTrackIndex, tags);

    connect(m_track.get(),
            &Track::changed,
            this,
            &DlgTagFetcher::slotTrackChanged);

    loadCurrentTrackCover();
    m_tagFetcher.startFetch(m_track);
}

void DlgTagFetcher::loadTrack(const QModelIndex& index) {
    m_currentTrackIndex = index;
    TrackPointer pTrack = m_pTrackModel->getTrack(index);
    loadTrack(pTrack);
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

    if (!m_fetchedCoverArtByteArrays.isNull()) {
        // Worker can be called here.
        QString coverArtLocation = m_track->getLocation() + ".jpg";
        QFile coverArtFile(coverArtLocation);
        coverArtFile.open(QIODevice::WriteOnly);
        coverArtFile.write(m_fetchedCoverArtByteArrays);
        coverArtFile.close();

        auto selectedCover = mixxx::FileAccess(mixxx::FileInfo(coverArtLocation));
        QImage updatedCoverArtFound(coverArtLocation);
        if (!updatedCoverArtFound.isNull()) {
            CoverInfoRelative coverInfo;
            coverInfo.type = CoverInfo::FILE;
            coverInfo.source = CoverInfo::USER_SELECTED;
            coverInfo.coverLocation = coverArtLocation;
            coverInfo.setImage(updatedCoverArtFound);
            m_pWCurrentCoverArtLabel->setCoverArt(
                    CoverInfo{}, QPixmap::fromImage(updatedCoverArtFound));
            m_track->setCoverInfo(coverInfo);
            m_fetchedCoverArtByteArrays.clear();
            m_pWFetchedCoverArtLabel->loadData(m_fetchedCoverArtByteArrays);
            m_pWFetchedCoverArtLabel->setCoverArt(CoverInfo{},
                    QPixmap(CoverArtUtils::defaultCoverLocation()));
            QString coverArtAppliedMessage = tr("Cover art applied.");
            statusMessage->setText(coverArtAppliedMessage);
        }
    }

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

void DlgTagFetcher::loadCurrentTrackCover() {
    m_pWCurrentCoverArtLabel->loadTrack(m_track);
    CoverArtCache* pCache = CoverArtCache::instance();
    pCache->requestTrackCover(this, m_track);
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
        loadingProgressBar->setVisible(false);
        statusMessage->setVisible(true);

        VERIFY_OR_DEBUG_ASSERT(m_track) {
            return;
        }

        addDivider(tr("Suggested tags"), tags);
        {
            int trackIndex = 0;
            QSet<QStringList> allColumnValues; // deduplication
            for (const auto& trackRelease : qAsConst(m_data.m_tags)) {
                const auto columnValues = trackReleaseColumnValues(trackRelease);
                // Add fetched tag into TreeItemWidget, if it is not added before
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

    QString fetchTagFinishedMessage = tr("The results are ready to be applied");
    statusMessage->setText(fetchTagFinishedMessage);

    // qDebug() << "number of tags = " << guessedTrackReleases.size();
}

void DlgTagFetcher::slotNetworkResult(
        int httpError,
        const QString& app,
        const QString& message,
        int code) {
    QString cantConnect = tr("Can't connect to %1: %2");
    loadingProgressBar->setFormat(cantConnect.arg(app, message));
    qWarning() << "Error while fetching track metadata!"
               << "Service:" << app
               << "HTTP Status:" << httpError
               << "Code:" << code
               << "Server message:" << message;
    btnRetry->setEnabled(true);
    loadingProgressBar->setValue(kMaximumValueOfQProgressBar);
    return;
}

void DlgTagFetcher::addDivider(const QString& text, QTreeWidget* pParent) const {
    QTreeWidgetItem* item = new QTreeWidgetItem(pParent);
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

    if (!m_fetchedCoverArtByteArrays.isNull()) {
        m_fetchedCoverArtByteArrays.clear();
        m_pWFetchedCoverArtLabel->loadData(m_fetchedCoverArtByteArrays);
        m_pWFetchedCoverArtLabel->setCoverArt(CoverInfo{},
                QPixmap(CoverArtUtils::defaultCoverLocation()));
    }

    const mixxx::musicbrainz::TrackRelease& trackRelease = m_data.m_tags[tagIndex];
    QUuid selectedTagAlbumId = trackRelease.albumReleaseId;
    statusMessage->setVisible(false);

    QString coverArtMessage = tr("Looking for cover art");
    loadingProgressBar->setVisible(true);
    loadingProgressBar->setFormat(coverArtMessage);
    loadingProgressBar->setValue(kPercentForCoverArtLinksTask);

    m_tagFetcher.startFetchCoverArtLinks(selectedTagAlbumId);
}

void DlgTagFetcher::slotCoverFound(
        const QObject* pRequestor,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap,
        mixxx::cache_key_t requestedCacheKey,
        bool coverInfoUpdated) {
    Q_UNUSED(requestedCacheKey);
    Q_UNUSED(coverInfoUpdated);
    if (pRequestor == this &&
            m_track &&
            m_track->getLocation() == coverInfo.trackLocation) {
        m_trackRecord.setCoverInfo(coverInfo);
        m_pWCurrentCoverArtLabel->setCoverArt(coverInfo, pixmap);
    }
}

void DlgTagFetcher::slotUpdateStatusMessage(const QString& message) {
    loadingProgressBar->setVisible(false);
    statusMessage->setVisible(true);
    statusMessage->setText(message);
}

void DlgTagFetcher::slotStartFetchCoverArt(const QList<QString>& allUrls) {
    int fetchedCoverArtQualityConfigValue =
            m_pConfig->getValue(mixxx::library::prefs::kCoverArtFetcherQualityConfigKey,
                    static_cast<int>(DlgPrefLibrary::CoverArtFetcherQuality::Low));

    DlgPrefLibrary::CoverArtFetcherQuality fetcherQuality =
            static_cast<DlgPrefLibrary::CoverArtFetcherQuality>(
                    fetchedCoverArtQualityConfigValue);

    // Cover art links task can retrieve us variable number of links with different cover art sizes
    // Every single successful response has 2 links.
    // These links are cover arts with 250 PX and 500 PX
    // Some of the tags might have link of a cover art with 1200 PX
    // Some of the tags might have link of a cover art more than 1200 PX
    // At final, we always retrieve 2 links, but for some tags this can be 3 or 4
    // We need to pick the correct size according to user choice

    // User choices and the possible fetched cover art sizes are:
    // Highest -> More than 1200 PX, if not available 1200 PX or 500 PX
    // High    -> 1200 PX if not available 500 PX
    // Medium  -> Always 500 PX fetched
    // Low     -> Always 250 PX fetched

    if (fetcherQuality == DlgPrefLibrary::CoverArtFetcherQuality::Highest) {
        getCoverArt(allUrls.last());
        return;
    } else if (fetcherQuality == DlgPrefLibrary::CoverArtFetcherQuality::High) {
        allUrls.size() < 3 ? getCoverArt(allUrls.last()) : getCoverArt(allUrls.at(2));
        return;
    } else if (fetcherQuality == DlgPrefLibrary::CoverArtFetcherQuality::Medium) {
        getCoverArt(allUrls.at(1));
        return;
    } else {
        getCoverArt(allUrls.first());
    }
}

void DlgTagFetcher::slotLoadBytesToLabel(const QByteArray& data) {
    QPixmap fetchedCoverArtPixmap;
    fetchedCoverArtPixmap.loadFromData(data);
    CoverInfo coverInfo;
    coverInfo.type = CoverInfo::NONE;
    coverInfo.source = CoverInfo::USER_SELECTED;
    coverInfo.setImage();

    loadingProgressBar->setVisible(false);
    QString coverArtMessage = tr("Cover art is ready to be applied");
    statusMessage->setVisible(true);
    statusMessage->setText(coverArtMessage);

    m_fetchedCoverArtByteArrays = data;
    m_pWFetchedCoverArtLabel->loadData(
            m_fetchedCoverArtByteArrays); //This data loaded because for full size.
    m_pWFetchedCoverArtLabel->setCoverArt(coverInfo, fetchedCoverArtPixmap);
}

void DlgTagFetcher::getCoverArt(const QString& url) {
    QString coverArtMessage = tr("Cover art found getting image");
    loadingProgressBar->setFormat(coverArtMessage);
    loadingProgressBar->setValue(kPercentForCoverArtImageTask);
    m_tagFetcher.startFetchCoverArtImage(url);
}

void DlgTagFetcher::slotCoverArtLinkNotFound() {
    loadingProgressBar->setVisible(false);
    statusMessage->setVisible(true);
    QString message = tr("Cover Art is not available for selected tag");
    statusMessage->setText(message);
}
