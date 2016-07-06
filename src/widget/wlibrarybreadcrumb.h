#ifndef SRC_WIDGET_WBREADCRUMB_H_
#define SRC_WIDGET_WBREADCRUMB_H_

#include <QLabel>
#include "library/treeitem.h"

class WLibraryBreadCrumb : public QLabel {
    Q_OBJECT

  public:

    WLibraryBreadCrumb(QWidget* parent = nullptr);

    void setText(const QString& text);
    QString text() const;
    
    virtual QSize minimumSizeHint() const;

  public slots:

    void showBreadCrumb(TreeItem* pTree);

  protected:
    
    virtual void resizeEvent(QResizeEvent* pEvent);
    
  private:

    QString m_longText;
};

#endif /* SRC_WIDGET_WBREADCRUMB_H_ */
