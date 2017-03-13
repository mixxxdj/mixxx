#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <QtDebug>

class Experiment {
  public:
    enum Mode {
        OFF = 0,
        BASE = 1,
        EXPERIMENT = 2
    };

    static inline bool isEnabled() {
        return s_mode != OFF;
    }

    static void disable() {
        qDebug() << "Experiment::setExperiment OFF";
        s_mode = OFF;
    }

    static void setExperiment() {
        qDebug() << "Experiment::setExperiment EXPERIMENT";
        s_mode = EXPERIMENT;
    }

    static void setBase() {
        qDebug() << "Experiment::setExperiment BASE";
        s_mode = BASE;
    }

    static inline bool isExperiment() {
        return s_mode == EXPERIMENT;
    }

    static inline bool isBase() {
        return s_mode == BASE;
    }

    static Mode mode() {
        return s_mode;
    }

  private:
    Experiment();

    static volatile Mode s_mode;
};

#endif /* EXPERIMENT_H */
