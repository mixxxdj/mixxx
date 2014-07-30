#include <QWidget>
#include <QString>
#include <QPair>
#include <QCheckBox>
#include <QLabel>

#include "dlgpreflv2.h"
#include "engine/enginefilterbessel4.h"
#include "controlobject.h"
#include "util/math.h"

DlgPrefLV2::DlgPrefLV2(QWidget* pParent, LV2Backend* lv2Backend,
                       ConfigObject<ConfigValue>* pConfig)
        : DlgPreferencePage(pParent) {
    Q_UNUSED(pConfig);
    setupUi(this);
    if (!lv2Backend) {
        return;
    }
    QList<QPair<QString, bool> > allPlugins = lv2Backend->getAllDiscoveredPlugins();
    for (int i = 0; i < allPlugins.size(); i++) {
        QLabel* label = new QLabel(this);
        if (!allPlugins[i].second) {
            label->setText(QString("<s>") + allPlugins[i].first + QString("</s>"));
        } else {
            label->setText(allPlugins[i].first);
        }
        lv2_vertical_layout->addWidget(label);
    }
}

DlgPrefLV2::~DlgPrefLV2() {
}
