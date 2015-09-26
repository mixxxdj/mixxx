#include "potmeterparameters.h"

EffectKnobParameters::EffectKnobParameters()
        : m_dMinValue(0.0), m_dMaxValue(1.0) {
}

EffectKnobParameters::~EffectKnobParameters() {
}

void EffectKnobParameters::minValue() {
    return m_dMinValue;
}

void EffectKnobParameters::maxValue() {
    return m_dMaxValue;
}

void EffectKnobParameters::setMinValue(double value) {
    m_dMinValue = value;
}

void EffectKnobParameters::setMaxValue(double value) {
    m_dMaxValue = value;
}

LinPotmeterParameters::LinPotmeterParameters()
        : m_dMinValue(0.0), m_dMaxValue(1.0), m_dNeutralValue(0.0),
          m_bAllowOutOfBounds(false), m_dStep(0.0),
          m_dSmallStepTrack(0.0) {
}

LinPotmeterParameters::~LinPotmeterParameters() {
}

double LinPotmeterParameters::minValue() {
    return m_dMinValue;
}

double LinPotmeterParameters::maxValue() {
    return m_dMaxValue;
}

double LinPotmeterParameters::neutralValue() {
    return m_dNeutralValue;
}

bool LinPotmeterParameters::allowOutOfBounds() {
    return m_bAllowOutOfBounds;
}

double LinPotmeterParameters::step() {
    return m_dStep;
}

double LinPotmeterParameters::smallStep() {
    return m_dSmallStep;
}

void LinPotmeterParameters::setMinValue(double value) {
    m_dMinValue = value;
}

void LinPotmeterParameters::setMaxValue(double value) {
    m_dMaxValue = value;
}

void LinPotmeterParameters::setNeutralValue(double value) {
    m_dNeutralValue = value;
}

void LinPotmeterParameters::setAllowOutOfBounds(bool value) {
    m_bAllowOutOfBounds = value;
}

void LinPotmeterParameters::setStep(double value) {
    m_dStep = value;
}

void LinPotmeterParameters::setSmallStep(double value) {
    m_dSmallStep = value;
}

LogPotmeterParameters::LogPotmeterParameters()
        : m_dMaxValue(1.0), m_dNeutralValue(0.0),
          m_bMinDB(60.) {
}

LogPotmeterParameters::~LogPotmeterParameters() {
}

double LogPotmeterParameters::maxValue() {
    return m_dMaxValue;
}

double LogPotmeterParameters::neutralValue() {
    return m_dNeutralValue;
}

double LogPotmeterParameters::minDB() {
    return m_dMinDB;
}

void LogPotmeterParameters::setMaxValue(double value) {
    m_dMaxValue = value;
}

void LogPotmeterParameters::setNeutralValue(double value) {
    m_dNeutralValue = value;
}

void LogPotmeterParameters::setMinDB(double value) {
    m_dMinDB = value;
}


PotmeterParameters::PotmeterParameters()
        : m_dMinValue(0.0), m_dMaxValue(1.0), m_dNeutralValue(0.0),
          m_bAllowOutOfBounds(false), m_bIgnoreNops(true),
          m_bTrack(false), m_bPersist(false) {
}

PotmeterParameters::PotmeterParameters(EffectKnobParameters& parameters)
        : m_dMinValue(parameters.minValue()),
          m_dMaxValue(parameters.maxValue()),
          m_dNeutralValue(0.0),
          m_bAllowOutOfBounds(false), m_bIgnoreNops(true),
          m_bTrack(false), m_bPersist(false) {
}

PotmeterParameters::PotmeterParameters(LinPotmeterParameters& parameters)
        : m_dMinValue(parameters.minValue()),
          m_dMaxValue(parameters.maxValue()),
          m_dNeutralValue(parameters.neutralValue()),
          m_bAllowOutOfBounds(parameters.allowOutOfBounds()),
          m_bIgnoreNops(true), m_bTrack(false), m_bPersist(false) {
}

PotmeterParameters::PotmeterParameters(LogPotmeterParameters& parameters)
        : m_dMinValue(0.0), m_dMaxValue(parameters.maxValue()),
          m_dNeutralValue(parameters.neutralValue()),
          m_bAllowOutOfBounds(false), m_bIgnoreNops(true),
          m_bTrack(false), m_bPersist(false) {
}

PotmeterParameters::~PotmeterParameters() {
}

double PotmeterParameters::minValue() {
    return m_dMinValue;
}

double PotmeterParameters::maxValue() {
    return m_dMaxValue;
}

double PotmeterParameters::neutralValue() {
    return m_dNeutralValue;
}

bool PotmeterParameters::allowOutOfBounds() {
    return m_bAllowOutOfBounds;
}

bool PotmeterParameters::ignoreNops() {
    return m_bIgnoreNops;
}

bool PotmeterParameters::track() {
    return m_bTrack;
}

bool PotmeterParameters::persist() {
    return m_bPersist;
}

void PotmeterParameters::setMinValue(double value) {
    m_dMinValue = value;
}

void PotmeterParameters::setMaxValue(double value) {
    m_dMaxValue = value;
}

void PotmeterParameters::setNeutralValue(double value) {
    m_dNeutralValue = value;
}

void PotmeterParameters::setAllowOutOfBounds(bool value) {
    m_bAllowOutOfBounds = value;
}

void PotmeterParameters::setIgnoreNops(bool value) {
    m_bIgnoreNops = value;
}

void PotmeterParameters::setTrack(bool value) {
    m_bTrack = value;
}

void PotmeterParameters::setPersist(bool value) {
    m_bPersist = value;
}
