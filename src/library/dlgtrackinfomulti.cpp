#include "library/dlgtrackinfomulti.h"

#include <QStyleFactory>
#include <QtDebug>

#include "defs_urls.h"
#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "library/library_prefs.h"
#include "moc_dlgtrackinfomulti.cpp"
#include "preferences/colorpalettesettings.h"
#include "sources/soundsourceproxy.h"
#include "track/beatutils.h"
#include "track/track.h"
#include "util/color/color.h"
#include "util/datetime.h"
#include "util/duration.h"
#include "widget/wcoverartlabel.h"
#include "widget/wcoverartmenu.h"
#include "widget/wstarrating.h"

namespace {

const QString kVariousText = char('<') + QObject::tr("various") + char('>');

/// If value differs from the current value, add it to the list.
/// If new and current are identical, keep only one. Later on we can use the
/// item count to maybe join the list and format the font accordingly.
/// List can be a QStringList or a QList<double>
template<typename T>
void appendToTagList(QList<T>* pList, const T& value) {
    if (pList->isEmpty() || !pList->contains(value)) {
        pList->append(value);
    }
}

void setItalic(QWidget* pEditor, bool italic) {
    auto font = pEditor->font();
    if (font.italic() == italic) {
        return;
    }
    font.setItalic(italic);
    pEditor->setFont(font);
}

void setBold(QWidget* pEditor, bool bold) {
    auto font = pEditor->font();
    if (font.bold() == bold) {
        return;
    }
    font.setBold(bold);
    pEditor->setFont(font);
}

/// Returns either the only value or a joint list of values.
/// If there's more than one value make the text italic to indicate multiple
/// values, also if only one is not empty.
/// pWidget can be a QLineEdit or a QPlainTextEdit (comment)
QString maybeJoinValuesAndFormatFont(QWidget* pWidget, QStringList& values) {
    if (values.size() == 1) {
        setItalic(pWidget, false);
        return values[0];
    } else {
        setItalic(pWidget, true);
        // remove empty strings to avoid redundant separators when joining
        values.removeAll("");
        if (qobject_cast<QPlainTextEdit*>(pWidget)) {
            // insert a separator line between comment values
            return values.join("\n---\n");
        } else {
            return values.join(';');
        }
    }
}

/// Returns either the only value or the 'various' string.
/// In the latter case make the text italic to indicate multiple values,
/// also if only one is not empty.
/// This is used for the Year and Track# fields as well as the sample rate and
/// file ytpe labels. Optionally toggle bold (non-editable fields).
/// pWidget can be a QLineEdit or a QPlainTextEdit (comment)
QString getCommonValueOrVariousStringAndFormatFont(QWidget* pWidget,
        QStringList& values,
        bool toggleBold = false,
        const QString& unit = QString()) {
    if (values.size() == 1) {
        setItalic(pWidget, false);
        if (toggleBold) {
            setBold(pWidget, true);
        }
        if (unit.isEmpty()) {
            return values[0];
        } else {
            return values[0] + QChar(' ') + unit;
        }
    } else {
        setItalic(pWidget, true);
        if (toggleBold) {
            setBold(pWidget, false);
        }
        return kVariousText;
    }
}

} // namespace

DlgTrackInfoMulti::DlgTrackInfoMulti(UserSettingsPointer pUserSettings)
        // No parent because otherwise it inherits the style parent's
        // style which can make it unreadable. Bug #673411
        : QDialog(nullptr),
          m_pUserSettings(std::move(pUserSettings)),
          m_pWCoverArtMenu(make_parented<WCoverArtMenu>(this)),
          m_pWCoverArtLabel(make_parented<WCoverArtLabel>(this, m_pWCoverArtMenu)),
          m_pWStarRating(make_parented<WStarRating>(this)),
          m_starRatingModified(false),
          m_newRating(0),
          m_colorChanged(false),
          m_newColor(mixxx::RgbColor::nullopt()),
          m_pColorPicker(make_parented<WColorPickerAction>(
                  WColorPicker::Option::AllowNoColor |
                          // TODO(xxx) remove this once the preferences are themed via QSS
                          WColorPicker::Option::NoExtStyleSheet,
                  ColorPaletteSettings(m_pUserSettings).getTrackColorPalette(),
                  this)) {
    init();
}

