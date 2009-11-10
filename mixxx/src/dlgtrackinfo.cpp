// dlgtrackinfo.cpp
// Created 11/10/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "dlgtrackinfo.h"
#include "trackinfoobject.h"

DlgTrackInfo::DlgTrackInfo(QWidget* parent) :
        QDialog(parent) {

    setupUi(this);

    connect(btnNext, SIGNAL(clicked()),
            this, SLOT(slotNext()));
    connect(btnPrev, SIGNAL(clicked()),
            this, SLOT(slotPrev()));
    connect(btnApply, SIGNAL(clicked()),
            this, SLOT(apply()));
    connect(btnCancel, SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(btnCueActivate, SIGNAL(clicked()),
            this, SLOT(cueActivate()));
    connect(btnCueDelete, SIGNAL(clicked()),
            this, SLOT(cueDelete()));

}

DlgTrackInfo::~DlgTrackInfo() {
    unloadTrack(m_pLoadedTrack);
    qDebug() << "~DlgTrackInfo()";
}

void DlgTrackInfo::apply() {
    unloadTrack(m_pLoadedTrack);
    accept();
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

}

void DlgTrackInfo::loadTrack(TrackInfoObject* pTrack) {
    m_pLoadedTrack = pTrack;
    clear();

    lblSong->setText(m_pLoadedTrack->getTitle());

    txtTrackName->setText(m_pLoadedTrack->getTitle());
    txtArtist->setText(m_pLoadedTrack->getArtist());
    txtComment->setText(m_pLoadedTrack->getComment());

    txtDuration->setText(m_pLoadedTrack->getDurationStr());
    txtFilepath->setText(m_pLoadedTrack->getFilename());
    txtFilepath->setCursorPosition(0);
    txtType->setText(m_pLoadedTrack->getType());

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
    cueTable->setRowCount(listPoints.size());
    cueTable->setSortingEnabled(false);
    int row = 0;
    while (it.hasNext()) {
        Cue* pCue = it.next();
        QString id = QString("%1").arg(pCue->getId());
        QString hotcue = QString("%1").arg(pCue->getHotCue());
        cueTable->setItem(row, 0, new QTableWidgetItem(id));
        cueTable->setItem(row, 1, new QTableWidgetItem(hotcue));
        cueTable->setItem(row, 2, new QTableWidgetItem(pCue->getLabel()));
    }
    cueTable->setSortingEnabled(true);
}

void DlgTrackInfo::unloadTrack(TrackInfoObject* pTrack) {
    if (!m_pLoadedTrack)
        return;

    m_pLoadedTrack->setTitle(txtTrackName->text());
    m_pLoadedTrack->setArtist(txtArtist->text());
    m_pLoadedTrack->setComment(txtComment->text());

    m_pLoadedTrack = NULL;
    // TODO(XXX) update cues
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

    cueTable->clearContents();
}
