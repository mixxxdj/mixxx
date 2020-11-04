#include "library/dlgtrackinfo.h"

#include <QDesktopServices>
#include <QStringBuilder>
#include <QtDebug>

#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "library/dlgtagfetcher.h"
#include "library/trackmodel.h"
#include "preferences/colorpalettesettings.h"
#include "sources/soundsourceproxy.h"
#include "track/beatfactory.h"
#include "track/keyfactory.h"
#include "track/keyutils.h"
#include "track/track.h"
#include "util/color/colorpalette.h"
#include "util/compatibility.h"
#include "util/datetime.h"
#include "util/desktophelper.h"
#include "util/duration.h"
#include "widget/wcoverartlabel.h"
#include "widget/wstarrating.h"

namespace {
constexpr int kFilterLength = 80;
constexpr int kMinBpm = 30;
} // namespace

// Maximum allowed interval between beats (calculated from kMinBpm).
const mixxx::Duration kMaxInterval = mixxx::Duration::fromMillis(
        static_cast<qint64>(1000.0 * (60.0 / kMinBpm)));

DlgTrackInfo::DlgTrackInfo(
        const TrackModel* trackModel)
        // No parent because otherwise it inherits the style parent's
        // style which can make it unreadable. Bug #673411
        : QDialog(nullptr),
          m_pTrackModel(trackModel),
          m_tapFilter(this, kFilterLength, kMaxInterval),
          m_dLastTapedBpm(-1.),
          m_pWCoverArtLabel(make_parented<WCoverArtLabel>(this)),
          m_pWStarRating(make_parented<WStarRating>(nullptr, this)) {
    init();
}

DlgTrackInfo::~DlgTrackInfo() {
    unloadTrack(false);
}

void DlgTrackInfo::init() {
    setupUi(this);

    coverLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);
    coverLayout->setSpacing(0);
    coverLayout->setContentsMargins(0, 0, 0, 0);
    coverLayout->insertWidget(0, m_pWCoverArtLabel.get());

    starsLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    starsLayout->setSpacing(0);
    starsLayout->setContentsMargins(0, 0, 0, 0);
    starsLayout->insertWidget(0, m_pWStarRating.get());
    // This is necessary to pass on mouseMove events to WStarRating
    m_pWStarRating->setMouseTracking(true);

    if (m_pTrackModel) {
        connect(btnNext,
                &QPushButton::clicked,
                this,
                &DlgTrackInfo::slotNextButton);
        connect(btnPrev,
                &QPushButton::clicked,
                this,
                &DlgTrackInfo::slotPrevButton);
    } else {
        btnNext->hide();
        btnPrev->hide();
    }

    connect(btnApply,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::slotApply);

    connect(btnOK,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::slotOk);

    connect(btnCancel,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::slotCancel);

    connect(bpmDouble,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::slotBpmDouble);
    connect(bpmHalve,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::slotBpmHalve);
    connect(bpmTwoThirds,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::slotBpmTwoThirds);
    connect(bpmThreeFourth,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::slotBpmThreeFourth);
    connect(bpmFourThirds,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::slotBpmFourThirds);
    connect(bpmThreeHalves,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::slotBpmThreeHalves);
    connect(bpmClear,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::slotBpmClear);

    connect(bpmConst,
            &QCheckBox::stateChanged,
            this,
            &DlgTrackInfo::slotBpmConstChanged);

    connect(spinBpm,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DlgTrackInfo::slotSpinBpmValueChanged);

    connect(txtKey,
            &QLineEdit::editingFinished,
            this,
            &DlgTrackInfo::slotKeyTextChanged);

    connect(bpmTap,
            &QPushButton::pressed,
            &m_tapFilter,
            &TapFilter::tap);
    connect(&m_tapFilter,
            &TapFilter::tapped,
            this,
            &DlgTrackInfo::slotBpmTap);

    connect(btnImportMetadataFromFile,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::slotImportMetadataFromFile);

    connect(btnImportMetadataFromMusicBrainz,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::slotImportMetadataFromMusicBrainz);

    connect(btnOpenFileBrowser,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::slotOpenInFileBrowser);

    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &DlgTrackInfo::slotCoverFound);
    }
    connect(m_pWCoverArtLabel.get(),
            &WCoverArtLabel::coverInfoSelected,
            this,
            &DlgTrackInfo::slotCoverInfoSelected);
    connect(m_pWCoverArtLabel.get(),
            &WCoverArtLabel::reloadCoverArt,
            this,
            &DlgTrackInfo::slotReloadCoverArt);
}

