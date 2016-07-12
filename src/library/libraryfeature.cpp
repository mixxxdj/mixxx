// libraryfeature.cpp
// Created 8/17/2009 by RJ Ryan (rryan@mit.edu)

#include <QDebug>
#include <QAbstractItemModel>
#include <QDesktopServices>
#include <QIcon>
#include <QLabel>
#include <QModelIndex>
#include <QTreeView>
#include <QUrl>
#include <QVariant>
#include <QVBoxLayout>

#include "library/library.h"
#include "library/libraryfeature.h"
#include "widget/wbaselibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wtracktableview.h"

// KEEP THIS cpp file to tell scons that moc should be called on the class!!!
// The reason for this is that LibraryFeature uses slots/signals and for this
// to work the code has to be precompiled by moc
LibraryFeature::LibraryFeature(UserSettingsPointer pConfig, 
                               Library* pLibrary,
                               TrackCollection* pTrackCollection,
                               QObject* parent)
        : QObject(parent),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary),
          m_pTrackCollection(pTrackCollection),
          m_featureFocus(-1) {
}

LibraryFeature::~LibraryFeature() {
    
}

QWidget* LibraryFeature::createPaneWidget(KeyboardEventFilter* pKeyboard, 
                                          int paneId) {
    return createTableWidget(pKeyboard, paneId);
}

QWidget *LibraryFeature::createSidebarWidget(KeyboardEventFilter* pKeyboard) {
    //qDebug() << "LibraryFeature::bindSidebarWidget";
    QFrame* pContainer = new QFrame(nullptr);
    pContainer->setContentsMargins(0,0,0,0);
    
    QVBoxLayout* pLayout = new QVBoxLayout(pContainer);
    pLayout->setContentsMargins(0,0,0,0);
    pLayout->setSpacing(0);
    pContainer->setLayout(pLayout);
    
    QLabel* pTitle = new QLabel(title().toString(), pContainer);
    pLayout->addWidget(pTitle);
    
    QWidget* pSidebar = createInnerSidebarWidget(pKeyboard);
    pSidebar->setParent(pContainer);
    pLayout->addWidget(pSidebar);
    
    return pContainer;
}

void LibraryFeature::setFeatureFocus(int focus) {
    m_featureFocus = focus;
}

int LibraryFeature::getFeatureFocus() {
    return m_featureFocus;
}

void LibraryFeature::setFocusedPane(int paneId) {
    m_focusedPane = paneId;
}

WTrackTableView* LibraryFeature::createTableWidget(KeyboardEventFilter* pKeyboard,
                                                   int paneId) {
    WTrackTableView* pTrackTableView = 
            new WTrackTableView(nullptr, m_pConfig, m_pTrackCollection);
    
    pTrackTableView->installEventFilter(pKeyboard);
    
    connect(pTrackTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(pTrackTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)));
    connect(pTrackTableView, SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));
    
    connect(m_pLibrary, SIGNAL(setTrackTableFont(QFont)),
            pTrackTableView, SLOT(setTrackTableFont(QFont)));
    connect(m_pLibrary, SIGNAL(setTrackTableRowHeight(int)),
            pTrackTableView, SLOT(setTrackTableRowHeight(int)));
    m_trackTables[paneId] = pTrackTableView;
    
    return pTrackTableView;
}

QWidget* LibraryFeature::createInnerSidebarWidget(KeyboardEventFilter *pKeyboard) {
    return createLibrarySidebarWidget(pKeyboard);
}

WLibrarySidebar *LibraryFeature::createLibrarySidebarWidget(KeyboardEventFilter *pKeyboard) {
    WLibrarySidebar* pSidebar = new WLibrarySidebar(nullptr);
    pSidebar->installEventFilter(pKeyboard);
    pSidebar->setModel(getChildModel());
    
    connect(pSidebar, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(activateChild(const QModelIndex&)));
    connect(pSidebar, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(onLazyChildExpandation(const QModelIndex&)));
    connect(pSidebar, SIGNAL(rightClicked(const QPoint&, const QModelIndex&)),
            this, SLOT(onRightClickChild(const QPoint&, const QModelIndex&)));
    connect(pSidebar, SIGNAL(expanded(const QModelIndex&)),
            this, SLOT(onLazyChildExpandation(const QModelIndex&)));
    return pSidebar;
}

void LibraryFeature::showTrackModel(QAbstractItemModel *model) {
    auto it = m_trackTables.find(m_featureFocus);
    if (it == m_trackTables.end() || it->isNull()) {
        return;
    }
    (*it)->loadTrackModel(model);
    switchToFeature();
}

void LibraryFeature::switchToFeature() {
    m_pLibrary->switchToFeature(this);
}

void LibraryFeature::restoreSearch(const QString& search) {
    m_pLibrary->restoreSearch(search);
}

void LibraryFeature::showBreadCrumb(TreeItem* pTree) {
    m_pLibrary->showBreadCrumb(pTree);
}

WTrackTableView *LibraryFeature::getFocusedTable() {
    auto it = m_trackTables.find(m_featureFocus);
    if (it == m_trackTables.end() || it->isNull()) {
        return nullptr;
    }
    return *it;
}

QStringList LibraryFeature::getPlaylistFiles(QFileDialog::FileMode mode) {
    QString lastPlaylistDirectory = m_pConfig->getValueString(
            ConfigKey("[Library]", "LastImportExportPlaylistDirectory"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

    QFileDialog dialog(nullptr,
                       tr("Import Playlist"),
                       lastPlaylistDirectory,
                       tr("Playlist Files (*.m3u *.m3u8 *.pls *.csv)"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(mode);
    dialog.setModal(true);

    // If the user refuses return
    if (!dialog.exec()) {
        return QStringList();
    }
    return dialog.selectedFiles();
}
