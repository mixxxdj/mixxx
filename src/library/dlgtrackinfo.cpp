#include "library/dlgtrackinfo.h"

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
#include "track/keyfactory.h"
#include "track/track.h"
#include "util/color/color.h"
#include "util/datetime.h"
#include "util/desktophelper.h"
#include "util/duration.h"
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
const QString kBpmPropertyName = QStringLiteral("bpm");

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
          m_pWStarRating(make_parented<WStarRating>(this)),
          m_pColorPicker(make_parented<WColorPickerAction>(
                  WColorPicker::Option::AllowNoColor |
                          // TODO(xxx) remove this once the preferences are themed via QSS
                          WColorPicker::Option::NoExtStyleSheet,
                  ColorPaletteSettings(m_pUserSettings).getTrackColorPalette(),
                  this)) {
    init();
}

void DlgTrackInfo::init() {
    setupUi(this);
    setWindowIcon(QIcon(MIXXX_ICON_PATH));

    // Store tag edit widget pointers to allow focusing a specific widgets when
    // this is opened by double-clicking a WTrackProperty label.
    // Associate with property strings taken from library/dao/trackdao.h
    m_propertyWidgets.insert("artist", txtArtist);
    m_propertyWidgets.insert("title", txtTitle);
    m_propertyWidgets.insert("titleInfo", txtTitle);
    m_propertyWidgets.insert("album", txtAlbum);
    m_propertyWidgets.insert("album_artist", txtAlbumArtist);
    m_propertyWidgets.insert("composer", txtComposer);
    m_propertyWidgets.insert("genre", txtGenre);
    m_propertyWidgets.insert("year", txtYear);
    m_propertyWidgets.insert(kBpmPropertyName, spinBpm);
    m_propertyWidgets.insert("tracknumber", txtTrackNumber);
    m_propertyWidgets.insert("key", txtKey);
    m_propertyWidgets.insert("grouping", txtGrouping);
    m_propertyWidgets.insert("comment", txtComment);

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
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
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
    connect(txtTitle,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                txtTitle->setText(txtTitle->text().trimmed());
                m_trackRecord.refMetadata().refTrackInfo().setTitle(
                        txtTitle->text());
            });
    connect(txtArtist,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                txtArtist->setText(txtArtist->text().trimmed());
                m_trackRecord.refMetadata().refTrackInfo().setArtist(
                        txtArtist->text());
            });
    connect(txtAlbum,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                txtAlbum->setText(txtAlbum->text().trimmed());
                m_trackRecord.refMetadata().refAlbumInfo().setTitle(
                        txtAlbum->text());
            });
    connect(txtAlbumArtist,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                txtAlbumArtist->setText(txtAlbumArtist->text().trimmed());
                m_trackRecord.refMetadata().refAlbumInfo().setArtist(
                        txtAlbumArtist->text());
            });
    connect(txtGenre,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                txtGenre->setText(txtGenre->text().trimmed());
                m_trackRecord.refMetadata().refTrackInfo().setGenre(
                        txtGenre->text());
            });
    connect(txtComposer,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                txtComposer->setText(txtComposer->text().trimmed());
                m_trackRecord.refMetadata().refTrackInfo().setComposer(
                        txtComposer->text());
            });
    connect(txtGrouping,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                txtGrouping->setText(txtGrouping->text().trimmed());
                m_trackRecord.refMetadata().refTrackInfo().setGrouping(
                        txtGrouping->text());
            });
    connect(txtYear,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                txtYear->setText(txtYear->text().trimmed());
                m_trackRecord.refMetadata().refTrackInfo().setYear(
                        txtYear->text());
            });
    connect(txtTrackNumber,
            &QLineEdit::editingFinished,
            this,
            [this]() {
                txtTrackNumber->setText(txtTrackNumber->text().trimmed());
                m_trackRecord.refMetadata().refTrackInfo().setTrackNumber(
                        txtTrackNumber->text());
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

    connect(m_pWStarRating,
            &WStarRating::ratingChangeRequest,
            this,
            &DlgTrackInfo::slotRatingChanged);

    btnColorPicker->setStyle(QStyleFactory::create(QStringLiteral("fusion")));
    QMenu* pColorPickerMenu = new QMenu(this);
    pColorPickerMenu->addAction(m_pColorPicker);
    btnColorPicker->setMenu(pColorPickerMenu);

    connect(btnColorPicker,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::slotColorButtonClicked);
    connect(m_pColorPicker.get(),
            &WColorPickerAction::colorPicked,
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

    m_pWStarRating->slotSetRating(m_pLoadedTrack->getRating());
}

void DlgTrackInfo::replaceTrackRecord(
        mixxx::TrackRecord trackRecord,
        const QString& trackLocation) {
    // Signals are already blocked
    m_trackRecord = std::move(trackRecord);

    const auto coverInfo = CoverInfo(
            m_trackRecord.getCoverInfo(),
            trackLocation);
    m_pWCoverArtLabel->setCoverInfoAndPixmap(coverInfo, QPixmap());
    // Executed concurrently
    CoverArtCache::requestCover(this, coverInfo);

    // Non-editable fields
    txtType->setText(
            m_trackRecord.getFileType());
    txtDateAdded->setText(
            mixxx::displayLocalDateTime(
                    mixxx::localDateTimeFromUtc(
                            m_trackRecord.getDateAdded())));

    updateTrackMetadataFields();
}

void DlgTrackInfo::updateTrackMetadataFields() {
    // Editable fields
    txtTitle->setText(
            m_trackRecord.getMetadata().getTrackInfo().getTitle());
    txtArtist->setText(
            m_trackRecord.getMetadata().getTrackInfo().getArtist());
    txtAlbum->setText(
            m_trackRecord.getMetadata().getAlbumInfo().getTitle());
    txtAlbumArtist->setText(
            m_trackRecord.getMetadata().getAlbumInfo().getArtist());
    txtGenre->setText(
            m_trackRecord.getMetadata().getTrackInfo().getGenre());
    txtComposer->setText(
            m_trackRecord.getMetadata().getTrackInfo().getComposer());
    txtGrouping->setText(
            m_trackRecord.getMetadata().getTrackInfo().getGrouping());
    txtYear->setText(
            m_trackRecord.getMetadata().getTrackInfo().getYear());
    txtTrackNumber->setText(
            m_trackRecord.getMetadata().getTrackInfo().getTrackNumber());
    txtComment->setPlainText(
            m_trackRecord.getMetadata().getTrackInfo().getComment());
    txtBpm->setText(
            m_trackRecord.getMetadata().getTrackInfo().getBpmText());
    displayKeyText();

    // Non-editable fields
    txtDuration->setText(
            m_trackRecord.getMetadata().getDurationText(mixxx::Duration::Precision::SECONDS));
    QString bitrate = m_trackRecord.getMetadata().getBitrateText();
    if (bitrate.isEmpty()) {
        txtBitrate->clear();
    } else {
        txtBitrate->setText(bitrate + QChar(' ') + mixxx::audio::Bitrate::unit());
    }
    txtReplayGain->setText(
            mixxx::ReplayGain::ratioToString(
                    m_trackRecord.getMetadata().getTrackInfo().getReplayGain().getRatio()));

    auto samplerate = m_trackRecord.getMetadata().getStreamInfo().getSignalInfo().getSampleRate();
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
    VERIFY_OR_DEBUG_ASSERT(pTrack) {
        return;
    }
    m_currentTrackIndex = index;
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
        if (property == kBpmPropertyName) {
            // If we shall focus the BPM spinbox, switch to BPM tab
            tabWidget->setCurrentIndex(tabWidget->indexOf(tabBPM));
        }
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
        m_pWCoverArtLabel->setCoverInfoAndPixmap(coverInfo, pixmap);
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

void DlgTrackInfo::slotColorButtonClicked() {
    if (!m_pLoadedTrack) {
        return;
    }
    btnColorPicker->showMenu();
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
    updateKeyText();

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

    m_pWStarRating->slotSetRating(0);
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgTrackInfo::slotBpmConstChanged(Qt::CheckState state) {
#else
void DlgTrackInfo::slotBpmConstChanged(int state) {
#endif
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

void DlgTrackInfo::updateKeyText() {
    const auto keyText = txtKey->text();
    m_trackRecord.updateGlobalKeyNormalizeText(
            keyText,
            mixxx::track::io::key::USER);
    displayKeyText();
}

void DlgTrackInfo::displayKeyText() {
    const QString keyText = KeyUtils::keyToString(m_trackRecord.getKeys().getGlobalKey());
    txtKey->setText(keyText);
}

void DlgTrackInfo::slotKeyTextChanged() {
    updateKeyText();
}

void DlgTrackInfo::slotRatingChanged(int rating) {
    if (!m_pLoadedTrack) {
        return;
    }
    if (m_trackRecord.isValidRating(rating) &&
            rating != m_trackRecord.getRating()) {
        m_pWStarRating->slotSetRating(rating);
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

    QString importedKeyText = trackRecord.getMetadata().getTrackInfo().getKeyText();
    {
        Keys newKeys = KeyFactory::makeBasicKeysKeepText(
                importedKeyText, mixxx::track::io::key::FILE_METADATA);
        if (newKeys.getGlobalKey() != mixxx::track::io::key::INVALID &&
                trackRecord.getKeys().getGlobalKeyText() != importedKeyText) {
            // Only replace the keys with a single new key if valid and different.
            // Otherwise preserve existing array of keys for different positions.
            trackRecord.setKeys(std::move(newKeys));
        }
    }

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

void DlgTrackInfo::resizeEvent(QResizeEvent* pEvent) {
    QDialog::resizeEvent(pEvent);

    if (!isVisible()) {
        // Likely one of the resize events before show().
        // Widgets don't have their final size, yet, so it
        // makes no sense to resize the cover label.
        return;
    }

    // Set a maximum size on the cover label so it can use the available space
    // but doesn't force-expand the dialog.
    // The cover label spans across three tag rows and the two rightmost columns.
    // Unfortunately we can't read row/column sizes directly, so we use the widgets.
    int contHeight = txtTitle->height() + txtArtist->height() + txtAlbum->height();
    int vSpacing = tags_layout->verticalSpacing();
    int totalHeight = vSpacing * 2 + contHeight;

    int contWidth = lblYear->width() + txtYear->width();
    int hSpacing = tags_layout->horizontalSpacing();
    int totalWidth = contWidth + hSpacing;

    m_pWCoverArtLabel->setMaxSize(QSize(totalWidth, totalHeight));

    // Also clamp height of the cover's parent widget. Keeping its height minimal
    // can't be accomplished with QSizePolicies alone unfortunately.
    coverWidget->setFixedHeight(totalHeight);
}
