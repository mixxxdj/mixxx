#ifndef WMINIVIEWSCROLLBAR_H
#define WMINIVIEWSCROLLBAR_H

#include <QAbstractItemModel>
#include <QMutex>
#include <QPointer>
#include <QScrollBar>

class WMiniViewScrollBar : public QScrollBar
{
  public:
    WMiniViewScrollBar(QWidget* parent = nullptr);

    void setShowLetters(bool show);
    bool showLetters() const;
    
    // Sets the letters to be shown and triggers a update
    void setLetters(const QVector<QPair<QChar, int> >& letters);
    void setModel(QAbstractItemModel *model);

  protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void resizeEvent(QResizeEvent*pEvent);
    virtual void refreshCharMap() = 0;
    void lettersPaint(QPaintEvent*);
    
    QPointer<QAbstractItemModel> m_pModel;
    
  private:
    // The purpose of this function is to avoid computing all the sizes in the
    // paintEvent function which can block the GUI thread
    void computeLettersSize();
    
    static float interpolHeight(float current, float min1, float max1, float min2, float max2);
    static int findSmallest(const QVector<QPair<QChar, int> >& vector);
    
    bool m_showLetters;
    QVector<QPair<QChar, int> > m_letters;
    QVector<QPair<QChar, int> > m_computedSize;
    QMutex m_computeMutex;
    QMutex m_lettersMutex;
};

#endif // WMINIVIEWSCROLLBAR_H
