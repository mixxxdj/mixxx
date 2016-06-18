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

bool WLibrary::registerView(QString name, QWidget* view) {
    QMutexLocker lock(&m_mutex);
    //qDebug() << "WLibrary::registerView" << name;
    if (dynamic_cast<LibraryView*>(view) == nullptr) {
        qDebug() << "WARNING: Attempted to register a view with WLibrary "
                 << "that does not implement the LibraryView interface. "
                 << "Ignoring.";
        return false;
    }
    return WBaseLibrary::registerView(name, view);
}

void WLibrary::switchToView(const QString& name) {
    QMutexLocker lock(&m_mutex);
    //qDebug() << "WLibrary::switchToView" << name;
    QWidget* widget = m_viewMap.value(name, nullptr);
    if (widget != nullptr) {
        LibraryView * lview = dynamic_cast<LibraryView*>(widget);
        if (lview == nullptr) {
            qDebug() << "WARNING: Attempted to register a view with WLibrary "
                     << "that does not implement the LibraryView interface. "
                     << "Ignoring.";
            return;
        }
        
        WBaseLibrary::switchToView(name);
        lview->onShow();
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
