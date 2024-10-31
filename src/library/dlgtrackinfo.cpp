#include "library/dlgtrackinfo.h"

#include <QShortcut>
#include <QSignalBlocker>
#include <QStyleFactory>
#include <QtDebug>

#include "defs_urls.h"
#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "library/dlgtagfetcher.h"
#include "library/library_prefs.h"
#include "library/trackmodel.h"
#include "moc_dlgtrackinfo.cpp"
#include "preferences/colorpalettesettings.h"
#include "sources/soundsourceproxy.h"
#include "track/beatutils.h"
#include "track/track.h"
#include "util/color/color.h"
#include "util/datetime.h"
#include "util/desktophelper.h"
#include "util/duration.h"
#include "widget/wcolorpickeractionmenu.h"
#include "widget/wcoverartlabel.h"
#include "widget/wcoverartmenu.h"
#include "widget/wstarrating.h"

namespace {

constexpr double kBpmTabRounding = 1 / 12.0;
constexpr int kFilterLength = 80;
constexpr int kMinBpm = 30;
// Maximum allowed interval between beats (calculated from kMinBpm).
const mixxx::Duration kMaxInterval = mixxx::Duration::fromMillis(
        static_cast<qint64>(1000.0 * (60.0 / kMinBpm)));

} // namespace

DlgTrackInfo::DlgTrackInfo(
        UserSettingsPointer pUserSettings,
        const TrackModel* trackModel)
        // No parent because otherwise it inherits the style parent's
        // style which can make it unreadable. Bug #673411
        : QDialog(nullptr),
          m_pUserSettings(std::move(pUserSettings)),
          m_pTrackModel(trackModel),
          m_tapFilter(this, kFilterLength, kMaxInterval),
          m_pWCoverArtMenu(make_parented<WCoverArtMenu>(this)),
          m_pWCoverArtLabel(make_parented<WCoverArtLabel>(this, m_pWCoverArtMenu)),
          m_pColorPicker(make_parented<WColorPickerActionMenu>(
                  WColorPicker::Option::AllowNoColor |
                          // TODO(xxx) remove this once the preferences are themed via QSS
                          WColorPicker::Option::NoExtStyleSheet,
                  ColorPaletteSettings(m_pUserSettings).getTrackColorPalette(),
                  this)) {
    init();
}

DlgTrackInfo::~DlgTrackInfo() {
    // ~parented_ptr() needs the definition of the wrapped type
    // upon deletion! Otherwise the behavior is undefined.
    // The wrapped types of some parented_ptr members are only
    // forward declared in the header file.
}

