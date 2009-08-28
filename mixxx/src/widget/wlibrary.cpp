// wlibrary.cpp
// Created 8/28/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "widget/wlibrary.h"
#include "library/libraryview.h"

WLibrary::WLibrary(QWidget* parent) : QStackedWidget(parent) {
    
}

WLibrary::~WLibrary() {
    
}

bool WLibrary::registerView(QString name, QWidget* view) {
    if (m_sViewMap.contains(name)) {
        return false;
    }
    addWidget(view);
    m_sViewMap[name] = view;
    return true;
}

void WLibrary::setup(QDomNode node) {
    QListIterator<QWidget*> views_it(m_sViewMap.values());

    while(views_it.hasNext()) {
        QWidget* widget = views_it.next();
        dynamic_cast<LibraryView*>(widget)->setup(node);
    }
}

void WLibrary::switchToView(QString name) {
    qDebug() << "WLibrary::switchToView" << name;
    if (m_sViewMap.contains(name)) {

        QWidget* widget = m_sViewMap[name];
        qDebug() << widget << " current:" << currentWidget();
        if (widget != NULL && currentWidget() != widget) {
            setCurrentWidget(m_sViewMap[name]);
        }
    }
}
