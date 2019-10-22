#include <QDesktopServices>
#include <QtDebug>
#include <QStringBuilder>
#include <QComboBox>

#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "library/dlgtrackinfo.h"
#include "sources/soundsourceproxy.h"
#include "track/beatfactory.h"
#include "track/cue.h"
#include "track/keyfactory.h"
#include "track/keyutils.h"
#include "util/color/color.h"
#include "util/compatibility.h"
#include "util/desktophelper.h"
#include "util/duration.h"

const int kFilterLength = 80;
const int kMinBpm = 30;

// Maximum allowed interval between beats (calculated from kMinBpm).
const mixxx::Duration kMaxInterval = mixxx::Duration::fromMillis(1000.0 * (60.0 / kMinBpm));

DlgTrackInfo::DlgTrackInfo(QWidget* parent)
            : QDialog(parent),
              m_pTapFilter(new TapFilter(this, kFilterLength, kMaxInterval)),
              m_dLastTapedBpm(-1.),
              m_pWCoverArtLabel(new WCoverArtLabel(this)) {
    init();
}

DlgTrackInfo::~DlgTrackInfo() {
    unloadTrack(false);
}

void DlgTrackInfo::init() {
    setupUi(this);

    cueTable->hideColumn(0);
    coverBox->insertWidget(1, m_pWCoverArtLabel);

    connect(btnNext, &QPushButton::clicked, this, &DlgTrackInfo::slotNext);
    connect(btnPrev, &QPushButton::clicked, this, &DlgTrackInfo::slotPrev);
    connect(btnApply, &QPushButton::clicked, this, &DlgTrackInfo::apply);
    connect(btnOK, &QPushButton::clicked, this, &DlgTrackInfo::OK);
    connect(btnCancel, &QPushButton::clicked, this, &DlgTrackInfo::cancel);

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

    connect(btnCueActivate,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::cueActivate);
    connect(btnCueDelete,
            &QPushButton::clicked,
            this,
            &DlgTrackInfo::cueDelete);
    connect(bpmTap,
            &QPushButton::pressed,
            m_pTapFilter.data(),
            &TapFilter::tap);
    connect(m_pTapFilter.data(),
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
    if (pCache != nullptr) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &DlgTrackInfo::slotCoverFound);
    }
    connect(m_pWCoverArtLabel,
            &WCoverArtLabel::coverInfoSelected,
            this,
            &DlgTrackInfo::slotCoverInfoSelected);
    connect(m_pWCoverArtLabel,
            &WCoverArtLabel::reloadCoverArt,
            this,
            &DlgTrackInfo::slotReloadCoverArt);
}

void DlgTrackInfo::OK() {
    unloadTrack(true);
    accept();
}

void DlgTrackInfo::apply() {
    saveTrack();
}

void DlgTrackInfo::cancel() {
    unloadTrack(false);
    reject();
}

void DlgTrackInfo::trackUpdated() {

}

void DlgTrackInfo::slotNext() {
    emit(next());
}

void DlgTrackInfo::slotPrev() {
    emit(previous());
}

void DlgTrackInfo::cueActivate() {

}

void DlgTrackInfo::cueDelete() {
    QList<QTableWidgetItem*> selected = cueTable->selectedItems();
    QListIterator<QTableWidgetItem*> item_it(selected);

    QSet<int> rowsToDelete;
    while(item_it.hasNext()) {
        QTableWidgetItem* item = item_it.next();
        rowsToDelete.insert(item->row());
    }

    QList<int> rowsList = QList<int>::fromSet(rowsToDelete);
    std::sort(rowsList.begin(), rowsList.end());

    QListIterator<int> it(rowsList);
    it.toBack();
    while (it.hasPrevious()) {
        cueTable->removeRow(it.previous());
    }
}

void DlgTrackInfo::populateFields(const Track& track) {
    setWindowTitle(track.getArtist() % " - " % track.getTitle());

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
    txtLocation->setText(QDir::toNativeSeparators(track.getLocation()));
    txtType->setText(track.getType());
    txtBitrate->setText(QString(track.getBitrateText()) + (" ") + tr(mixxx::AudioSource::Bitrate::unit()));
    txtBpm->setText(track.getBpmText());
    m_keysClone = track.getKeys();
    txtKey->setText(KeyUtils::getGlobalKeyText(m_keysClone));
    const mixxx::ReplayGain replayGain(track.getReplayGain());
    txtReplayGain->setText(mixxx::ReplayGain::ratioToString(replayGain.getRatio()));

    reloadTrackBeats(track);

    m_loadedCoverInfo = track.getCoverInfoWithLocation();
    m_pWCoverArtLabel->setCoverArt(m_loadedCoverInfo, QPixmap());
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache != NULL) {
        pCache->requestCover(m_loadedCoverInfo, this, 0, false, true);
    }
}

