// libraryfeature.cpp
// Created 8/17/2009 by RJ Ryan (rryan@mit.edu)

#include <QDebug>
#include <QAbstractItemModel>
#include <QDesktopServices>
#include <QIcon>
#include <QLabel>
#include <QModelIndex>
#include <QString>
#include <QTreeView>
#include <QUrl>
#include <QVariant>
#include <QVBoxLayout>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/libraryfeature.h"
#include "library/trackcollection.h"
#include "library/treeitemmodel.h"
#include "widget/wbaselibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wtracktableview.h"
#include "widget/wminiviewscrollbar.h"
#include "widget/wpixmapstore.h"

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
          m_savedDAO(m_pTrackCollection->getSavedQueriesDAO()),
          m_featurePane(-1) {
}

LibraryFeature::~LibraryFeature() {
}

QString LibraryFeature::getSettingsName() const {
    return QString("");
}

bool LibraryFeature::isSinglePane() const {
    return true;
}

QIcon LibraryFeature::getIcon() {    
    return WPixmapStore::getLibraryIcon(getIconPath());
}

bool LibraryFeature::dropAcceptChild(const QModelIndex&, QList<QUrl>, QObject*) {
    return false;
}

bool LibraryFeature::dragMoveAccept(QUrl) {
    return false;
}

bool LibraryFeature::dragMoveAcceptChild(const QModelIndex &, QUrl) {
    return false;
}

QWidget* LibraryFeature::createPaneWidget(KeyboardEventFilter* pKeyboard, 
                                          int paneId) {
    Q_UNUSED(pKeyboard);
    return createTableWidget(paneId);
}

QWidget *LibraryFeature::createSidebarWidget(KeyboardEventFilter* pKeyboard) {
    //qDebug() << "LibraryFeature::bindSidebarWidget";
    QFrame* pContainer = new QFrame(nullptr);
    pContainer->setContentsMargins(0,0,0,0);
    
    QVBoxLayout* pLayout = new QVBoxLayout(pContainer);
    pLayout->setContentsMargins(0,0,0,0);
    pLayout->setSpacing(0);
    pContainer->setLayout(pLayout);
    
    QHBoxLayout* pLayoutTitle = new QHBoxLayout(pContainer);
    
    QLabel* pIcon = new QLabel(pContainer);
    int height = pIcon->fontMetrics().height();
    pIcon->setPixmap(getIcon().pixmap(height));
    pLayoutTitle->addWidget(pIcon);
    
    QLabel* pTitle = new QLabel(title().toString(), pContainer);
    pLayoutTitle->addWidget(pTitle);
    pLayoutTitle->addSpacerItem(new QSpacerItem(0, 0, 
                                                QSizePolicy::Expanding, 
                                                QSizePolicy::Minimum));
    pLayout->addLayout(pLayoutTitle);
    
    QWidget* pSidebar = createInnerSidebarWidget(pKeyboard);
    pSidebar->setParent(pContainer);
    pLayout->addWidget(pSidebar);
    
    return pContainer;
}

void LibraryFeature::setFeaturePaneId(int paneId) {
    m_featurePane = paneId;
}

int LibraryFeature::getFeaturePaneId() {
    return m_featurePane;
}

int LibraryFeature::getFocusedPane() {
    return m_pLibrary->getFocusedPaneId();
}

void LibraryFeature::adoptPreselectedPane() {
    int preselectedPane = m_pLibrary->getPreselectedPaneId();
    if (preselectedPane >= 0 &&
            m_featurePane != preselectedPane) {
        m_featurePane = preselectedPane;
        // Refresh preselect button
        emit focusIn(this);
    }
}

SavedSearchQuery LibraryFeature::saveQuery(SavedSearchQuery sQuery) {
    WTrackTableView* pTable = getFocusedTable();
    if (pTable == nullptr) {
        return SavedSearchQuery();
    }
    
    sQuery = pTable->saveQuery(sQuery);
    int qId = m_savedDAO.getQueryId(sQuery);
    if (qId >= 0) {
        QMessageBox box;
        box.setWindowTitle(tr("Query already exists"));
        box.setText(tr("The query already exists in the database. Do you want "
                       "to overwrite it?"));
        box.setDetailedText(tr("The query has title: \"%1\" and query: \"%2\"")
                                .arg(sQuery.title, sQuery.query));
        box.setIcon(QMessageBox::Warning);
        box.addButton(QMessageBox::Yes);
        box.addButton(QMessageBox::No);
        box.setDefaultButton(QMessageBox::Yes);
        box.setEscapeButton(QMessageBox::No);
        
        if (box.exec() == QMessageBox::No) {
            // No pressed
            m_savedDAO.moveToFirst(this, qId);
            return sQuery;
        } else {
            // Yes pressed 
            m_savedDAO.deleteSavedQuery(qId);
        }
    }
    
    // A saved query goes the first in the list
    return m_savedDAO.saveQuery(this, sQuery);
}

