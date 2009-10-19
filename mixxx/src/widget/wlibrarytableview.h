// wlibrarytableview.h
// Created 10/19/2009 by RJ Ryan (rryan@mit.edu)

#ifndef WLIBRARYTABLEVIEW_H
#define WLIBRARYTABLEVIEW_H

#include <QString>
#include <QTableView>

#include "configobject.h"
#include "library/libraryview.h"

class WLibraryTableView : public QTableView, public LibraryView {
    Q_OBJECT
  public:
    WLibraryTableView(QWidget* parent,
                      ConfigObject<ConfigValue>* pConfig);
    virtual ~WLibraryTableView();
    virtual void setup(QDomNode node);

  public slots:
    void saveVScrollBarPos();
    void restoreVScrollBarPos();

  private:
    virtual ConfigKey getHeaderStateKey();
    virtual ConfigKey getVScrollBarPosKey();

    void loadHeaderState();
    void saveHeaderState();
    void loadVScrollBarPosState();
    void saveVScrollBarPosState();

    ConfigObject<ConfigValue>* m_pConfig;
    // The position of the vertical scrollbar slider, eg. before a search is
    // executed
    int m_iSavedVScrollBarPos;
};


#endif /* WLIBRARYTABLEVIEW_H */