void DlgTrackInfo::reloadTrackBeats(const Track& track) {
    BeatsPointer pBeats = track.getBeats();
    if (pBeats) {
        spinBpm->setValue(pBeats->getBpm());
        m_pBeatsClone = pBeats->clone();
    } else {
        m_pBeatsClone.clear();
        spinBpm->setValue(0.0);
    }
    m_trackHasBeatMap = pBeats && !(pBeats->getCapabilities() & Beats::BEATSCAP_SETBPM);
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

void DlgTrackInfo::loadTrack(TrackPointer pTrack) {
    clear();

    if (!pTrack) {
        return;
    }

    m_pLoadedTrack = pTrack;

    populateFields(*m_pLoadedTrack);
    populateCues(m_pLoadedTrack);
    m_pWCoverArtLabel->loadTrack(m_pLoadedTrack);

    // We already listen to changed() so we don't need to listen to individual
    // signals such as cuesUpdates, coverArtUpdated(), etc.
    connect(pTrack.get(),
            &Track::changed,
            this,
            &DlgTrackInfo::updateTrackMetadata);
}

void DlgTrackInfo::slotCoverFound(const QObject* pRequestor,
                                  const CoverInfoRelative& info,
                                  QPixmap pixmap, bool fromCache) {
    Q_UNUSED(fromCache);
    if (pRequestor == this && m_pLoadedTrack &&
            m_loadedCoverInfo.hash == info.hash) {
        qDebug() << "DlgTrackInfo::slotPixmapFound" << pRequestor << info
                 << pixmap.size();
        m_pWCoverArtLabel->setCoverArt(m_loadedCoverInfo, pixmap);
    }
}

void DlgTrackInfo::slotReloadCoverArt() {
    VERIFY_OR_DEBUG_ASSERT(m_pLoadedTrack) {
        return;
    }
    CoverInfo coverInfo =
            CoverArtUtils::guessCoverInfo(*m_pLoadedTrack);
    slotCoverInfoSelected(coverInfo);
}

void DlgTrackInfo::slotCoverInfoSelected(const CoverInfoRelative& coverInfo) {
    qDebug() << "DlgTrackInfo::slotCoverInfoSelected" << coverInfo;
    VERIFY_OR_DEBUG_ASSERT(m_pLoadedTrack) {
        return;
    }
    m_loadedCoverInfo = CoverInfo(coverInfo, m_pLoadedTrack->getLocation());
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        pCache->requestCover(m_loadedCoverInfo, this, 0, false, true);
    }
}

void DlgTrackInfo::slotOpenInFileBrowser() {
    if (!m_pLoadedTrack) {
        return;
    }

    mixxx::DesktopHelper::openInFileBrowser(QStringList(m_pLoadedTrack->getLocation()));
}