void DlgTrackInfo::init() {
    setupUi(this);
    setWindowIcon(QIcon(MIXXX_ICON_PATH));

    // Store tag edit widget pointers to allow focusing a specific widgets when
    // this is opened by double-clicking a WTrackProperty label.
    // Associate with property strings taken from library/dao/trackdao.cpp
    m_propertyWidgets.insert("artist", txtArtist);
    m_propertyWidgets.insert("title", txtTrackName);
    m_propertyWidgets.insert("titleInfo", txtTrackName);
    m_propertyWidgets.insert("album", txtAlbum);
    m_propertyWidgets.insert("album_artist", txtAlbumArtist);
    m_propertyWidgets.insert("composer", txtComposer);
    m_propertyWidgets.insert("genre", txtGenre);
    m_propertyWidgets.insert("year", txtYear);
    m_propertyWidgets.insert("bpm", spinBpm);
    m_propertyWidgets.insert("tracknumber", txtTrackNumber);
    m_propertyWidgets.insert("key", txtKey);
    m_propertyWidgets.insert("grouping", txtGrouping);
    m_propertyWidgets.insert("comment", txtComment);
    m_propertyWidgets.insert("datetime_added", txtDateAdded);
    m_propertyWidgets.insert("duration", txtDuration);
    m_propertyWidgets.insert("timesplayed", txtDateLastPlayed);
    m_propertyWidgets.insert("last_played_at", txtDateLastPlayed);
    m_propertyWidgets.insert("filetype", txtType);
    m_propertyWidgets.insert("bitrate", txtBitrate);
    m_propertyWidgets.insert("samplerate", txtSamplerate);
    m_propertyWidgets.insert("replaygain", txtReplayGain);
    m_propertyWidgets.insert("replaygain_peak", txtReplayGain);
    m_propertyWidgets.insert("track_locations.location", txtLocation);

    coverLayout->insertWidget(0, m_pWCoverArtLabel.get());

    // Workaround: Align the baseline of the "Comments" label
    // with the baseline of the text inside the comments field
    const int topMargin = txtComment->frameWidth() + int(txtComment->document()->documentMargin());
    lblTrackComment->setContentsMargins(0, topMargin, 0, 0);

    if (m_pTrackModel) {
        connect(btnNext,
                &QPushButton::clicked,
                this,
                &DlgTrackInfo::slotNextButton);
        connect(btnPrev,
                &QPushButton::clicked,
                this,
                &DlgTrackInfo::slotPrevButton);

        QShortcut* nextShortcut = new QShortcut(QKeySequence("Alt+Right"), this);
        QShortcut* prevShortcut = new QShortcut(QKeySequence("Alt+Left"), this);

        connect(nextShortcut,
                &QShortcut::activated,
                btnNext,
                [this] { btnNext->animateClick(); });
        connect(prevShortcut,
                &QShortcut::activated,
                btnPrev,
                [this] { btnPrev->animateClick(); });
    } else {
        btnNext->hide();
        btnPrev->hide();
    }

    // QDialog buttons
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

    // BPM edit buttons
    connect(bpmDouble, &QPushButton::clicked, this, [this] {
        slotBpmScale(mixxx::Beats::BpmScale::Double);
    });
    connect(bpmHalve, &QPushButton::clicked, this, [this] {
        slotBpmScale(mixxx::Beats::BpmScale::Halve);
    });
    connect(bpmTwoThirds, &QPushButton::clicked, this, [this] {
        slotBpmScale(mixxx::Beats::BpmScale::TwoThirds);
    });
    connect(bpmThreeFourth, &QPushButton::clicked, this, [this] {
        slotBpmScale(mixxx::Beats::BpmScale::ThreeFourths);
    });
    connect(bpmFourThirds, &QPushButton::clicked, this, [this] {
        slotBpmScale(mixxx::Beats::BpmScale::FourThirds);
    });
    connect(bpmThreeHalves, &QPushButton::clicked, this, [this] {
        slotBpmScale(mixxx::Beats::BpmScale::ThreeHalves);
    });
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

    // Metadata fields
    connect(txtTrackName,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                m_trackRecord.refMetadata().refTrackInfo().setTitle(
                        txtTrackName->text().trimmed());
            });
    connect(txtArtist,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                m_trackRecord.refMetadata().refTrackInfo().setArtist(
                        txtArtist->text().trimmed());
            });
    connect(txtAlbum,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                m_trackRecord.refMetadata().refAlbumInfo().setTitle(
                        txtAlbum->text().trimmed());
            });
    connect(txtAlbumArtist,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                m_trackRecord.refMetadata().refAlbumInfo().setArtist(
                        txtAlbumArtist->text().trimmed());
            });
    connect(txtGenre,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                m_trackRecord.refMetadata().refTrackInfo().setGenre(
                        txtGenre->text().trimmed());
            });
    connect(txtComposer,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                m_trackRecord.refMetadata().refTrackInfo().setComposer(
                        txtComposer->text().trimmed());
            });
    connect(txtGrouping,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                m_trackRecord.refMetadata().refTrackInfo().setGrouping(
                        txtGrouping->text().trimmed());
            });
    connect(txtYear,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                m_trackRecord.refMetadata().refTrackInfo().setYear(
                        txtYear->text().trimmed());
            });
    connect(txtTrackNumber,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                m_trackRecord.refMetadata().refTrackInfo().setTrackNumber(
                        txtTrackNumber->text().trimmed());
            });

    // Import and file browser buttons
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

    // Cover art
    m_pWCoverArtLabel->setToolTip(
            tr("Left-click to show larger preview") + "\n" +
            tr("Right-click for more options"));

    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &DlgTrackInfo::slotCoverFound);
    }

    connect(m_pWCoverArtMenu,
            &WCoverArtMenu::coverInfoSelected,
            this,
            &DlgTrackInfo::slotCoverInfoSelected);

    connect(m_pWCoverArtMenu,
            &WCoverArtMenu::reloadCoverArt,
            this,
            &DlgTrackInfo::slotReloadCoverArt);

    connect(starRating,
            &WStarRating::ratingChangeRequest,
            this,
            &DlgTrackInfo::slotRatingChanged);

    btnColorPicker->setStyle(QStyleFactory::create(QStringLiteral("fusion")));
    btnColorPicker->setMenu(m_pColorPicker.get());

    connect(m_pColorPicker.get(),
            &WColorPickerActionMenu::colorPicked,
            this,
            [this](const mixxx::RgbColor::optional_t& newColor) {
                trackColorDialogSetColor(newColor);
                m_trackRecord.setColor(newColor);
            });
}

