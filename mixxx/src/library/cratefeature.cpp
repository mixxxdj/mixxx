// cratefeature.cpp
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#include <QInputDialog>
#include <QMenu>
#include <QLineEdit>

#include "library/cratefeature.h"
#include "library/parser.h"
#include "library/parserm3u.h"
#include "library/parserpls.h"

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

    m_pRenameCrateAction = new QAction(tr("Rename"),this);
    connect(m_pRenameCrateAction, SIGNAL(triggered()),
            this, SLOT(slotRenameCrate()));

    m_pLockCrateAction = new QAction(tr("Lock"),this);
    connect(m_pLockCrateAction, SIGNAL(triggered()),
            this, SLOT(slotSetCrateLocked()));

    m_pImportPlaylistAction = new QAction(tr("Import Playlist"),this);
    connect(m_pImportPlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotImportPlaylist()));

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
            CrateDAO crateDao = m_pTrackCollection->getCrateDAO();
            int crateID = crateDao.getCrateIdByName(crate_name);
            bool locked = crateDao.isCrateLocked(crateID);
            
            if (locked) {
                playlist_item->setIcon(QIcon(":/images/library/ic_library_crates.png"));
            }
            else {
                playlist_item->setIcon(QIcon());
            }
            
            rootItem->appendChild(playlist_item);
    }
    m_childModel.setRootItem(rootItem);

}

CrateFeature::~CrateFeature() {
    //delete QActions
    delete m_pCreateCrateAction;
    delete m_pDeleteCrateAction;
    delete m_pRenameCrateAction;
    delete m_pLockCrateAction;
    delete m_pImportPlaylistAction;

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

    CrateDAO& crateDao = m_pTrackCollection->getCrateDAO();

    if (crateDao.isCrateLocked(crateId)) {
        QMessageBox::warning(NULL,
                             tr("Unable to add tracks to the crate"),
                             tr("The crate is locked. Please unlock it before adding tracks."));
    }
    else if (trackId >= 0)
        return crateDao.addTrackToCrate(trackId, crateId);
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
    QString crateName = index.data().toString();
    CrateDAO& crateDAO = m_pTrackCollection->getCrateDAO();
    int crateId = crateDAO.getCrateIdByName(crateName);
    bool locked = crateDAO.isCrateLocked(crateId);

    if (locked) {
        m_pLockCrateAction->setText(tr("Unlock"));
    }
    else {
        m_pLockCrateAction->setText(tr("Lock"));
    }

    QMenu menu(NULL);
    menu.addAction(m_pCreateCrateAction);
    menu.addSeparator();
    menu.addAction(m_pRenameCrateAction);

    if (!locked) {
        menu.addAction(m_pDeleteCrateAction);
    }
    
    menu.addAction(m_pLockCrateAction);
    menu.addSeparator();
    menu.addAction(m_pImportPlaylistAction);
    menu.exec(globalPos);
}

void CrateFeature::slotCreateCrate() {

    QString name;
    bool validNameGiven = false;
    CrateDAO& crateDao = m_pTrackCollection->getCrateDAO();
    
    do {
        bool ok = false;
        name = QInputDialog::getText(NULL,
                                     tr("New Crate"),
                                     tr("Crate name:"),
                                     QLineEdit::Normal, tr("New Crate"),
                                     &ok).trimmed();

        if (!ok)
            return;

        int existingId = crateDao.getCrateIdByName(name);
        
        if (existingId != -1) {
            QMessageBox::warning(NULL,
                                 tr("Creating Crate Failed"),
                                 tr("A crate by that name already exists."));
        }
        else if (name.isEmpty()) {
            QMessageBox::warning(NULL,
                                 tr("Creating Crate Failed"),
                                 tr("A crate cannot have a blank name."));
        }
        else {
            validNameGiven = true;
        }
        
    } while (!validNameGiven);

    bool crateCreated = crateDao.createCrate(name);
    
    if (crateCreated) {
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
        qDebug() << "Error creating crate with name " << name;
        QMessageBox::warning(NULL,
                             tr("Creating Crate Failed"),
                             tr("An unknown error occurred while creating crate: ")
                             + name);

    }
}

