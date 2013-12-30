#ifndef WWIDGETGROUP_H
#define WWIDGETGROUP_H

#include <QDomNode>
#include <QGroupBox>
#include <QPaintEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QString>
#include <QWidget>

#include "widget/wpixmapstore.h"

class WWidgetGroup : public QGroupBox {
    Q_OBJECT
  public:
    WWidgetGroup(QWidget* pParent=NULL);
    virtual ~WWidgetGroup();

    // QLayouts are not stylable using Qt style sheets. These properties let us
    // style the layout properties using the QProperty support in Qt style
    // sheets:
    //
    // WWidgetGroup {
    //  qproperty-spacing: 10;
    //  qproperty-contentsMargins: rect(1 1 1 1);
    //}
    //
    // The property must be DESIGNABLE to style it with Qt CSS.
    Q_PROPERTY(int spacing WRITE setLayoutSpacing DESIGNABLE true);
    Q_PROPERTY(QRect contentsMargins WRITE setLayoutContentsMargins DESIGNABLE true);

    void setLayoutSpacing(int spacing);
    void setLayoutContentsMargins(QRect margins);

    void setup(QDomNode node);
    void setPixmapBackground(const QString &filename);
    void addWidget(QWidget* pChild);

  protected:
    virtual void paintEvent(QPaintEvent* pe);
    virtual void resizeEvent(QResizeEvent* re);

  private:
    // Associated background pixmap
    QPixmapPointer m_pPixmapBack;
    QPixmap m_pixmapBackScaled;
};

#endif // WWIDGETGROUP_H
