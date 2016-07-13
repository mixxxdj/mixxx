#ifndef WMINIVIEWSCROLLBAR_H
#define WMINIVIEWSCROLLBAR_H

#include <QScrollBar>

class WMiniViewScrollBar : public QScrollBar
{
  public:
    WMiniViewScrollBar(QWidget* parent = nullptr);

    void setShowLetters(bool show);
    bool showLetters() const;

  protected:
    void paintEvent(QPaintEvent* event) override;
    void lettersPaint(QPaintEvent* event);

    QHash<QChar, int> m_count;
    QVector<QChar> m_letters;
    
  private:
    int interpolHeight(int current, int min1, int max1, int min2, int max2);
    
    bool m_showLetters;
};

#endif // WMINIVIEWSCROLLBAR_H