void DlgTrackInfo::populateCues(TrackPointer pTrack) {
    int sampleRate = pTrack->getSampleRate();

    QList<CuePointer> listPoints;
    const QList<CuePointer> cuePoints = pTrack->getCuePoints();
    QListIterator<CuePointer> it(cuePoints);
    while (it.hasNext()) {
        CuePointer pCue = it.next();
        Cue::CueType type = pCue->getType();
        if (type == Cue::CUE || type == Cue::LOAD || type == Cue::INTRO
                || type == Cue::OUTRO) {
            listPoints.push_back(pCue);
        }
    }
    it = QListIterator<CuePointer>(listPoints);
    cueTable->setSortingEnabled(false);
    int row = 0;

    while (it.hasNext()) {
        CuePointer pCue(it.next());

        QString rowStr = QString("%1").arg(row);

        // All hotcues are stored in Cue's as 0-indexed, but the GUI presents
        // them to the user as 1-indexex. Add 1 here. rryan 9/2010
        int iHotcue = pCue->getHotCue() + 1;
        QString hotcue = "";
        hotcue = QString("%1").arg(iHotcue);
        double position = pCue->getPosition();
        if (position == -1) {
            continue;
        }

        double totalSeconds = position / sampleRate / 2.0;

        bool negative = false;
        if (totalSeconds < 0) {
            totalSeconds *= -1;
            negative = true;
        }

        int iTotalSeconds = static_cast<int>(totalSeconds);
        int fraction = 100 * (totalSeconds - iTotalSeconds);
        int seconds = iTotalSeconds % 60;
        int mins = iTotalSeconds / 60;
        //int hours = mins / 60; //Not going to worry about this for now. :)

        //Construct a nicely formatted duration string now.
        QString duration = QString("%1%2:%3.%4").arg(
            negative ? QString("-") : QString(),
            QString::number(mins),
            QString("%1").arg(seconds, 2, 10, QChar('0')),
            QString("%1").arg(fraction, 2, 10, QChar('0')));

        QTableWidgetItem* durationItem = new QTableWidgetItem(duration);
        // Make the duration read only
        durationItem->setFlags(Qt::NoItemFlags);

        // Decode cue type to display text
        QString cueType;
        switch (pCue->getType()) {
            case Cue::INVALID:
                cueType = "?";
                break;
            case Cue::CUE:
                cueType = "Hotcue";
                break;
            case Cue::LOAD:
                cueType = "Main Cue";
                break;
            case Cue::BEAT:
                cueType = "Beat";
                break;
            case Cue::LOOP:
                cueType = "Loop";
                break;
            case Cue::JUMP:
                cueType = "Jump";
                break;
            case Cue::INTRO:
                cueType = "Intro";
                break;
            case Cue::OUTRO:
                cueType = "Outro";
                break;
            default:
                break;
        }

        QTableWidgetItem* typeItem = new QTableWidgetItem(cueType);
        // Make the type read only
        typeItem->setFlags(Qt::NoItemFlags);

        QComboBox* colorComboBox = new QComboBox();
        const QList<PredefinedColorPointer> predefinedColors = Color::kPredefinedColorsSet.allColors;
        for (int i = 0; i < predefinedColors.count(); i++) {
            PredefinedColorPointer color = predefinedColors.at(i);
            QColor defaultRgba = color->m_defaultRgba;
            colorComboBox->addItem(color->m_sDisplayName, defaultRgba);
            if (*color != *Color::kPredefinedColorsSet.noColor) {
                QPixmap pixmap(80, 80);
                pixmap.fill(defaultRgba);
                QIcon icon(pixmap);
                colorComboBox->setItemIcon(i, icon);
            }
        }
        PredefinedColorPointer cueColor = pCue->getColor();
        colorComboBox->setCurrentIndex(Color::kPredefinedColorsSet.predefinedColorIndex(cueColor));

        m_cueMap[row] = pCue;
        cueTable->insertRow(row);
        cueTable->setItem(row, 0, new QTableWidgetItem(rowStr));
        cueTable->setItem(row, 1, durationItem);
        cueTable->setItem(row, 2, typeItem);
        cueTable->setItem(row, 3, new QTableWidgetItem(hotcue));
        cueTable->setCellWidget(row, 4, colorComboBox);
        cueTable->setItem(row, 5, new QTableWidgetItem(pCue->getLabel()));
        row += 1;
    }
    cueTable->setSortingEnabled(true);
    cueTable->horizontalHeader()->setStretchLastSection(true);
    cueTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
}

