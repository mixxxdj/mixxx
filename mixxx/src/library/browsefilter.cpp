// browsefilter.cpp
// Created 10/20/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>
#include <QFileSystemModel>

#include "defs_audiofiles.h"
#include "library/browsefilter.h"

BrowseFilter::BrowseFilter(QObject* parent)
        : QSortFilterProxyModel(parent),
          m_regexp(MIXXX_SUPPORTED_AUDIO_FILETYPES_REGEX, Qt::CaseInsensitive) {
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

BrowseFilter::~BrowseFilter() {

}

void BrowseFilter::setProxyParent(const QModelIndex& proxyParent) {
    m_sourceParent = mapToSource(proxyParent);
}

bool BrowseFilter::filterAcceptsRow(int sourceRow,
                                    const QModelIndex& sourceParent) const {

    // If we do not accept the hierarchy above m_parent, then nothing will be
    // shown.
    if (sourceParent != m_sourceParent)
        return true;

    // TODO(XXX) Assumes the model's 0'th column is the filename.
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    QString name = sourceModel()->data(index).toString();

    // Exclude .
    if (name == ".")
        return false;

    // Include ..
    if (name == "..")
        return true;

    bool isDir = ((QFileSystemModel*)sourceModel())->isDir(index);

    // Only include directories that match the search string.
    if (isDir && name.contains(filterRegExp()))
        return true;

    // Skip files that don't match the extension filter.
    if (!name.contains(m_regexp))
        return false;

    // Only include files that match the search string.
    return name.contains(filterRegExp());
}
