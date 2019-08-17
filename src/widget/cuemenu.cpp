#include <QInputDialog>

#include "widget/cuemenu.h"
#include "util/color/color.h"

CueMenu::CueMenu(QWidget *parent, PredefinedColorsRepresentation* pColorRepresentation)
        : QMenu(parent) {
    m_pEditLabel = new QAction(tr("Edit label"));
    addAction(m_pEditLabel);
    connect(m_pEditLabel, &QAction::triggered, this, &CueMenu::slotEditLabel);

    m_pColorMenu = new ColorMenu(this, pColorRepresentation);
    connect(m_pColorMenu, &ColorMenu::colorPicked, this, &CueMenu::slotChangeCueColor);
    addMenu(m_pColorMenu);

    m_pRemoveCue = new QAction(tr("Remove"));
    addAction(m_pRemoveCue);
    connect(m_pRemoveCue, &QAction::triggered, this, &CueMenu::slotRemoveCue);
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
    bool okay = false;
    QString newLabel = QInputDialog::getText(this, tr("Edit cue label"),
                                             tr("Cue label"), QLineEdit::Normal,
                                             m_pCue->getLabel(), &okay);
    if (okay) {
        m_pCue->setLabel(newLabel);
    }
}

void CueMenu::slotChangeCueColor(PredefinedColorPointer pColor) {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(pColor != nullptr) {
        return;
    }
    m_pCue->setColor(pColor);
}

void CueMenu::slotRemoveCue() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    m_pTrack->removeCue(m_pCue);
}
