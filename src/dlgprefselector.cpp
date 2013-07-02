#include <qwidget.h>
#include <QtDebug>

#include "dlgprefselector.h"
#include "ui_dlgprefselectordlg.h"

DlgPrefSelector::DlgPrefSelector(QWidget *parent, ConfigObject<ConfigValue> *pConfig)
    : QWidget(parent),
      Ui::DlgPrefSelectorDlg(),
      m_pConfig(pConfig) {
    setupUi(this);
    populate();
    loadSettings();
}

DlgPrefSelector::~DlgPrefSelector() {
}

void DlgPrefSelector::slotApply() {
    m_pConfig->Save();
}

void DlgPrefSelector::slotUpdate() {
    slotApply();
}


void DlgPrefSelector::populate() {
}

void DlgPrefSelector::loadSettings() {
}
