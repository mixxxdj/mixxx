// browsefilter.h
// Created 10/20/2009 by RJ Ryan (rryan@mit.edu)

#ifndef BROWSEFILTER_H
#define BROWSEFILTER_H

#include <QRegExp>
#include <QSortFilterProxyModel>

class BrowseFilter : public QSortFilterProxyModel {
    Q_OBJECT
  public:
    BrowseFilter(QObject* parent = 0);
    virtual ~BrowseFilter();

    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex& parent) const;
  public slots:
    // Set the filter's root to the Proxy index proxyParent
    void setProxyParent(const QModelIndex& proxyParent);
  private:
    bool lessThan(const QModelIndex& left,
                  const QModelIndex& right) const;

    QRegExp m_regexp;
    QModelIndex m_sourceParent;
};


#endif /* BROWSEFILTER_H */
