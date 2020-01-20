//
// C++ Implementation: wnumberrate
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "widget/wnumberrate.h"

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "moc_wnumberrate.cpp"
#include "util/math.h"

namespace {

inline QChar sign(double number) {
    if (number > 0) {
        return '+';
    }
    if (number < 0) {
        return '-';
    }
    return ' ';
}

} // namespace

WNumberRate::WNumberRate(const QString& group, QWidget* parent)
        : WNumber(parent) {
    m_pRateRatio = new ControlProxy(group, "rate_ratio", this, ControlFlag::NoAssertIfMissing);
    m_pRateRatio->connectValueChanged(this, &WNumberRate::setValue);
}

void WNumberRate::setup(const QDomNode& node, const SkinContext& context) {
    WNumber::setup(node, context);

    // Initialize the widget (overrides the base class initial value.
    setValue(m_pRateRatio->get());
}

void WNumberRate::setValue(double dValue) {
    const double digitFactor = pow(10, m_iNoDigits);
    // Calculate percentage rounded to the number of digits specified by iNoDigits
    const double percentage = round((dValue - 1) * 100.0 * digitFactor) / digitFactor;
    setText(m_skinText + sign(percentage) + QString::number(fabs(percentage), 'f', m_iNoDigits));
}
