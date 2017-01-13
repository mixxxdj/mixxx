#ifndef WMINIVIEWSCROLLBAR_H
#define WMINIVIEWSCROLLBAR_H

#include <QAbstractItemModel>
#include <QPointer>
#include <QScrollBar>
#include <QTreeView>

#include "library/columncache.h"

class WMiniViewScrollBar : public QScrollBar
{
  Q_OBJECT
  public:
    WMiniViewScrollBar(QWidget* parent = nullptr);

    void setShowLetters(bool show);
    bool showLetters() const;
    
    void setTreeView(QPointer<QTreeView> pTreeView);
    QPointer<QTreeView> getTreeView();

    void setModel(QAbstractItemModel* model);
    
  public slots:
    void triggerUpdate();

  protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void resizeEvent(QResizeEvent* pEvent);
    virtual void mouseMoveEvent(QMouseEvent* pEvent);
    virtual void mousePressEvent(QMouseEvent* pEvent);
    virtual void leaveEvent(QEvent*pEvent);

  private:
    struct CharCount {
        QChar character;
        int count;
    };
    
    struct CharPosition {
        QChar character;
        int position;
        bool bold;
    };
    
  private:
    // The purpose of this function is to avoid computing all the sizes in the
    // paintEvent function which can block the GUI thread
    void refreshCharMap();
    void computeLettersSize();
    QStyleOptionSlider getStyleOptions();
    void addToLastCharCount(const QChar& c, int sum = 1);
    int getVisibleChildCount(const QModelIndex &index);

    bool m_showLetters;
    
    // Contains the times each character appears in the model
    QVector<CharCount> m_letters;
    
    // Contains each character's vertical position
    QVector<CharPosition> m_computedPosition;
    QPointer<QAbstractItemModel> m_pModel;
    QPointer<QTreeView> m_pTreeView;
};

#endif // WMINIVIEWSCROLLBAR_H
