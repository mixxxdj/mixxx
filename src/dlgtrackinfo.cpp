// dlgtrackinfo.cpp
// Created 11/10/2009 by RJ Ryan (rryan@mit.edu)

#include <QDesktopServices>
#include <QtDebug>

#include "dlgtrackinfo.h"
#include "trackinfoobject.h"
#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "library/dao/cue.h"

const int kMinBPM = 30;
const int kMaxBPM = 240;
// Maximum allowed interval between beats in milli seconds (calculated from
// minBPM)
const int kMaxInterval = static_cast<int>(1000.0 * (60.0 / kMinBPM));

DlgTrackInfo::DlgTrackInfo(QWidget* parent,
                           DlgTagFetcher& DlgTagFetcher)
            : QDialog(parent),
              m_pLoadedTrack(NULL),
              m_DlgTagFetcher(DlgTagFetcher),
              m_pWCoverArtLabel(new WCoverArtLabel(this)) {
    init();
}

DlgTrackInfo::~DlgTrackInfo() {
    unloadTrack(false);
    qDebug() << "~DlgTrackInfo()";
}

void DlgTrackInfo::init() {
    setupUi(this);

    cueTable->hideColumn(0);
    coverBox->insertWidget(1, m_pWCoverArtLabel);

    // It is essential to make the QPlainTextEdit transparent.
    // Without this, the background is always solid (white by default).
    txtLocation->viewport()->setAutoFillBackground(false);

    connect(btnNext, SIGNAL(clicked()),
            this, SLOT(slotNext()));
    connect(btnPrev, SIGNAL(clicked()),
            this, SLOT(slotPrev()));
    connect(btnApply, SIGNAL(clicked()),
            this, SLOT(apply()));
    connect(btnOK, SIGNAL(clicked()),
            this, SLOT(OK()));
    connect(btnCancel, SIGNAL(clicked()),
            this, SLOT(cancel()));

    connect(btnFetchTag, SIGNAL(clicked()),
            this, SLOT(fetchTag()));

    connect(bpmDouble, SIGNAL(clicked()),
            this, SLOT(slotBpmDouble()));
    connect(bpmHalve, SIGNAL(clicked()),
            this, SLOT(slotBpmHalve()));
    connect(bpmTwoThirds, SIGNAL(clicked()),
            this, SLOT(slotBpmTwoThirds()));
    connect(bpmThreeFourth, SIGNAL(clicked()),
            this, SLOT(slotBpmThreeFourth()));

    connect(btnCueActivate, SIGNAL(clicked()),
            this, SLOT(cueActivate()));
    connect(btnCueDelete, SIGNAL(clicked()),
            this, SLOT(cueDelete()));
    connect(bpmTap, SIGNAL(pressed()),
            this, SLOT(slotBpmTap()));
    connect(btnReloadFromFile, SIGNAL(clicked()),
            this, SLOT(reloadTrackMetadata()));
    connect(btnOpenFileBrowser, SIGNAL(clicked()),
            this, SLOT(slotOpenInFileBrowser()));
    m_bpmTapTimer.start();
    for (int i = 0; i < kFilterLength; ++i) {
        m_bpmTapFilter[i] = 0.0f;
    }

    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache != NULL) {
        connect(pCache, SIGNAL(coverFound(const QObject*, const int, const CoverInfo&, QPixmap, bool)),
                this, SLOT(slotCoverFound(const QObject*, const int, const CoverInfo&, QPixmap, bool)));
    }
    connect(m_pWCoverArtLabel, SIGNAL(coverArtSelected(const CoverArt&)),
            this, SLOT(slotCoverArtSelected(const CoverArt&)));
    connect(m_pWCoverArtLabel, SIGNAL(reloadCoverArt()),
            this, SLOT(slotReloadCoverArt()));
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
    qSort(rowsList);

    QListIterator<int> it(rowsList);
    it.toBack();
    while (it.hasPrevious()) {
        cueTable->removeRow(it.previous());
    }
}