void DlgTrackInfo::slotOk() {
    unloadTrack(true);
    accept();
}

void DlgTrackInfo::slotApply() {
    saveTrack();
}

void DlgTrackInfo::slotCancel() {
    unloadTrack(false);
    reject();
}

void DlgTrackInfo::trackUpdated() {
}

void DlgTrackInfo::slotNextButton() {
    loadNextTrack();
}

void DlgTrackInfo::slotPrevButton() {
    loadPrevTrack();
}

void DlgTrackInfo::slotNextDlgTagFetcher() {
    loadNextTrack();
    // Do not load track back into DlgTagFetcher since
    // it will cause a reload of the same track.
}

void DlgTrackInfo::slotPrevDlgTagFetcher() {
    loadPrevTrack();
}

void DlgTrackInfo::loadNextTrack() {
    auto nextRow = m_currentTrackIndex.sibling(
            m_currentTrackIndex.row() + 1, m_currentTrackIndex.column());
    if (nextRow.isValid()) {
        loadTrack(nextRow);
        emit next();
    }
}

void DlgTrackInfo::loadPrevTrack() {
    QModelIndex prevRow = m_currentTrackIndex.sibling(
            m_currentTrackIndex.row() - 1, m_currentTrackIndex.column());
    if (prevRow.isValid()) {
        loadTrack(prevRow);
        emit previous();
    }
}

void DlgTrackInfo::populateFields(const Track& track) {
    setWindowTitle(track.getInfo());

    // Editable fields
    txtTrackName->setText(track.getTitle());
    txtArtist->setText(track.getArtist());
    txtAlbum->setText(track.getAlbum());
    txtAlbumArtist->setText(track.getAlbumArtist());
    txtGenre->setText(track.getGenre());
    txtComposer->setText(track.getComposer());
    txtGrouping->setText(track.getGrouping());
    txtYear->setText(track.getYear());
    txtTrackNumber->setText(track.getTrackNumber());
    txtComment->setPlainText(track.getComment());

    // Non-editable fields
    txtDuration->setText(track.getDurationText(mixxx::Duration::Precision::SECONDS));
    txtDateAdded->setText(mixxx::displayLocalDateTime(track.getDateAdded()));
    txtLocation->setText(QDir::toNativeSeparators(track.getLocation()));
    txtType->setText(track.getType());
    txtBitrate->setText(QString(track.getBitrateText()) + (" ") +
            tr(mixxx::audio::Bitrate::unit()));
    txtBpm->setText(track.getBpmText());
    m_keysClone = track.getKeys();
    txtKey->setText(KeyUtils::getGlobalKeyText(m_keysClone));
    const mixxx::ReplayGain replayGain(track.getReplayGain());
    txtReplayGain->setText(mixxx::ReplayGain::ratioToString(replayGain.getRatio()));

    reloadTrackBeats(track);

    m_loadedCoverInfo = track.getCoverInfoWithLocation();
    m_pWCoverArtLabel->setCoverArt(m_loadedCoverInfo, QPixmap());
    CoverArtCache::requestCover(this, m_loadedCoverInfo);
    m_pWStarRating->slotTrackLoaded(m_pLoadedTrack);
}

void DlgTrackInfo::reloadTrackBeats(const Track& track) {
    mixxx::BeatsPointer pBeats = track.getBeats();
    if (pBeats) {
        spinBpm->setValue(pBeats->getBpm());
        m_pBeatsClone = pBeats->clone();
    } else {
        m_pBeatsClone.clear();
        spinBpm->setValue(0.0);
    }
    m_trackHasBeatMap = pBeats && !(pBeats->getCapabilities() & mixxx::Beats::BEATSCAP_SETBPM);
    bpmConst->setChecked(!m_trackHasBeatMap);
    bpmConst->setEnabled(m_trackHasBeatMap); // We cannot make turn a BeatGrid to a BeatMap
    spinBpm->setEnabled(!m_trackHasBeatMap); // We cannot change bpm continuously or tab them
    bpmTap->setEnabled(!m_trackHasBeatMap);  // when we have a beatmap

    if (track.isBpmLocked()) {
        tabBPM->setEnabled(false);
    } else {
        tabBPM->setEnabled(true);
    }
}

