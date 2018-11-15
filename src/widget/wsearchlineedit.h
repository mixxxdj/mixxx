#pragma once

#include <QColor>
#include <QDomNode>
#include <QEvent>
#include <QLineEdit>
#include <QTimer>
#include <QToolButton>

#include "widget/wbasewidget.h"

class SkinContext;

class WSearchLineEdit : public QLineEdit, public WBaseWidget {
    Q_OBJECT
  public:
    enum class State {
        Inactive,
        Active,
    };

    explicit WSearchLineEdit(QWidget* pParent);
    ~WSearchLineEdit() override = default;

    void setup(const QDomNode& node, const SkinContext& context);

  protected:
    void resizeEvent(QResizeEvent*) override;
    void focusInEvent(QFocusEvent*) override;
    void focusOutEvent(QFocusEvent*) override;
    bool event(QEvent*) override;

  signals:
    void search(const QString& text);

  public slots:
    void restoreSearch(const QString& text);
    void disableSearch();

  private slots:
    void setShortcutFocus();
    void updateText(const QString& text);

    void clearSearch();
    void triggerSearch();

  private:
    void showPlaceholder();
    void showSearchText(const QString& text);
    void updateEditBox(const QString& text);

    void updateClearButton(const QString& text);

    QString getSearchText() const;

    QToolButton* const m_clearButton;

    QColor m_foregroundColor;
    QTimer m_debouncingTimer;

    State m_state;
};
