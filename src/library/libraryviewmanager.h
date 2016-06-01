#ifndef LIBRARYVIEWMANAGER_H
#define LIBRARYVIEWMANAGER_H

#include <QObject>
#include <QWidget>
#include <QStackedWidget>

#include "library/libraryviewfeature.h"
#include "widget/wbuttonbar.h"
#include "widget/wsearchlineedit.h"

class LibraryViewManager : public QObject {
    Q_OBJECT

  public:

    const int RIGHT_PANE_COUNT = 2;

    LibraryViewManager(QObject* parent = nullptr);

    bool initialize();

    inline WButtonBar* getButtonBar() const {
        return m_pButtonBar;
    }
    inline QStackedWidget* getLeftPane() const {
        return m_pLeftPane;
    }
    inline const QVector<QWidget*>& getRightPane() const {
        return m_rightPane;
    }

    void addFeature(LibraryViewFeature* feature);

  public slots:

    void onSearch(QString& text);
    
  private:

    WButtonBar* m_pButtonBar;
    QStackedWidget* m_pLeftPane;
    QVector<QWidget*> m_rightPane;
    QVector<QStackedWidget*> m_rightPaneStack;
    QVector<WSearchLineEdit*> m_searchBar;
    QVector<LibraryViewFeature*> m_features;
    QVector<int> m_currentFeature;
    int m_currentPane;

  private slots:

    // TODO(jmigual): Still needs to v
    void onFocusChange();
};

#endif // LIBRARYVIEWMANAGER_H