void DlgTrackInfo::populateFields(TrackPointer pTrack) {
    setWindowTitle(pTrack->getArtist() % " - " % pTrack->getTitle());

    // Editable fields
    txtTrackName->setText(pTrack->getTitle());
    txtArtist->setText(pTrack->getArtist());
    txtAlbum->setText(pTrack->getAlbum());
    txtAlbumArtist->setText(pTrack->getAlbumArtist());
    txtGenre->setText(pTrack->getGenre());
    txtComposer->setText(pTrack->getComposer());
    txtGrouping->setText(pTrack->getGrouping());
    txtYear->setText(pTrack->getYear());
    txtTrackNumber->setText(pTrack->getTrackNumber());
    txtComment->setPlainText(pTrack->getComment());
    spinBpm->setValue(pTrack->getBpm());
    // Non-editable fields
    txtDuration->setText(pTrack->getDurationStr());
    txtLocation->setPlainText(pTrack->getLocation());
    txtType->setText(pTrack->getType());
    txtBitrate->setText(QString(pTrack->getBitrateStr()) + (" ") + tr("kbps"));
    txtBpm->setText(pTrack->getBpmStr());
    txtKey->setText(pTrack->getKeyText());
    BeatsPointer pBeats = pTrack->getBeats();
    bool beatsSupportsSet = !pBeats || (pBeats->getCapabilities() & Beats::BEATSCAP_SET);
    bool enableBpmEditing = !pTrack->hasBpmLock() && beatsSupportsSet;
    spinBpm->setEnabled(enableBpmEditing);
    bpmTap->setEnabled(enableBpmEditing);
    bpmDouble->setEnabled(enableBpmEditing);
    bpmHalve->setEnabled(enableBpmEditing);
    bpmTwoThirds->setEnabled(enableBpmEditing);
    bpmThreeFourth->setEnabled(enableBpmEditing);

    m_loadedCoverInfo = pTrack->getCoverInfo();
    int reference = pTrack->getId();
    m_loadedCoverInfo.trackLocation = pTrack->getLocation();
    m_pWCoverArtLabel->setCoverArt(pTrack, m_loadedCoverInfo, QPixmap());
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache != NULL) {
        pCache->requestCover(m_loadedCoverInfo, this, reference);
    }
}

void DlgTrackInfo::loadTrack(TrackPointer pTrack) {
    m_pLoadedTrack = pTrack;
    clear();

    if (m_pLoadedTrack.isNull()) {
        return;
    }

    populateFields(m_pLoadedTrack);
    populateCues(m_pLoadedTrack);

    disconnect(this, SLOT(updateTrackMetadata()));

    // We already listen to changed() so we don't need to listen to individual
    // signals such as cuesUpdates, coverArtUpdated(), etc.
    connect(pTrack.data(), SIGNAL(changed(TrackInfoObject*)),
            this, SLOT(updateTrackMetadata()));
}

void DlgTrackInfo::slotCoverFound(const QObject* pRequestor,
                                  int requestReference, const CoverInfo& info,
                                  QPixmap pixmap, bool fromCache) {
    Q_UNUSED(fromCache);
    if (pRequestor == this && m_pLoadedTrack &&
            m_pLoadedTrack->getId() == requestReference) {
        qDebug() << "DlgTrackInfo::slotPixmapFound" << pRequestor << info
                 << pixmap.size();
        m_pWCoverArtLabel->setCoverArt(m_pLoadedTrack, m_loadedCoverInfo, pixmap);
    }
}

void DlgTrackInfo::slotReloadCoverArt() {
    if (m_pLoadedTrack) {
        // TODO(rryan) move this out of the main thread. The issue is that
        // CoverArtCache::requestGuessCover mutates the provided track whereas
        // in DlgTrackInfo we delay changing the track until the user hits apply
        // (or cancels the edit).
        CoverArt art = CoverArtUtils::guessCoverArt(m_pLoadedTrack);
        slotCoverArtSelected(art);
    }
}

void DlgTrackInfo::slotCoverArtSelected(const CoverArt& art) {
    qDebug() << "DlgTrackInfo::slotCoverArtSelected" << art;
    m_loadedCoverInfo = art.info;
    // TODO(rryan) don't use track ID as a reference
    int reference = 0;
    if (m_pLoadedTrack) {
        reference = m_pLoadedTrack->getId();
        m_loadedCoverInfo.trackLocation = m_pLoadedTrack->getLocation();
    }
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache != NULL) {
        pCache->requestCover(m_loadedCoverInfo, this, reference);
    }
}

