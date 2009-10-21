// wlibrary.cpp
// Created 8/28/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>
#include <QMutexLocker>

#include "widget/wlibrary.h"
#include "library/libraryview.h"

WLibrary::WLibrary(QWidget* parent) : QStackedWidget(parent) {

}

WLibrary::~WLibrary() {

}

bool WLibrary::registerView(QString name, QWidget* view) {
    QMutexLocker lock(&m_mutex);
    if (m_viewMap.contains(name)) {
        return false;
    }
    addWidget(view);
    m_viewMap[name] = view;
    return true;
}

void WLibrary::setup(QDomNode node) {
    QMutexLocker lock(&m_mutex);
    QListIterator<QWidget*> views_it(m_viewMap.values());

    while(views_it.hasNext()) {
        QWidget* widget = views_it.next();
        dynamic_cast<LibraryView*>(widget)->setup(node);
    }
}

void WLibrary::switchToView(const QString& name) {
    QMutexLocker lock(&m_mutex);
    qDebug() << "WLibrary::switchToView" << name;
    if (m_viewMap.contains(name)) {
        QWidget* widget = m_viewMap[name];
        qDebug() << widget << " current:" << currentWidget();
        if (widget != NULL && currentWidget() != widget) {
            setCurrentWidget(m_viewMap[name]);
            m_currentView = name;
        }
    }
}

void WLibrary::search(const QString& name) {
    QMutexLocker lock(&m_mutex);
    QWidget* current = currentWidget();
    LibraryView* view = dynamic_cast<LibraryView*>(current);
    m_searchMap[m_currentView] = name;
    lock.unlock();
    view->onSearch(name);
}

void WLibrary::searchCleared() {
    QMutexLocker lock(&m_mutex);
    QWidget* current = currentWidget();
    LibraryView* view = dynamic_cast<LibraryView*>(current);
    m_searchMap.remove(m_currentView);
    lock.unlock();
    view->onSearchCleared();
}

void WLibrary::searchStarting() {
    QMutexLocker lock(&m_mutex);
    QWidget* current = currentWidget();
    LibraryView* view = dynamic_cast<LibraryView*>(current);
    m_searchMap[m_currentView] = "";
    lock.unlock();
    view->onSearchStarting();
}
