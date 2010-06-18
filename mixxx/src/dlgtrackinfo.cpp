// dlgtrackinfo.cpp
// Created 11/10/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "dlgtrackinfo.h"
#include "library/dao/cue.h"
#include "trackinfoobject.h"

DlgTrackInfo::DlgTrackInfo(QWidget* parent) :
        QDialog(parent),
        m_pLoadedTrack(NULL) {

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

    connect(btnCueActivate, SIGNAL(clicked()),
            this, SLOT(cueActivate()));
    connect(btnCueDelete, SIGNAL(clicked()),
            this, SLOT(cueDelete()));

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

void DlgTrackInfo::loadTrack(TrackInfoObject* pTrack) {
    m_pLoadedTrack = pTrack;
    clear();

    if (m_pLoadedTrack == NULL)
        return;

    lblSong->setText(m_pLoadedTrack->getTitle());

    txtTrackName->setText(m_pLoadedTrack->getTitle());
    txtArtist->setText(m_pLoadedTrack->getArtist());
    txtComment->setText(m_pLoadedTrack->getComment());

    txtDuration->setText(m_pLoadedTrack->getDurationStr());
    txtFilepath->setText(m_pLoadedTrack->getFilename());
    txtFilepath->setCursorPosition(0);
    txtType->setText(m_pLoadedTrack->getType());

    int sampleRate = m_pLoadedTrack->getSampleRate();

    QList<Cue*> listPoints;
    const QList<Cue*>& cuePoints = m_pLoadedTrack->getCuePoints();
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

        int iHotcue = pCue->getHotCue();
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
        QString duration = QString("%1:%2.%3").arg(mins).arg(seconds, 2, 10, QChar('0')).arg(fraction, 2,10, QChar('0'));

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
        m_pLoadedTrack->setComment(txtComment->text());

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
                pCue->setHotCue(vHotcue.toInt());
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

    m_pLoadedTrack = NULL;
    clear();
}

void DlgTrackInfo::clear() {
    txtTrackName->setText("");
    lblSong->setText("Track:");

    txtTrackName->setText("");
    txtArtist->setText("");
    txtComment->setText("");

    txtDuration->setText("");
    txtFilepath->setText("");
    txtType->setText("");

    m_cueMap.clear();
    cueTable->clearContents();
    cueTable->setRowCount(0);
}
