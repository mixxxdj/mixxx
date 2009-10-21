// wbrowsetableview.cpp
// Created 10/19/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "widget/wbrowsetableview.h"

WBrowseTableView::WBrowseTableView(QWidget* parent,
                                   ConfigObject<ConfigValue>* pConfig)
        : WLibraryTableView(parent, pConfig,
                            ConfigKey("[Library]", "BrowseHeaderState"),
                            ConfigKey("[Library]", "BrowseVScrollBarPos")) {
}

WBrowseTableView::~WBrowseTableView() {

}

