#ifndef SRC_WIDGET_WBREADCRUMB_H_
#define SRC_WIDGET_WBREADCRUMB_H_

#include <QLabel>
#include "library/treeitem.h"

class WLibraryBreadCrumb: public QLabel {
	Q_OBJECT
	
  public:
	
    WLibraryBreadCrumb(QWidget* parent = nullptr);
	
  public slots:
    
    void showBreadCrumb(TreeItem* pTree);
};

#endif /* SRC_WIDGET_WBREADCRUMB_H_ */
