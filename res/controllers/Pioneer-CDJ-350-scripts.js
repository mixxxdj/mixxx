function PioneerCDJ350() {}

PioneerCDJ350.playlistMode = false;

PioneerCDJ350.init = function(id) {}
PioneerCDJ350.shutdown = function(id) {}

PioneerCDJ350.jog_wheel = function(channel, control, value, status, group)
{
    // CCW:  00 -> 3f = [x0.5, x1)
    // STOP: 40       = [x1]
    // CW:   41 -> 7f = (x1, x4]
    
    newValue = (value - 0x40) / 5.0; // was too sensitive
    engine.setValue(group, "jog", newValue);
};

PioneerCDJ350.select_turn = function(channel, control, value, status, group)
{
    // CCW:  98 -> 127
    // CW:    1 ->  30
    
    if (value > 64)
    {
        value -= 128;
    }
    
    if (PioneerCDJ350.playlistMode) // playlist
    {
        engine.setValue("[Playlist]", "SelectTrackKnob", value);
    }
    else // tree
    {
        if (value > 0)
        {
            engine.setValue("[Playlist]", "SelectNextPlaylist", 1);
        }
        else if (value < 0)
        {
            engine.setValue("[Playlist]", "SelectPrevPlaylist", 1);
        }
    }
};

PioneerCDJ350.select_push = function(channel, control, value, status, group)
{
    if (value == 0x00)
    {
        return;
    }
    
    if (PioneerCDJ350.playlistMode) // playlist
    {
        engine.setValue(group, "LoadSelectedTrack", 1);
    }
    else // tree
    {
        PioneerCDJ350.playlistMode = true;
    }
};

PioneerCDJ350.exit_playlist = function(channel, control, value, status, group)
{
    PioneerCDJ350.playlistMode = false;
};


PioneerCDJ350.toggle_playlist = function(channel, control, value, status, group)
{
    if (value == 0x00)
    {
        return;
    }
    PioneerCDJ350.playlistMode = !PioneerCDJ350.playlistMode;
};
PioneerCDJ350.tempo_btn = function(channel, control, value, status, group)
{
    if (value == 0x00)
    {
        return;
    }
    
    oldValue = engine.getValue(group, "rateRange");
    newValue = 0.06;
    
    if (oldValue > 0.11)
    {
        newValue = 0.03;
    }
    else if (oldValue > 0.05)
    {
        newValue = 0.12;
    }
    
    engine.setValue(group, "rateRange", newValue);
};

PioneerCDJ350.loop_end_minus = function(channel, control, value, status, group)
{
    if (value == 0x00)
    {
        return;
    }
    engine.setValue(group, "loop_end_position", engine.getValue(group, "loop_end_position") - 256);
};
PioneerCDJ350.loop_end_plus = function(channel, control, value, status, group)
{
    if (value == 0x00)
    {
        return;
    }
    engine.setValue(group, "loop_end_position", engine.getValue(group, "loop_end_position") + 256);
};