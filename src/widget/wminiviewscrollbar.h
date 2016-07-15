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

    void setSortColumn(int column);
    int sortColumn();
    
    void setRole(int role);
    int role();

    void setModel(QAbstractItemModel* model);

  protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void resizeEvent(QResizeEvent* pEvent);

  private:
    struct CharCount {
        QChar character;
        int count;
    };
    
  private slots:
    void refreshCharMap();

  private:
    void lettersPaint(QPaintEvent*);

    // The purpose of this function is to avoid computing all the sizes in the
    // paintEvent function which can block the GUI thread
    void computeLettersSize();
    void triggerUpdate();

    static int findSmallest(const QVector<QPair<QChar, int> >& vector);
    static float interpolHeight(float current, float min1, float max1, float min2,
                                float max2);

    int m_sortColumn;
    int m_dataRole;
    bool m_showLetters;
    QVector<QPair<QChar, int> > m_letters;
    QVector<QPair<QChar, int> > m_computedSize;
    QPointer<QAbstractItemModel> m_pModel;
    QMutex m_mutexCompute;
    QMutex m_mutexLetters;
};

#endif // WMINIVIEWSCROLLBAR_H
