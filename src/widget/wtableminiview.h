#ifndef WTABLEMINIVIEW_H
#define WTABLEMINIVIEW_H

#include <QAbstractTableModel>
#include <QPointer>

#include "widget/wminiviewscrollbar.h"

class WTableMiniView : public WMiniViewScrollBar
{
  public:
    WTableMiniView(QWidget* parent = nullptr);

    void setSortColumn(int column);

  private slots:
    void refreshCharMap() override;

  private:    
    int m_sortColumn;
};

#endif // WTABLEMINIVIEW_H
