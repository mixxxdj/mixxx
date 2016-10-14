// recordingfeature.cpp
// Created 03/26/2010 by Tobias Rafreider

#include <widget/wlibrarypane.h>
#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/features/recording/dlgrecording.h"
#include "library/features/recording/recordingfeature.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "track/track.h"
#include "widget/wtracktableview.h"

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

QString RecordingFeature::getIconPath() {
    return ":/images/library/ic_library_recordings.png";
}

QString RecordingFeature::getSettingsName() const {
    return "RecordingFeature";
}

TreeItemModel* RecordingFeature::getChildModel() {
    return &m_childModel;
}

QWidget* RecordingFeature::createPaneWidget(
        KeyboardEventFilter* pKeyboard, int paneId) {
    Q_UNUSED(pKeyboard);
    WTrackTableView* pTable = LibraryFeature::createTableWidget(paneId);
    pTable->setSorting(false);    
    return pTable;
}

QWidget *RecordingFeature::createInnerSidebarWidget(
        KeyboardEventFilter* pKeyboard) {
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
    showTrackModel(getProxyTrackModel());
    showBreadCrumb();
    restoreSearch("");
    
}

BrowseTableModel* RecordingFeature::getBrowseTableModel() {
    if (m_pBrowseModel.isNull()) {
        m_pBrowseModel = new BrowseTableModel(
                this, m_pTrackCollection, m_pRecordingManager);
    }
    
    return m_pBrowseModel;
}

ProxyTrackModel* RecordingFeature::getProxyTrackModel() {
    if (m_pProxyModel.isNull()) {
        m_pProxyModel = new ProxyTrackModel(getBrowseTableModel());
    }
    
    return m_pProxyModel;
}
