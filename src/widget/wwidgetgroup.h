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
    // </WidgetGroup>

    // The highlight property is used to restyle the widget with CSS.
    // The declaration #MyGroup[highlight="1"] { } will define the style
    // for the highlighted state. Note: The background property does not
    // support color schemes for images, a workaround is to set the background
    // image via <BackPath> and <BackPathHighlighted> from the skin.
    Q_PROPERTY(int highlight READ getHighlight WRITE setHighlight NOTIFY highlightChanged)
    // Similar to the property above the following properties can be used to highlight
    // deck control groups (channels).
    Q_PROPERTY(int highlight_input_allow READ getHighlightInputAllow WRITE
                    setHighlightInputAllow NOTIFY highlightInputAllowChanged)
    Q_PROPERTY(int highlight_channel READ getHighlightChannel WRITE setHighlightChannel
                    NOTIFY highlightChannelChanged)

    int layoutSpacing() const;
    void setLayoutSpacing(int spacing);
    QRect layoutContentsMargins() const;
    void setLayoutContentsMargins(QRect rectMargins);
    Qt::Alignment layoutAlignment() const;
    void setLayoutAlignment(Qt::Alignment alignment);
    int getHighlight() const;
    void setHighlight(int highlight);
    int getHighlightInputAllow() const;
    void setHighlightInputAllow(int highlight_input_allow);
    int getHighlightChannel() const;
    void setHighlightChannel(int highlight_channel);

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
    void highlightInputAllowChanged(int highlight_input_allow);
    void highlightChannelChanged(int highlight_channel);

  protected:
    void paintEvent(QPaintEvent* pe) override;
    void resizeEvent(QResizeEvent* re) override;
    bool event(QEvent* pEvent) override;
    void fillDebugTooltip(QStringList* debug) override;

  private:
    // Associated background pixmap
    PaintablePointer m_pPixmapBack;
    PaintablePointer m_pPixmapBackHighlighted;
    int m_highlight;
    int m_highlight_input_allow;
    int m_highlight_channel;
};
