#pragma once

#include <QCheckBox>

/// Custom QCheckBox that has tristate() enabled by default
/// (checked, unchecked, partially checked / indeterminate) but clicking it does
/// only toggle between checked and unchecked.
/// Useful for cases here we want to display three states but toggling the
/// indeterminate state via user input makes no sense.
class WTristateCheckBox : public QCheckBox {
  public:
    using QCheckBox::QCheckBox;

    explicit WTristateCheckBox(QWidget* pParent = nullptr)
            : QCheckBox(pParent) {
        setTristate(true);
    }
    explicit WTristateCheckBox(const QString& label, QWidget* pParent = nullptr)
            : QCheckBox(label, pParent) {
        setTristate(true);
    }

  protected:
    void nextCheckState() override {
        setCheckState(checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
    }
};
