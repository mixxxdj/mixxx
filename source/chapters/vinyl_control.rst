Vinyl Control
*************

Vinyl control allows a user to manipulate the playback of a song in Mixxx using
a real turntable as a controller. In effect, it simulates the sound and feel of
having your digital music collection on vinyl. Many DJs prefer the tactile feel
of vinyl, and vinyl control allows that feel to be preserved while retaining the
benefits of using digital audio.

You can configure vinyl control through the Vinyl Control pane in the
preferences. More information about Mixxx's vinyl control and supported hardware
configurations is available on the `vinyl control wiki page
<http://mixxx.org/wiki/doku.php/vinyl_control>`_.

Input Device Selection
======================

Mixxx can be controlled by up to 2 decks with either timecoded vinyl or
timecoded CDs. In the Sound Hardware Input tab in the Mixxx Preferences, select
the soundcard(s) which your decks are plugged into for Vinyl Control 1 and
2. In the channel selection drop-down menu, select the channel pairs
corresponding to the plug on your soundcard that your deck(s) are plugged into.

Mixxx supports vinyl control input through a single soundcard with 4-channels of
input (two stereo line-in jacks), or through two separate soundcards which each
have 2-channels of input (a single stereo line-in jack). Vinyl control requires
the use of stereo line-in jacks - Mono or microphone inputs will not work.

Turntable Input Preamp
======================

Many turntables provide unamplified "phono level" output, which must be boosted
to a "line level" signal. Normally, a mixer provides this amplification, but if
you're plugging a turntable's phono output directly into your soundcard, Mixxx
can do the amplification. The "Turntable Input Preamp" slider allows you to
adjust the level of preamplification applied to your decks' signal.

Vinyl Configuration
===================

Several different types of timecoded media can be used to control
Mixxx. Configure the "Vinyl Type" drop-down menu to match what type of
timecoded vinyl or CD you are using on your decks.

The "Lead-in Time" setting allows you to set a dead-zone at the start of your
vinyl or CD, where the time code will be ignored. This is useful in situations
where the time code begins very close to the edge of a vinyl, which can make
back-cueing over the first beat in a song tricky to do without your turntable's
needle falling off the vinyl. Setting a lead-in time of 20 seconds or more helps
alleviate this by giving you more room on the vinyl to cue with. It's also
useful when you've worn the grooves at the edge of a control record to the point
that it no longer works reliably: you simply set the lead-in so that the start
of the songs begin in good groove area. You can keep doing this until you run
out of groove area, thereby decreasing your record replacement frequency.

Control Mode
============

Mixxx supports three control types on all of the timecodes we support. "Absolute
Mode" provides Mixxx with both pitch and position information from the timecode,
and allows you to seek by needle dropping on your vinyl or seeking on your
CDJ. "Relative Mode" takes the position to be relative to your deck's starting
point, and only controls the pitch in Mixxx. It is not possible to seek using
your deck in relative mode. "Constant Mode" is a special mode that is usually
only activated when Mixxx needs to keep playing a track even if the vinyl 
control signal isn't present.  Constant mode is automatically enabled when the
needle reaches the end of a vinyl control record, or if "Needle-skip Prevention"
is enabled and a needle skip is detected. "Needle-skip Prevention" allows Mixxx
to detect and ignore small changes in needle position, such as when you've 
accidentally bumped your turntable. This can be advantageous in a live
performance environment, but the downside is that it reduces responsiveness 
during scratching. Consequently, disabling needle-skip prevention is
recommended for scratch performances.

Signal Quality
==============

A successful vinyl control setup hinges on good signal quality. Many factors can
affect signal quality, but the most important one is ensuring the volume level
of your timecode signal is moderate. A signal that is too loud or too quiet will
cause adverse performance, often characterized by a loss of position data
causing absolute mode to behave like relative mode. For more information on
improving signal quality and troubleshooting, please see the vinyl control wiki
page.

Mixxx represents your timecode signal quality as a circular representation of 
the vinyl control audio signal itself. The two graphs correspond to your "Deck 1" and "Deck 2" input
devices.  If your vinyl signal is working, you should see a round, green circle
drawn with two lines. This vinyl "doughnut" should be as circular as possible,
and the lines should be clear with a minimum amount of fuzz.  As the vinyl signal
gets worse, you'll see the color of the doughnut change from green to red.  An
otherwise good-looking doughnut that is solid red indicates an incorrect choice 
of vinyl control source.  Maybe you selected side B of the Serato vinyl, but
your record is on side A.  Also, there should be a rotating radar sweep that represents
the direction your record is spinning.  If the radar sweep is going backwards,
you probably have a wiring problem in your turntable.

The doughnut should be about half the width of the graph.  If the volume is too low or two high, 
the doughnut will be much smaller or much larger.  The "Turntable Input Preamp" setting can
be adjusted to boost the volume.
