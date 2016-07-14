#ifndef WTABLEMINIVIEW_H
#define WTABLEMINIVIEW_H

#include <QAbstractTableModel>
#include <QPointer>

#include "widget/wminiviewscrollbar.h"

class WTableMiniView : public WMiniViewScrollBar
{
  public:
    WTableMiniView(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel *model);
    void setSortColumn(int column);

  private slots:
    void refreshCharMap();

  private:    
    QPointer<QAbstractItemModel> m_pModel;
    int m_sortColumn;
};

#endif // WTABLEMINIVIEW_H
