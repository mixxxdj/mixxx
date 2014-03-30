#include "widget/wkey.h"

#include "track/keyutils.h"
#include "track/keys.h"

WKey::WKey(const char* group, QWidget* pParent)
        : WLabel(pParent),
          m_dOldValue(0),
          m_preferencesUpdated(ConfigKey("[Preferences]", "updated")),
          m_engineKeyDistance(ConfigKey(group, "visual_key_distance")) {
    setValue(m_dOldValue);
    connect(&m_preferencesUpdated, SIGNAL(valueChanged(double)),
            this, SLOT(preferencesUpdated(double)));
    connect(&m_engineKeyDistance, SIGNAL(valueChanged(double)),
            this, SLOT(setCents()));
}

WKey::~WKey() {
}

void WKey::onConnectedControlValueChanged(double v) {
    setValue(v);
    setCents();
}

void WKey::setValue(double dValue) {
    m_dOldValue = dValue;
    mixxx::track::io::key::ChromaticKey key =
            KeyUtils::keyFromNumericValue(dValue);
    if (key != mixxx::track::io::key::INVALID) {
        // Render this key with the user-provided notation.
        setText(KeyUtils::keyToString(key));
    } else {
        setText("");
    }
}

void WKey::setCents() {
    int diff_cents = m_engineKeyDistance.get();
    char sign;
    if (diff_cents < 0) {
        sign = ' ';
    } else {
        sign = '+';
    }
    // Remove the previous cent difference
    QString old = text();
    if (old.contains('(')) {
        old = old.section('(', 0, 0);
    }
    setText(old + QString("(%1%2 c)").arg(sign).arg(diff_cents));
}


void WKey::preferencesUpdated(double dValue) {
    if (dValue > 0) {
        setValue(m_dOldValue);
    }
}