void DlgTrackInfoMulti::init() {
    setupUi(this);
    setWindowIcon(QIcon(MIXXX_ICON_PATH));

    // QDialog buttons
    connect(btnApply,
            &QPushButton::clicked,
            this,
            &DlgTrackInfoMulti::slotApply);

    connect(btnOK,
            &QPushButton::clicked,
            this,
            &DlgTrackInfoMulti::slotOk);

    connect(btnCancel,
            &QPushButton::clicked,
            this,
            &DlgTrackInfoMulti::slotCancel);

    connect(btnReset,
            &QPushButton::clicked,
            this,
            &DlgTrackInfoMulti::updateFromTracks);

    connect(btnImportMetadataFromFile,
            &QPushButton::clicked,
            this,
            &DlgTrackInfoMulti::slotImportMetadataFromFiles);

    QList<QWidget*> lineEdits;
    lineEdits.append(txtArtist);
    lineEdits.append(txtTitle);
    lineEdits.append(txtTitle);
    lineEdits.append(txtAlbum);
    lineEdits.append(txtAlbumArtist);
    lineEdits.append(txtComposer);
    lineEdits.append(txtGenre);
    lineEdits.append(txtYear);
    lineEdits.append(txtTrackNumber);
    lineEdits.append(txtGrouping);

    // Restore default font as soon as mulit-value fields are edited.
    // Make font italic when multiple tag values are restored from file.
    // Single-line QLineEdits:
    for (auto* pEditor : lineEdits) {
        // just to be safe
        QLineEdit* pLine = qobject_cast<QLineEdit*>(pEditor);
        if (!pLine) {
            continue;
        }
        connect(pLine,
                &QLineEdit::textEdited,
                this,
                [pLine]() {
                    setItalic(pLine, !pLine->isModified());
                });
    }
    // Same for the comment editor, emits a different signal
    connect(txtComment,
            &QPlainTextEdit::modificationChanged,
            this,
            [this](bool modified) {
                setItalic(txtComment, !modified);
            });

    btnColorPicker->setStyle(QStyleFactory::create(QStringLiteral("fusion")));
    QMenu* pColorPickerMenu = new QMenu(this);
    pColorPickerMenu->addAction(m_pColorPicker);
    btnColorPicker->setMenu(pColorPickerMenu);

    connect(btnColorPicker,
            &QPushButton::clicked,
            this,
            &DlgTrackInfoMulti::slotColorButtonClicked);
    connect(m_pColorPicker.get(),
            &WColorPickerAction::colorPicked,
            this,
            &DlgTrackInfoMulti::slotColorPicked);

    // Insert the star rating widget
    starsLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    starsLayout->setSpacing(0);
    starsLayout->setContentsMargins(0, 0, 0, 0);
    starsLayout->insertWidget(0, m_pWStarRating.get());
    // This is necessary to pass on mouseMove events to WStarRating
    m_pWStarRating->setMouseTracking(true);
    connect(m_pWStarRating,
            &WStarRating::ratingChangeRequest,
            this,
            &DlgTrackInfoMulti::slotStarRatingChanged);

    // Insert the cover widget
    coverLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);
    coverLayout->setSpacing(0);
    coverLayout->setContentsMargins(0, 0, 0, 0);
    coverLayout->insertWidget(0, m_pWCoverArtLabel.get());
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &DlgTrackInfoMulti::slotCoverFound);
    }

    connect(m_pWCoverArtMenu,
            &WCoverArtMenu::coverInfoSelected,
            this,
            &DlgTrackInfoMulti::slotCoverInfoSelected);

    connect(m_pWCoverArtMenu,
            &WCoverArtMenu::reloadCoverArt,
            this,
            &DlgTrackInfoMulti::slotReloadCoverArt);
}