void DlgTrackInfo::slotApply() {
    saveTrack();
}

void DlgTrackInfo::slotOk() {
    saveTrack();
    accept();
}

void DlgTrackInfo::slotCancel() {
    reject();
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

QModelIndex DlgTrackInfo::getPrevNextTrack(bool next) {
    return m_currentTrackIndex.sibling(
            m_currentTrackIndex.row() + (next ? 1 : -1),
            m_currentTrackIndex.column());
}

void DlgTrackInfo::loadNextTrack() {
    auto nextRow = getPrevNextTrack(true);
    if (nextRow.isValid()) {
        loadTrack(nextRow);
        refocusCurrentWidget();
        emit next();
    }
}

void DlgTrackInfo::loadPrevTrack() {
    auto prevRow = getPrevNextTrack(false);
    if (prevRow.isValid()) {
        loadTrack(prevRow);
        refocusCurrentWidget();
        emit previous();
    }
}

/// Simulate moving the focus out of and then back into the currently
/// focused widget to trigger behaviors like the "Select all on focus"
/// for QLineEdit.
///
/// This is done when moving to the previous/next track, because,
/// logically, the user has moved to a new dialog, even though we
/// reuse the current dialog instance internally.
void DlgTrackInfo::refocusCurrentWidget() {
    QWidget* focusedWidget = QApplication::focusWidget();
    if (focusedWidget && isAncestorOf(focusedWidget)) {
        focusedWidget->clearFocus();
        focusedWidget->setFocus(Qt::ShortcutFocusReason);
    }
}

void DlgTrackInfo::updateFromTrack(const Track& track) {
    const QSignalBlocker signalBlocker(this);

    setWindowTitle(track.getInfo());

    // Cover art, file type and 'date added'
    replaceTrackRecord(
            track.getRecord(),
            track.getLocation());

    // paint the color selector and check the respective color picker button
    trackColorDialogSetColor(track.getColor());

    txtLocation->setText(QDir::toNativeSeparators(track.getLocation()));

    reloadTrackBeats(track);

    starRating->slotSetRating(m_pLoadedTrack->getRating());
}

void DlgTrackInfo::replaceTrackRecord(
        mixxx::TrackRecord trackRecord,
        const QString& trackLocation) {
    // Signals are already blocked
    m_trackRecord = std::move(trackRecord);

    const auto coverInfo = CoverInfo(
            m_trackRecord.getCoverInfo(),
            trackLocation);
    m_pWCoverArtLabel->setCoverArt(coverInfo, QPixmap());
    // Executed concurrently
    CoverArtCache::requestCover(this, coverInfo);

    // Non-editable fields
    txtType->setText(
            m_trackRecord.getFileType());
    txtDateAdded->setText(
            mixxx::displayLocalDateTime(
                    mixxx::localDateTimeFromUtc(
                            m_trackRecord.getDateAdded())));
    auto lastPlayed = m_trackRecord.getPlayCounter().getLastPlayedAt();
    txtDateLastPlayed->setText(lastPlayed.isValid()
                    ? mixxx::displayLocalDateTime(mixxx::localDateTimeFromUtc(lastPlayed))
                    : QStringLiteral("-"));

    updateTrackMetadataFields();
}

void DlgTrackInfo::updateTrackMetadataFields() {
    const auto metadata = m_trackRecord.getMetadata();
    const auto trackInfo = metadata.getTrackInfo();
    const auto albumInfo = metadata.getAlbumInfo();
    const auto signalInfo = metadata.getStreamInfo().getSignalInfo();

    // Editable fields
    txtTrackName->setText(trackInfo.getTitle());
    txtArtist->setText(trackInfo.getArtist());
    txtAlbum->setText(albumInfo.getTitle());
    txtAlbumArtist->setText(albumInfo.getArtist());
    txtGenre->setText(trackInfo.getGenre());
    txtComposer->setText(trackInfo.getComposer());
    txtGrouping->setText(trackInfo.getGrouping());
    txtYear->setText(trackInfo.getYear());
    txtTrackNumber->setText(trackInfo.getTrackNumber());
    txtComment->setPlainText(trackInfo.getComment());
    txtBpm->setText(trackInfo.getBpmText());
    displayKeyText();

    // Set cursor / scroll position of editable fields (only relevant
    // when the content's width is larger than the width of the QLineEdit)
    txtTrackName->setCursorPosition(0);
    txtArtist->setCursorPosition(0);
    txtAlbum->setCursorPosition(0);
    txtAlbumArtist->setCursorPosition(0);
    txtGenre->setCursorPosition(0);
    txtComposer->setCursorPosition(0);
    txtGrouping->setCursorPosition(0);
    txtYear->setCursorPosition(0);
    txtTrackNumber->setCursorPosition(0);

    // Non-editable fields
    txtDuration->setText(
            metadata.getDurationText(mixxx::Duration::Precision::SECONDS));
    QString bitrate = metadata.getBitrateText();
    if (bitrate.isEmpty()) {
        txtBitrate->clear();
    } else {
        txtBitrate->setText(bitrate + QChar(' ') + mixxx::audio::Bitrate::unit());
    }
    txtReplayGain->setText(
            mixxx::ReplayGain::ratioToString(
                    trackInfo.getReplayGain().getRatio()));

    auto samplerate = signalInfo.getSampleRate();
    if (samplerate.isValid()) {
        txtSamplerate->setText(QString::number(samplerate.value()) + " Hz");
    } else {
        txtSamplerate->clear();
    }
}

void DlgTrackInfo::updateSpinBpmFromBeats() {
    auto bpmValue = mixxx::Bpm::kValueUndefined;
    if (m_pLoadedTrack && m_pBeatsClone) {
        const auto trackEndPosition = mixxx::audio::FramePos{
                m_pLoadedTrack->getDuration() * m_pBeatsClone->getSampleRate()};
        bpmValue = m_pBeatsClone
                           ->getBpmInRange(mixxx::audio::kStartFramePos,
                                   trackEndPosition)
                           .valueOr(mixxx::Bpm::kValueUndefined);
    }
    spinBpm->setValue(bpmValue);
}

void DlgTrackInfo::reloadTrackBeats(const Track& track) {
    m_pBeatsClone = track.getBeats();
    updateSpinBpmFromBeats();
    m_trackHasBeatMap = m_pBeatsClone && !m_pBeatsClone->hasConstantTempo();
    bpmConst->setChecked(!m_trackHasBeatMap);
    bpmConst->setEnabled(m_trackHasBeatMap); // We cannot turn a BeatGrid to a BeatMap
    spinBpm->setEnabled(!m_trackHasBeatMap); // We cannot change bpm continuously or tap them
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

    updateFromTrack(*m_pLoadedTrack);
    m_pWCoverArtLabel->loadTrack(m_pLoadedTrack);

    // Listen to changed() so we don't need to listen to individual
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
    if (m_pDlgTagFetcher && m_pDlgTagFetcher->isVisible()) {
        m_pDlgTagFetcher->loadTrack(m_pLoadedTrack);
    }
}

void DlgTrackInfo::loadTrack(const QModelIndex& index) {
    VERIFY_OR_DEBUG_ASSERT(m_pTrackModel) {
        return;
    }
    TrackPointer pTrack = m_pTrackModel->getTrack(index);

    m_currentTrackIndex = index;
    btnPrev->setEnabled(getPrevNextTrack(false).isValid());
    btnNext->setEnabled(getPrevNextTrack(true).isValid());

    loadTrackInternal(pTrack);
    if (m_pDlgTagFetcher && m_pDlgTagFetcher->isVisible()) {
        m_pDlgTagFetcher->loadTrack(m_currentTrackIndex);
    }
}

void DlgTrackInfo::focusField(const QString& property) {
    if (property.isEmpty()) {
        return;
    }
    auto it = m_propertyWidgets.constFind(property);
    if (it != m_propertyWidgets.constEnd()) {
        it.value()->setFocus();
    }
}

void DlgTrackInfo::slotCoverFound(
        const QObject* pRequester,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap) {
    if (pRequester == this &&
            m_pLoadedTrack &&
            m_pLoadedTrack->getLocation() == coverInfo.trackLocation) {
        m_trackRecord.setCoverInfo(coverInfo);
        m_pWCoverArtLabel->setCoverArt(coverInfo, pixmap);
    }
}

void DlgTrackInfo::slotReloadCoverArt() {
    VERIFY_OR_DEBUG_ASSERT(m_pLoadedTrack) {
        return;
    }
    slotCoverInfoSelected(
            CoverInfoGuesser().guessCoverInfoForTrack(
                    m_pLoadedTrack));
}

void DlgTrackInfo::slotCoverInfoSelected(const CoverInfoRelative& coverInfo) {
    qDebug() << "DlgTrackInfo::slotCoverInfoSelected" << coverInfo;
    VERIFY_OR_DEBUG_ASSERT(m_pLoadedTrack) {
        return;
    }
    m_trackRecord.setCoverInfo(coverInfo);
    CoverArtCache::requestCover(this, CoverInfo(coverInfo, m_pLoadedTrack->getLocation()));
}

void DlgTrackInfo::slotOpenInFileBrowser() {
    if (!m_pLoadedTrack) {
        return;
    }
    mixxx::DesktopHelper::openInFileBrowser(QStringList(m_pLoadedTrack->getLocation()));
}

void DlgTrackInfo::trackColorDialogSetColor(const mixxx::RgbColor::optional_t& newColor) {
    m_pColorPicker->setSelectedColor(newColor);
    btnColorPicker->menu()->close();

    if (newColor) {
        btnColorPicker->setText("");
        const QColor ccolor = mixxx::RgbColor::toQColor(newColor);
        const QString styleSheet =
                QStringLiteral(
                        "QPushButton { background-color: %1; color: %2; }")
                        .arg(ccolor.name(QColor::HexRgb),
                                Color::isDimColor(ccolor)
                                        ? "white"
                                        : "black");
        btnColorPicker->setStyleSheet(styleSheet);
    } else { // no color
        btnColorPicker->setText(tr("(no color)"));
        // clear custom stylesheet, i.e. restore Fusion style,
        btnColorPicker->setStyleSheet("");
    }
}

void DlgTrackInfo::saveTrack() {
    if (!m_pLoadedTrack) {
        return;
    }

    // In case Apply is triggered by hotkey AND a QLineEdit with pending changes
    // is focused AND the user did not hit Enter to finish editing,
    // the content of that focused line edit would be reset to the last confirmed state.
    // This hack makes a focused QLineEdit emit editingFinished() (clearFocus()
    // implicitly emits a focusOutEvent()
    if (this == QApplication::activeWindow()) {
        auto* pFocusWidget = QApplication::focusWidget();
        if (pFocusWidget) {
            pFocusWidget->clearFocus();
            pFocusWidget->setFocus();
        }
    }

    // Special case handling for the comment field that is not
    // updated by the editingFinished signal.
    m_trackRecord.refMetadata().refTrackInfo().setComment(txtComment->toPlainText());

    // First, disconnect the track changed signal. Otherwise we signal ourselves
    // and repopulate all these fields.
    const QSignalBlocker signalBlocker(this);
    // If the user is editing the bpm or key and hits enter to close DlgTrackInfo,
    // the editingFinished signal will not fire in time. Invoke the connected
    // handlers manually to capture any changes. If the bpm or key was unchanged
    // or invalid then the change will be ignored/rejected.
    slotSpinBpmValueChanged(spinBpm->value());
    static_cast<void>(updateKeyText()); // discard result

    // Update the cached track
    //
    // If replaceRecord() returns true then both m_trackRecord and m_pBeatsClone
    // will be updated by the subsequent Track::changed() signal to keep them
    // synchronized with the track. Otherwise the track has not been modified and
    // both members must remain valid. Do not use std::move() for passing arguments!
    // Else triggering apply twice in quick succession might clear the metadata.
    m_pLoadedTrack->replaceRecord(m_trackRecord, m_pBeatsClone);
}

void DlgTrackInfo::clear() {
    const QSignalBlocker signalBlocker(this);

    setWindowTitle(QString());

    if (m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(),
                &Track::changed,
                this,
                &DlgTrackInfo::slotTrackChanged);
        m_pLoadedTrack.reset();
    }

    resetTrackRecord();

    m_pBeatsClone.reset();
    updateSpinBpmFromBeats();

    txtLocation->setText("");

    starRating->slotSetRating(0);
}

