#pragma once

#include <QCloseEvent>
#include <QDialog>
#include <QWidget>

/// A dialog hosting effect-specific UI. This is mainly intended for audio
/// plugins (e.g. AU/LV2/VST/...) to display their UI, but may also be useful to
/// other effect backends. While subclassing is recommended to customize the
/// dialog as needed, e.g. with a custom window title, this class already
/// styles the dialog and configures it to float above other windows.
class DlgEffect : public QDialog {
    Q_OBJECT

  public:
    DlgEffect(QWidget* customUI = nullptr);
    virtual ~DlgEffect();

    void setCustomUI(QWidget* customUI);
    void setClosesWithoutSignal(bool closesWithoutSignal);

  signals:
    void closed();

  protected:
    void closeEvent(QCloseEvent* e) override;

  private:
    QWidget* m_customUI;
    bool m_closesWithoutSignal;
};
