function PioneerCDJ350() {}

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
    engine.setValue(group, "loop_end_position", engine.getValue(group, "loop_end_position") - 255);
};
PioneerCDJ350.loop_end_plus = function(channel, control, value, status, group)
{
    engine.setValue(group, "loop_end_position", engine.getValue(group, "loop_end_position") + 255);
};