void DlgTrackInfo::slotBpmScale(mixxx::Beats::BpmScale bpmScale) {
    if (!m_pBeatsClone) {
        return;
    }
    const auto scaledBeats = m_pBeatsClone->tryScale(bpmScale);
    if (scaledBeats) {
        m_pBeatsClone = *scaledBeats;
        updateSpinBpmFromBeats();
    }
}

void DlgTrackInfo::slotBpmClear() {
    m_pBeatsClone.reset();
    updateSpinBpmFromBeats();

    bpmConst->setChecked(true);
    bpmConst->setEnabled(m_trackHasBeatMap);
    spinBpm->setEnabled(true);
    bpmTap->setEnabled(true);
}

void DlgTrackInfo::slotBpmConstChanged(int state) {
    if (state == Qt::Unchecked) {
        // try to reload BeatMap from the Track
        reloadTrackBeats(*m_pLoadedTrack);
        return;
    }
    spinBpm->setEnabled(true);
    bpmTap->setEnabled(true);
    slotSpinBpmValueChanged(spinBpm->value());
}

void DlgTrackInfo::slotBpmTap(double averageLength, int numSamples) {
    Q_UNUSED(numSamples);
    if (averageLength == 0) {
        return;
    }
    auto averageBpm = mixxx::Bpm(60.0 * 1000.0 / averageLength);
    averageBpm = BeatUtils::roundBpmWithinRange(averageBpm - kBpmTabRounding,
            averageBpm,
            averageBpm + kBpmTabRounding);
    if (averageBpm != m_lastTapedBpm) {
        m_lastTapedBpm = averageBpm;
        spinBpm->setValue(averageBpm.valueOr(mixxx::Bpm::kValueUndefined));
    }
}

