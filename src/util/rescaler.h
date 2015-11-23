#ifndef RESCALER_H
#define RESCALER_H



class RescalerUtils {
  public:

    static double linearToOneByX(double in, double inMin, double inMax, double outMax) {
        return outMax / (((inMax - in) / (inMax - inMin) * (outMax - 1)) + 1);
    }

    static double oneByXToLinear(double in, double inMax, double outMin, double outMax) {
        return outMax - (((inMax / in) - 1) / (inMax - 1) * (outMax - outMin));
    }

  private:
    RescalerUtils() {}
};



#endif // RESCALER_H