void DlgTrackInfo::slotOpenInFileBrowser() {
    if (m_pLoadedTrack.isNull()) {
        return;
    }

    QDir dir;
    QStringList splittedPath = m_pLoadedTrack->getDirectory().split("/");
    do {
        dir = QDir(splittedPath.join("/"));
        splittedPath.removeLast();
    } while (!dir.exists() && splittedPath.size());

    // This function does not work for a non-existent directory!
    // so it is essential that in the worst case it try opening
    // a valid directory, in this case, 'QDir::home()'.
    // Otherwise nothing would happen...
    if (!dir.exists()) {
        // it ensures a valid dir for any OS (Windows)
        dir = QDir::home();
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir.absolutePath()));
}

void DlgTrackInfo::populateCues(TrackPointer pTrack) {
    int sampleRate = pTrack->getSampleRate();

    QList<Cue*> listPoints;
    const QList<Cue*>& cuePoints = pTrack->getCuePoints();
    QListIterator<Cue*> it(cuePoints);
    while (it.hasNext()) {
        Cue* pCue = it.next();
        if (pCue->getType() == Cue::CUE || pCue->getType() == Cue::LOAD) {
            listPoints.push_back(pCue);
        }
    }
    it = QListIterator<Cue*>(listPoints);
    cueTable->setSortingEnabled(false);
    int row = 0;

    while (it.hasNext()) {
        Cue* pCue = it.next();

        QString rowStr = QString("%1").arg(row);

        // All hotcues are stored in Cue's as 0-indexed, but the GUI presents
        // them to the user as 1-indexex. Add 1 here. rryan 9/2010
        int iHotcue = pCue->getHotCue() + 1;
        QString hotcue = "";
        if (iHotcue != -1) {
            hotcue = QString("%1").arg(iHotcue);
        }

        int position = pCue->getPosition();
        double totalSeconds;
        if (position == -1)
            continue;
        else {
            totalSeconds = float(position) / float(sampleRate) / 2.0;
        }

        int fraction = 100*(totalSeconds - floor(totalSeconds));
        int seconds = int(totalSeconds) % 60;
        int mins = int(totalSeconds) / 60;
        //int hours = mins / 60; //Not going to worry about this for now. :)

        //Construct a nicely formatted duration string now.
        QString duration = QString("%1:%2.%3").arg(
            QString::number(mins),
            QString("%1").arg(seconds, 2, 10, QChar('0')),
            QString("%1").arg(fraction, 2, 10, QChar('0')));

        QTableWidgetItem* durationItem = new QTableWidgetItem(duration);
        // Make the duration read only
        durationItem->setFlags(Qt::NoItemFlags);

        m_cueMap[row] = pCue;
        cueTable->insertRow(row);
        cueTable->setItem(row, 0, new QTableWidgetItem(rowStr));
        cueTable->setItem(row, 1, durationItem);
        cueTable->setItem(row, 2, new QTableWidgetItem(hotcue));
        cueTable->setItem(row, 3, new QTableWidgetItem(pCue->getLabel()));
        row += 1;
    }
    cueTable->setSortingEnabled(true);
    cueTable->horizontalHeader()->setStretchLastSection(true);
}

