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

    // To add a feature to the selected pane (0 <= pane < RIGHT_PANE_COUNT)
    void addFeature(LibraryViewFeature* feature, int pane);

  public slots:

    void onSearch(QString& text);

  private:

    WButtonBar* m_pButtonBar;
    QStackedWidget* m_pLeftPane;
    QVector<QWidget*> m_rightPane;
    QVector<QStackedWidget*> m_rightPaneStack;
    QVector<WSearchLineEdit*> m_searchBar;
    QVector<QVector<LibraryViewFeature*> > m_features;
    QVector<int> m_currentFeature;
    int m_currentPane;

  private slots:

    // Used to handle focus change
    // TODO(jmigual): Still needs to be implemented
    bool eventFilter(QObject* object, QEvent* event);
};

#endif // LIBRARYVIEWMANAGER_H
