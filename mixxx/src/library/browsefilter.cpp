// browsefilter.cpp
// Created 10/20/2009 by RJ Ryan (rryan@mit.edu)

#include <QFileSystemModel>

#include "defs_audiofiles.h"
#include "library/browsefilter.h"

BrowseFilter::BrowseFilter(QObject* parent)
        : QSortFilterProxyModel(parent),
          m_regexp(MIXXX_SUPPORTED_AUDIO_FILETYPES_REGEX, Qt::CaseInsensitive){
}

BrowseFilter::~BrowseFilter() {

}

bool BrowseFilter::filterAcceptsRow(int row,
                                    const QModelIndex& parent) const {
    // TODO(XXX) Assumes the model's 0'th column is the filename.
    QModelIndex index = sourceModel()->index(row, 0, parent);
    QString name = sourceModel()->data(index).toString();

    // Exclude .
    if (name == ".")
        return false;

    // Include ..
    if (name == "..")
        return true;

    bool isDir = ((QFileSystemModel*)sourceModel())->isDir(index);

    // TODO(XXX) match directory on search string
    if (isDir)
        return true;

    if (!name.contains(m_regexp))
        return false;

    // TODO(XXX) exclude files that dont match the search string
    return true;
}
