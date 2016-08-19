#ifndef SRC_WIDGET_WBREADCRUMB_H_
#define SRC_WIDGET_WBREADCRUMB_H_

#include <QLabel>
#include <QWidget>

class TreeItem;

class WLibraryBreadCrumb : public QWidget {
    Q_OBJECT

  public:

    WLibraryBreadCrumb(QWidget* parent = nullptr);

    virtual QSize minimumSizeHint() const;

  public slots:

    void showBreadCrumb(TreeItem* pTree);
    void showBreadCrumb(const QString& text, const QIcon& icon);
    void setBreadIcon(const QIcon& icon);

  protected:
    
    virtual void resizeEvent(QResizeEvent* pEvent);
    
  private:
    
    void setText(const QString& text);
    void refreshWidth();
    
    QLabel* m_pIcon;
    QLabel* m_pText;
    
    QString m_longText;
};

#endif /* SRC_WIDGET_WBREADCRUMB_H_ */
