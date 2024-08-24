#pragma once

#include <QString>

#include "widget/slidereventhandler.h"
#include "widget/wwidget.h"
#include "widget/wpixmapstore.h"

class QDomNode;
class SkinContext;

/// A widget for a slider composed of a background pixmap and a handle.
class WSliderComposed : public WWidget  {
    Q_OBJECT
  public:
    explicit WSliderComposed(QWidget* parent = nullptr);
    ~WSliderComposed() override;

    // The highlight property is used to restyle the widget with CSS.
    // The declaration #MySlider[highlight="1"] { } will define the style
    // for the highlighted state. Note: The background property does not
    // support color schemes for images, a workaround is to set the background
    // image via <BackPath> and <BackPathHighlighted> from the skin.
    Q_PROPERTY(int highlight READ getHighlight WRITE setHighlight NOTIFY highlightChanged)

    int getHighlight() const;
    void setHighlight(int highlight);

    void setup(const QDomNode& node, const SkinContext& context);
    void setSliderPixmap(
            const PixmapSource& sourceSlider,
            Paintable::DrawMode drawMode,
            double scaleFactor);
    void setHandlePixmap(
            const PixmapSource& sourceHandle,
            Paintable::DrawMode mode,
            double scaleFactor);
    // This is called by LegacySkinParser::setupConnections() before setup()
    // because it needs 'horizontal' for picking the correct keyboard shortcut
    // command (left/right or up/down.
    // Doesn't recognize variables, hence we don't store the result in m_bHorizontal,
    // that's done in setup() where we have a SkinContext, i.e. variable support.
    bool tryParseHorizontal(const QDomNode& node) const;
    void inputActivity();

  signals:
    void highlightChanged(int highlight);

  public slots:
    void onConnectedControlChanged(double dParameter, double dValue) override;
    void fillDebugTooltip(QStringList* debug) override;

  protected:
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    void drawBar(QPainter* pPainter);
    void wheelEvent(QWheelEvent* e) override;
    void resizeEvent(QResizeEvent* pEvent) override;

  private:
    double calculateHandleLength();
    void unsetPixmaps();

    int m_highlight;
    // Length of handle in pixels
    double m_dHandleLength;
    // Length of the slider in pixels.
    double m_dSliderLength;
    // True if it's a horizontal slider
    bool m_bHorizontal;
    // Properties to draw the level bar
    double m_dBarWidth;
    double m_dBarBgWidth;
    double m_dBarStart;
    double m_dBarEnd;
    double m_dBarBgStart;
    double m_dBarBgEnd;
    double m_dBarAxisPos;
    bool m_bBarUnipolar;
    QColor m_barColor;
    QColor m_barBgColor;
    Qt::PenCapStyle m_barPenCap;
    // Pointer to pixmap of the slider
    PaintablePointer m_pSlider;
    // Pointer to pixmap of the handle
    PaintablePointer m_pHandle;
    SliderEventHandler<WSliderComposed> m_handler;

    friend class SliderEventHandler<WSliderComposed>;
};
