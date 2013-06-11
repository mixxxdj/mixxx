#ifndef CONTROLVALIDATOR_H
#define CONTROLVALIDATOR_H

class ControlValidator {
  public:
    ControlValidator() { }
    virtual ~ControlValidator() {};

    // Subclasses override this function to provide a way to reject invalid settings.
    // Returns true if the change is valid.
    virtual bool validateChange(double value) const = 0;
};

#endif // CONTROLVALIDATOR_H
