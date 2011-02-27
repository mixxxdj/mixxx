#include "effects/effectparameter.h"

EffectParameter::EffectParameter() {
}

EffectParameter::~EffectParameter() {
}

const QString EffectParameter::name() const {
    return m_name;
}

void EffectParameter::setName(QString name) {
    m_name = name;
}

const QString EffectParameter::description() const {
    return m_description;
}

void EffectParameter::setDescription(QString description) {
    m_description = description;
}

EffectParameter::ControlHint EffectParameter::controlHint() const {
    return m_controlHint;
}

void EffectParameter::setControlHint(ControlHint controlHint) {
    m_controlHint = controlHint;
}

EffectParameter::ValueHint EffectParameter::valueHint() const {
    return m_valueHint;
}

void EffectParameter::setValueHint(ValueHint valueHint) {
    m_valueHint = valueHint;
}

EffectParameter::SemanticHint EffectParameter::semanticHint() const {
    return m_semanticHint;
}

void EffectParameter::setSemanticHint(SemanticHint semanticHint) {
    m_semanticHint = semanticHint;
}

EffectParameter::UnitsHint EffectParameter::unitsHint() const {
    return m_unitsHint;
}

void EffectParameter::setUnitsHint(UnitsHint unitsHint) {
    m_unitsHint = unitsHint;
}

bool EffectParameter::hasDefault() const {
    return m_default.isValid();
}

QVariant EffectParameter::getDefault() const {
    return m_default;
}

void EffectParameter::setDefault(QVariant defaultValue) {
    m_default = defaultValue;
}

bool EffectParameter::hasMinimum() const {
    return m_minimum.isValid();
}

QVariant EffectParameter::getMinimum() const {
    return m_minimum;
}

void EffectParameter::setMinimum(QVariant minimum) {
    m_minimum = minimum;
}

bool EffectParameter::hasMaximum() const {
    return m_maximum.isValid();
}

QVariant EffectParameter::getMaximum() const {
    return m_maximum;
}

void EffectParameter::setMaximum(QVariant maximum) {
    m_maximum = maximum;
}
