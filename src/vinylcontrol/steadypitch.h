#pragma once

/// Tells whether the pitch read from a timecode has been steady, and for how
/// long.
///
/// VinylControlXwax uses two instances to derive deck states from the vinyl
/// pitch: one with a narrow threshold switches the play button on once the
/// record runs steadily, and one with a wide threshold detects scratching,
/// which disables keylock in the engine.
///
/// The pitch counts as steady while it stays within the threshold of a
/// reference pitch. The steady duration is measured along the given time
/// axis, which is the track position in seconds. This is why a stopped
/// record never counts as steady, and why position jumps from loops and cue
/// points have to be tolerated, see resyncDetected().
class SteadyPitch {
  public:
    /// @param threshold the maximum deviation from the reference pitch that
    ///        still counts as steady
    /// @param assumeSteady report the pitch as steady right after a reset,
    ///        for CDJs whose pitch is accurate from the start
    SteadyPitch(double threshold, bool assumeSteady);

    /// Restarts the measurement with `pitch` as the new reference pitch at
    /// track position `time`.
    void reset(double pitch, double time);

    /// Feeds the current pitch at track position `time`.
    ///
    /// @return the duration in seconds the pitch has been steady, or 0.0 if
    ///         it is not steady. The duration never exceeds 2 seconds, since
    ///         the reference pitch is renewed every second after that.
    double check(double pitch, double time);

    /// @return the reference pitch that was last confirmed steady for more
    ///         than 2 seconds
    double steadyValue(void) const;

    /// Whether `pitch` plays in the other direction than the reference pitch
    bool directionChanged(double pitch) const;

    /// Whether the track position jumped against the play direction since
    /// the previous check(), e.g. because of a loop or a cue point
    bool resyncDetected(double new_time) const;

  private:
    const bool m_bAssumeSteady;
    /// The reference pitch the current pitch is compared with
    double m_dSteadyPitch;
    /// See steadyValue()
    double m_dOldSteadyPitch;
    /// The track position when the reference pitch was taken
    double m_dSteadyPitchTime;
    /// The steady duration returned by the previous check()
    double m_dLastSteadyDur;
    /// The track position of the previous check()
    double m_dLastTime;
    double m_dPitchThreshold;
    /// 1 when playing forward, -1 when playing backward
    int m_iPlayDirection;
};
