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

void WBreadCrumb::setBreadText(TreeItem *pTree) {
    QString text = getData(pTree);
    setText(text);
}

QString& WBreadCrumb::getData(TreeItem* pTree) {
    // Base case
    if (pTree == nullptr) {
        return "";
    }
    
    // Recursive case
    QString text = pTree->data().toString();
    QString next = getData(pTree->parent());
    return (next == "" ? text : next + " > " + text);
}
