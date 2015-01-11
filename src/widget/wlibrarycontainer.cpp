#include "widget/wlibrarycontainer.h"

#include <QLayout>

#include "widget/wlibrary.h"


WLibraryContainer::WLibraryContainer(WLibrary* library,
                                     QDomNode node,
                                     const SkinContext& context,
                                     QWidget* pParent)
        : WWidgetGroup(pParent), m_pLibrary(library) {
    setContentsMargins(0, 0, 0, 0);

    QLayout* pLayout = new QVBoxLayout();
    pLayout->setSpacing(0);
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->setAlignment(Qt::AlignCenter);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setMaximumSize(1920, 1080);
    pLayout->setSizeConstraint(QLayout::SetNoConstraint);

//    if (pLayout && context.hasNode(node, "SizeConstraint")) {
//        QMap<QString, QLayout::SizeConstraint> constraints;
//        constraints["SetDefaultConstraint"] = QLayout::SetDefaultConstraint;
//        constraints["SetFixedSize"] = QLayout::SetFixedSize;
//        constraints["SetMinimumSize"] = QLayout::SetMinimumSize;
//        constraints["SetMaximumSize"] = QLayout::SetMaximumSize;
//        constraints["SetMinAndMaxSize"] = QLayout::SetMinAndMaxSize;
//        constraints["SetNoConstraint"] = QLayout::SetNoConstraint;
//
//        QString sizeConstraintStr = context.selectString(node, "SizeConstraint");
//        if (constraints.contains(sizeConstraintStr)) {
//            pLayout->setSizeConstraint(constraints[sizeConstraintStr]);
//        } else {
//            qDebug() << "Could not parse SizeConstraint:" << sizeConstraintStr;
//        }
//    }

    if (pLayout) {
        setLayout(pLayout);
    }
}

void WLibraryContainer::setVisible(bool visible) {
    if (visible) {
        qDebug() << this << "REPARENTING!";
        m_pLibrary->setParent(this);
        m_pLibrary->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        m_pLibrary->setMaximumSize(1920,1080);
        //m_pLibrary->setFixedSize(1920,1080);
        m_pLibrary->show();
    } else {
        qDebug() << this << "gone invisible";
    }
}
