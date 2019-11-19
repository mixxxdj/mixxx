#include "widget/cuemenu.h"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QVBoxLayout>

#include "util/color/color.h"

CueMenu::CueMenu(QWidget* parent)
        : QWidget(parent) {
    QWidget::hide();
    setWindowFlags(Qt::Popup);
    setObjectName("CueMenu");

    m_pEditLabel = new QLineEdit(this);
    m_pEditLabel->setToolTip(tr("Edit cue label"));
    m_pEditLabel->setObjectName("CueLabelEdit");
    connect(m_pEditLabel, &QLineEdit::textEdited, this, &CueMenu::slotEditLabel);

    m_pColorMenu = new ColorMenu(this);
    m_pColorMenu->setObjectName("CueColorPicker");
    connect(m_pColorMenu, &ColorMenu::colorPicked, this, &CueMenu::slotChangeCueColor);

    QIcon icon(":/images/library/ic_library_cross.svg");
    m_pRemoveCue = new QPushButton(icon, "", this);
    m_pRemoveCue->setToolTip(tr("Remove this cue point"));
    m_pRemoveCue->setFixedHeight(m_pEditLabel->sizeHint().height());
    m_pRemoveCue->setObjectName("CueRemoveButton");
    connect(m_pRemoveCue, &QPushButton::clicked, this, &CueMenu::slotRemoveCue);

    QVBoxLayout* pMainLayout = new QVBoxLayout();
    QHBoxLayout* pTopLayout = new QHBoxLayout();
    pTopLayout->addWidget(m_pEditLabel);
    pTopLayout->addWidget(m_pRemoveCue);
    pMainLayout->addLayout(pTopLayout);
    pMainLayout->addWidget(m_pColorMenu);
    setLayout(pMainLayout);
}

CueMenu::~CueMenu() {
    delete m_pEditLabel;
    delete m_pColorMenu;
    delete m_pRemoveCue;
}

void CueMenu::slotEditLabel() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    m_pCue->setLabel(m_pEditLabel->text());
}

void CueMenu::slotChangeCueColor(PredefinedColorPointer pColor) {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(pColor != nullptr) {
        return;
    }
    m_pCue->setColor(pColor);
    hide();
}

void CueMenu::slotRemoveCue() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    m_pTrack->removeCue(m_pCue);
    hide();
}
