#include "widget/wlibrarycontainer.h"

#include <QLayout>

#include "widget/wlibrary.h"


WLibraryContainer::WLibraryContainer(WLibrary* library,
                                     QDomNode node,
                                     const SkinContext& context,
                                     QWidget* pParent)
        : WWidgetGroup(pParent), m_pLibrary(library) {
    setContentsMargins(0, 0, 0, 0);
    m_pLayout = new QVBoxLayout();
    m_pLayout->setSpacing(0);
    m_pLayout->setContentsMargins(0, 0, 0, 0);
    m_pLayout->setAlignment(Qt::AlignCenter);
    setLayout(m_pLayout);
}

void WLibraryContainer::showEvent(QShowEvent* event) {
    m_pLibrary->parentWidget()->layout()->removeWidget(m_pLibrary);
    m_pLayout->addWidget(m_pLibrary);
}