void DlgTrackInfoMulti::slotApply() {
    saveTracks();
}

void DlgTrackInfoMulti::slotOk() {
    slotApply();
    clear();
    accept();
}

void DlgTrackInfoMulti::slotCancel() {
    clear();
    reject();
}

void DlgTrackInfoMulti::loadTracks(const QList<TrackPointer>& pTracks) {
    clear();

    if (pTracks.isEmpty()) {
        return;
    }

    m_pLoadedTracks.clear();
    for (const auto& pTrack : pTracks) {
        m_pLoadedTracks.insert(pTrack.get()->getId(), pTrack);
    }

    updateFromTracks();

    // Listen to all tracks' changed() signal so we don't need to listen to
    // individual signals such as cuesUpdates, coverArtUpdated(), etc.
    connectTracksChanged();
}

void DlgTrackInfoMulti::updateFromTracks() {
    const QSignalBlocker signalBlocker(this);

    QList<mixxx::TrackRecord> trackRecords;
    trackRecords.reserve(m_pLoadedTracks.size());
    for (auto recIt = m_pLoadedTracks.constBegin();
            recIt != m_pLoadedTracks.constEnd();
            recIt++) {
        const auto pTrack = recIt.value();
        const auto rec = pTrack.get()->getRecord();
        trackRecords.append(rec);
    }
    replaceTrackRecords(trackRecords);

    // Collect star ratings and track colors
    // If track value differs from the current value, add it to the list.
    // If new and current are identical, keep only one.
    int commonRating = m_trackRecords.first().getRating();
    for (auto& rec : m_trackRecords) {
        if (commonRating != rec.getRating()) {
            commonRating = 0;
            break;
        }
    }
    // Update the star widget
    // Block signals to not set the 'modified' flag.
    m_pWStarRating->blockSignals(true);
    m_pWStarRating->slotSetRating(commonRating);
    m_starRatingModified = false;
    m_pWStarRating->blockSignals(false);

    // Same procedure for the track color
    mixxx::RgbColor::optional_t commonColor = m_trackRecords.first().getColor();
    bool multipleColors = false;
    for (auto& rec : m_trackRecords) {
        if (commonColor != rec.getColor()) {
            commonColor = mixxx::RgbColor::nullopt();
            multipleColors = true;
            break;
        }
    }
    // Paint the color selector and check the respective color picker button.
    // Paints a rainbow gardient in case of multiple colors.
    trackColorDialogSetColorStyleButton(commonColor, multipleColors);
    m_colorChanged = false;
    m_newColor = mixxx::RgbColor::nullopt();

    // And the track directory
    QString commonDir = m_pLoadedTracks.constBegin().value()->getFileInfo().canonicalLocationPath();
    QStringList dirs;
    dirs.append(commonDir);
    for (auto dirIt = m_pLoadedTracks.constBegin();
            dirIt != m_pLoadedTracks.constEnd();
            dirIt++) {
        const auto dir = dirIt.value()->getFileInfo().canonicalLocationPath();
        if (dir != commonDir) {
            dirs.append(dir);
            break;
        }
    }
    txtLocation->setText(getCommonValueOrVariousStringAndFormatFont(txtLocation, dirs));

    // And the cover label
    updateCoverArtFromTracks();
}

void DlgTrackInfoMulti::replaceTrackRecords(const QList<mixxx::TrackRecord>& trackRecords) {
    // Signals are already blocked
    m_trackRecords = std::move(trackRecords);

    updateTrackMetadataFields();
}

