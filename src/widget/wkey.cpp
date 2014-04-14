#include "widget/wkey.h"
#include "track/keys.h"
#include "track/keyutils.h"

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

void WKey::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dParameter);
    // Enums are not currently represented using parameter space so it doesn't
    // make sense to use the parameter here yet.
    setValue(dValue);
    setCents();
}

void WKey::setup(QDomNode node, const SkinContext& context) {
    WLabel::setup(node, context);
    if (context.selectBool(node, "DisplayCents", false)) {
        m_displayCents = true;
    } else {
        m_displayCents = false;
    }
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
    if (m_displayCents) {
        double diff_cents = m_engineKeyDistance.get();
        int cents_to_display = static_cast<int>(diff_cents * 100);
        char sign;
        if (diff_cents < 0) {
            sign = '-';
        } else {
            sign = '+';
        }
        // Remove the previous cent difference
        QString old = text();
        if (old.contains(' ')) {
            old = old.section(' ', 0, 0);
        }
        setText(old + QString(" %1%2c").arg(sign).arg(qAbs(cents_to_display)));
    }
}

void WKey::preferencesUpdated(double dValue) {
    if (dValue > 0) {
        setValue(m_dOldValue);
    }
}
