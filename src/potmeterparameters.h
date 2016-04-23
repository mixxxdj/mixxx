#ifndef POTMETERPARAMETERS_H
#define POTMETERPARAMETERS_H

// This classes encapsulate the parameters needed to set
// ControlPotmeters and its derived classes.
// They provide default values if some parameter is not explicitly set.

// DO NOT CHANGE THE DEFAULT VALUES. A lot of Mixxx code relies on them.

// NOTE(Ferran Pujol): An inheritance based approach was considered for
// these classes. Since the only common property is maxValue, and the
// size of the classes doesn't make up for code complexity I discarded
// the idea.

class EffectKnobParameters {
  public:
    EffectKnobParameters();
    ~EffectKnobParameters();
    double minValue();                          // Default: 0.
    double maxValue();                          // Default: 1.
    void setMinValue(double value);
    void setMaxValue(double value);

  private:
    double m_dMinValue;
    double m_dMaxValue;
};

class LinPotmeterParameters {
  public:
    LinPotmeterParameters();
    virtual ~LinPotmeterParameters();

    double minValue();                          // Default: 0.
    double maxValue();                          // Default: 1.
    double scaleStartParameter();               // Default: 0.
    bool allowOutOfBounds();                    // Default: false
    double step();                              // Default: 0.
    double smallStep();                         // Default: 0.

    void setMinValue(double value);
    void setMaxValue(double value);
    void setScaleStartParameter(double value);
    void setAllowOutOfBounds(bool value);
    void setStep(double value);
    void setSmallStep(double value);

  private:
    double m_dMinValue;
    double m_dMaxValue;
    double m_dScaleStartParameter;
    bool m_bAllowOutOfBounds;
    double m_dStep;
    double m_dSmallStep;
};

class LogPotmeterParameters {
  public:
    LogPotmeterParameters();
    virtual ~LogPotmeterParameters();

    double maxValue();                          // Default: 1.
    double scaleStartParameter();               // Default: 0.
    double minDB();                             // Default: 60.

    void setMaxValue(double value);
    void setScaleStartParameter(double value);
    void setMinDB(double value);

  private:
    double m_dMaxValue;
    double m_dScaleStartParameter;
    double m_dMinDB;
};

class PotmeterParameters {
  public:
    PotmeterParameters();
    PotmeterParameters(EffectKnobParameters& parameters);
    PotmeterParameters(LinPotmeterParameters& parameters);
    PotmeterParameters(LogPotmeterParameters& parameters);
    virtual ~PotmeterParameters();

    double minValue();                          // Default: 0.
    double maxValue();                          // Default: 1.
    double scaleStartParameter();               // Default: 0.
    bool allowOutOfBounds();                    // Default: false
    bool ignoreNops();                          // Default: true
    bool track();                               // Default: false
    bool persist();                             // Default: false

    void setMinValue(double value);
    void setMaxValue(double value);
    void setScaleStartParameter(double value);
    void setAllowOutOfBounds(bool value);
    void setIgnoreNops(bool value);
    void setTrack(bool value);
    void setPersist(bool value);

  private:
    double m_dMinValue;
    double m_dMaxValue;
    double m_dScaleStartParameter;
    bool m_bAllowOutOfBounds;
    bool m_bIgnoreNops;
    bool m_bTrack;
    bool m_bPersist;
};

#endif