void DlgTrackInfoMulti::updateTrackMetadataFields() {
    // Editable fields
    QStringList titles;
    QStringList artists;
    QStringList aTitles;
    QStringList aArtists;
    QStringList genres;
    QStringList composers;
    QStringList grouping;
    QStringList years;
    QStringList nums;
    QStringList comments;
    QList<double> bpms;
    QList<uint32_t> bitrates;
    QList<double> durations;
    QStringList samplerates;
    QStringList filetypes;

    for (auto& rec : m_trackRecords) {
        appendToTagList(&titles, rec.getMetadata().getTrackInfo().getTitle());
        appendToTagList(&artists, rec.getMetadata().getTrackInfo().getArtist());
        appendToTagList(&aTitles, rec.getMetadata().getAlbumInfo().getTitle());
        appendToTagList(&aArtists, rec.getMetadata().getAlbumInfo().getArtist());
        appendToTagList(&genres, rec.getMetadata().getTrackInfo().getGenre());
        appendToTagList(&composers, rec.getMetadata().getTrackInfo().getComposer());
        appendToTagList(&grouping, rec.getMetadata().getTrackInfo().getGrouping());
        appendToTagList(&years, rec.getMetadata().getTrackInfo().getYear());
        appendToTagList(&nums, rec.getMetadata().getTrackInfo().getTrackNumber());
        appendToTagList(&comments, rec.getMetadata().getTrackInfo().getComment());

        auto bpm = rec.getMetadata().getTrackInfo().getBpm();
        appendToTagList(&bpms, bpm.isValid() ? bpm.value() : mixxx::Bpm::kValueMin);

        auto bitrate = rec.getMetadata().getStreamInfo().getBitrate();
        appendToTagList(&bitrates, bitrate.isValid() ? bitrate.value() : 0);

        appendToTagList(&durations, rec.getMetadata().getDurationSecondsRounded());

        auto samplerate = rec.getMetadata().getStreamInfo().getSignalInfo().getSampleRate();
        if (samplerate.isValid()) {
            appendToTagList(&samplerates, QString::number(samplerate.value()));
        } else {
            appendToTagList(&samplerates, QStringLiteral("--"));
        }

        appendToTagList(&filetypes, rec.getFileType());
    }

    // Since Qt6 QLineEdit->setText() also calls setModified(false). When saving
    // tracks we can use this flag to only write tags the user modified.
    txtTitle->setText(maybeJoinValuesAndFormatFont(txtTitle, titles));
    txtArtist->setText(maybeJoinValuesAndFormatFont(txtArtist, artists));
    txtAlbum->setText(maybeJoinValuesAndFormatFont(txtAlbum, aTitles));
    txtAlbumArtist->setText(maybeJoinValuesAndFormatFont(txtAlbumArtist, aArtists));
    txtGenre->setText(maybeJoinValuesAndFormatFont(txtGenre, genres));
    txtComposer->setText(maybeJoinValuesAndFormatFont(txtComposer, composers));
    txtGrouping->setText(maybeJoinValuesAndFormatFont(txtGrouping, grouping));
    txtComment->setPlainText(maybeJoinValuesAndFormatFont(txtComment, comments));
    txtYear->setText(getCommonValueOrVariousStringAndFormatFont(txtYear, years));
    txtTrackNumber->setText(getCommonValueOrVariousStringAndFormatFont(txtTrackNumber, nums));

    // Non-editable fields
    if (bpms.size() > 1) {
        std::sort(bpms.begin(), bpms.end());
        txtBpm->setText(QString("%1").arg(bpms.first(), 3, 'f', 1) +
                QChar('-') +
                QString("%1").arg(bpms.last(), 3, 'f', 1));
    } else {
        txtBpm->setText(QString::number(bpms.first()));
    }

    QString bitrate;
    if (bitrates.size() > 1) {
        std::sort(bitrates.begin(), bitrates.end());
        bitrate = QString::number(bitrates.first()) + QChar('-') + QString::number(bitrates.last());
    } else {
        bitrate = QString::number(bitrates.first());
    }
    txtBitrate->setText(bitrate + QChar(' ') + mixxx::audio::Bitrate::unit());

    txtSamplerate->setText(getCommonValueOrVariousStringAndFormatFont(txtSamplerate,
            samplerates,
            true, // bold if common value
            QStringLiteral("Hz")));
    txtType->setText(getCommonValueOrVariousStringAndFormatFont(txtType, filetypes, true));

    if (durations.size() > 1) {
        std::sort(durations.begin(), durations.end());
        txtDuration->setText(mixxx::Duration::formatTime(durations.first()) +
                QChar('-') +
                mixxx::Duration::formatTime(durations.last()));
    } else {
        txtDuration->setText(mixxx::Duration::formatTime(durations.first()));
    }
}

