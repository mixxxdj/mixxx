#pragma once

#include <QCheckBox>

/// This is a custom QCheckBox fixing some bugs/quirks that occur when QCheckBox
/// is packed into QWidgetAction for use in QMenu.
/// 1. fixed hover behavior: get focused (highlighted) on hover, no matter
/// where the pointer is. With original QCheckBox it'd only get focused
/// when the pointer hovers the label or the [ ] indicator -- hovering the
/// whitespace next to narrow labels had no effect.
/// 2. block double-clicks: originally, in conjunction with QWidgetAction, those
/// would first toggle the checkbox, then make the QWidgetAction emit the
/// 'activated' signal and thereby close the menu unexpectedly.
///
/// Currently used in WTrackTableViewHeader menu, WTrackMenu's Crates menu and
/// WSearchrelatedMenu.
class WMenuCheckBox : public QCheckBox {
    Q_OBJECT
  public:
    explicit WMenuCheckBox(QWidget* pParent = nullptr);
    explicit WMenuCheckBox(const QString& label, QWidget* pParent = nullptr);

    bool eventFilter(QObject* pObj, QEvent* pEvent) override;
};