void DlgTrackInfo::loadTrackInternal(const TrackPointer& pTrack) {
    clear();

    if (!pTrack) {
        return;
    }

    m_pLoadedTrack = pTrack;

    populateFields(*m_pLoadedTrack);
    m_pWCoverArtLabel->loadTrack(m_pLoadedTrack);

    // We already listen to changed() so we don't need to listen to individual
    // signals such as cuesUpdates, coverArtUpdated(), etc.
    connect(pTrack.get(),
            &Track::changed,
            this,
            &DlgTrackInfo::slotTrackChanged);
}

void DlgTrackInfo::loadTrack(TrackPointer pTrack) {
    VERIFY_OR_DEBUG_ASSERT(!m_pTrackModel) {
        return;
    }
    loadTrackInternal(pTrack);
    if (m_pDlgTagFetcher && m_pLoadedTrack) {
        m_pDlgTagFetcher->loadTrack(m_pLoadedTrack);
    }
}

void DlgTrackInfo::loadTrack(QModelIndex index) {
    VERIFY_OR_DEBUG_ASSERT(m_pTrackModel) {
        return;
    }
    TrackPointer pTrack = m_pTrackModel->getTrack(index);
    m_currentTrackIndex = index;
    loadTrackInternal(pTrack);
    if (m_pDlgTagFetcher && m_currentTrackIndex.isValid()) {
        m_pDlgTagFetcher->loadTrack(m_currentTrackIndex);
    }
}

void DlgTrackInfo::slotCoverFound(
        const QObject* pRequestor,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap,
        mixxx::cache_key_t requestedCacheKey,
        bool coverInfoUpdated) {
    Q_UNUSED(requestedCacheKey);
    Q_UNUSED(coverInfoUpdated);
    if (pRequestor == this &&
            m_pLoadedTrack &&
            m_loadedCoverInfo.trackLocation == coverInfo.trackLocation) {
        m_loadedCoverInfo = coverInfo;
        m_pWCoverArtLabel->setCoverArt(coverInfo, pixmap);
    }
}

void DlgTrackInfo::slotReloadCoverArt() {
    VERIFY_OR_DEBUG_ASSERT(m_pLoadedTrack) {
        return;
    }
    slotCoverInfoSelected(
            CoverInfoGuesser().guessCoverInfoForTrack(
                    *m_pLoadedTrack));
}

void DlgTrackInfo::slotCoverInfoSelected(const CoverInfoRelative& coverInfo) {
    qDebug() << "DlgTrackInfo::slotCoverInfoSelected" << coverInfo;
    VERIFY_OR_DEBUG_ASSERT(m_pLoadedTrack) {
        return;
    }
    m_loadedCoverInfo = CoverInfo(coverInfo, m_pLoadedTrack->getLocation());
    CoverArtCache::requestCover(this, m_loadedCoverInfo);
}

void DlgTrackInfo::slotOpenInFileBrowser() {
    if (!m_pLoadedTrack) {
        return;
    }

    mixxx::DesktopHelper::openInFileBrowser(QStringList(m_pLoadedTrack->getLocation()));
}

void DlgTrackInfo::saveTrack() {
    if (!m_pLoadedTrack)
        return;

    // First, disconnect the track changed signal. Otherwise we signal ourselves
    // and repopulate all these fields.
    disconnect(m_pLoadedTrack.get(),
            &Track::changed,
            this,
            &DlgTrackInfo::slotTrackChanged);

    m_pLoadedTrack->setTitle(txtTrackName->text());
    m_pLoadedTrack->setArtist(txtArtist->text());
    m_pLoadedTrack->setAlbum(txtAlbum->text());
    m_pLoadedTrack->setAlbumArtist(txtAlbumArtist->text());
    m_pLoadedTrack->setGenre(txtGenre->text());
    m_pLoadedTrack->setComposer(txtComposer->text());
    m_pLoadedTrack->setGrouping(txtGrouping->text());
    m_pLoadedTrack->setYear(txtYear->text());
    m_pLoadedTrack->setTrackNumber(txtTrackNumber->text());
    m_pLoadedTrack->setComment(txtComment->toPlainText());

    if (!m_pLoadedTrack->isBpmLocked()) {
        m_pLoadedTrack->setBeats(m_pBeatsClone);
        reloadTrackBeats(*m_pLoadedTrack);
    }

    // If the user is editing the key and hits enter to close DlgTrackInfo, the
    // editingFinished signal will not fire in time. Run the key text changed
    // handler now to see if the key was edited. If the key was unchanged or
    // invalid then the change will be rejected.
    slotKeyTextChanged();

    m_pLoadedTrack->setKeys(m_keysClone);
    m_pLoadedTrack->setCoverInfo(m_loadedCoverInfo);

    // Reconnect changed signals now.
    connect(m_pLoadedTrack.get(),
            &Track::changed,
            this,
            &DlgTrackInfo::slotTrackChanged);
}

