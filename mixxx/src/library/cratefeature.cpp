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
}

CrateFeature::~CrateFeature() {
}

QVariant CrateFeature::title() {
    return tr("Crates");
}

QIcon CrateFeature::getIcon() {
    return QIcon();
}

bool CrateFeature::dropAccept(QUrl url) {
    return false;
}

bool CrateFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    QString crateName = index.data().toString();
    int crateId = m_pTrackCollection->getCrateDAO().getCrateIdByName(crateName);
    int trackId = m_pTrackCollection->getTrackDAO().getTrackId(url.toString());

    //XXX: See the comment in PlaylistFeature::dropAcceptChild() about
    //     QUrl::toLocalFile() vs. QUrl::toString() usage.

    //If the track wasn't found in the database, add it to the DB first.
    if (trackId <= 0)
    {
        trackId = m_pTrackCollection->getTrackDAO().addTrack(url.toString());
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

QAbstractItemModel* CrateFeature::getChildModel() {
    return &m_crateListTableModel;
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

    QString name = QInputDialog::getText(NULL,
                                         tr("New Crate"),
                                         tr("Crate name:"),
                                         QLineEdit::Normal, tr("New Crate"));
    if (name == "")
        return;
    else {
        CrateDAO& crateDao = m_pTrackCollection->getCrateDAO();
        if (crateDao.createCrate(name)) {
            m_crateListTableModel.select();
            // Switch to the new crate.
            int crate_id = crateDao.getCrateIdByName(name);
            m_crateTableModel.setCrate(crate_id);
            emit(showTrackModel(&m_crateTableModel));
            // TODO(XXX) set sidebar selection
            emit(featureUpdated());
        } else {
            qDebug() << "Error creating crate (may already exist) with name " << name;
        }
    }
}

void CrateFeature::slotDeleteCrate() {
    QString crateName = m_lastRightClickedIndex.data().toString();
    int crateId = m_pTrackCollection->getCrateDAO().getCrateIdByName(crateName);

    if (m_pTrackCollection->getCrateDAO().deleteCrate(crateId)) {
        m_crateListTableModel.select();
        emit(featureUpdated());
    } else {
        qDebug() << "Failed to delete crateId" << crateId;
    }
}
