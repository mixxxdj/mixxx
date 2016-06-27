#ifndef SRC_WIDGET_WBREADCRUMB_H_
#define SRC_WIDGET_WBREADCRUMB_H_

#include <QLabel>
#include "library/treeitem.h"

class WBreadCrumb: public QLabel {
	Q_OBJECT
	
  public:
	
    WBreadCrumb(QWidget* parent = nullptr);
	
  public slots:
    
    void setBreadText(TreeItem* pTree);
    
  private:
    
    static QString& getData(TreeItem* pTree);
};

#endif /* SRC_WIDGET_WBREADCRUMB_H_ */
