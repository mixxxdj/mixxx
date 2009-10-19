// wbrowsetableview.cpp
// Created 10/19/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "widget/wbrowsetableview.h"

WBrowseTableView::WBrowseTableView(QWidget* parent,
                                   ConfigObject<ConfigValue>* pConfig)
        : WLibraryTableView(parent, pConfig) {

}

WBrowseTableView::~WBrowseTableView() {

}

ConfigKey WBrowseTableView::getHeaderStateKey() {
    return ConfigKey("[Library]", "BrowseHeaderState");
}

ConfigKey WBrowseTableView::getVScrollBarPosKey() {
    return ConfigKey("[Library]", "BrowseVScrollBarPos");
}
