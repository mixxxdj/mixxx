// wlibrary.cpp
// Created 8/28/2009 by RJ Ryan (rryan@mit.edu)

#include <widget/wlibrarypane.h>
#include <QtDebug>
#include <QMutexLocker>

#include "library/libraryview.h"
#include "controllers/keyboard/keyboardeventfilter.h"

namespace {
void showLibraryWarning() {
    qDebug() << "WARNING: Attempted to register a view with WLibrary "
      << "that does not implement the LibraryView interface. Ignoring.";
}
}

WLibraryPane::WLibraryPane(QWidget* parent)
        : WBaseLibrary(parent) {
}

bool WLibraryPane::registerView(LibraryFeature* pFeature, QWidget* pView) {
    if (pFeature == nullptr || dynamic_cast<LibraryView*>(pView) == nullptr) {
        showLibraryWarning();
        return false;
    }
    return WBaseLibrary::registerView(pFeature, pView);
}

LibraryView* WLibraryPane::getActiveView() const {
    LibraryView* pView = dynamic_cast<LibraryView*>(currentWidget());
    if (pView == nullptr) {
        showLibraryWarning();
    }
    return pView;
}


void WLibraryPane::switchToFeature(LibraryFeature* pFeature) {
    qDebug() << "switchToFeature" << pFeature;
    auto it = m_viewsByFeature.find(pFeature);
    if (it != m_viewsByFeature.end()) {
        LibraryView* pView = dynamic_cast<LibraryView*>(*it);
        if (pView == nullptr) {
            showLibraryWarning();
            return;
        }
        WBaseLibrary::switchToFeature(pFeature);
        pView->onShow();
    }
}

void WLibraryPane::search(const QString& name) {
    LibraryView* view = getActiveView();
    if (view == nullptr) {
        return;
    }
    view->onSearch(name);
}

void WLibraryPane::searchCleared() {
    LibraryView* view = getActiveView();
    if (view == nullptr) {
        return;
    }
    view->onSearchCleared();
}

void WLibraryPane::searchStarting() {
    LibraryView* view = getActiveView();
    if (view == nullptr) {
        return;
    }
    view->onSearchStarting();
}
