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

class SkinContext;

class WWidgetGroup : public QFrame, public WBaseWidget {
    Q_OBJECT
  public:
    explicit WWidgetGroup(QWidget* pParent=nullptr);

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
    void setLayoutContentsMargins(QRect rectMargins);
    Qt::Alignment layoutAlignment() const;
    void setLayoutAlignment(int alignment);

    void setup(const QDomNode& node, const SkinContext& context);
    void setPixmapBackground(PixmapSource source, Paintable::DrawMode mode);
    void addWidget(QWidget* pChild);

  protected:
    void paintEvent(QPaintEvent* pe) override;
    void resizeEvent(QResizeEvent* re) override;
    bool event(QEvent* pEvent) override;
    void fillDebugTooltip(QStringList* debug) override;

  private:
    // Associated background pixmap
    PaintablePointer m_pPixmapBack;
};

#endif // WWIDGETGROUP_H
