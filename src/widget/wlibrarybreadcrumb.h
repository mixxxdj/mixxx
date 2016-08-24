#ifndef SRC_WIDGET_WBREADCRUMB_H_
#define SRC_WIDGET_WBREADCRUMB_H_

#include <QIcon>
#include <QLabel>
#include <QToolButton>
#include <QWidget>

class TreeItem;

class WLibraryBreadCrumb : public QWidget {
    Q_OBJECT

  public:

    WLibraryBreadCrumb(QWidget* parent = nullptr);

    virtual QSize minimumSizeHint() const;
    void setPreselected(bool value);
    bool isPreselected();

  signals:
    void preselected(bool);
    
  public slots:

    void showBreadCrumb(TreeItem* pTree);
    void showBreadCrumb(const QString& text, const QIcon& icon);
    void setBreadIcon(const QIcon& icon);

  protected:
    
    virtual void resizeEvent(QResizeEvent* pEvent);
    
  private slots:
    void slotPreselectClicked();
    
  private:
    
    void setText(const QString& text);
    void refreshWidth();
    
    QLabel* m_pIcon;
    QLabel* m_pText;
    QToolButton* m_pPreselectButton;
    QIcon m_preselectIcon;
    bool m_preselected;
    
    QString m_longText;
};

#endif /* SRC_WIDGET_WBREADCRUMB_H_ */
