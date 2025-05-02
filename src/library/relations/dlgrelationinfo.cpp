#include "library/relations/dlgrelationinfo.h"

#include "defs_urls.h"
#include "moc_dlgrelationinfo.cpp"

DlgRelationInfo::DlgRelationInfo(Relation* relation)
        : m_pRelation(relation) {
    init();
}

void DlgRelationInfo::init() {
    setupUi(this);
    setWindowIcon(QIcon(MIXXX_ICON_PATH));
}