void DlgTrackInfoMulti::saveTracks() {
    if (m_pLoadedTracks.isEmpty()) {
        return;
    }

    for (auto& rec : m_trackRecords) {
        if (txtTitle->isModified()) {
            rec.refMetadata().refTrackInfo().setTitle(
                    txtTitle->text().trimmed());
        }
        if (txtArtist->isModified()) {
            rec.refMetadata().refTrackInfo().setArtist(
                    txtArtist->text().trimmed());
        }
        if (txtAlbum->isModified()) {
            rec.refMetadata().refAlbumInfo().setTitle(
                    txtAlbum->text().trimmed());
        }
        if (txtAlbumArtist->isModified()) {
            rec.refMetadata().refAlbumInfo().setArtist(
                    txtAlbumArtist->text().trimmed());
        }
        if (txtGenre->isModified()) {
            rec.refMetadata().refTrackInfo().setGenre(
                    txtGenre->text().trimmed());
        }
        if (txtComposer->isModified()) {
            rec.refMetadata().refTrackInfo().setComposer(
                    txtComposer->text().trimmed());
        }
        if (txtGrouping->isModified()) {
            rec.refMetadata().refTrackInfo().setGrouping(
                    txtGrouping->text().trimmed());
        }
        if (txtYear->isModified()) {
            rec.refMetadata().refTrackInfo().setYear(
                    txtYear->text().trimmed());
        }
        if (txtTrackNumber->isModified()) {
            rec.refMetadata().refTrackInfo().setTrackNumber(
                    txtTrackNumber->text().trimmed());
        }
        if (txtComment->document()->isModified()) {
            rec.refMetadata().refTrackInfo().setComment(
                    txtComment->toPlainText());
        }
        if (m_colorChanged) {
            rec.setColor(m_newColor);
        }
        if (m_starRatingModified) {
            rec.setRating(m_newRating);
        }
    }

    // First, disconnect the track changed signal. Otherwise we signal ourselves
    // and repopulate all these fields.
    disconnectTracksChanged();
    // Update the cached tracks
    for (auto& rec : m_trackRecords) {
        auto pTrack = m_pLoadedTracks.value(rec.getId());
        // If replaceRecord() returns true then both m_trackRecord and m_pBeatsClone
        // will be updated by the subsequent Track::changed() signal to keep them
        // synchronized with the track. Otherwise the track has not been modified and
        // both members must remain valid. Do not use std::move() for passing arguments!
        // See https://github.com/mixxxdj/mixxx/issues/12963
        pTrack->replaceRecord(rec);
    }

    connectTracksChanged();

    // Repopulate the dialog and update the UI
    updateFromTracks();
}

void DlgTrackInfoMulti::clear() {
    const QSignalBlocker signalBlocker(this);

    disconnectTracksChanged();
    m_pLoadedTracks.clear();
    m_trackRecords.clear();

    m_pWStarRating->slotSetRating(0);
    trackColorDialogSetColorStyleButton(mixxx::RgbColor::nullopt());
    m_pWCoverArtLabel->loadTrack(TrackPointer());
    m_pWCoverArtLabel->setCoverArt(CoverInfo(), QPixmap());
}

void DlgTrackInfoMulti::connectTracksChanged() {
    for (auto it = m_pLoadedTracks.constBegin();
            it != m_pLoadedTracks.constEnd();
            it++) {
        const auto pTrack = it.value();
        connect(pTrack.get(),
                &Track::changed,
                this,
                &DlgTrackInfoMulti::slotTrackChanged);
    }
}