void CrateFeature::slotDeleteCrate() {
    QString crateName = m_lastRightClickedIndex.data().toString();
    CrateDAO crateDao = m_pTrackCollection->getCrateDAO();
    int crateId = crateDao.getCrateIdByName(crateName);

    if (!crateDao.isCrateLocked(crateId) &&
        crateDao.deleteCrate(crateId)) {
        
        clearChildModel();
        m_crateListTableModel.select();
        constructChildModel();
        emit(featureUpdated());
    } else {
        qDebug() << "Failed to delete crateId" << crateId;
    }
}

void CrateFeature::slotRenameCrate() {
    QString oldName = m_lastRightClickedIndex.data().toString();
    int crateId = m_pTrackCollection->getCrateDAO().getCrateIdByName(oldName);

    QString newName;
    bool validNameGiven = false;
    
    do {
        bool ok = false;
        newName = QInputDialog::getText(NULL,
                                        tr("Rename Crate"),
                                        tr("New crate name:"),
                                        QLineEdit::Normal,
                                        oldName,
                                        &ok).trimmed();
                                        
        if (!ok || newName == oldName) {
            return;
        }

        int existingId = m_pTrackCollection->getCrateDAO().getCrateIdByName(newName);

        if (existingId != -1) {
            QMessageBox::warning(NULL,
                                tr("Renaming Crate Failed"),
                                tr("A crate by that name already exists."));
        }
        else if (newName.isEmpty()) {
            QMessageBox::warning(NULL,
                                tr("Renaming Crate Failed"),
                                tr("A crate cannot have a blank name."));
        }
        else {
            validNameGiven = true;
        }
    } while (!validNameGiven);


    if (m_pTrackCollection->getCrateDAO().renameCrate(crateId, newName)) {
        clearChildModel();
        m_crateListTableModel.select();
        constructChildModel();
        emit(featureUpdated());
        m_crateTableModel.setCrate(crateId);
    } else {
        qDebug() << "Failed to rename crateId" << crateId;
    }
}

void CrateFeature::slotSetCrateLocked()
{
    QString crateName = m_lastRightClickedIndex.data().toString();
    CrateDAO& crateDAO = m_pTrackCollection->getCrateDAO();
    int crateId = crateDAO.getCrateIdByName(crateName);
    bool locked = !crateDAO.isCrateLocked(crateId);

    if (!crateDAO.setCrateLocked(crateId, locked)) {
        qDebug() << "Failed to toggle lock of crateId " << crateId;
    }

    TreeItem* crateItem = m_childModel.getItem(m_lastRightClickedIndex);
    
    if (locked) {
        crateItem->setIcon(QIcon(":/images/library/ic_library_crates.png"));
    }
    else {
        crateItem->setIcon(QIcon());
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
void CrateFeature::slotImportPlaylist()
{
    qDebug() << "slotImportPlaylist() row:" ; //<< m_lastRightClickedIndex.data();


    QString playlist_file = QFileDialog::getOpenFileName
            (
            NULL,
            tr("Import Playlist"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation),
            tr("Playlist Files (*.m3u *.pls)") 
            );
    //Exit method if user cancelled the open dialog.
    if (playlist_file.isNull() || playlist_file.isEmpty() ) return;

    Parser* playlist_parser = NULL;
    
    if(playlist_file.endsWith(".m3u", Qt::CaseInsensitive))
    {
        playlist_parser = new ParserM3u();
    }
    else if(playlist_file.endsWith(".pls", Qt::CaseInsensitive))
    {
        playlist_parser = new ParserPls();
    }
    else
    {
        return;
    }
    
    QList<QString> entries = playlist_parser->parse(playlist_file);

    //Iterate over the List that holds URLs of playlist entires
    for (int i = 0; i < entries.size(); ++i) {
        m_crateTableModel.addTrack(QModelIndex(), entries[i]);
        
    }

    //delete the parser object
    if(playlist_parser) delete playlist_parser;
    
    
}