void DlgTrackInfo::unloadTrack(bool save) {
    if (!m_pLoadedTrack)
        return;

    if (save) {
        saveTrack();
    }

    clear();
}

void DlgTrackInfo::clear() {
    if (m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(),
                &Track::changed,
                this,
                &DlgTrackInfo::slotTrackChanged);
        m_pLoadedTrack.reset();
    }

    txtTrackName->setText("");
    txtArtist->setText("");
    txtAlbum->setText("");
    txtAlbumArtist->setText("");
    txtGenre->setText("");
    txtComposer->setText("");
    txtGrouping->setText("");
    txtYear->setText("");
    txtTrackNumber->setText("");
    txtComment->setPlainText("");
    spinBpm->setValue(0.0);
    m_pBeatsClone.clear();
    m_keysClone = Keys();

    txtDuration->setText("");
    txtType->setText("");
    txtLocation->setText("");
    txtBitrate->setText("");
    txtBpm->setText("");
    txtKey->setText("");
    txtReplayGain->setText("");

    m_loadedCoverInfo = CoverInfo();
    m_pWCoverArtLabel->setCoverArt(m_loadedCoverInfo, QPixmap());
}

void DlgTrackInfo::slotBpmDouble() {
    m_pBeatsClone->scale(mixxx::Beats::DOUBLE);
    // read back the actual value
    double newValue = m_pBeatsClone->getBpm();
    spinBpm->setValue(newValue);
}

void DlgTrackInfo::slotBpmHalve() {
    m_pBeatsClone->scale(mixxx::Beats::HALVE);
    // read back the actual value
    double newValue = m_pBeatsClone->getBpm();
    spinBpm->setValue(newValue);
}

void DlgTrackInfo::slotBpmTwoThirds() {
    m_pBeatsClone->scale(mixxx::Beats::TWOTHIRDS);
    // read back the actual value
    double newValue = m_pBeatsClone->getBpm();
    spinBpm->setValue(newValue);
}

void DlgTrackInfo::slotBpmThreeFourth() {
    m_pBeatsClone->scale(mixxx::Beats::THREEFOURTHS);
    // read back the actual value
    double newValue = m_pBeatsClone->getBpm();
    spinBpm->setValue(newValue);
}

void DlgTrackInfo::slotBpmFourThirds() {
    m_pBeatsClone->scale(mixxx::Beats::FOURTHIRDS);
    // read back the actual value
    double newValue = m_pBeatsClone->getBpm();
    spinBpm->setValue(newValue);
}

void DlgTrackInfo::slotBpmThreeHalves() {
    m_pBeatsClone->scale(mixxx::Beats::THREEHALVES);
    // read back the actual value
    double newValue = m_pBeatsClone->getBpm();
    spinBpm->setValue(newValue);
}

void DlgTrackInfo::slotBpmClear() {
    spinBpm->setValue(0);
    m_pBeatsClone.clear();

    bpmConst->setChecked(true);
    bpmConst->setEnabled(m_trackHasBeatMap);
    spinBpm->setEnabled(true);
    bpmTap->setEnabled(true);
}

void DlgTrackInfo::slotBpmConstChanged(int state) {
    if (state != Qt::Unchecked) {
        // const beatgrid requested
        if (spinBpm->value() > 0) {
            // Since the user is not satisfied with the beat map,
            // it is hard to predict a fitting beat. We know that we
            // cannot use the first beat, since it is out of sync in
            // almost all cases.
            // The cue point should be set on a beat, so this seams
            // to be a good alternative
            CuePosition cue = m_pLoadedTrack->getCuePoint();
            m_pBeatsClone = BeatFactory::makeBeatGrid(
                    *m_pLoadedTrack, spinBpm->value(), cue.getPosition());
        } else {
            m_pBeatsClone.clear();
        }
        spinBpm->setEnabled(true);
        bpmTap->setEnabled(true);
    } else {
        // try to reload BeatMap from the Track
        reloadTrackBeats(*m_pLoadedTrack);
    }
}