void DlgTrackInfo::slotSpinBpmValueChanged(double value) {
    const auto bpm = mixxx::Bpm(value);
    if (!bpm.isValid()) {
        m_pBeatsClone.reset();
        return;
    }

    if (m_pLoadedTrack) {
        if (m_pBeatsClone) {
            const auto trackEndPosition = mixxx::audio::FramePos{
                    m_pLoadedTrack->getDuration() * m_pBeatsClone->getSampleRate()};
            const mixxx::Bpm oldBpm = m_pBeatsClone->getBpmInRange(
                    mixxx::audio::kStartFramePos, trackEndPosition);
            if (oldBpm == bpm) {
                return;
            }
            m_pBeatsClone = m_pBeatsClone->trySetBpm(bpm).value_or(m_pBeatsClone);
        } else {
            mixxx::audio::FramePos cuePosition = m_pLoadedTrack->getMainCuePosition();
            // This should never happen, but we cannot be sure
            VERIFY_OR_DEBUG_ASSERT(cuePosition.isValid()) {
                cuePosition = mixxx::audio::kStartFramePos;
            }
            m_pBeatsClone = mixxx::Beats::fromConstTempo(
                    m_pLoadedTrack->getSampleRate(),
                    // Cue positions might be fractional, i.e. not on frame boundaries!
                    cuePosition.toNearestFrameBoundary(),
                    bpm);
        }
    }

    updateSpinBpmFromBeats();
}