void DlgTrackInfoMulti::disconnectTracksChanged() {
    for (auto it = m_pLoadedTracks.constBegin();
            it != m_pLoadedTracks.constEnd();
            it++) {
        const auto pTrack = it.value();
        disconnect(pTrack.get(),
                &Track::changed,
                this,
                &DlgTrackInfoMulti::slotTrackChanged);
    }
}

void DlgTrackInfoMulti::slotImportMetadataFromFiles() {
    if (m_pLoadedTracks.isEmpty()) {
        return;
    }
    // Initialize the metadata with the current metadata to avoid
    // losing existing metadata or to lose the beat grid by replacing
    // it with a default grid created from an imprecise BPM.
    // See also: https://github.com/mixxxdj/mixxx/issues/10420
    // In addition we need to preserve all other track properties
    // that are stored in TrackRecord, which serves as the underlying
    // model for this dialog.
    QList<mixxx::TrackRecord> trackRecords;
    trackRecords.reserve(m_pLoadedTracks.size());
    for (auto it = m_pLoadedTracks.constBegin();
            it != m_pLoadedTracks.constEnd();
            it++) {
        const auto pTrack = it.value();
        auto trackRecord = pTrack->getRecord();
        auto trackMetadata = trackRecord.getMetadata();
        QImage coverImage;
        const auto resetMissingTagMetadata = m_pUserSettings->getValue<bool>(
                mixxx::library::prefs::kResetMissingTagMetadataOnImportConfigKey);
        const auto [importResult, sourceSynchronizedAt] =
                SoundSourceProxy(pTrack)
                        .importTrackMetadataAndCoverImage(
                                &trackMetadata, &coverImage, resetMissingTagMetadata);
        if (importResult != mixxx::MetadataSource::ImportResult::Succeeded) {
            // One track failed, abort. User feedback would be good.
            qWarning() << "DlgTrackInfoMulti::slotImportMetadataFromFiles: "
                          "failed to load metadata from file for track"
                       << it.key() << pTrack->getLocation();
            return;
        }
        auto guessedCoverInfo = CoverInfoGuesser().guessCoverInfo(
                pTrack->getFileInfo(),
                trackMetadata.getAlbumInfo().getTitle(),
                coverImage);
        trackRecord.replaceMetadataFromSource(
                std::move(trackMetadata),
                sourceSynchronizedAt);
        trackRecord.setCoverInfo(
                std::move(guessedCoverInfo));
        trackRecords.append(trackRecord);
    }
    replaceTrackRecords(trackRecords);
}

void DlgTrackInfoMulti::slotTrackChanged(TrackId trackId) {
    auto it = m_pLoadedTracks.constFind(trackId);
    if (it != m_pLoadedTracks.constEnd()) {
        updateFromTracks();
    }
}

void DlgTrackInfoMulti::slotColorButtonClicked() {
    if (m_pLoadedTracks.isEmpty()) {
        return;
    }
    btnColorPicker->showMenu();
}

void DlgTrackInfoMulti::slotColorPicked(const mixxx::RgbColor::optional_t& newColor) {
    m_newColor = newColor;
    trackColorDialogSetColorStyleButton(newColor);
}

void DlgTrackInfoMulti::trackColorDialogSetColorStyleButton(
        const mixxx::RgbColor::optional_t& newColor,
        bool variousColors) {
    btnColorPicker->menu()->close();

    QString styleSheet;
    btnColorPicker->setText("");
    if (newColor) {
        const QColor ccolor = mixxx::RgbColor::toQColor(newColor);
        styleSheet = QStringLiteral(
                "QPushButton { background-color: %1; color: %2; }")
                             .arg(ccolor.name(QColor::HexRgb),
                                     Color::isDimColor(ccolor)
                                             ? "white"
                                             : "black");
        btnColorPicker->setText(ccolor.name(QColor::HexRgb));
        m_pColorPicker->setSelectedColor(newColor);
    } else if (variousColors) { // paint a horizontal rainbow gradient
        styleSheet = QStringLiteral(
                "QPushButton {"
                "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                "           stop: 0 #FF0000,"
                "           stop: 0.2 #FFFF00,"
                "           stop: 0.4 #00FF00,"
                "           stop: 0.6 #00FFFF,"
                "           stop: 0.8 #0000FF,"
                "           stop: 1 #FF00FF)}");
        btnColorPicker->setText(kVariousText);
        m_pColorPicker->resetSelectedColor();
    } else { // no color
        btnColorPicker->setText(tr("(no color)"));
        m_pColorPicker->setSelectedColor(newColor);
    }
    btnColorPicker->setStyleSheet(styleSheet);
}

