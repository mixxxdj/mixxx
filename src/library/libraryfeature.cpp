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

parented_ptr<QWidget> LibraryFeature::createPaneWidget(KeyboardEventFilter* pKeyboard, 
                                          int paneId, QWidget* parent) {
    Q_UNUSED(pKeyboard);
    return std::move(createTableWidget(paneId, parent));
}

parented_ptr<QWidget> LibraryFeature::createSidebarWidget(KeyboardEventFilter* pKeyboard, QWidget* parent) {
    //qDebug() << "LibraryFeature::bindSidebarWidget";
    auto pContainer = make_parented<QFrame>(parent);
    pContainer->setContentsMargins(0,0,0,0);
    
    auto pLayout = make_parented<QVBoxLayout>(pContainer.get());
    pLayout->setContentsMargins(0,0,0,0);
    pLayout->setSpacing(0);
    pContainer->setLayout(pLayout.get());
    
    auto pLayoutTitle = make_parented<QHBoxLayout>(pContainer.get());
    
    auto pIcon = make_parented<QLabel>(pContainer.get());
    int height = pIcon->fontMetrics().height();
    pIcon->setPixmap(getIcon().pixmap(height));
    pLayoutTitle->addWidget(pIcon.get());
    
    auto pTitle = make_parented<QLabel>(title().toString(), pContainer.get());
    pLayoutTitle->addWidget(pTitle.get());
    pLayoutTitle->addSpacerItem(new QSpacerItem(0, 0, 
                                                QSizePolicy::Expanding, 
                                                QSizePolicy::Minimum));
    pLayout->addLayout(pLayoutTitle.get());
    
    auto pSidebar = createInnerSidebarWidget(pKeyboard, pContainer.get());
    pLayout->addWidget(pSidebar.get());
    
    return std::move(pContainer);
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

parented_ptr<WTrackTableView> LibraryFeature::createTableWidget(int paneId, QWidget* parent) {
    auto pTrackTableView = make_parented<WTrackTableView>(parent, m_pConfig, 
            m_pTrackCollection, true);
    
    m_trackTablesByPaneId[paneId] = pTrackTableView.get();
        
    auto pScrollBar = make_parented<WMiniViewScrollBar>(pTrackTableView.get());
    pTrackTableView->setScrollBar(pScrollBar.get());
    
    connect(pTrackTableView.get(), SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(pTrackTableView.get(), SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)));
    connect(pTrackTableView.get(), SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));
    connect(pTrackTableView.get(), SIGNAL(tableChanged()),
            this, SLOT(restoreSaveButton()));
    
    connect(m_pLibrary, SIGNAL(setTrackTableFont(QFont)),
            pTrackTableView.get(), SLOT(setTrackTableFont(QFont)));
    connect(m_pLibrary, SIGNAL(setTrackTableRowHeight(int)),
            pTrackTableView.get(), SLOT(setTrackTableRowHeight(int)));
    
    return std::move(pTrackTableView);
}

parented_ptr<QWidget> LibraryFeature::createInnerSidebarWidget(KeyboardEventFilter* pKeyboard, QWidget* parent) {
    Q_UNUSED(pKeyboard);
    return std::move(createLibrarySidebarWidget(parent));
}

parented_ptr<WLibrarySidebar> LibraryFeature::createLibrarySidebarWidget(QWidget* parent) {
    auto pSidebar = make_parented<WLibrarySidebar>(parent);
    QAbstractItemModel* pModel = getChildModel();
    pSidebar->setModel(pModel);
    
    // Set sidebar mini view
    auto pMiniView = make_parented<WMiniViewScrollBar>(pSidebar.get());
    pMiniView->setTreeView(pSidebar.get());
    pMiniView->setModel(pModel);
    pSidebar->setVerticalScrollBar(pMiniView.get());
    // invalidate probably stored QModelIndex
    invalidateChild();
    
    connect(pSidebar.get(), SIGNAL(pressed(const QModelIndex&)),
            this, SLOT(activateChild(const QModelIndex&)));
    connect(pSidebar.get(), SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(onLazyChildExpandation(const QModelIndex&)));
    connect(pSidebar.get(), SIGNAL(rightClicked(const QPoint&, const QModelIndex&)),
            this, SLOT(onRightClickChild(const QPoint&, const QModelIndex&)));
    connect(pSidebar.get(), SIGNAL(expanded(const QModelIndex&)),
            this, SLOT(onLazyChildExpandation(const QModelIndex&)));
    connect(this, SIGNAL(selectIndex(const QModelIndex&)),
            pSidebar.get(), SLOT(selectIndex(const QModelIndex&)));
    
    connect(pSidebar.get(), SIGNAL(hovered()),
            this, SLOT(slotSetHoveredSidebar()));
    connect(pSidebar.get(), SIGNAL(leaved()),
            this, SLOT(slotResetHoveredSidebar()));
    connect(pSidebar.get(), SIGNAL(focusIn()),
            this, SLOT(slotSetFocusedSidebar()));
    connect(pSidebar.get(), SIGNAL(focusOut()),
            this, SLOT(slotResetFocusedSidebar()));

    return std::move(pSidebar);
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

QStringList LibraryFeature::getPlaylistFiles(QFileDialog::FileMode mode) const {
    QString lastPlaylistDirectory = m_pConfig->getValue(
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
