#ifndef LIBRARYVIEWMANAGER_H
#define LIBRARYVIEWMANAGER_H

#include <QObject>
#include <QWidget>
#include <QStackedWidget>

#include "library/libraryviewfeature.h"
#include "widget/wbuttonbar.h"

class LibraryViewManager : public QObject {
    Q_OBJECT

  public:

    LibraryViewManager(QObject* parent = nullptr);

    inline void setButtonBar(WButtonBar* button) {
        m_pButtonBar = button;
    }
    inline void setLeftPane(QStackedWidget* pane) {
        m_pLeftPane = pane;
    }
    inline void addRightPane(QStackedWidget* pane) {
        m_pRightPane.append(pane);
    }

    void addFeature(LibraryViewFeature* feature);
    void featureSelected();

  private:

    WButtonBar* m_pButtonBar;
    QStackedWidget* m_pLeftPane;
    QVector<QStackedWidget*> m_pRightPane;
    QVector<LibraryViewFeature*> m_pFeatures;

  private slots:

    void onFocusChange();
};

#endif // LIBRARYVIEWMANAGER_H