void DlgTrackInfoMulti::slotStarRatingChanged(int rating) {
    if (!m_pLoadedTracks.isEmpty() && mixxx::TrackRecord::isValidRating(rating)) {
        m_starRatingModified = true;
        m_pWStarRating->slotSetRating(rating);
        m_newRating = rating;
    }
}

void DlgTrackInfoMulti::updateCoverArtFromTracks() {
    VERIFY_OR_DEBUG_ASSERT(!m_pLoadedTracks.isEmpty()) {
        return;
    }
    CoverInfoRelative refCover = m_trackRecords.first().getCoverInfo();
    for (auto& rec : m_trackRecords) {
        if (rec.getCoverInfo() != refCover) {
            refCover.reset();
            break;
        }
    }

    TrackPointer pRefTrack = m_pLoadedTracks.cbegin().value();
    // Regardless of cover match we load the reference track. That way,
    // the cover label has a track and location which is required to provide
    // the context menu and allow us to clear or change the cover.
    m_pWCoverArtLabel->loadTrack(pRefTrack);
    if (refCover.hasImage()) {
        // Covers are identical, we could load any track to the cover widget.
        // Just make sure the same track is used in slotCoverFound(): the track
        // location has to match in order to load the cover image to the label.
        auto trCover = pRefTrack->getCoverInfoWithLocation();
        m_pWCoverArtLabel->setCoverArt(trCover, QPixmap());
        CoverArtCache::requestCover(this, trCover);
    } else {
        // Set empty cover + track location
        auto trCover = CoverInfo(CoverInfoRelative(), pRefTrack->getLocation());
        m_pWCoverArtLabel->setCoverArt(trCover, QPixmap());
    }
}

void DlgTrackInfoMulti::slotCoverFound(
        const QObject* pRequester,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap) {
    if (pRequester != this) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(!m_pLoadedTracks.isEmpty()) {
        return;
    }
    // TODO Is this check really necessary? Is it possible that tracks
    // have changed while CoverArtCache was working on our request?
    if (m_pLoadedTracks.cbegin().value()->getLocation() == coverInfo.trackLocation) {
        // Track records have already been updated in slotCoverInfoSelected,
        // now load the image to the label.
        m_pWCoverArtLabel->setCoverArt(coverInfo, pixmap);
    }
}

void DlgTrackInfoMulti::slotCoverInfoSelected(const CoverInfoRelative& coverInfo) {
    VERIFY_OR_DEBUG_ASSERT(!m_pLoadedTracks.isEmpty()) {
        return;
    }
    for (auto& rec : m_trackRecords) {
        rec.setCoverInfo(coverInfo);
    }
    // Covers are now identical, we could load any track to cover widget.
    // Just make sure the same track is used in slotCoverFound(): the track
    // location has to match in order to load the cover image to the label.
    auto pFirstTrack = m_pLoadedTracks.constBegin().value();
    CoverArtCache::requestCover(this, CoverInfo(coverInfo, pFirstTrack->getLocation()));
}

void DlgTrackInfoMulti::slotReloadCoverArt() {
    for (auto& rec : m_trackRecords) {
        auto pTrack = getTrackFromSetById(rec.getId());
        if (pTrack == nullptr) {
            return;
        }
        auto cover = CoverInfoGuesser().guessCoverInfoForTrack(pTrack);
        rec.setCoverInfo(cover);
    }
    updateCoverArtFromTracks();
}
