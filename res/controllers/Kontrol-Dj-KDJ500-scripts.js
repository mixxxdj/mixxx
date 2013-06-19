KDJ500 = new Controller();

KDJ500.init = function (id, debugging) {
    
}

KDJ500.shutdown = function () {
    
}

KDJ500.jog = function (channel, control, value, status, group) {
    if(value==0x7F) engine.setValue(group,"jog",1);
    if(value==0x01) engine.setValue(group,"jog",-1);
}

KDJ500.track = function (channel, control, value, status, group) {
    if(value==0x7F) engine.setValue("[Playlist]","SelectTrackKnob",1); 
    if(value==0x01) engine.setValue("[Playlist]","SelectTrackKnob",-1); 
}
