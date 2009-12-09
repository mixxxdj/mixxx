#include <QDebug>
#include <QModelIndexList>
#include <QModelIndex>
#include <QItemSelectionModel>
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "wtracktableview.h"
#include "wtracktableviewcontroller.h"


WTrackTableViewController::WTrackTableViewController(QObject* parent, WTrackTableView* pView) : QObject(parent) {
    m_pView = pView;

    // Make controls for library navigation and track loading
    m_pLoadSelectedTrackCh1 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel1]","LoadSelectedTrack")));
    m_pLoadSelectedTrackCh2 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel2]","LoadSelectedTrack")));
    m_pSelectNextTrack = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectNextTrack")));
    m_pSelectPrevTrack = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectPrevTrack")));
    m_pLoadSelectedIntoFirstStopped = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","LoadSelectedIntoFirstStopped")));
    m_pSelectTrackKnob = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectTrackKnob")));


    connect(m_pLoadSelectedTrackCh1, SIGNAL(valueChanged(double)), this, SLOT(slotLoadSelectedTrackCh1(double)));
    connect(m_pLoadSelectedTrackCh2, SIGNAL(valueChanged(double)), this, SLOT(slotLoadSelectedTrackCh2(double)));
    connect(m_pLoadSelectedIntoFirstStopped, SIGNAL(valueChanged(double)), this, SLOT(slotLoadSelectedIntoFirstStopped(double)));
    connect(m_pSelectNextTrack, SIGNAL(valueChanged(double)), this, SLOT(slotSelectNextTrack(double)));
    connect(m_pSelectPrevTrack, SIGNAL(valueChanged(double)), this, SLOT(slotSelectPrevTrack(double)));
    connect(m_pSelectTrackKnob, SIGNAL(valueChanged(double)), this, SLOT(slotSelectTrackKnob(double)));
}
        
WTrackTableViewController::~WTrackTableViewController()
{
   delete m_pLoadSelectedTrackCh1;
   delete m_pLoadSelectedTrackCh2;
   delete m_pSelectNextTrack;
   delete m_pSelectPrevTrack;
   delete m_pLoadSelectedIntoFirstStopped;
   delete m_pSelectTrackKnob;
}

void WTrackTableViewController::slotLoadSelectedTrackCh1(double v)
{
    if (v)
        m_pView->slotLoadPlayer1();
}
void WTrackTableViewController::slotLoadSelectedTrackCh2(double v)
{
    if (v)
        m_pView->slotLoadPlayer2();
}

void WTrackTableViewController::slotLoadSelectedIntoFirstStopped(double v)
{
    if (v)
    {
        //Only load if there is a single track selected.
        QItemSelectionModel* selectionModel = m_pView->selectionModel();
        QModelIndexList selectedRows = selectionModel->selectedRows();
        if (selectedRows.count() == 1)
            m_pView->slotMouseDoubleClicked(selectedRows.first());

    }
}

void WTrackTableViewController::slotSelectNextTrack(double v)
{
    // Only move on key presses
    if (v) m_pView->selectNext();
}
void WTrackTableViewController::slotSelectPrevTrack(double v)
{
    // Only move on key presses
    if (v) m_pView->selectPrevious();
}

void WTrackTableViewController::slotSelectTrackKnob(double v)
{
    int i = (int)v;
    //TODO: Make sure the logic in that while loop makes sense... it looks
    //      suspicious at first glance. - Albert Dec 8, 2009
    qDebug() << "TODO: slotSelectTrackKnob has super sketchy while loop that might"
                " cause GUI freeze... (copypasta from old track.cpp)";
    //m_pView->m_pTrackTableView->setUpdatesEnabled(false);
    while(i != 0)
    {
        if(i > 0)
        {
            m_pView->selectNext();
            i--;
        }
        else
        {
            m_pView->selectPrevious();
            i++;
        }
    }
    //m_pView->m_pTrackTableView->setUpdatesEnabled(true);
}
