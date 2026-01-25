#pragma once

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

/// Dialog that allows the user to select stem conversion parameters
class DlgStemConversionOptions : public QDialog {
    Q_OBJECT

  public:
    enum class Resolution {
        Low,    // 16 kHz
        High    // 44.1 kHz or higher
    };

    explicit DlgStemConversionOptions(const QString& trackPath, QWidget* parent = nullptr);
    ~DlgStemConversionOptions() override = default;

    /// Get the selected resolution
    Resolution getSelectedResolution() const;

  private slots:
    void onAccepted();
    void onRejected();

  private:
    void createUI();
    void connectSignals();

    QString m_trackPath;
    QLabel* m_pTrackPathLabel;
    QComboBox* m_pResolutionComboBox;
    QPushButton* m_pOkButton;
    QPushButton* m_pCancelButton;
    Resolution m_selectedResolution;
};