mixxx::UpdateResult DlgTrackInfo::updateKeyText() {
    const auto keyText = txtKey->text().trimmed();
    const auto updateResult =
            m_trackRecord.updateGlobalKeyNormalizeText(
                    keyText,
                    mixxx::track::io::key::USER);
    if (updateResult == mixxx::UpdateResult::Rejected) {
        // Restore the current key text
        displayKeyText();
    }
    return updateResult;
}

void DlgTrackInfo::displayKeyText() {
    const QString keyText = m_trackRecord.getMetadata().getTrackInfo().getKeyText();
    txtKey->setText(keyText);
}

void DlgTrackInfo::slotKeyTextChanged() {
    if (updateKeyText() != mixxx::UpdateResult::Unchanged) {
        // Ensure that the text field always reflects the actual value
        displayKeyText();
    }
}

void DlgTrackInfo::slotRatingChanged(int rating) {
    if (!m_pLoadedTrack) {
        return;
    }
    if (m_trackRecord.isValidRating(rating) &&
            rating != m_trackRecord.getRating()) {
        starRating->slotSetRating(rating);
        m_trackRecord.setRating(rating);
    }
}

void DlgTrackInfo::slotImportMetadataFromFile() {
    if (!m_pLoadedTrack) {
        return;
    }
    // Initialize the metadata with the current metadata to avoid
    // losing existing metadata or to lose the beat grid by replacing
    // it with a default grid created from an imprecise BPM.
    // See also: https://github.com/mixxxdj/mixxx/issues/10420
    // In addition we need to preserve all other track properties
    // that are stored in TrackRecord, which serves as the underlying
    // model for this dialog.
    mixxx::TrackRecord trackRecord = m_pLoadedTrack->getRecord();
    mixxx::TrackMetadata trackMetadata = trackRecord.getMetadata();
    QImage coverImage;
    const auto resetMissingTagMetadata = m_pUserSettings->getValue<bool>(
            mixxx::library::prefs::kResetMissingTagMetadataOnImportConfigKey);
    const auto [importResult, sourceSynchronizedAt] =
            SoundSourceProxy(m_pLoadedTrack)
                    .importTrackMetadataAndCoverImage(
                            &trackMetadata, &coverImage, resetMissingTagMetadata);
    if (importResult != mixxx::MetadataSource::ImportResult::Succeeded) {
        return;
    }
    const mixxx::FileInfo fileInfo = m_pLoadedTrack->getFileInfo();
    auto guessedCoverInfo = CoverInfoGuesser().guessCoverInfo(
            fileInfo,
            trackMetadata.getAlbumInfo().getTitle(),
            coverImage);
    trackRecord.replaceMetadataFromSource(
            std::move(trackMetadata),
            sourceSynchronizedAt);
    trackRecord.setCoverInfo(
            std::move(guessedCoverInfo));
    replaceTrackRecord(
            std::move(trackRecord),
            fileInfo.location());
}

void DlgTrackInfo::slotTrackChanged(TrackId trackId) {
    if (m_pLoadedTrack && m_pLoadedTrack->getId() == trackId) {
        updateFromTrack(*m_pLoadedTrack);
    }
}

void DlgTrackInfo::slotImportMetadataFromMusicBrainz() {
    if (!m_pDlgTagFetcher) {
        m_pDlgTagFetcher = std::make_unique<DlgTagFetcher>(
                m_pUserSettings, m_pTrackModel);
        connect(m_pDlgTagFetcher.get(),
                &QDialog::finished,
                this,
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
