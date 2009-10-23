// cratefeature.cpp
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#include <QInputDialog>
#include <QMenu>
#include <QLineEdit>

#include "library/cratefeature.h"

#include "library/trackcollection.h"

CrateFeature::CrateFeature(QObject* parent,
                           TrackCollection* pTrackCollection)
        : m_pTrackCollection(pTrackCollection),
          m_crateTableModel(this, pTrackCollection->getDatabase()) {

    m_pCreateCrateAction = new QAction(tr("New Crate"),this);
    connect(m_pCreateCrateAction, SIGNAL(triggered()),
            this, SLOT(slotCreateCrate()));

    m_pDeleteCrateAction = new QAction(tr("Remove"),this);
    connect(m_pDeleteCrateAction, SIGNAL(triggered()),
            this, SLOT(slotDeleteCrate()));

    m_crateTableModel.setTable("crates");
    m_crateTableModel.setSort(m_crateTableModel.fieldIndex("name"),
                              Qt::AscendingOrder);
    m_crateTableModel.setFilter("show = 1");
    m_crateTableModel.select();
}

CrateFeature::~CrateFeature() {
}

QVariant CrateFeature::title() {
    return tr("Crates");
}

QIcon CrateFeature::getIcon() {
    return QIcon();
}

int CrateFeature::numChildren() {
    return m_crateTableModel.rowCount();
}

QVariant CrateFeature::child(int n) {
    int nameColumn = m_crateTableModel.fieldIndex("name");
    QModelIndex nameIndex = m_crateTableModel.index(n, nameColumn);
    return m_crateTableModel.data(nameIndex);
}

bool CrateFeature::dropAccept(const QModelIndex& index, QUrl url) {
    return false;
}

bool CrateFeature::dragMoveAccept(const QModelIndex& index, QUrl url) {
    return false;
}

QAbstractItemModel* CrateFeature::getChildModel() {
    return &m_crateTableModel;
}

void CrateFeature::activate() {

}

void CrateFeature::activateChild(int n) {

}

void CrateFeature::onRightClick(const QPoint& globalPos, QModelIndex index) {
    //Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;

    QMenu menu(NULL);
    menu.addAction(m_pCreateCrateAction);
    menu.addSeparator();
    menu.addAction(m_pDeleteCrateAction);
    menu.exec(globalPos);
}

void CrateFeature::onClick(QModelIndex index) {

}


void CrateFeature::slotCreateCrate() {

    QString name = QInputDialog::getText(NULL,
                                         tr("New Crate"),
                                         tr("Crate name:"),
                                         QLineEdit::Normal, tr("New Crate"));
    if (name == "")
        return;
    else {
        if (m_pTrackCollection->getCrateDAO().createCrate(name)) {
            m_crateTableModel.select();
            emit(featureUpdated());
        } else {
            qDebug() << "Error creating crate (may already exist) with name " << name;
        }
    }
}

void CrateFeature::slotDeleteCrate() {
    int n = m_lastRightClickedIndex.row();
    int idColumn = m_crateTableModel.fieldIndex("id");
    QModelIndex idIndex = m_crateTableModel.index(n, idColumn);
    int crateId = m_crateTableModel.data(idIndex).toInt();
    if (m_pTrackCollection->getCrateDAO().deleteCrate(crateId)) {
        m_crateTableModel.select();
        emit(featureUpdated());
    } else {
        qDebug() << "Failed to delete crateId" << crateId;
    }
}
