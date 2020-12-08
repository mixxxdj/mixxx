#pragma once

#include <QLabel>
#include <QEvent>

#include "widget/wbasewidget.h"
#include "skin/skincontext.h"

class WLabel : public QLabel, public WBaseWidget {
    Q_OBJECT
  public:
    explicit WLabel(QWidget* pParent=nullptr);

    virtual void setup(const QDomNode& node, const SkinContext& context);

    QString text() const;
    void setText(const QString& text);

    // The highlight property is used to restyle the widget with CSS.
    // The declaration #MyLabel[highlight="1"] { } will define the style
    // for the highlighted state.
    // See ../wwidgetgroup.h for more info
    Q_PROPERTY(int highlight READ getHighlight WRITE setHighlight NOTIFY highlightChanged)

    int getHighlight() const;
    void setHighlight(int highlight);

  signals:
    void highlightChanged(int highlight);

  protected:
    bool event(QEvent* pEvent) override;
    void resizeEvent(QResizeEvent* event) override;
    void fillDebugTooltip(QStringList* debug) override;
    QString m_skinText;
    // Foreground and background colors.
    QColor m_qFgColor;
    QColor m_qBgColor;
  private:
    QString m_longText;
    Qt::TextElideMode m_elideMode;
    double m_scaleFactor;
    int m_highlight;
};
