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
    
    m_childModel.setRootItem(std::make_unique<TreeItem>(this));
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

QPointer<TreeItemModel> RecordingFeature::getChildModel() {
    return &m_childModel;
}

parented_ptr<QWidget> RecordingFeature::createPaneWidget(KeyboardEventFilter*, 
            int paneId, QWidget* parent) {
    auto pTable = LibraryFeature::createTableWidget(paneId, parent);
    pTable->setSorting(false);    
    return pTable;
}

parented_ptr<QWidget> RecordingFeature::createInnerSidebarWidget(
            KeyboardEventFilter* pKeyboard, QWidget* parent) {
    auto pRecordingView = make_parented<DlgRecording>(parent, 
                                                      m_pTrackCollection,
                                                      m_pRecordingManager);
    m_pRecordingView = pRecordingView.toWeakRef();
    m_pRecordingView->installEventFilter(pKeyboard);
    m_pRecordingView->setBrowseTableModel(getBrowseTableModel());
    m_pRecordingView->setProxyTrackModel(getProxyTrackModel());
    
    return pRecordingView;
}


void RecordingFeature::activate() {
    VERIFY_OR_DEBUG_ASSERT(!m_pRecordingView.isNull()) {
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
