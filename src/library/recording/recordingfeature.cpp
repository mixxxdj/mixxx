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

RecordingFeature::RecordingFeature(UserSettingsPointer pConfig,
                                   Library* pLibrary,
                                   QObject* parent,
                                   TrackCollection* pTrackCollection,
                                   RecordingManager* pRecordingManager)
        : LibraryFeature(pConfig, pLibrary, pTrackCollection, parent),
          m_pTrackCollection(pTrackCollection),
          m_pRecordingManager(pRecordingManager),
          m_pRecordingView(nullptr),
          m_pBrowseModel(nullptr),
          m_pProxyModel(nullptr) {
    
    TreeItem* pRoot = new TreeItem();
    pRoot->setLibraryFeature(this);
    m_childModel.setRootItem(pRoot);
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

QWidget* RecordingFeature::createPaneWidget(KeyboardEventFilter* pKeyboard, int) {
    WTrackTableView* pTrackTableView = new WTrackTableView(nullptr, 
                                                           m_pConfig, 
                                                           m_pTrackCollection, 
                                                           false); // No sorting
    pTrackTableView->installEventFilter(pKeyboard);
    
    connect(m_pLibrary, SIGNAL(setTrackTableFont(QFont)),
            pTrackTableView, SLOT(setTrackTableFont(QFont)));
    connect(m_pLibrary, SIGNAL(setTrackTableRowHeight(int)),
            pTrackTableView, SLOT(setTrackTableRowHeight(int)));
    pTrackTableView->loadTrackModel(getProxyTrackModel());
    
    return pTrackTableView;
}

QWidget *RecordingFeature::createInnerSidebarWidget(KeyboardEventFilter* pKeyboard) {
    m_pRecordingView = new DlgRecording(nullptr, 
                                        m_pTrackCollection,
                                        m_pRecordingManager);
    m_pRecordingView->installEventFilter(pKeyboard);
    m_pRecordingView->setBrowseTableModel(getBrowseTableModel());
    m_pRecordingView->setProxyTrackModel(getProxyTrackModel());
    
    return m_pRecordingView;
}


void RecordingFeature::activate() {
    DEBUG_ASSERT_AND_HANDLE(!m_pRecordingView.isNull()) {
        return;
    }
    
    m_pRecordingView->refreshBrowseModel();
    m_pLibrary->switchToFeature(this);
    m_pLibrary->showBreadCrumb(m_childModel.getItem(QModelIndex()));
    m_pLibrary->restoreSearch("");
    
    emit(enableCoverArtDisplay(false));
}

BrowseTableModel* RecordingFeature::getBrowseTableModel() {
    if (m_pBrowseModel.isNull()) {
        m_pBrowseModel = new BrowseTableModel(this, m_pTrackCollection, m_pRecordingManager);
    }
    
    return m_pBrowseModel;
}

ProxyTrackModel* RecordingFeature::getProxyTrackModel() {
    if (m_pProxyModel.isNull()) {
        m_pProxyModel = new ProxyTrackModel(getBrowseTableModel());
    }
    
    return m_pProxyModel;
}