void DlgTrackInfo::slotBpmTap(double averageLength, int numSamples) {
    Q_UNUSED(numSamples);
    if (averageLength == 0) {
        return;
    }
    double averageBpm = 60.0 * 1000.0 / averageLength;
    // average bpm needs to be truncated for this comparison:
    if (averageBpm != m_dLastTapedBpm) {
        m_dLastTapedBpm = averageBpm;
        spinBpm->setValue(averageBpm);
    }
}

void DlgTrackInfo::slotSpinBpmValueChanged(double value) {
    if (value <= 0) {
        m_pBeatsClone.clear();
        return;
    }

    if (!m_pBeatsClone) {
        CuePosition cue = m_pLoadedTrack->getCuePoint();
        m_pBeatsClone = BeatFactory::makeBeatGrid(
                *m_pLoadedTrack, value, cue.getPosition());
    }

    double oldValue = m_pBeatsClone->getBpm();
    if (oldValue == value) {
        return;
    }

    if (m_pBeatsClone->getCapabilities() & mixxx::Beats::BEATSCAP_SETBPM) {
        m_pBeatsClone->setBpm(value);
    }

    // read back the actual value
    double newValue = m_pBeatsClone->getBpm();
    spinBpm->setValue(newValue);
}

void DlgTrackInfo::slotKeyTextChanged() {
    // Try to parse the user's input as a key.
    const QString newKeyText = txtKey->text();
    Keys newKeys = KeyFactory::makeBasicKeysFromText(newKeyText,
            mixxx::track::io::key::USER);
    const mixxx::track::io::key::ChromaticKey globalKey(newKeys.getGlobalKey());

    // If the new key string is invalid and not empty them reject the new key.
    if (globalKey == mixxx::track::io::key::INVALID && !newKeyText.isEmpty()) {
        txtKey->setText(KeyUtils::getGlobalKeyText(m_keysClone));
        return;
    }

    // If the new key is the same as the old key, reject the change.
    if (globalKey == m_keysClone.getGlobalKey()) {
        return;
    }

    // Otherwise, accept.
    m_keysClone = newKeys;
}

void DlgTrackInfo::slotImportMetadataFromFile() {
    if (m_pLoadedTrack) {
        // Allocate a temporary track object for reading the metadata.
        // We cannot reuse m_pLoadedTrack, because it might already been
        // modified and we want to read fresh metadata directly from the
        // file. Otherwise the changes in m_pLoadedTrack would be lost.
        TrackPointer pTrack = SoundSourceProxy::importTemporaryTrack(
                m_pLoadedTrack->getFileInfo(),
                m_pLoadedTrack->getSecurityToken());
        DEBUG_ASSERT(pTrack);
        populateFields(*pTrack);
    }
}

void DlgTrackInfo::slotTrackChanged(TrackId trackId) {
    if (m_pLoadedTrack && m_pLoadedTrack->getId() == trackId) {
        populateFields(*m_pLoadedTrack);
    }
}

void DlgTrackInfo::slotImportMetadataFromMusicBrainz() {
    if (!m_pDlgTagFetcher) {
        m_pDlgTagFetcher = std::make_unique<DlgTagFetcher>(
                m_pTrackModel);
        connect(m_pDlgTagFetcher.get(),
                &QDialog::finished,
                [this]() {
                    if (m_pDlgTagFetcher.get() == sender()) {
                        m_pDlgTagFetcher.release()->deleteLater();
                    }
                });
        if (m_pTrackModel) {
            connect(m_pDlgTagFetcher.get(),
                    &DlgTagFetcher::next,
                    this,
                    &DlgTrackInfo::slotNextDlgTagFetcher);

            connect(m_pDlgTagFetcher.get(),
                    &DlgTagFetcher::previous,
                    this,
                    &DlgTrackInfo::slotPrevDlgTagFetcher);
        }
    }
    if (m_pTrackModel) {
        DEBUG_ASSERT(m_currentTrackIndex.isValid());
        m_pDlgTagFetcher->loadTrack(m_currentTrackIndex);
    } else {
        DEBUG_ASSERT(m_pLoadedTrack);
        m_pDlgTagFetcher->loadTrack(m_pLoadedTrack);
    }
    m_pDlgTagFetcher->show();
}
