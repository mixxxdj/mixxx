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
    //  qproperty-layoutSpacing: 10;
    //  qproperty-layoutContentsMargins: rect(1 1 1 1);
    //  qproperty-layoutAlignment: 'AlignRight | AlignBottom';
    //}
    //
    // The property must be DESIGNABLE to style it with Qt CSS.
    Q_PROPERTY(int layoutSpacing WRITE setLayoutSpacing DESIGNABLE true);
    Q_PROPERTY(QRect layoutContentsMargins WRITE setLayoutContentsMargins DESIGNABLE true);
    Q_PROPERTY(Qt::Alignment layoutAlignment WRITE setLayoutAlignment DESIGNABLE true);

    void setLayoutSpacing(int spacing);
    void setLayoutContentsMargins(QRect margins);
    void setLayoutAlignment(int alignment);

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
