#include "wmenubar.h"

WMenuBar::WMenuBar(QWidget* pParent, QList<QMenu*> pMenus)
            : QMenuBar(pParent), WBaseWidget(pParent) {
    foreach (QMenu* pMenu, pMenus) {
        addMenu(pMenu);
    }
}
