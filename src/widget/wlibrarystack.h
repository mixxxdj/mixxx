#ifndef WLIBRARYSTACK_H
#define WLIBRARYSTACK_H

#include <QStackedWidget>

#include "library/libraryview.h"

/* This is a stacked widget that contains LibraryViews inside to, it can be
 * added to the WLibrary and allows compatibility
 */

class WLibraryStack : public QStackedWidget, public LibraryView {
    Q_OBJECT

  public:
    WLibraryStack(QWidget* parent = nullptr);
    ~WLibraryStack();

    int addWidget(QWidget* w);
    int insertWidget(int index, QWidget *w);

    void onShow();
    void onSearch(const QString& text);

    void loadSelectedTrack();
    void slotSendToAutoDJ();
    void slotSendToAutoDJTop();

  private:
    
    bool checkAndWarning(QWidget *w);
};

#endif /* WLIBRARYSTACK_H */
