#include "library/relations/dlgrelationinfo.h"

#include <QIcon>

#include "defs_urls.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlgrelationinfo.cpp"

DlgRelationInfo::DlgRelationInfo(Relation* relation)
        : QDialog(nullptr),
          m_pRelation(relation) {
    init();
}

void DlgRelationInfo::init() {
    setupUi(this);
    setWindowIcon(QIcon(MIXXX_ICON_PATH));

    // QDialog buttons
    connect(btnOK,
            &QPushButton::clicked,
            this,
            &DlgRelationInfo::slotOk);
    connect(btnCancel,
            &QPushButton::clicked,
            this,
            &DlgRelationInfo::slotCancel);
}

void DlgRelationInfo::slotOk() {
    // saveRelation();
    accept();
}

void DlgRelationInfo::slotCancel() {
    reject();
}

void DlgRelationInfo::updateRelationMetadataFields() {
    txtTitleA->setText(m_pTrackA->getTitle());
    txtArtistA->setText(m_pTrackA->getArtist());
    txtAlbumA->setText(m_pTrackA->getAlbum());
    txtTitleB->setText(m_pTrackB->getTitle());
    txtArtistB->setText(m_pTrackB->getArtist());
    txtAlbumB->setText(m_pTrackB->getAlbum());
    txtComment->setPlainText(m_pRelation->getComment());
    txtDateAdded->setText(m_pRelation->getDateAdded().date().toString());
}

void DlgRelationInfo::saveRelation() {
    if (!m_pRelation) {
        return;
    }

    m_pRelation->setComment(txtComment->toPlainText());

    // m_pLibrary
    //         ->trackCollectionManager()
    //         ->internalCollection()
    //         ->getRelationDAO()
    //         .saveRelation(m_pRelation);
}
