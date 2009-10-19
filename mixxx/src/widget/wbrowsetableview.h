
#ifndef WBROWSETABLEVIEW_H
#define WBROWSETABLEVIEW_H

#include "configobject.h"
#include "widget/wlibrarytableview.h"

class WBrowseTableView : public WLibraryTableView {
    Q_OBJECT
  public:
    WBrowseTableView(QWidget* parent, ConfigObject<ConfigValue>* pConfig);
    virtual ~WBrowseTableView();
  private:
    virtual ConfigKey getHeaderStateKey();
    virtual ConfigKey getVScrollBarPosKey();
};

#endif /* WBROWSETABLEVIEW_H */
