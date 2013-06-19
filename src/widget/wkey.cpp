#include "widget/wkey.h"

#include "track/keyutils.h"
#include "track/keys.h"

WKey::WKey(QWidget* pParent) : WLabel(pParent) {
    setValue(0);
}

WKey::~WKey() {
}

void WKey::setValue(double dValue) {
    mixxx::track::io::key::ChromaticKey key =
            KeyUtils::keyFromNumericValue(dValue);
    m_pLabel->setText(KeyUtils::keyToString(key));
}
