#include "widget/wlibrarysidebar.h"

WLibrarySidebar::WLibrarySidebar(QWidget* parent) : QTreeView(parent) {
    setHeaderHidden(true);
}

WLibrarySidebar::~WLibrarySidebar() {
}