void LibraryFeature::restoreQuery(int id) {
    WTrackTableView* pTable = getFocusedTable();
    if (pTable == nullptr) {
        return;
    }
    
    // Move the query to the first position to be reused later by the user
    const SavedSearchQuery& sQuery = m_savedDAO.moveToFirst(this, id);
    pTable->restoreQuery(sQuery);
    restoreSearch(sQuery.query.isNull() ? "" : sQuery.query);
}

QList<SavedSearchQuery> LibraryFeature::getSavedQueries() const {    
    return m_savedDAO.getSavedQueries(this);
}

WTrackTableView* LibraryFeature::createTableWidget(int paneId) {
    WTrackTableView* pTrackTableView = 
            new WTrackTableView(nullptr, m_pConfig, m_pTrackCollection, true);
    m_trackTablesByPaneId[paneId] = pTrackTableView;
        
    WMiniViewScrollBar* pScrollBar = new WMiniViewScrollBar(pTrackTableView);
    pTrackTableView->setScrollBar(pScrollBar);
    
    connect(pTrackTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(pTrackTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)));
    connect(pTrackTableView, SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));
    connect(pTrackTableView, SIGNAL(tableChanged()),
            this, SLOT(restoreSaveButton()));
    
    connect(m_pLibrary, SIGNAL(setTrackTableFont(QFont)),
            pTrackTableView, SLOT(setTrackTableFont(QFont)));
    connect(m_pLibrary, SIGNAL(setTrackTableRowHeight(int)),
            pTrackTableView, SLOT(setTrackTableRowHeight(int)));
    
    return pTrackTableView;
}

QWidget* LibraryFeature::createInnerSidebarWidget(KeyboardEventFilter *pKeyboard) {
    return createLibrarySidebarWidget(pKeyboard);
}

WLibrarySidebar* LibraryFeature::createLibrarySidebarWidget(KeyboardEventFilter*) {
    WLibrarySidebar* pSidebar = new WLibrarySidebar(nullptr);
    QAbstractItemModel* pModel = getChildModel();
    pSidebar->setModel(pModel);
    
    // Set sidebar mini view
    WMiniViewScrollBar* pMiniView = new WMiniViewScrollBar(pSidebar);
    pMiniView->setTreeView(pSidebar);
    pMiniView->setModel(pModel);
    pSidebar->setVerticalScrollBar(pMiniView);
    // invalidate probably stored QModelIndex
    invalidateChild();
    
    connect(pSidebar, SIGNAL(pressed(const QModelIndex&)),
            this, SLOT(activateChild(const QModelIndex&)));
    connect(pSidebar, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(onLazyChildExpandation(const QModelIndex&)));
    connect(pSidebar, SIGNAL(rightClicked(const QPoint&, const QModelIndex&)),
            this, SLOT(onRightClickChild(const QPoint&, const QModelIndex&)));
    connect(pSidebar, SIGNAL(expanded(const QModelIndex&)),
            this, SLOT(onLazyChildExpandation(const QModelIndex&)));
    connect(this, SIGNAL(selectIndex(const QModelIndex&)),
            pSidebar, SLOT(selectIndex(const QModelIndex&)));
    
    connect(pSidebar, SIGNAL(hovered()),
            this, SLOT(slotSetHoveredSidebar()));
    connect(pSidebar, SIGNAL(leaved()),
            this, SLOT(slotResetHoveredSidebar()));
    connect(pSidebar, SIGNAL(focusIn()),
            this, SLOT(slotSetFocusedSidebar()));
    connect(pSidebar, SIGNAL(focusOut()),
            this, SLOT(slotResetFocusedSidebar()));

    return pSidebar;
}

void LibraryFeature::showTrackModel(QAbstractItemModel *model) {
    auto it = m_trackTablesByPaneId.constFind(m_featurePane);
    if (it == m_trackTablesByPaneId.constEnd() || it->isNull()) {
        return;
    }
    (*it)->loadTrackModel(model);
    switchToFeature();
}

void LibraryFeature::switchToFeature() {
    m_pLibrary->switchToFeature(this);
}

void LibraryFeature::restoreSearch(const QString& search) {
    m_pLibrary->restoreSearch(m_featurePane, search);
}

void LibraryFeature::restoreSaveButton() {
    if (m_featurePane >= 0) {
        m_pLibrary->restoreSaveButton(m_featurePane);
    }
}

void LibraryFeature::showBreadCrumb(TreeItem *pTree) {
    m_pLibrary->showBreadCrumb(m_featurePane, pTree);
}

void LibraryFeature::showBreadCrumb(const QModelIndex& index) {
    showBreadCrumb(index.data(AbstractRole::RoleBreadCrumb).toString(),
                   getIcon());
}

void LibraryFeature::showBreadCrumb(const QString &text, const QIcon& icon) {
    m_pLibrary->showBreadCrumb(m_featurePane, text, icon);
}

void LibraryFeature::showBreadCrumb() {
    showBreadCrumb(title().toString(), getIcon());
}

WTrackTableView* LibraryFeature::getFocusedTable() {
    auto it = m_trackTablesByPaneId.constFind(m_featurePane);
    if (it == m_trackTablesByPaneId.constEnd() || it->isNull()) {
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
