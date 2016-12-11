#ifndef WHIGHLIGHTINGGROUP_H
#define WHIGHLIGHTINGGROUP_H

#include "widget/wwidgetgroup.h"

class SkinContext;

// The WHighlightingGroup can be used to highlight a group of widgets by
// changing the background or any other css style option.
// Example skin:
// <HighlightingGroup>
//  <Id>1</Id>
//  <BackPathHighlighted>style/style_bg_effect1_high</BackPathHighlighted>
//  <Connection>
//   <ConfigKey>[EffectRack1_EffectUnit1],single_effect_focus</ConfigKey>
//   <BindProperty>highlightedId</BindProperty>
//  </Connection>
// </HighlightingGroup>

class WHighlightingGroup : public WWidgetGroup {
    Q_OBJECT
  public:
    explicit WHighlightingGroup(QWidget* pParent = nullptr);

    // The highlight property is used to restyle the widget with CSS.
    // The declaration #MyGroup[highlight="1"] { } will define the style
    // for the highlighted state. Note: The background property does not
    // support color schemes for images, a workaround is to set the background
    // image via <BackPath> and <BackPathHighlighted> from the skin.
    Q_PROPERTY(int highlight READ getHighlight WRITE setHighlight NOTIFY highlightChanged)

    // The highlightedId property is used to select the highlighted widget
    // group by its Id, which can be set from skin by <Id>4</Id>
    Q_PROPERTY(int highlightedId READ getHighlight WRITE setHighlightedId NOTIFY highlightChanged)

    int getHighlight() const;
    int getHighlightedId() const;
    void setHighlight(int highlight);
    void setHighlightedId(int id);
    void setPixmapBackgroundHighlighted(PixmapSource source, Paintable::DrawMode mode);

    void setup(const QDomNode& node, const SkinContext& context) override;

  signals:
    void highlightChanged(int highlight);
    void highlightedIdChanged(int id);

  protected:
    void paintEvent(QPaintEvent* pe) override;

  private:
    // Associated background pixmap
    PaintablePointer m_pPixmapBackHighlighted;
    int m_highlight;
    int m_id;
    int m_highlightedId;
};

#endif // WHIGHLIGHTINGGROUP_H
