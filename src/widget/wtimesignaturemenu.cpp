#include "wtimesignaturemenu.h"

#include <QtWidgets/QHBoxLayout>

WTimeSignatureMenu::WTimeSignatureMenu(QWidget* parent)
        : QWidget(parent),
          m_timeSignature(mixxx::TimeSignature()),
          m_pBeatCountInputBox(make_parented<QLineEdit>(this)),
          m_beat(mixxx::kInvalidFramePos) {
    hide();
    QHBoxLayout* pMainLayout = new QHBoxLayout();
    pMainLayout->addWidget(m_pBeatCountInputBox);
    setLayout(pMainLayout);
    connect(m_pBeatCountInputBox,
            &QLineEdit::textChanged,
            this,
            &WTimeSignatureMenu::slotBeatCountChanged);
}

WTimeSignatureMenu::~WTimeSignatureMenu() {
}

void WTimeSignatureMenu::slotBeatCountChanged(QString value) {
    bool intConversionSuccessful;
    int beatsPerBar = value.toInt(&intConversionSuccessful);
    if (intConversionSuccessful && beatsPerBar > 1 && beatsPerBar <= 32) {
        m_timeSignature.setBeatsPerBar(beatsPerBar);
        m_pBeats->setSignature(m_timeSignature, m_beat.getBarIndex());
    }
}
