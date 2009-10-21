
#ifndef WBROWSETABLEVIEW_H
#define WBROWSETABLEVIEW_H

#include "configobject.h"
#include "widget/wlibrarytableview.h"

class WBrowseTableView : public virtual WLibraryTableView {
    Q_OBJECT
  public:
    WBrowseTableView(QWidget* parent, ConfigObject<ConfigValue>* pConfig);
    virtual ~WBrowseTableView();
  private:
};

#endif /* WBROWSETABLEVIEW_H */
