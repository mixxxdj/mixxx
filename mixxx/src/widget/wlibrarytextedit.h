// wlibrarytextedit.h
// Created 10/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef WLIBRARYTEXTEDIT_H
#define WLIBRARYTEXTEDIT_H

#include <QTextEdit>

#include "library/libraryview.h"

class WLibraryTextEdit : public QTextEdit, public virtual LibraryView {
    Q_OBJECT
  public:
    WLibraryTextEdit(QWidget* parent = NULL);
    virtual ~WLibraryTextEdit();

    virtual void setup(QDomNode node);
    void onSearchStarting();
    void onSearchCleared();
    void onSearch(const QString& text);
    void onShow();

};


#endif /* WLIBRARYTEXTEDIT_H */
