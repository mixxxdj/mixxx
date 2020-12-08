#pragma once

class RescalerUtils {
  public:

    // converts a value on a linear scale to a 1/x scale staring at 1
    static double linearToOneByX(double in, double inMin, double inMax, double outMax) {
        double outRange = outMax - 1;
        double inRange = inMax - inMin;
        return outMax / (((inMax - in) / inRange * outRange) + 1);
    }

    // converts value on a 1/x scale starting by 1 to a linear scale
    static double oneByXToLinear(double in, double inMax, double outMin, double outMax) {
        double outRange = outMax - outMin;
        double inRange = inMax - 1;
        return outMax - (((inMax / in) - 1) / inRange * outRange);
    }

  private:
    RescalerUtils() {}
};
