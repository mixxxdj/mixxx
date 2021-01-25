#ifndef DLGKEYWHEEL_H
#define DLGKEYWHEEL_H

#include <QDialog>
#include <QDomDocument>
#include <QEvent>
#include <QSvgWidget>

#include "dialog/ui_dlgkeywheel.h"
#include "track/keyutils.h"

class DlgKeywheel : public QDialog, public Ui::DlgKeywheel {
    Q_OBJECT

  public:
    explicit DlgKeywheel(QWidget* parent, const UserSettingsPointer& pConfig);
    void switchNotation(int dir = 1);
    void updateSvg();
    ~DlgKeywheel() = default;

  protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void resizeEvent(QResizeEvent* ev) override;

  private:
    bool isHiddenNotation(KeyUtils::KeyNotation notation);
    KeyUtils::KeyNotation m_notation;
    QDomDocument m_domDocument;
    QSvgWidget* m_wheel;
    const UserSettingsPointer m_pConfig;
};

#endif // DLGKEYWHEEL_H
