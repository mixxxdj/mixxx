/*
 * WBreadCrumb.cpp
 *
 *  Created on: Jun 24, 2016
 *      Author: joan
 */

#include <wbreadcrumb.h>


WBreadCrumb::WBreadCrumb(QWidget* parent) 
		: QLabel(parent) {
	
    
}

void WBreadCrumb::switchToView(TreeItem *pTree) {
    
}

QString WBreadCrumb::getData(TreeItem* pTree) {
    QString text = pTree->data().toString();
}
