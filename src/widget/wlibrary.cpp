// wlibrary.cpp
// Created 8/28/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>
#include <QMutexLocker>

#include "widget/wlibrary.h"
#include "library/libraryview.h"
#include "controllers/keyboard/keyboardeventfilter.h"

WLibrary::WLibrary(QWidget* parent)
        : WBaseLibrary(parent),
          m_mutex(QMutex::Recursive) {
    
}

bool WLibrary::registerView(LibraryFeature *pFeature, QWidget* pView) {
    QMutexLocker lock(&m_mutex);
    if (pFeature == nullptr || dynamic_cast<LibraryView*>(pView) == nullptr) {
        qDebug() << "WARNING: Attempted to register a view with WLibrary "
                 << "that does not implement the LibraryView interface. "
                 << "Ignoring.";
        return false;
    }
    return WBaseLibrary::registerView(pFeature, pView);
}

void WLibrary::switchToFeature(LibraryFeature *pFeature) {
    QMutexLocker lock(&m_mutex);
    auto it = m_featureMap.find(pFeature);
    if (it != m_featureMap.end()) {
        LibraryView* pView = dynamic_cast<LibraryView*>(*it);
        if (pView == nullptr) {
            qDebug() << "WARNING: Attempted to register a view with WLibrary "
                     << "that does not implement the LibraryView interface. "
                     << "Ignoring.";
            return;
        }
        WBaseLibrary::switchToFeature(pFeature);
        pView->onShow();
    }
}

void WLibrary::search(const QString& name) {
    QMutexLocker lock(&m_mutex);
    QWidget* current = currentWidget();
    LibraryView* view = dynamic_cast<LibraryView*>(current);
    if (view == nullptr) {
        qDebug() << "WARNING: Attempted to register a view with WLibrary "
          << "that does not implement the LibraryView interface. Ignoring.";
        return;
    }
    lock.unlock();
    view->onSearch(name);
}

LibraryView* WLibrary::getActiveView() const {
    return dynamic_cast<LibraryView*>(currentWidget());
}
