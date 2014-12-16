#ifndef WWIDGETGROUP_H
#define WWIDGETGROUP_H

#include <QDomNode>
#include <QFrame>
#include <QPaintEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QString>
#include <QWidget>
#include <QEvent>

#include "widget/wbasewidget.h"
#include "widget/wpixmapstore.h"
#include "skin/skincontext.h"

class WWidgetGroup : public QFrame, public WBaseWidget {
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
    Q_PROPERTY(int layoutSpacing READ layoutSpacing WRITE setLayoutSpacing DESIGNABLE true);
    Q_PROPERTY(QRect layoutContentsMargins READ layoutContentsMargins WRITE setLayoutContentsMargins DESIGNABLE true);
    Q_PROPERTY(Qt::Alignment layoutAlignment READ layoutAlignment WRITE setLayoutAlignment DESIGNABLE true);

    int layoutSpacing() const;
    void setLayoutSpacing(int spacing);
    QRect layoutContentsMargins() const;
    void setLayoutContentsMargins(QRect margins);
    Qt::Alignment layoutAlignment() const;
    void setLayoutAlignment(int alignment);

    void setup(QDomNode node, const SkinContext& context);
    void setPixmapBackground(PixmapSource source, Paintable::DrawMode mode);
    void addWidget(QWidget* pChild);

  protected:
    virtual void paintEvent(QPaintEvent* pe);
    virtual void resizeEvent(QResizeEvent* re);
    bool event(QEvent* pEvent);
    void fillDebugTooltip(QStringList* debug);

  private:
    // Associated background pixmap
    PaintablePointer m_pPixmapBack;
};

#endif // WWIDGETGROUP_H
