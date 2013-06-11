class ControlValidator {
  public:
    // Subclasses can choose to override this function to provide a way to reject invalid
    // settings.  Returns true if the change is valid.
    virtual bool validateChange(double value) const = 0;
};
