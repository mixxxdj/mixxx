#pragma once

#include <QVector>
#include <QPixmap>
#include <QPaintEvent>
#include <QString>

#include "widget/wwidget.h"
#include "widget/wpixmapstore.h"
#include "skin/skincontext.h"

class WDisplay : public WWidget {
   Q_OBJECT
  public:
    explicit WDisplay(QWidget *parent=nullptr);
    ~WDisplay() override;

    void setup(const QDomNode& node, const SkinContext& context);

    void onConnectedControlChanged(double dParameter, double dValue) override;

  protected:
    void paintEvent(QPaintEvent* /*unused*/) override;

    int numPixmaps() const {
        return m_pixmaps.size();
    }

  private:
    void setPixmap(
            QVector<PaintablePointer>* pPixmaps,
            int iPos,
            const QString& filename,
            Paintable::DrawMode mode,
            double scaleFactor);

    void setPixmapBackground(
            const PixmapSource& source,
            Paintable::DrawMode mode,
            double scaleFactor);

    void setPositions(int iNoPos);

    int getPixmapForParameter(double dParameter) const;

    int m_iCurrentPixmap;

    // Free existing pixmaps.
    void resetPositions();

    // Associated background pixmap
    PaintablePointer m_pPixmapBack;

    // List of associated pixmaps.
    QVector<PaintablePointer> m_pixmaps;

    // Whether disabled pixmaps are loaded.
    bool m_bDisabledLoaded;

    // List of disabled pixmaps.
    QVector<PaintablePointer> m_disabledPixmaps;
};
