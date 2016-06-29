// recordingfeature.cpp
// Created 03/26/2010 by Tobias Rafreider

#include "library/recording/dlgrecording.h"
#include "track/track.h"
#include "library/treeitem.h"
#include "library/recording/recordingfeature.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "widget/wlibrary.h"
#include "controllers/keyboard/keyboardeventfilter.h"

const QString RecordingFeature::m_sRecordingViewName = QString("Recording");

RecordingFeature::RecordingFeature(UserSettingsPointer pConfig,
                                   Library* pLibrary,
                                   QObject* parent,
                                   TrackCollection* pTrackCollection,
                                   RecordingManager* pRecordingManager)
        : LibraryFeature(pConfig, pLibrary, parent),
          m_pTrackCollection(pTrackCollection),
          m_pRecordingManager(pRecordingManager),
          m_pRecordingView(nullptr) {
}

RecordingFeature::~RecordingFeature() {

}

QVariant RecordingFeature::title() {
    return QVariant(tr("Recordings"));
}

QIcon RecordingFeature::getIcon() {
    return QIcon(":/images/library/ic_library_recordings.png");
}

TreeItemModel* RecordingFeature::getChildModel() {
    return &m_childModel;
}

void RecordingFeature::bindPaneWidget(WLibrary* pLibraryWidget,
                                      KeyboardEventFilter *pKeyboard, int) {
    
    WTrackTableView* pTrackTableView = 
            new WTrackTableView(pLibraryWidget, m_pConfig, m_pTrackCollection, false); // No sorting
    pTrackTableView->installEventFilter(pKeyboard);
    
    connect(m_pLibrary, SIGNAL(setTrackTableFont(QFont)),
            pTrackTableView, SLOT(setTrackTableFont(QFont)));
    connect(m_pLibrary, SIGNAL(setTrackTableRowHeight(int)),
            pTrackTableView, SLOT(setTrackTableRowHeight(int)));
    
    connect(pTrackTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(pTrackTableView, SIGNAL(loadTrackToPlayer(TrackPointer,QString,bool)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer,QString,bool)));
    
    pLibraryWidget->registerView(m_sRecordingViewName, pTrackTableView);
    
    if (m_pRecordingView) {
        m_pRecordingView->setTrackTable(pTrackTableView);
    } else {
        m_trackTables.append(pTrackTableView);
    }
}

QWidget *RecordingFeature::createPaneWidget(KeyboardEventFilter* pKeyboard, int) {
    WTrackTableView* pTrackTableView = new WTrackTableView(nullptr, 
                                                           m_pConfig, 
                                                           m_pTrackCollection, 
                                                           false); // No sorting
    pTrackTableView->installEventFilter(pKeyboard);
    
    connect(m_pLibrary, SIGNAL(setTrackTableFont(QFont)),
            pTrackTableView, SLOT(setTrackTableFont(QFont)));
    connect(m_pLibrary, SIGNAL(setTrackTableRowHeight(int)),
            pTrackTableView, SLOT(setTrackTableRowHeight(int)));
    
    connect(pTrackTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(pTrackTableView, SIGNAL(loadTrackToPlayer(TrackPointer,QString,bool)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer,QString,bool)));
    
    if (m_pRecordingView) {
        m_pRecordingView->setTrackTable(pTrackTableView);
    } else {
        m_trackTables.append(pTrackTableView);
    }
    
    return pTrackTableView;
}

void RecordingFeature::bindSidebarWidget(WBaseLibrary* pBaseLibrary,
                                         KeyboardEventFilter* pKeyboard) {
    m_pRecordingView = new DlgRecording(nullptr, 
                                        m_pTrackCollection,
                                        m_pRecordingManager);
    m_pRecordingView->installEventFilter(pKeyboard);
    
    connect(this, SIGNAL(refreshBrowseModel()),
            m_pRecordingView, SLOT(refreshBrowseModel()));
    
    for (WTrackTableView* pTable : m_trackTables) {
        m_pRecordingView->setTrackTable(pTable);
    }
    m_trackTables.clear();
    
    pBaseLibrary->registerView(m_sRecordingViewName, m_pRecordingView);
}


void RecordingFeature::activate() {
    m_pRecordingView->refreshBrowseModel();
    emit(switchToView(m_sRecordingViewName));
    emit(restoreSearch(""));
    emit(enableCoverArtDisplay(false));
}