void DlgTrackInfo::saveTrack() {
    if (!m_pLoadedTrack)
        return;

    // First, disconnect the track changed signal. Otherwise we signal ourselves
    // and repopulate all these fields.
    disconnect(m_pLoadedTrack.get(),
            &Track::changed,
            this,
            &DlgTrackInfo::updateTrackMetadata);

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

    QSet<int> updatedRows;
    for (int row = 0; row < cueTable->rowCount(); ++row) {
        QTableWidgetItem* rowItem = cueTable->item(row, 0);
        QTableWidgetItem* hotcueItem = cueTable->item(row, 3);
        QWidget* colorWidget = cueTable->cellWidget(row, 4);
        QTableWidgetItem* labelItem = cueTable->item(row, 5);

        VERIFY_OR_DEBUG_ASSERT(rowItem && hotcueItem && colorWidget && labelItem) {
            qWarning() << "unable to retrieve cells from cueTable row";
            continue;
        }

        int oldRow = rowItem->data(Qt::DisplayRole).toInt();
        CuePointer pCue(m_cueMap.value(oldRow, CuePointer()));
        if (!pCue) {
            continue;
        }
        updatedRows.insert(oldRow);

        QVariant vHotcue = hotcueItem->data(Qt::DisplayRole);
        bool ok;
        int iTableHotcue = vHotcue.toInt(&ok);
        if (ok) {
            // The GUI shows hotcues as 1-indexed, but they are actually
            // 0-indexed, so subtract 1
            pCue->setHotCue(iTableHotcue - 1);
        } else {
            pCue->setHotCue(-1);
        }

        if (pCue->getType() == Cue::CUE) {
            auto colorComboBox = qobject_cast<QComboBox*>(colorWidget);
            if (colorComboBox) {
                PredefinedColorPointer color = Color::kPredefinedColorsSet.allColors.at(colorComboBox->currentIndex());
                pCue->setColor(color);
            }
        }
        // do nothing for now.

        QString label = labelItem->data(Qt::DisplayRole).toString();
        pCue->setLabel(label);
    }

    QMutableHashIterator<int,CuePointer> it(m_cueMap);
    // Everything that was not processed above was removed.
    while (it.hasNext()) {
        it.next();
        int oldRow = it.key();

        // If cue's old row is not in updatedRows then it must have been
        // deleted.
        if (updatedRows.contains(oldRow)) {
            continue;
        }
        CuePointer pCue(it.value());
        it.remove();
        qDebug() << "Deleting cue" << pCue->getId() << pCue->getHotCue();
        m_pLoadedTrack->removeCue(pCue);
    }

    m_pLoadedTrack->setCoverInfo(m_loadedCoverInfo);

    // Reconnect changed signals now.
    connect(m_pLoadedTrack.get(),
            &Track::changed,
            this,
            &DlgTrackInfo::updateTrackMetadata);
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
    disconnect(m_pLoadedTrack.get(),
            &Track::changed,
            this,
            &DlgTrackInfo::updateTrackMetadata);
    m_pLoadedTrack.reset();

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

    m_cueMap.clear();
    cueTable->clearContents();
    cueTable->setRowCount(0);

    m_loadedCoverInfo = CoverInfo();
    m_pWCoverArtLabel->setCoverArt(m_loadedCoverInfo, QPixmap());
}

void DlgTrackInfo::slotBpmDouble() {
    m_pBeatsClone->scale(Beats::DOUBLE);
    // read back the actual value
    double newValue = m_pBeatsClone->getBpm();
    spinBpm->setValue(newValue);
}

void DlgTrackInfo::slotBpmHalve() {
    m_pBeatsClone->scale(Beats::HALVE);
    // read back the actual value
    double newValue = m_pBeatsClone->getBpm();
    spinBpm->setValue(newValue);
}

void DlgTrackInfo::slotBpmTwoThirds() {
    m_pBeatsClone->scale(Beats::TWOTHIRDS);
    // read back the actual value
    double newValue = m_pBeatsClone->getBpm();
    spinBpm->setValue(newValue);
}

void DlgTrackInfo::slotBpmThreeFourth() {
    m_pBeatsClone->scale(Beats::THREEFOURTHS);
    // read back the actual value
    double newValue = m_pBeatsClone->getBpm();
    spinBpm->setValue(newValue);
}

void DlgTrackInfo::slotBpmFourThirds() {
    m_pBeatsClone->scale(Beats::FOURTHIRDS);
    // read back the actual value
    double newValue = m_pBeatsClone->getBpm();
    spinBpm->setValue(newValue);
}

void DlgTrackInfo::slotBpmThreeHalves() {
    m_pBeatsClone->scale(Beats::THREEHALVES);
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

    if (m_pBeatsClone->getCapabilities() & Beats::BEATSCAP_SETBPM) {
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

void DlgTrackInfo::updateTrackMetadata() {
    if (m_pLoadedTrack) {
        populateFields(*m_pLoadedTrack);
    }
}

void DlgTrackInfo::slotImportMetadataFromMusicBrainz() {
    emit(showTagFetcher(m_pLoadedTrack));
}
