#include "widget/dlgstemconversionoptions.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>

DlgStemConversionOptions::DlgStemConversionOptions(QWidget* parent)
        : QDialog(parent),
          m_selectedResolution(Resolution::High) {
    setWindowTitle("Stem Conversion Options");
    setMinimumWidth(400);
    setMinimumHeight(200);
    setWindowModality(Qt::ApplicationModal);

    createUI();
    connectSignals();
}

void DlgStemConversionOptions::createUI() {
    QVBoxLayout* pMainLayout = new QVBoxLayout(this);

    // Title
    QLabel* pTitleLabel = new QLabel("Select Stem Conversion Parameters", this);
    QFont titleFont = pTitleLabel->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    pTitleLabel->setFont(titleFont);
    pMainLayout->addWidget(pTitleLabel);

    pMainLayout->addSpacing(10);

    // Resolution selection group
    QGroupBox* pResolutionGroup = new QGroupBox("Audio Resolution", this);
    QVBoxLayout* pResolutionLayout = new QVBoxLayout(pResolutionGroup);

    QLabel* pResolutionLabel = new QLabel("Select the output resolution for stem separation:", this);
    pResolutionLayout->addWidget(pResolutionLabel);

    m_pResolutionComboBox = new QComboBox(this);
    m_pResolutionComboBox->addItem("High Resolution (44.1 kHz or higher) - Recommended", 
                                    static_cast<int>(Resolution::High));
    m_pResolutionComboBox->addItem("Low Resolution (16 kHz) - Faster processing", 
                                   static_cast<int>(Resolution::Low));
    m_pResolutionComboBox->setCurrentIndex(0);
    pResolutionLayout->addWidget(m_pResolutionComboBox);

    QLabel* pInfoLabel = new QLabel(
            "High resolution provides better audio quality but takes longer to process.\n"
            "Low resolution is faster but may result in lower quality separation.",
            this);
    pInfoLabel->setStyleSheet("color: gray; font-size: 10px;");
    pInfoLabel->setWordWrap(true);
    pResolutionLayout->addWidget(pInfoLabel);

    pMainLayout->addWidget(pResolutionGroup);

    pMainLayout->addStretch();

    // Buttons
    QHBoxLayout* pButtonLayout = new QHBoxLayout();
    pButtonLayout->addStretch();

    m_pOkButton = new QPushButton("Start Conversion", this);
    m_pOkButton->setMinimumWidth(120);
    pButtonLayout->addWidget(m_pOkButton);

    m_pCancelButton = new QPushButton("Cancel", this);
    m_pCancelButton->setMinimumWidth(100);
    pButtonLayout->addWidget(m_pCancelButton);

    pMainLayout->addLayout(pButtonLayout);
}

void DlgStemConversionOptions::connectSignals() {
    connect(m_pOkButton, &QPushButton::clicked, this, &DlgStemConversionOptions::onAccepted);
    connect(m_pCancelButton, &QPushButton::clicked, this, &DlgStemConversionOptions::onRejected);
}

void DlgStemConversionOptions::onAccepted() {
    int selectedIndex = m_pResolutionComboBox->currentIndex();
    m_selectedResolution = static_cast<Resolution>(
            m_pResolutionComboBox->itemData(selectedIndex).toInt());
    accept();
}

void DlgStemConversionOptions::onRejected() {
    reject();
}

DlgStemConversionOptions::Resolution DlgStemConversionOptions::getSelectedResolution() const {
    return m_selectedResolution;
}

#include "moc_dlgstemconversionoptions.cpp"
