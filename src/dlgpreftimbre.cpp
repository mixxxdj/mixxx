#include "dlgpreftimbre.h"
#include "ui_dlgpreftimbredlg.h"

DlgPrefTimbre::DlgPrefTimbre(QWidget *parent, ConfigObject<ConfigValue> *pConfig):
    QWidget(parent), Ui::DlgPrefTimbreDlg(),
    m_pConfig(pConfig),
    m_bAnalyserEnabled(false) {
    setupUi(this);
}

DlgPrefTimbre::~DlgPrefTimbre() {
}

void DlgPrefTimbre::slotApply() {
    m_pConfig->Save();
}

void DlgPrefTimbre::slotUpdate() {
    slotApply();
}

void DlgPrefTimbre::pluginSelected(int i) {
}

void DlgPrefTimbre::analyserEnabled(int i) {
}

void DlgPrefTimbre::setDefaults() {
}


void DlgPrefTimbre::populate() {
}

void DlgPrefTimbre::loadSettings() {
}

