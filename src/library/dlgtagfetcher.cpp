#include "library/dlgtagfetcher.h"

#include <QTreeWidget>
#include <QtDebug>

#include "defs_urls.h"
#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "library/library_prefs.h"
#include "library/trackmodel.h"
#include "moc_dlgtagfetcher.cpp"
#include "preferences/dialog/dlgpreflibrary.h"
#include "track/track.h"
#include "track/tracknumbers.h"
#include "util/imagefiledata.h"

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

void addTagRow(
        const QStringList& trackRow,
        int tagIndex,
        QTreeWidget* pParent) {
    QTreeWidgetItem* pItem = new QTreeWidgetItem(pParent, trackRow);
    pItem->setData(0, Qt::UserRole, tagIndex);
    pItem->setData(0, Qt::TextAlignmentRole, Qt::AlignLeft);
    if (tagIndex == kOriginalTrackIndex) {
        // Disable the original tag row so it can't be selected.
        // Only setDisabled() prevents currentItemChanged() signal, removing the
        // Qt::ItemIsSelectable is not sufficient.
        // Store the normal text brush
        const auto brush = pParent->palette().windowText();
        pItem->setDisabled(true);
        // Restore the normal text color to ensure the tags are readable
        for (int col = 0; col < pItem->columnCount(); col++) {
            pItem->setForeground(col, brush);
        }
    }
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
          m_isCoverArtCopyWorkerRunning(false),
          m_pWCurrentCoverArtLabel(make_parented<WCoverArtLabel>(this)),
          m_pWFetchedCoverArtLabel(make_parented<WCoverArtLabel>(this)) {
    init();
}

void DlgTagFetcher::init() {
    setupUi(this);
    setWindowIcon(QIcon(MIXXX_ICON_PATH));

    currentCoverArtLayout->insertWidget(0, m_pWCurrentCoverArtLabel.get());
    fetchedCoverArtLayout->insertWidget(0, m_pWFetchedCoverArtLabel.get());

    if (m_pTrackModel) {
        connect(btnPrev, &QPushButton::clicked, this, &DlgTagFetcher::slotPrev);
        connect(btnNext, &QPushButton::clicked, this, &DlgTagFetcher::slotNext);
    } else {
        btnNext->hide();
        btnPrev->hide();
    }

    connect(btnApply, &QPushButton::clicked, this, &DlgTagFetcher::applyTagsAndCover);
    connect(btnApplyCover, &QPushButton::clicked, this, &DlgTagFetcher::applyCover);
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
    btnApplyCover->setDisabled(true);

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
    if (m_pTrack) {
        tags->clear();
        disconnect(m_pTrack.get(),
                &Track::changed,
                this,
                &DlgTagFetcher::slotTrackChanged);
        m_data = Data();
    }
    tags->clear();

    m_pWFetchedCoverArtLabel->setCoverArt(CoverInfo{}, QPixmap{});

    m_pTrack = pTrack;
    if (!m_pTrack) {
        return;
    }

    btnRetry->setDisabled(true);
    btnApply->setDisabled(true);
    btnApplyCover->setDisabled(true);
    statusMessage->setVisible(false);
    loadingProgressBar->setVisible(true);
    loadingProgressBar->setValue(kMinimumValueOfQProgressBar);
    addDivider(tr("Original tags"), tags);
    addTagRow(trackColumnValues(*m_pTrack), kOriginalTrackIndex, tags);

    connect(m_pTrack.get(),
            &Track::changed,
            this,
            &DlgTagFetcher::slotTrackChanged);

    loadCurrentTrackCover();
    m_tagFetcher.startFetch(m_pTrack);
}

void DlgTagFetcher::loadTrack(const QModelIndex& index) {
    m_currentTrackIndex = index;
    TrackPointer pTrack = m_pTrackModel->getTrack(index);
    loadTrack(pTrack);
}

void DlgTagFetcher::slotTrackChanged(TrackId trackId) {
    if (m_pTrack && m_pTrack->getId() == trackId) {
        updateOriginalTag(*m_pTrack, tags);
    }
}

void DlgTagFetcher::applyTagsAndCover() {
    int tagIndex = m_data.m_selectedTag;
    if (tagIndex < 0) {
        return;
    }
    DEBUG_ASSERT(tagIndex < m_data.m_tags.size());
    const mixxx::musicbrainz::TrackRelease& trackRelease =
            m_data.m_tags[tagIndex];
    mixxx::TrackMetadata trackMetadata = m_pTrack->getMetadata();
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

    applyCover();

    m_pTrack->replaceMetadataFromSource(
            std::move(trackMetadata),
            // Prevent re-import of outdated metadata from file tags
            // by explicitly setting the synchronization time stamp
            // to the current time.
            QDateTime::currentDateTimeUtc());

    statusMessage->setText(tr("Metadata & Cover Art applied"));
}

void DlgTagFetcher::applyCover() {
    if (!m_fetchedCoverArtByteArrays.isNull()) {
        VERIFY_OR_DEBUG_ASSERT(m_isCoverArtCopyWorkerRunning == false) {
            return;
        };

        QFileInfo trackFileInfo = QFileInfo(m_pTrack->getLocation());

        // Compose the Cover Art file path. Use the correct file extension
        // by checking from the fetched image bytes (disregard extension
        // from file link).
        QString coverArtCopyFilePath =
                trackFileInfo.absoluteFilePath().left(
                        trackFileInfo.absoluteFilePath().lastIndexOf('.') + 1) +
                ImageFileData::readFormatFrom(m_fetchedCoverArtByteArrays);

        m_pWorker.reset(new CoverArtCopyWorker(
                QString(), coverArtCopyFilePath, m_fetchedCoverArtByteArrays));

        connect(m_pWorker.data(),
                &CoverArtCopyWorker::started,
                this,
                &DlgTagFetcher::slotWorkerStarted);

        connect(m_pWorker.data(),
                &CoverArtCopyWorker::askOverwrite,
                this,
                &DlgTagFetcher::slotWorkerAskOverwrite);

        connect(m_pWorker.data(),
                &CoverArtCopyWorker::coverArtCopyFailed,
                this,
                &DlgTagFetcher::slotWorkerCoverArtCopyFailed);

        connect(m_pWorker.data(),
                &CoverArtCopyWorker::coverArtUpdated,
                this,
                &DlgTagFetcher::slotWorkerCoverArtUpdated);

        connect(m_pWorker.data(),
                &CoverArtCopyWorker::finished,
                this,
                &DlgTagFetcher::slotWorkerFinished);

        m_pWorker->start();
    }
}

void DlgTagFetcher::retry() {
    btnRetry->setDisabled(true);
    btnApply->setDisabled(true);
    btnApplyCover->setDisabled(true);
    loadingProgressBar->setValue(kMinimumValueOfQProgressBar);
    m_tagFetcher.startFetch(m_pTrack);
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
    m_pWCurrentCoverArtLabel->loadTrack(m_pTrack);
    CoverArtCache* pCache = CoverArtCache::instance();
    pCache->requestTrackCover(this, m_pTrack);
}

void DlgTagFetcher::showProgressOfRecordingTask() {
    loadingProgressBar->setFormat(tr("Fetching track data from the MusicBrainz database"));
    loadingProgressBar->setValue(loadingProgressBar->value() + m_percentForOneRecording);
}

// TODO(fatihemreyildiz): display the task tags one by one.
void DlgTagFetcher::setPercentOfEachRecordings(int totalRecordingsFound) {
    m_percentForOneRecording = kPercentLeftForRecordingsFound / totalRecordingsFound;
}

void DlgTagFetcher::fetchTagFinished(
        TrackPointer pTrack,
        const QList<mixxx::musicbrainz::TrackRelease>& guessedTrackReleases) {
    VERIFY_OR_DEBUG_ASSERT(pTrack == m_pTrack) {
        return;
    }
    m_data.m_tags = guessedTrackReleases;
    if (guessedTrackReleases.size() == 0) {
        loadingProgressBar->setValue(kMaximumValueOfQProgressBar);
        QString emptyMessage = tr("Could not find this track in the MusicBrainz database.");
        loadingProgressBar->setFormat(emptyMessage);
        return;
    } else {
        btnApply->setDisabled(true);
        btnApplyCover->setDisabled(true);
        btnRetry->setDisabled(true);
        loadingProgressBar->setVisible(false);
        statusMessage->setVisible(true);

        VERIFY_OR_DEBUG_ASSERT(m_pTrack) {
            return;
        }

        addDivider(tr("Suggested tags"), tags);
        {
            int trackIndex = 0;
            QSet<QStringList> allColumnValues; // deduplication
            for (const auto& trackRelease : std::as_const(m_data.m_tags)) {
                const auto columnValues = trackReleaseColumnValues(trackRelease);
                // Add fetched tag into TreeItemWidget, if it is not added before
                if (!allColumnValues.contains(columnValues)) {
                    allColumnValues.insert(columnValues);
                    addTagRow(columnValues, trackIndex, tags);
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

    statusMessage->setText(tr("The results are ready to be applied"));

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
    QTreeWidgetItem* pItem = new QTreeWidgetItem(pParent);
    pItem->setFirstColumnSpanned(true);
    pItem->setText(0, text);
    pItem->setFlags(Qt::NoItemFlags);
    pItem->setForeground(0, palette().color(QPalette::Disabled, QPalette::Text));

    QFont bold_font(font());
    bold_font.setBold(true);
    pItem->setFont(0, bold_font);
}

void DlgTagFetcher::tagSelected() {
    if (!tags->currentItem()) {
        btnApply->setDisabled(true);
        return;
    }

    if (tags->currentItem()->data(0, Qt::UserRole).toInt() == kOriginalTrackIndex) {
        tags->currentItem()->setFlags(Qt::ItemIsEnabled);
        btnApply->setDisabled(true);
        return;
    }
    // Allow applying the tags, regardless the cover art
    btnApply->setEnabled(true);

    const int tagIndex = tags->currentItem()->data(0, Qt::UserRole).toInt();
    m_data.m_selectedTag = tagIndex;

    m_fetchedCoverArtByteArrays.clear();
    m_pWFetchedCoverArtLabel->loadData(QByteArray());
    m_pWFetchedCoverArtLabel->setCoverArt(CoverInfo{},
            QPixmap(CoverArtUtils::defaultCoverLocation()));
    btnApplyCover->setDisabled(true);

    const mixxx::musicbrainz::TrackRelease& trackRelease = m_data.m_tags[tagIndex];
    QUuid selectedTagAlbumId = trackRelease.albumReleaseId;
    statusMessage->setVisible(false);

    loadingProgressBar->setFormat(tr("Looking for cover art"));
    loadingProgressBar->setValue(kPercentForCoverArtLinksTask);
    loadingProgressBar->setVisible(true);

    m_tagFetcher.startFetchCoverArtLinks(selectedTagAlbumId);
}

void DlgTagFetcher::slotCoverFound(
        const QObject* pRequester,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap) {
    if (pRequester == this &&
            m_pTrack &&
            m_pTrack->getLocation() == coverInfo.trackLocation) {
        m_trackRecord.setCoverInfo(coverInfo);
        m_pWCurrentCoverArtLabel->setCoverArt(coverInfo, pixmap);
    }
}

void DlgTagFetcher::slotStartFetchCoverArt(const QList<QString>& allUrls) {
    DlgPrefLibrary::CoverArtFetcherQuality fetcherQuality =
            static_cast<DlgPrefLibrary::CoverArtFetcherQuality>(
                    m_pConfig->getValue(mixxx::library::prefs::kCoverArtFetcherQualityConfigKey,
                            static_cast<int>(DlgPrefLibrary::CoverArtFetcherQuality::Medium)));

    // Cover art links task can retrieve us variable number of links with different cover art sizes
    // Every single successful response has 2 links.
    // These links belongs to cover arts with 250 PX and 500 PX
    // Some of the releases might have link of a cover art with 1200 PX
    // Some of the releases might have link of a cover art which is more than 1200 PX
    // To sum up, 2 links are always retrieved, but for some releases this can be 3 or 4
    // Picking the correct size according to user choice is a must.

    // User choices and the possible fetched cover art sizes are:
    // Highest -> >1200 PX, if not available highest possible
    // High    -> 1200 PX, if not available highest possible
    // Medium  -> 500 PX, at all times
    // Low     -> 250 PX, at all times

    VERIFY_OR_DEBUG_ASSERT(!allUrls.isEmpty()) {
        return;
    }

    if (allUrls.size() > static_cast<int>(fetcherQuality)) {
        getCoverArt(allUrls.at(static_cast<int>(fetcherQuality)));
    } else {
        getCoverArt(allUrls.last());
    }
}

void DlgTagFetcher::slotLoadBytesToLabel(const QByteArray& data) {
    QPixmap fetchedCoverArtPixmap;
    fetchedCoverArtPixmap.loadFromData(data);
    CoverInfo coverInfo;
    coverInfo.source = CoverInfo::USER_SELECTED;

    loadingProgressBar->setVisible(false);
    statusMessage->clear();
    statusMessage->setVisible(true);

    m_fetchedCoverArtByteArrays = data;
    m_pWFetchedCoverArtLabel->loadData(
            m_fetchedCoverArtByteArrays); // This data loaded because for full size.
    m_pWFetchedCoverArtLabel->setCoverArt(coverInfo, fetchedCoverArtPixmap);

    btnApplyCover->setDisabled(data.isNull());
}

void DlgTagFetcher::getCoverArt(const QString& url) {
    loadingProgressBar->setFormat(tr("Cover art found, receiving image."));
    loadingProgressBar->setValue(kPercentForCoverArtImageTask);
    m_tagFetcher.startFetchCoverArtImage(url);
}

void DlgTagFetcher::slotCoverArtLinkNotFound() {
    loadingProgressBar->setVisible(false);
    statusMessage->setText(tr("Cover Art is not available for selected metadata"));
    statusMessage->setVisible(true);
}

void DlgTagFetcher::slotWorkerStarted() {
    m_isCoverArtCopyWorkerRunning = true;
}

void DlgTagFetcher::slotWorkerCoverArtUpdated(const CoverInfoRelative& coverInfo) {
    qDebug() << "DlgTagFetcher::slotWorkerCoverArtUpdated" << coverInfo;
    m_pTrack->setCoverInfo(coverInfo);
    loadCurrentTrackCover();
    statusMessage->setText(tr("Selected cover art applied"));
}

void DlgTagFetcher::slotWorkerAskOverwrite(const QString& coverArtAbsolutePath,
        std::promise<CoverArtCopyWorker::OverwriteAnswer>* pPromise) {
    QFileInfo coverArtInfo(coverArtAbsolutePath);
    QString coverArtName = coverArtInfo.completeBaseName();
    QString coverArtFolder = coverArtInfo.absolutePath();
    QMessageBox overwrite_box(
            QMessageBox::Warning,
            tr("Cover Art File Already Exists"),
            tr("File: %1\n"
               "Folder: %2\n"
               "Override existing file?\n"
               "This can not be undone!")
                    .arg(coverArtName, coverArtFolder));
    overwrite_box.addButton(QMessageBox::Yes);
    overwrite_box.addButton(QMessageBox::No);

    switch (overwrite_box.exec()) {
    case QMessageBox::No:
        pPromise->set_value(CoverArtCopyWorker::OverwriteAnswer::Cancel);
        return;
    case QMessageBox::Yes:
        pPromise->set_value(CoverArtCopyWorker::OverwriteAnswer::Overwrite);
        return;
    }
}

void DlgTagFetcher::slotWorkerCoverArtCopyFailed(const QString& errorMessage) {
    QMessageBox copyFailBox;
    copyFailBox.setText(errorMessage);
    copyFailBox.exec();
}

void DlgTagFetcher::slotWorkerFinished() {
    m_isCoverArtCopyWorkerRunning = false;
}
