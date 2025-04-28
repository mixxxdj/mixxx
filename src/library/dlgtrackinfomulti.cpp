#include "library/dlgtrackinfomulti.h"

#include <QLineEdit>
#include <QListView>
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
#include "util/desktophelper.h"
#include "util/duration.h"
#include "util/string.h"
#include "util/stringformat.h"
#include "widget/wcoverartlabel.h"
#include "widget/wcoverartmenu.h"
#include "widget/wstarrating.h"

namespace {

const QString kVariousText = QChar('<') + QObject::tr("various") + QChar('>');
const char* kOrigValProp = "origVal";
const QString kClearItem = QStringLiteral("clearItem");
const QString kClearIconPath = QStringLiteral(":images/library/ic_library_cross_grey.svg");

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

/// Check if the text has been edited
QString validEditText(QComboBox* pBox) {
    // For multi-value boxes we check if there is a placeholder (our 'modified'
    // flag). For single-value boxes we compare with the original value.
    auto* pLine = pBox->lineEdit();
    const QString origVal = pBox->property(kOrigValProp).toString();
    const QString currText = pLine->text();
    if ((pBox->count() > 0 && !pLine->placeholderText().isNull()) ||
            (pBox->count() == 0 && currText == origVal)) {
        return QString();
    }
    // We have a new text.
    // Remove trailing whitespaces. Keep Leading whitespaces to be consistent
    // with the track table's inline editor.
    return mixxx::removeTrailingWhitespaces(currText);
}

/// Sets the text of a QLabel, either the only/common value or the 'various' string.
/// In case of `various`, the text is also set italic.
/// This is used for bitrate, sample rate and file directories.
/// Optionally toggle bold (bitrate and sample rate).
template<typename T>
void setCommonValueOrVariousStringAndFormatFont(QLabel* pLabel,
        QSet<T>& values,
        bool toggleBold = false,
        const QString& unit = QString()) {
    if (values.size() == 1) {
        QString text = convertToQStringConvertible(*values.constBegin());
        if (text.isNull()) {
            pLabel->clear();
            return;
        }
        if (!unit.isEmpty()) {
            text.append(QChar(' ') + unit);
        }
        pLabel->setText(text);
        setItalic(pLabel, false);
        if (toggleBold) {
            setBold(pLabel, true);
        }
    } else {
        pLabel->setText(kVariousText);
        setItalic(pLabel, true);
        if (toggleBold) {
            setBold(pLabel, false);
        }
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
    m_propertyWidgets.insert("tracknumber", txtTrackNumber);
    m_propertyWidgets.insert("key", txtKey);
    m_propertyWidgets.insert("grouping", txtGrouping);
    m_propertyWidgets.insert("comment", txtComment);

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

    connect(btnOpenFileBrowser,
            &QPushButton::clicked,
            this,
            &DlgTrackInfoMulti::slotOpenInFileBrowser);

    QList<QComboBox*> valueComboBoxes;
    valueComboBoxes.append(txtArtist);
    valueComboBoxes.append(txtTitle);
    valueComboBoxes.append(txtAlbum);
    valueComboBoxes.append(txtAlbumArtist);
    valueComboBoxes.append(txtComposer);
    valueComboBoxes.append(txtGenre);
    valueComboBoxes.append(txtYear);
    valueComboBoxes.append(txtKey);
    valueComboBoxes.append(txtTrackNumber);
    valueComboBoxes.append(txtGrouping);

    for (QComboBox* pBox : valueComboBoxes) {
        // This sets/enables the QLineEdit.
        pBox->setEditable(true);
        // We allow editing but we don't want to add each edit to the item list.
        pBox->setInsertPolicy(QComboBox::NoInsert);
        // Avoid showing scrollbars if not needed. The dialog has at least 17
        // QLineEdit/QLabel/QPushButton rows + layout spacing + title bar, so
        // even with very tight Qt themes we can show at least 25 items before
        // the list gets taller than the dialog.
        pBox->setMaxVisibleItems(25);

        connect(pBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &DlgTrackInfoMulti::slotTagBoxIndexChanged);

        auto* pLine = pBox->lineEdit();
        // editingFinished() is also emitted when the list view is opened.
        connect(pLine,
                &QLineEdit::editingFinished,
                this,
                [this, pBox, pLine]() {
                    slotEditingFinished(pBox, pLine);
                });
    }
    // Note: unlike other tags, comments can be multi-line, though while QComboBox
    // can have multi-line items its Q*Line*Edit is not suitable for editing multi-
    // line content. In order to get the same UX for comments like we have for
    // regular tags, the two buddies require a special setup:
    // * txtCommentBox is not editable
    // * if an item is selected in txtCommentBox, the text is shown in txtComment
    // * for multiple values, we show the <various> placeholder in txtComment
    // This also requires some special handling in saveTracks().
    txtCommentBox->setInsertPolicy(QComboBox::NoInsert);
    // We create a view in order to enable word-wrap.
    auto* pView = new QListView();
    pView->setWordWrap(true);
    // Even though we enabled word-wrap, and even if we'd set the view's max width,
    // the view (actually its container) would still expand wider than that for
    // long lines. To work around that we manually set the max width for that, too.
    // We do this in resizeEvent().
    txtCommentBox->setView(pView);
    txtCommentBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(txtCommentBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgTrackInfoMulti::slotCommentBoxIndexChanged);
    // There is no editingFinished() signal for QPlaintextEdit that would allow
    // catching text changes. Listen for focusOut event instead.
    txtComment->installEventFilter(this);

    // Set up key validation, i.e. check manually entered key texts
    // Note: this is also triggered if the popup is opened.
    connect(txtKey->lineEdit(),
            &QLineEdit::editingFinished,
            this,
            &DlgTrackInfoMulti::slotKeyTextChanged);

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
    accept();
}

void DlgTrackInfoMulti::slotCancel() {
    reject();
}

void DlgTrackInfoMulti::loadTracks(const QList<TrackPointer>& pTracks) {
    if (pTracks.isEmpty()) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(m_pLoadedTracks.isEmpty()) {
        m_pLoadedTracks.clear();
    }
    for (const auto& pTrack : pTracks) {
        m_pLoadedTracks.insert(pTrack.get()->getId(), pTrack);
    }

    updateFromTracks();

    // Listen to all tracks' changed() signal so we don't need to listen to
    // individual signals such as cuesUpdates, coverArtUpdated(), etc.
    connectTracksChanged();
}

void DlgTrackInfoMulti::focusField(const QString& property) {
    if (property.isEmpty()) {
        return;
    }
    auto it = m_propertyWidgets.constFind(property);
    if (it != m_propertyWidgets.constEnd()) {
        it.value()->setFocus();
    }
}

void DlgTrackInfoMulti::updateFromTracks() {
    const QSignalBlocker signalBlocker(this);

    QList<mixxx::TrackRecord> trackRecords;
    trackRecords.reserve(m_pLoadedTracks.size());
    for (const auto& pTrack : std::as_const(m_pLoadedTracks)) {
        const auto rec = pTrack.get()->getRecord();
        trackRecords.append(rec);
    }
    replaceTrackRecords(trackRecords);

    // Collect star ratings and track colors
    // If track value differs from the current value, add it to the list.
    // If new and current are identical, keep only one.
    int commonRating = m_trackRecords.first().getRating();
    for (const auto& rec : std::as_const(m_trackRecords)) {
        if (commonRating != rec.getRating()) {
            commonRating = 0;
            break;
        }
    }
    // Update the star widget
    m_pWStarRating->slotSetRating(commonRating);
    m_starRatingModified = false;

    // Same procedure for the track color
    mixxx::RgbColor::optional_t commonColor = m_trackRecords.first().getColor();
    bool multipleColors = false;
    for (const auto& rec : std::as_const(m_trackRecords)) {
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
    QSet<QString> dirs;
    QString firstDir = m_pLoadedTracks.constBegin().value()->getFileInfo().canonicalLocationPath();
    dirs.insert(firstDir);
    for (const auto& pTrack : std::as_const(m_pLoadedTracks)) {
        const auto dir = pTrack->getFileInfo().canonicalLocationPath();
        if (dir != firstDir) {
            dirs.insert(dir);
            break;
        }
    }
    setCommonValueOrVariousStringAndFormatFont(txtLocation, dirs);
    // Enable the button if all tracks are in the same directory.
    btnOpenFileBrowser->setEnabled(dirs.size() == 1 ? true : false);

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
    QSet<QString> titles;
    QSet<QString> artists;
    QSet<QString> aTitles;
    QSet<QString> aArtists;
    QSet<QString> genres;
    QSet<QString> composers;
    QSet<QString> grouping;
    QSet<QString> years;
    QSet<QString> keys;
    QSet<QString> nums;
    QSet<QString> comments;
    QSet<double> bpms;
    QSet<uint32_t> bitrates;
    QSet<double> durations;
    QSet<uint32_t> samplerates;
    QSet<QString> filetypes;

    for (const auto& rec : std::as_const(m_trackRecords)) {
        titles.insert(rec.getMetadata().getTrackInfo().getTitle());
        artists.insert(rec.getMetadata().getTrackInfo().getArtist());
        aTitles.insert(rec.getMetadata().getAlbumInfo().getTitle());
        aArtists.insert(rec.getMetadata().getAlbumInfo().getArtist());
        genres.insert(rec.getMetadata().getTrackInfo().getGenre());
        composers.insert(rec.getMetadata().getTrackInfo().getComposer());
        grouping.insert(rec.getMetadata().getTrackInfo().getGrouping());
        years.insert(rec.getMetadata().getTrackInfo().getYear());
        auto pTrack = getTrackFromSetById(rec.getId());
        DEBUG_ASSERT(pTrack);
        keys.insert(KeyUtils::keyFromKeyTextAndIdValues(
                pTrack->getKeyText(),
                pTrack->getKey()));
        nums.insert(rec.getMetadata().getTrackInfo().getTrackNumber());
        comments.insert(rec.getMetadata().getTrackInfo().getComment());

        auto bpm = rec.getMetadata().getTrackInfo().getBpm();
        bpms.insert(bpm.isValid() ? bpm.value() : mixxx::Bpm::kValueMin);

        auto bitrate = rec.getMetadata().getStreamInfo().getBitrate();
        bitrates.insert(bitrate.isValid() ? bitrate.value() : 0);

        durations.insert(rec.getMetadata().getDurationSecondsRounded());

        auto samplerate = rec.getMetadata().getStreamInfo().getSignalInfo().getSampleRate();
        samplerates.insert(samplerate.isValid() ? samplerate.value() : 0);

        filetypes.insert(rec.getFileType());
    }

    addValuesToComboBox(txtTitle, titles);
    addValuesToComboBox(txtArtist, artists);
    addValuesToComboBox(txtAlbum, aTitles);
    addValuesToComboBox(txtAlbumArtist, aArtists);
    addValuesToComboBox(txtGenre, genres);
    addValuesToComboBox(txtComposer, composers);
    addValuesToComboBox(txtGrouping, grouping);
    addValuesToComboBox(txtYear, years, true);
    // temporarily disable key validation
    txtKey->blockSignals(true);
    addValuesToComboBox(txtKey, keys, true);
    txtKey->blockSignals(false);
    addValuesToComboBox(txtTrackNumber, nums, true);

    // The comment tag is special: it's the only one that may have multiple lines,
    // but we can't have a multi-line editor and a combobox at the same time.
    addValuesToCommentBox(comments);

    // Non-editable fields: BPM, bitrate, samplerate, type and directory
    // For BPM, bitrate and samplerate we show a span if we have multiple values.
    // TODO Add BPM line edit
    if (bpms.size() > 1) {
        QList<double> bpmList = bpms.values();
        std::sort(bpmList.begin(), bpmList.end());
        txtBpm->setText(QString("%1").arg(bpmList.first(), 3, 'f', 1) +
                QChar('-') +
                QString("%1").arg(bpmList.last(), 3, 'f', 1));
    } else { // we have at least one value, might be invalid (0)
        double bpm = *bpms.constBegin();
        if (bpm == mixxx::Bpm::kValueMin) {
            txtBpm->clear();
        } else {
            txtBpm->setText(QString::number(bpm));
        }
    }

    QString bitrate;
    if (bitrates.size() > 1) {
        QList<uint32_t> brList = bitrates.values();
        std::sort(brList.begin(), brList.end());
        bitrate = QString::number(brList.first()) +
                QChar('-') +
                QString::number(brList.last());
    } else { // we have at least one value, though 0 is not necessarily invalid
        bitrate = QString::number(*bitrates.constBegin());
    }
    txtBitrate->setText(bitrate + QChar(' ') + mixxx::audio::Bitrate::unit());

    setCommonValueOrVariousStringAndFormatFont(txtSamplerate,
            samplerates,
            true, // bold if common value
            QStringLiteral("Hz"));

    setCommonValueOrVariousStringAndFormatFont(txtType, filetypes, true);

    if (durations.size() > 1) {
        QList<double> durList = durations.values();
        std::sort(durList.begin(), durList.end());
        txtDuration->setText(mixxx::Duration::formatTime(durList.first()) +
                QChar('-') +
                mixxx::Duration::formatTime(durList.last()));
    } else {
        txtDuration->setText(mixxx::Duration::formatTime(*durations.constBegin()));
    }
}

template<typename T>
void DlgTrackInfoMulti::addValuesToComboBox(QComboBox* pBox, QSet<T>& values, bool sort) {
    // Verify that T can be used for QComboBox::addItem()
    DEBUG_ASSERT(isOrCanConvertToQString(*values.constBegin()));

    pBox->blockSignals(true);
    pBox->clear();
    pBox->lineEdit()->setPlaceholderText(QString());

    VERIFY_OR_DEBUG_ASSERT(!values.isEmpty()) {
        pBox->setProperty(kOrigValProp, QString());
        pBox->blockSignals(false);
        return;
    }

    if (values.size() == 1) {
        pBox->setCurrentText(*values.constBegin());
        pBox->setProperty(kOrigValProp, *values.constBegin());
        pBox->removeEventFilter(this);
    } else {
        // Remove empty items. For explicit clearing we'll add the Clear item.
        // QSet doesn't hold duplicates so we need to do this only once.
        // This doesn't remove items with multiple spaces though.
        values.remove("");
        pBox->addItems(values.values());
        if (sort) {
            pBox->model()->sort(0);
        }
        // After sorting add the Clear item to allow to clearing the tag for all tracks.
        pBox->insertItem(0,
                QIcon(kClearIconPath),
                QString(),
                kClearItem);
        pBox->setCurrentIndex(-1);
        // Show '<various>' placeholder.
        // The QComboBox::lineEdit() placeholder actually providex a nice UX:
        // it's displayed with a dim color and it persists until new text is
        // entered. However, this prevents clearing the tag by clearing the
        // current text manually.
        //
        // This also acts as 'modified' flag. It'll be removed when the Clear item
        // or a text/number item is selected, or (maybe) in slotEditingFinished().
        pBox->lineEdit()->setPlaceholderText(kVariousText);
        pBox->setProperty(kOrigValProp, kVariousText);
        pBox->installEventFilter(this);
    }
    pBox->blockSignals(false);
}

void DlgTrackInfoMulti::addValuesToCommentBox(QSet<QString>& comments) {
    txtComment->clear();
    txtCommentBox->clear();
    txtComment->setPlaceholderText(QString());

    VERIFY_OR_DEBUG_ASSERT(!comments.isEmpty()) {
        txtCommentBox->setProperty(kOrigValProp, QString());
        return;
    }

    txtCommentBox->blockSignals(true);
    txtComment->blockSignals(true);
    if (comments.size() == 1) {
        txtCommentBox->setEnabled(false);
        txtComment->setPlainText(*comments.constBegin());
        txtComment->setProperty(kOrigValProp, *comments.constBegin());
        txtComment->removeEventFilter(this);
    } else {
        txtCommentBox->setEnabled(true);
        // The empty item allows to clear the text for all tracks.
        // Nice to have: make the text italic
        txtCommentBox->addItem(
                QIcon(kClearIconPath),
                QString(),
                kClearItem);
        txtCommentBox->addItems(comments.values());
        txtCommentBox->setCurrentIndex(-1);
        // Show '<various>' placeholder.
        // As long as the placeholder exists we know the text has not been changed.
        // It'll be removed when the Clear item or a text item is selected,
        // or (maybe) when we respond to the focusOut event of the editor.
        // Nice to have: set the <various> text also for the combobox, but since
        // editing is disabled that's not possible (only by subclassing
        // QComboBox and overriding the paintEvent(), or maybe other hacks).
        txtComment->setPlaceholderText(kVariousText);
        // For some reason we need to clear the box again in order to show
        // the placeholder text.
        txtComment->clear();
        txtComment->setProperty(kOrigValProp, kVariousText);
        txtComment->installEventFilter(this);
    }
    txtCommentBox->blockSignals(false);
    txtComment->blockSignals(false);
}

void DlgTrackInfoMulti::resizeEvent(QResizeEvent* pEvent) {
    Q_UNUSED(pEvent);
    if (!isVisible()) {
        // Likely one of the resize events before show().
        // Dialog & widgets don't have their final size, yet,
        // so it makes no sense to resize the cover label.
        return;
    }

    // Limit comment popup to dialog width. This may introduce some linebreaks
    // but is still much better than letting the popup expand to screen width,
    // which it would do regrardless if it's actually necessary.
    txtCommentBox->view()->parentWidget()->setMaximumWidth(width());

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

void DlgTrackInfoMulti::saveTracks() {
    if (m_pLoadedTracks.isEmpty()) {
        return;
    }

    // In case Apply is triggered by hotkey AND user did not yet hit Enter to
    // finish editing, we might have an editor with pending changes.
    QComboBox* pFocusedBox = qobject_cast<QComboBox*>(QApplication::focusWidget());
    if (pFocusedBox) {
        auto* pLine = pFocusedBox->lineEdit();
        slotEditingFinished(pFocusedBox, pLine);
    }
    // Same for the comment editor, see comment below.
    if (txtComment->hasFocus()) {
        commentTextChanged();
    }

    // Check the values so we don't have to do it for every track record
    const QString title = validEditText(txtTitle);
    const QString artist = validEditText(txtArtist);
    const QString album = validEditText(txtAlbum);
    const QString albumArtist = validEditText(txtAlbumArtist);
    const QString genre = validEditText(txtGenre);
    const QString composer = validEditText(txtComposer);
    const QString grouping = validEditText(txtGrouping);
    const QString year = validEditText(txtYear);
    const QString key = validEditText(txtKey);
    const QString num = validEditText(txtTrackNumber);
    // Check if the Comment has been changed.
    // (same as in validEditText(), just for the QPlainTextEdit)
    QString comment;
    const QString origText = txtComment->property(kOrigValProp).toString();
    const QString currText = txtComment->toPlainText();
    if ((txtCommentBox->count() > 0 && txtComment->placeholderText().isNull()) ||
            (txtCommentBox->count() == 0 && currText != origText)) {
        // Remove trailing whitespaces.
        comment = mixxx::removeTrailingWhitespaces(currText);
    }

    for (auto& rec : m_trackRecords) {
        if (!title.isNull()) {
            rec.refMetadata().refTrackInfo().setTitle(title);
        }
        if (!artist.isNull()) {
            rec.refMetadata().refTrackInfo().setArtist(artist);
        }
        if (!album.isNull()) {
            rec.refMetadata().refAlbumInfo().setTitle(album);
        }
        if (!albumArtist.isNull()) {
            rec.refMetadata().refAlbumInfo().setArtist(albumArtist);
        }
        if (!genre.isNull()) {
            rec.refMetadata().refTrackInfo().setGenre(genre);
        }
        if (!composer.isNull()) {
            rec.refMetadata().refTrackInfo().setComposer(composer);
        }
        if (!grouping.isNull()) {
            rec.refMetadata().refTrackInfo().setGrouping(grouping);
        }
        if (!year.isNull()) {
            rec.refMetadata().refTrackInfo().setYear(year);
        }
        if (!key.isNull()) {
            rec.updateGlobalKeyNormalizeText(
                    key,
                    mixxx::track::io::key::USER);
        }
        if (!num.isNull()) {
            rec.refMetadata().refTrackInfo().setTrackNumber(num);
        }
        if (!comment.isNull()) {
            rec.refMetadata().refTrackInfo().setComment(comment);
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
    for (const auto& rec : std::as_const(m_trackRecords)) {
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

void DlgTrackInfoMulti::connectTracksChanged() {
    for (const auto& pTrack : std::as_const(m_pLoadedTracks)) {
        connect(pTrack.get(),
                &Track::changed,
                this,
                &DlgTrackInfoMulti::slotTrackChanged);
    }
}

void DlgTrackInfoMulti::disconnectTracksChanged() {
    for (const auto& pTrack : std::as_const(m_pLoadedTracks)) {
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
    for (const auto& pTrack : std::as_const(m_pLoadedTracks)) {
        auto trackRecord = pTrack->getRecord();
        auto trackMetadata = trackRecord.getMetadata();
        QImage coverImage;
        const auto resetMissingTagMetadata = m_pUserSettings->getValue<bool>(
                mixxx::library::prefs::kResetMissingTagMetadataOnImportConfigKey);
        const auto [importResult, sourceSynchronizedAt] =
                SoundSourceProxy(pTrack)
                        .importTrackMetadataAndCoverImage(
                                &trackMetadata, &coverImage, resetMissingTagMetadata);
        // Skip tracks that don't have metadata (Unavailable) or where retrieval failed
        // for other reasons (Failed). For fails user feedback would be good.
        if (importResult != mixxx::MetadataSource::ImportResult::Succeeded) {
            if (importResult == mixxx::MetadataSource::ImportResult::Failed) {
                qWarning() << "DlgTrackInfoMulti::slotImportMetadataFromFiles: "
                              "failed to load metadata from file for track"
                           << pTrack->getId() << pTrack->getLocation();
                // TODO Collect failed files and show a popup.
            }
            continue;
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

void DlgTrackInfoMulti::slotOpenInFileBrowser() {
    if (m_pLoadedTracks.isEmpty()) {
        return;
    }
    auto pFirstTrack = m_pLoadedTracks.constBegin().value();
    mixxx::DesktopHelper::openInFileBrowser(QStringList(pFirstTrack->getLocation()));
}

void DlgTrackInfoMulti::slotTrackChanged(TrackId trackId) {
    auto it = m_pLoadedTracks.constFind(trackId);
    if (it != m_pLoadedTracks.constEnd()) {
        updateFromTracks();
    }
}

void DlgTrackInfoMulti::slotTagBoxIndexChanged() {
    QComboBox* pBox = qobject_cast<QComboBox*>(sender());
    VERIFY_OR_DEBUG_ASSERT(pBox && pBox != txtCommentBox) {
        return;
    }

    pBox->blockSignals(true); // Prevent recursive calls
    // If we have multiple values we also added the Clear item.
    // If that item has been selected, remove the placeholder in order to have a
    // somewhat safe indicator for whether the box has been edited.
    // Used in validEditText().
    auto data = pBox->currentData(Qt::UserRole);
    if (data.isValid()) {
        if (data.toString() == kClearItem) {
            updateTagPlaceholder(pBox, true);
            pBox->blockSignals(true); // Prevent recursive calls
            pBox->setCurrentIndex(-1);
        }
    } else {
        // If another item has been selected we enable the Clear and Reset items
        updateTagPlaceholder(pBox, true);
    }
    if (pBox == txtKey) {
        // Since we've blocked change signals we need to trigger
        // the key validation manually.
        slotKeyTextChanged();
    }
    pBox->blockSignals(false);
}

void DlgTrackInfoMulti::slotEditingFinished(QComboBox* pBox, QLineEdit* pLine) {
    if (pBox->count() == 0 || pBox == txtKey) {
        return;
    }
    updateTagPlaceholder(pBox, !pLine->text().isEmpty());
}

void DlgTrackInfoMulti::slotCommentBoxIndexChanged() {
    QComboBox* pBox = qobject_cast<QComboBox*>(sender());
    VERIFY_OR_DEBUG_ASSERT(pBox && pBox == txtCommentBox) {
        return;
    }

    txtCommentBox->blockSignals(true);
    txtComment->blockSignals(true);
    txtComment->setPlaceholderText(QString());
    // If we have multiple value we also added the Clear All item.
    // If the Clear item has been selected, remove the placeholder
    // in order to have a safe indicator in validEditText() whether
    // the box has been edited.
    auto data = txtCommentBox->currentData(Qt::UserRole);
    if (data.isValid() && data.toString() == kClearItem) {
        txtCommentBox->setCurrentIndex(-1); // This clears the combobox selection.
        txtComment->clear();
        updateCommentPlaceholder(true);
    } else {
        txtComment->setPlainText(txtCommentBox->currentText());
        updateCommentPlaceholder(true);
    }
    txtCommentBox->blockSignals(false);
    txtComment->blockSignals(false);
}

void DlgTrackInfoMulti::commentTextChanged() {
    if (!txtComment->placeholderText().isNull() &&
            !txtComment->toPlainText().isEmpty()) {
        // The combobox has multiple values and has not been cleared yet,
        // and the user typed/pasted a text (might be a whitespace).
        // Let's clear the placeholder text so we know this is new text.
        txtCommentBox->blockSignals(true);
        txtComment->setPlaceholderText(QString());
        // The Clear item is not needed anymore, so remove it.
        txtCommentBox->blockSignals(false);
    }
}

bool DlgTrackInfoMulti::eventFilter(QObject* pObj, QEvent* pEvent) {
    // Del/Backspace in empty edit field switches between
    // empty and <various> (original values)
    // The Clear and Reset items are enabled/disabled accordingly
    QComboBox* pBox = nullptr;
    if (pEvent->type() == QEvent::KeyPress &&
            (pObj == txtComment || (pBox = qobject_cast<QComboBox*>(pObj)))) {
        QKeyEvent* ke = static_cast<QKeyEvent*>(pEvent);
        if (ke->key() == Qt::Key_Backspace || ke->key() == Qt::Key_Delete) {
            // We don't care about modifiers since we act only if the box is empty
            // TODO move item operations to separate function.
            // Also update items when text changes:
            if (pBox && pBox->lineEdit()->text().isEmpty()) {
                bool dirty = !pBox->lineEdit()->placeholderText().isEmpty();
                updateTagPlaceholder(pBox, dirty);
            } else if (pObj == txtComment && txtComment->toPlainText().isEmpty()) {
                txtCommentBox->setCurrentIndex(-1);
                updateCommentPlaceholder(!txtComment->placeholderText().isEmpty());
            }
        }
    } else if (pEvent->type() == QEvent::FocusOut && pObj == txtComment) {
        commentTextChanged();
    }
    return QDialog::eventFilter(pObj, pEvent);
}

void DlgTrackInfoMulti::updateTagPlaceholder(QComboBox* pBox, bool dirty) {
    if (pBox->count() == 0) {
        return;
    }
    pBox->blockSignals(true);
    if (dirty) {
        pBox->lineEdit()->setPlaceholderText(QString());
    } else {
        pBox->lineEdit()->setPlaceholderText(kVariousText);
    }
    pBox->blockSignals(false);
}

void DlgTrackInfoMulti::updateCommentPlaceholder(bool dirty) {
    txtComment->blockSignals(true);
    if (dirty) {
        txtComment->setPlaceholderText(QString());
    } else {
        txtComment->setPlaceholderText(kVariousText);
    }
    txtComment->blockSignals(false);
}

void DlgTrackInfoMulti::slotKeyTextChanged() {
    // editingFinished() is also emitted when the popup is opened.
    // No need to validate in that case.
    if (txtKey->view()->isVisible()) {
        return;
    }

    const QString newTextInput = txtKey->currentText().trimmed();
    QString newKeyText;
    // Empty text is not a valid key but indicates we want to clear the key.
    if (!newTextInput.isEmpty()) {
        mixxx::track::io::key::ChromaticKey newKey =
                KeyUtils::guessKeyFromText(newTextInput);
        if (newKey != mixxx::track::io::key::INVALID) {
            newKeyText = KeyUtils::keyToString(newKey);
        }
    }

    txtKey->blockSignals(true);
    if (newKeyText.isEmpty()) {
        // Revert if we can't guess a valid key from it.
        if (txtKey->count() > 0) {
            // This is a multi-value box and the key has not yet been cleared manually.
            // Just clear the text to restore <various>.
            txtKey->clearEditText();
        } else {
            // This is a single-value box. Restore the original key text.
            const QString origKeyStr = txtKey->property(kOrigValProp).toString();
            txtKey->setCurrentText(origKeyStr);
        }
    } else {
        txtKey->setCurrentText(newKeyText);
    }
    txtKey->blockSignals(false);
}

void DlgTrackInfoMulti::slotColorButtonClicked() {
    if (m_pLoadedTracks.isEmpty()) {
        return;
    }
    btnColorPicker->showMenu();
}

void DlgTrackInfoMulti::slotColorPicked(const mixxx::RgbColor::optional_t& newColor) {
    m_colorChanged = true;
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
    for (const auto& rec : std::as_const(m_trackRecords)) {
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
        m_pWCoverArtLabel->setCoverInfoAndPixmap(trCover, QPixmap());
        CoverArtCache::requestCover(this, trCover);
    } else {
        // Set empty cover + track location
        auto trCover = CoverInfo(CoverInfoRelative(), pRefTrack->getLocation());
        m_pWCoverArtLabel->setCoverInfoAndPixmap(trCover, QPixmap());
    }
}

void DlgTrackInfoMulti::slotCoverFound(
        const QObject* pRequester,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap) {
    if (pRequester == this &&
            !m_pLoadedTracks.isEmpty() &&
            m_pLoadedTracks.cbegin().value()->getLocation() == coverInfo.trackLocation) {
        // Track records have already been updated in slotCoverInfoSelected,
        // now load the image to the label.
        m_pWCoverArtLabel->setCoverInfoAndPixmap(coverInfo, pixmap);
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