void DlgTrackInfo::saveTrack() {
    if (!m_pLoadedTrack)
        return;

    // First, disconnect the track changed signal. Otherwise we signal ourselves
    // and repopulate all these fields.
    disconnect(m_pLoadedTrack.data(), SIGNAL(changed(TrackInfoObject*)),
               this, SLOT(updateTrackMetadata()));

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

    if (!m_pLoadedTrack->hasBpmLock()) {
        m_pLoadedTrack->setBpm(spinBpm->value());
    }

    QSet<int> updatedRows;
    for (int row = 0; row < cueTable->rowCount(); ++row) {
        QTableWidgetItem* rowItem = cueTable->item(row, 0);
        QTableWidgetItem* hotcueItem = cueTable->item(row, 2);
        QTableWidgetItem* labelItem = cueTable->item(row, 3);

        if (!rowItem || !hotcueItem || !labelItem)
            continue;

        int oldRow = rowItem->data(Qt::DisplayRole).toInt();
        Cue* pCue = m_cueMap.value(oldRow, NULL);
        if (pCue == NULL) {
            continue;
        }

        updatedRows.insert(oldRow);

        QVariant vHotcue = hotcueItem->data(Qt::DisplayRole);
        if (vHotcue.canConvert<int>()) {
            int iTableHotcue = vHotcue.toInt();
            // The GUI shows hotcues as 1-indexed, but they are actually
            // 0-indexed, so subtract 1
            pCue->setHotCue(iTableHotcue-1);
        } else {
            pCue->setHotCue(-1);
        }

        QString label = labelItem->data(Qt::DisplayRole).toString();
        pCue->setLabel(label);
    }

    QMutableHashIterator<int,Cue*> it(m_cueMap);
    // Everything that was not processed above was removed.
    while (it.hasNext()) {
        it.next();
        int oldRow = it.key();

        // If cue's old row is not in updatedRows then it must have been
        // deleted.
        if (updatedRows.contains(oldRow)) {
            continue;
        }
        Cue* pCue = it.value();
        it.remove();
        qDebug() << "Deleting cue" << pCue->getId() << pCue->getHotCue();
        m_pLoadedTrack->removeCue(pCue);
    }

    m_pLoadedTrack->setCoverInfo(m_loadedCoverInfo);

    // Reconnect changed signals now.
    connect(m_pLoadedTrack.data(), SIGNAL(changed(TrackInfoObject*)),
            this, SLOT(updateTrackMetadata()));
}

void DlgTrackInfo::unloadTrack(bool save) {
    if (!m_pLoadedTrack)
        return;

    if (save) {
        saveTrack();
    }

    clear();
    disconnect(this, SLOT(updateTrackMetadata()));
    m_pLoadedTrack.clear();
}

void DlgTrackInfo::clear() {

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

    txtDuration->setText("");
    txtType->setText("");
    txtLocation->setPlainText("");
    txtBitrate->setText("");
    txtBpm->setText("");

    m_cueMap.clear();
    cueTable->clearContents();
    cueTable->setRowCount(0);

    m_loadedCoverInfo = CoverInfo();
    m_pWCoverArtLabel->setCoverArt(TrackPointer(), m_loadedCoverInfo, QPixmap());
}

void DlgTrackInfo::slotBpmDouble() {
    spinBpm->setValue(spinBpm->value() * 2.0);
}

void DlgTrackInfo::slotBpmHalve() {
    spinBpm->setValue(spinBpm->value() / 2.0);
}

void DlgTrackInfo::slotBpmTwoThirds() {
    spinBpm->setValue(spinBpm->value() * (2./3.));
}

void DlgTrackInfo::slotBpmThreeFourth() {
    spinBpm->setValue(spinBpm->value() * (3./4.));
}

void DlgTrackInfo::slotBpmTap() {
    int elapsed = m_bpmTapTimer.elapsed();
    m_bpmTapTimer.restart();

    if (elapsed <= kMaxInterval) {
        // Move back in filter one sample
        for (int i = 1; i < kFilterLength; ++i) {
            m_bpmTapFilter[i-1] = m_bpmTapFilter[i];
        }

        m_bpmTapFilter[kFilterLength-1] = 60000.0f/elapsed;
        if (m_bpmTapFilter[kFilterLength-1] > kMaxBPM)
            m_bpmTapFilter[kFilterLength-1] = kMaxBPM;

        double temp = 0.;
        for (int i = 0; i < kFilterLength; ++i) {
            temp += m_bpmTapFilter[i];
        }
        temp /= kFilterLength;
        spinBpm->setValue(temp);
    }
}

void DlgTrackInfo::reloadTrackMetadata() {
    if (m_pLoadedTrack) {
        TrackPointer pTrack(new TrackInfoObject(m_pLoadedTrack->getLocation(),
                                                m_pLoadedTrack->getSecurityToken()));
        populateFields(pTrack);
    }
}

void DlgTrackInfo::updateTrackMetadata() {
    if (m_pLoadedTrack) {
        populateFields(m_pLoadedTrack);
    }
}

void DlgTrackInfo::fetchTag() {
    m_DlgTagFetcher.loadTrack(m_pLoadedTrack);
    m_DlgTagFetcher.show();
}
