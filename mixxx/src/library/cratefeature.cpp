// cratefeature.cpp
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#include <QInputDialog>
#include <QMenu>
#include <QLineEdit>

#include "library/cratefeature.h"

#include "library/cratetablemodel.h"
#include "library/trackcollection.h"
#include "widget/wlibrarytextbrowser.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "mixxxkeyboard.h"
#include "treeitem.h"

CrateFeature::CrateFeature(QObject* parent,
                           TrackCollection* pTrackCollection)
        : m_pTrackCollection(pTrackCollection),
          m_crateListTableModel(this, pTrackCollection->getDatabase()),
          m_crateTableModel(this, pTrackCollection) {
    m_pCreateCrateAction = new QAction(tr("New Crate"),this);
    connect(m_pCreateCrateAction, SIGNAL(triggered()),
            this, SLOT(slotCreateCrate()));

    m_pDeleteCrateAction = new QAction(tr("Remove"),this);
    connect(m_pDeleteCrateAction, SIGNAL(triggered()),
            this, SLOT(slotDeleteCrate()));

    m_crateListTableModel.setTable("crates");
    m_crateListTableModel.removeColumn(m_crateListTableModel.fieldIndex("id"));
    m_crateListTableModel.removeColumn(m_crateListTableModel.fieldIndex("show"));
    m_crateListTableModel.setSort(m_crateListTableModel.fieldIndex("name"),
                              Qt::AscendingOrder);
    m_crateListTableModel.setFilter("show = 1");
    m_crateListTableModel.select();
    
    //construct child model
    TreeItem *rootItem = new TreeItem("$root","$root", this);

    int idColumn = m_crateListTableModel.record().indexOf("name");
    for (int row = 0; row < m_crateListTableModel.rowCount(); ++row) {
            QModelIndex ind = m_crateListTableModel.index(row, idColumn);
            QString crate_name = m_crateListTableModel.data(ind).toString();
            TreeItem *playlist_item = new TreeItem(crate_name, crate_name, this, rootItem);
            rootItem->appendChild(playlist_item);
            
    }
    m_childModel.setRootItem(rootItem);

}

CrateFeature::~CrateFeature() {
}

QVariant CrateFeature::title() {
    return tr("Crates");
}

QIcon CrateFeature::getIcon() {
    return QIcon(":/images/library/ic_library_crates.png");
}

bool CrateFeature::dropAccept(QUrl url) {
    return false;
}

bool CrateFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    QString crateName = index.data().toString();
    int crateId = m_pTrackCollection->getCrateDAO().getCrateIdByName(crateName);

    //XXX: See the comment in PlaylistFeature::dropAcceptChild() about
    //     QUrl::toLocalFile() vs. QUrl::toString() usage.
    QFileInfo file(url.toLocalFile());
    QString trackLocation = file.absoluteFilePath();

    int trackId = m_pTrackCollection->getTrackDAO().getTrackId(trackLocation);
    //If the track wasn't found in the database, add it to the DB first.
    if (trackId <= 0)
    {
        trackId = m_pTrackCollection->getTrackDAO().addTrack(trackLocation);
    }
    qDebug() << "CrateFeature::dropAcceptChild adding track"
             << trackId << "to crate" << crateId;

    if (trackId >= 0)
        return m_pTrackCollection->getCrateDAO().addTrackToCrate(trackId, crateId);
    return false;
}

bool CrateFeature::dragMoveAccept(QUrl url) {
    return false;
}

bool CrateFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    //TODO: Filter by supported formats regex and reject anything that doesn't match.
    return true;
}

void CrateFeature::bindWidget(WLibrarySidebar* sidebarWidget,
                              WLibrary* libraryWidget,
                              MixxxKeyboard* keyboard) {
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    connect(this, SIGNAL(showPage(const QUrl&)),
            edit, SLOT(setSource(const QUrl&)));
    libraryWidget->registerView("CRATEHOME", edit);
}

TreeItemModel* CrateFeature::getChildModel() {
    return &m_childModel;
}

void CrateFeature::activate() {
    emit(showPage(QUrl("qrc:/html/crates.html")));
    emit(switchToView("CRATEHOME"));
}

void CrateFeature::activateChild(const QModelIndex& index) {
    if (!index.isValid())
        return;
    QString crateName = index.data().toString();
    int crateId = m_pTrackCollection->getCrateDAO().getCrateIdByName(crateName);
    m_crateTableModel.setCrate(crateId);
    emit(showTrackModel(&m_crateTableModel));
}

void CrateFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();
    QMenu menu(NULL);
    menu.addAction(m_pCreateCrateAction);
    menu.exec(globalPos);
}

void CrateFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
    //Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;

    QMenu menu(NULL);
    menu.addAction(m_pCreateCrateAction);
    menu.addSeparator();
    menu.addAction(m_pDeleteCrateAction);
    menu.exec(globalPos);
}

void CrateFeature::slotCreateCrate() {

    bool ok = false;
    QString name = QInputDialog::getText(NULL,
                                         tr("New Crate"),
                                         tr("Crate name:"),
                                         QLineEdit::Normal, tr("New Crate"),
                                         &ok);

    if (!ok)
        return;

    CrateDAO& crateDao = m_pTrackCollection->getCrateDAO();

    if (name == "") {
        QMessageBox::warning(NULL,
                             tr("Crate Creation Failed"),
                             tr("A crate cannot have a blank name."));
        return;
    } else if (crateDao.createCrate(name)) {
    	clearChildModel();
        m_crateListTableModel.select();
        constructChildModel();
        // Switch to the new crate.
        int crate_id = crateDao.getCrateIdByName(name);
        m_crateTableModel.setCrate(crate_id);
        emit(showTrackModel(&m_crateTableModel));
        // TODO(XXX) set sidebar selection
        emit(featureUpdated());
    } else {
        qDebug() << "Error creating crate (may already exist) with name " << name;
        QMessageBox::warning(NULL,
                             tr("Creating Crate Failed"),
                             tr("A crate by that name already exists."));

    }
}

void CrateFeature::slotDeleteCrate() {
    QString crateName = m_lastRightClickedIndex.data().toString();
    int crateId = m_pTrackCollection->getCrateDAO().getCrateIdByName(crateName);

    if (m_pTrackCollection->getCrateDAO().deleteCrate(crateId)) {
        clearChildModel();
        m_crateListTableModel.select();
        constructChildModel();
        emit(featureUpdated());
    } else {
        qDebug() << "Failed to delete crateId" << crateId;
    }
}
/**
  * Purpose: When inserting or removing playlists,
  * we require the sidebar model not to reset.
  * This method queries the database and does dynamic insertion 
*/
void CrateFeature::constructChildModel()
{
    QList<QString> data_list;
    int idColumn = m_crateListTableModel.record().indexOf("name");
    for (int row = 0; row < m_crateListTableModel.rowCount(); ++row) {
            QModelIndex ind = m_crateListTableModel.index(row, idColumn);
            QString crate_name = m_crateListTableModel.data(ind).toString();
            data_list.append(crate_name);
    }    
    m_childModel.insertRows(data_list, 0, m_crateListTableModel.rowCount());  
}
/**
  * Clears the child model dynamically
  */
void CrateFeature::clearChildModel()
{
    m_childModel.removeRows(0,m_crateListTableModel.rowCount());
}

