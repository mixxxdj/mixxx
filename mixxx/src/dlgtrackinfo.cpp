// dlgtrackinfo.cpp
// Created 11/10/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "dlgtrackinfo.h"
#include "library/dao/cue.h"
#include "trackinfoobject.h"

DlgTrackInfo::DlgTrackInfo(QWidget* parent) :
        QDialog(parent),
        m_pLoadedTrack() {

    setupUi(this);

    cueTable->hideColumn(0);

    connect(btnNext, SIGNAL(clicked()),
            this, SLOT(slotNext()));
    connect(btnPrev, SIGNAL(clicked()),
            this, SLOT(slotPrev()));
    connect(btnApply, SIGNAL(clicked()),
            this, SLOT(apply()));
    connect(btnCancel, SIGNAL(clicked()),
            this, SLOT(cancel()));

    connect(bpmDouble, SIGNAL(clicked()),
            this, SLOT(slotBpmDouble()));
    connect(bpmHalve, SIGNAL(clicked()),
            this, SLOT(slotBpmHalve()));

    connect(btnCueActivate, SIGNAL(clicked()),
            this, SLOT(cueActivate()));
    connect(btnCueDelete, SIGNAL(clicked()),
            this, SLOT(cueDelete()));
    connect(bpmTap, SIGNAL(pressed()),
            this, SLOT(slotBpmTap()));
    connect(btnReloadFromFile, SIGNAL(clicked()),
            this, SLOT(reloadTrackMetadata()));
    m_bpmTapTimer.start();
    for (int i = 0; i < filterLength; ++i) {
        m_bpmTapFilter[i] = 0.0f;
    }
}

DlgTrackInfo::~DlgTrackInfo() {
    unloadTrack(false);
    qDebug() << "~DlgTrackInfo()";
}

void DlgTrackInfo::apply() {
    unloadTrack(true);
    accept();
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
    setWindowTitle(pTrack->getTitle());

    // Editable fields
    txtTrackName->setText(pTrack->getTitle());
    txtArtist->setText(pTrack->getArtist());
    txtAlbum->setText(pTrack->getAlbum());
    txtGenre->setText(pTrack->getGenre());
    txtComposer->setText(pTrack->getComposer());
    txtYear->setText(pTrack->getYear());
    txtTrackNumber->setText(pTrack->getTrackNumber());
    txtComment->setText(pTrack->getComment());
    spinBpm->setValue(pTrack->getBpm());

    // Non-editable fields
    txtDuration->setText(pTrack->getDurationStr());
    txtFilepath->setText(pTrack->getFilename());
    txtLocation->setText(pTrack->getLocation());
    txtType->setText(pTrack->getType());
    txtBitrate->setText(QString(pTrack->getBitrateStr()) + (" ") + tr("kbps"));
    txtBpm->setText(pTrack->getBpmStr());

    BeatsPointer pBeats = pTrack->getBeats();
    bool beatsSupportsSet = !pBeats || (pBeats->getCapabilities() & Beats::BEATSCAP_SET);
    bool enableBpmEditing = !pTrack->hasBpmLock() && beatsSupportsSet;
    spinBpm->setEnabled(enableBpmEditing);
    bpmTap->setEnabled(enableBpmEditing);
    bpmDouble->setEnabled(enableBpmEditing);
    bpmHalve->setEnabled(enableBpmEditing);
}

void DlgTrackInfo::loadTrack(TrackPointer pTrack) {
    m_pLoadedTrack = pTrack;
    clear();

    if (m_pLoadedTrack == NULL)
        return;

    populateFields(m_pLoadedTrack);
    populateCues(m_pLoadedTrack);
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
}

void DlgTrackInfo::unloadTrack(bool save) {
    if (!m_pLoadedTrack)
        return;

    if (save) {
        m_pLoadedTrack->setTitle(txtTrackName->text());
        m_pLoadedTrack->setArtist(txtArtist->text());
        m_pLoadedTrack->setAlbum(txtAlbum->text());
        m_pLoadedTrack->setGenre(txtGenre->text());
        m_pLoadedTrack->setComposer(txtComposer->text());
        m_pLoadedTrack->setYear(txtYear->text());
        m_pLoadedTrack->setTrackNumber(txtTrackNumber->text());
        m_pLoadedTrack->setComment(txtComment->toPlainText());

        if (!m_pLoadedTrack->hasBpmLock()) {
            m_pLoadedTrack->setBpm(spinBpm->value());
        }

        QHash<int, Cue*> cueMap;
        for (int row = 0; row < cueTable->rowCount(); ++row) {

            QTableWidgetItem* rowItem = cueTable->item(row, 0);
            QTableWidgetItem* hotcueItem = cueTable->item(row, 2);
            QTableWidgetItem* labelItem = cueTable->item(row, 3);

            if (!rowItem || !hotcueItem || !labelItem)
                continue;

            int oldRow = rowItem->data(Qt::DisplayRole).toInt();
            Cue* pCue = m_cueMap.take(oldRow);

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
        // Everything remaining in m_cueMap must have been deleted.
        while (it.hasNext()) {
            it.next();
            Cue* pCue = it.value();
            it.remove();
            qDebug() << "Deleting cue" << pCue->getId() << pCue->getHotCue();
            m_pLoadedTrack->removeCue(pCue);
        }
    }

    m_pLoadedTrack.clear();
    clear();
}

void DlgTrackInfo::clear() {

    txtTrackName->setText("");
    txtArtist->setText("");
    txtAlbum->setText("");
    txtGenre->setText("");
    txtComposer->setText("");
    txtYear->setText("");
    txtTrackNumber->setText("");
    txtComment->setText("");
    spinBpm->setValue(0.0);

    txtDuration->setText("");
    txtFilepath->setText("");
    txtType->setText("");
    txtLocation->setText("");
    txtBitrate->setText("");
    txtBpm->setText("");

    m_cueMap.clear();
    cueTable->clearContents();
    cueTable->setRowCount(0);
}

void DlgTrackInfo::slotBpmDouble() {
    spinBpm->setValue(spinBpm->value() * 2.0);
}

void DlgTrackInfo::slotBpmHalve() {
    spinBpm->setValue(spinBpm->value() / 2.0);
}

void DlgTrackInfo::slotBpmTap() {
    int elapsed = m_bpmTapTimer.elapsed();
    m_bpmTapTimer.restart();

    if (elapsed <= maxInterval) {
        // Move back in filter one sample
        for (int i = 1; i < filterLength; ++i) {
            m_bpmTapFilter[i-1] = m_bpmTapFilter[i];
        }

        m_bpmTapFilter[filterLength-1] = 60000.0f/elapsed;
        if (m_bpmTapFilter[filterLength-1] > maxBPM)
            m_bpmTapFilter[filterLength-1] = maxBPM;

        double temp = 0.;
        for (int i = 0; i < filterLength; ++i) {
            temp += m_bpmTapFilter[i];
        }
        temp /= filterLength;
        spinBpm->setValue(temp);
    }
}

void DlgTrackInfo::reloadTrackMetadata() {
    if (m_pLoadedTrack) {
        TrackPointer pTrack(new TrackInfoObject(m_pLoadedTrack->getLocation()));
        populateFields(pTrack);
    }
}
