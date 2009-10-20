// browsefilter.h
// Created 10/20/2009 by RJ Ryan (rryan@mit.edu)

#ifndef BROWSEFILTER_H
#define BROWSEFILTER_H

#include <QRegExp>
#include <QSortFilterProxyModel>

class BrowseFilter : public QSortFilterProxyModel {
  public:
    BrowseFilter(QObject* parent = 0);
    virtual ~BrowseFilter();

    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex& parent) const;
  private:
    QRegExp m_regexp;
};


#endif /* BROWSEFILTER_H */
