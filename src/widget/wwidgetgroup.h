#pragma once

#include <QFrame>
#include <QPixmap>
#include <QString>
#include <QWidget>

#include "widget/wbasewidget.h"
#include "widget/wpixmapstore.h"

class QDomNode;
class SkinContext;

class WWidgetGroup : public QFrame, public WBaseWidget {
    Q_OBJECT
  public:
    explicit WWidgetGroup(QWidget* pParent=nullptr);

    // QLayouts are not stylable using Qt style sheets. These properties let us
    // style the layout properties using the QProperty support in Qt style
    // sheets:
    //
    // WidgetGroup {
    //  qproperty-layoutSpacing: 10;
    //  qproperty-layoutContentsMargins: rect(1 1 1 1);
    //  qproperty-layoutAlignment: 'AlignRight | AlignBottom';
    //}
    //
    // The property must be DESIGNABLE to style it with Qt CSS.
    Q_PROPERTY(int layoutSpacing READ layoutSpacing WRITE setLayoutSpacing DESIGNABLE true);
    Q_PROPERTY(QRect layoutContentsMargins READ layoutContentsMargins WRITE setLayoutContentsMargins DESIGNABLE true);
    Q_PROPERTY(Qt::Alignment layoutAlignment READ layoutAlignment WRITE setLayoutAlignment DESIGNABLE true);

    // A WWidgetGroup can also be used to highlight a group of widgets by
    // changing the background or any other css style option.
    // Example skin:
    // <WidgetGroup>
    //  <BackPathHighlighted>style/style_bg_effect1_high</BackPathHighlighted>
    //  <Connection>
    //   <ConfigKey>[EffectRack1_EffectUnit1],single_effect_focus</ConfigKey>
    //   <BindProperty>highlight</BindProperty>
    //   <Transform>
    //    <IsEqual>2<IsEqual>
    //   <Transform>
    //  </Connection>
    // </HighlightingGroup>

    // The highlight property is used to restyle the widget with CSS.
    // The declaration #MyGroup[highlight="1"] { } will define the style
    // for the highlighted state. Note: The background property does not
    // support color schemes for images, a workaround is to set the background
    // image via <BackPath> and <BackPathHighlighted> from the skin.
    Q_PROPERTY(int highlight READ getHighlight WRITE setHighlight NOTIFY highlightChanged)

    int layoutSpacing() const;
    void setLayoutSpacing(int spacing);
    QRect layoutContentsMargins() const;
    void setLayoutContentsMargins(QRect rectMargins);
    Qt::Alignment layoutAlignment() const;
    void setLayoutAlignment(Qt::Alignment alignment);
    int getHighlight() const;
    void setHighlight(int highlight);

    virtual void setup(const QDomNode& node, const SkinContext& context);
    void setPixmapBackground(
            const PixmapSource& source,
            Paintable::DrawMode mode,
            double scaleFactor);
    void setPixmapBackgroundHighlighted(
            const PixmapSource& source,
            Paintable::DrawMode mode,
            double scaleFactor);
    void addWidget(QWidget* pChild);

  signals:
    void highlightChanged(int highlight);

  protected:
    void paintEvent(QPaintEvent* pe) override;
    void resizeEvent(QResizeEvent* re) override;
    bool event(QEvent* pEvent) override;
    void fillDebugTooltip(QStringList* debug) override;

    // By default QFrame::sizeHint() (QWidget::sizeHint()) would return an
    // invalid size when no layout (and no fixed size) has been set.
    // If we'd set <Size>100min,100f</Size>, hiding a child would not cause the
    // parent to shrink (or grow when show, depends on initial state??).
    //
    // We override these to get the proper sizeHint for WidgetGroups in such
    // cases and only consider visible children.
    // Note: this only works in conjunction with updateGeometry() in
    // ControlWidgetPropertyConnection::slotControlValueChanged()
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  private:
    // Returns the bounding box of all visible children with absolute
    // positioning (no layout).
    QSize visibleChildrenBoundingBox() const;

    // Associated background pixmap
    PaintablePointer m_pPixmapBack;
    PaintablePointer m_pPixmapBackHighlighted;
    int m_highlight;
};
