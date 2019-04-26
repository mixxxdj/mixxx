/**
 * Microsoft Xbox One Wired Controller HID mapping
 * Copyright (C) 2019, Luxray5474
 * For Mixxx v2.2.x 
*/

var xbc = {};

/**
 * Remaps a specified byte from one offset to another byte in another offset.
 *  data         the array representing one packet from the controller
 *  oldOffset    offset in decimal at which a byte
 *  matchByte    byte to look for
 *  newOffset    the new offset in decimal of the new byte
 *  newByte      The byte to set in newOffset
*/
remapHex = function(data, oldOffset, matchByte, newOffset, newByte) {
  if(data[oldOffset] == matchByte) {
    data[newOffset] = newByte;
    data[oldOffset] = 0x00;
    return;
  } else return;
}

function XBC() {
  this.controller = new HIDController();
  this.controller.activeDeck = 2;

  this.registerInputPackets = function () {
    packet = new HIDPacket("control",[],14);

    // abxy buttons
    packet.addControl("hid","a",10,"B",0x1);
    packet.addControl("hid","b",10,"B",0x2);
    packet.addControl("hid","x",10,"B",0x4);
    packet.addControl("hid","y",10,"B",0x8);

    // dpad
    packet.addControl("hid","pov_u",10,"B",0x3);
    packet.addControl("hid","pov_d",10,"B",0x5);
    packet.addControl("hid","pov_l",10,"B",0x6);
    packet.addControl("hid","pov_r",10,"B",0x7);

/*     // left stick
    packet.addControl("hid","lsx",0x04,"H");
    packet.setMinDelta("hid","lsx",1);
    packet.addControl("hid","lsy",0x02,"H");
    packet.setMinDelta("hid","lsy",1);
    packet.addControl("hid","ls",0x0b,"B",0x01);
    
    // right stick
    packet.addControl("hid","rsx",0x04,"H",0xFF);
    packet.setMinDelta("hid","rsx",1);
    packet.addControl("hid","rsy",0x06,"H",0x00);
    packet.setMinDelta("hid","rsy",1);
    packet.addControl("hid","rs",0x0b,"H",0x02); */

    // sel/sta
    packet.addControl("hid","sel",0x0a,"H",0x40);
    packet.addControl("hid","sta",0x0a,"H",0x80);

    // shoulder buttons
    packet.addControl("hid","lb",0x0a,"H",0x10);
    packet.addControl("hid","rb",0x0a,"H",0x20);

    // triggers
    packet.addControl("hid","lt",9,"H",0xFF);
    packet.addControl("hid","rt",9,"H",0x00);
    packet.addControl("hid","t_min",9,"H",0x80);

    this.controller.registerInputPacket(packet);
  };

  this.registerOutputPackets = function () {}; // No default scalers: all controls done with callbacks anyway


  this.registerScalers = function () {}; // Register your own callbacks in caller by overriding this


  this.registerCallbacks = function () {};
}

xbc = new XBC();

xbc.init = function (id, debugging) {
  var con = xbc.controller;
  
  xbc.id = id;

  xbc.registerInputPackets();
  xbc.registerCallbacks();

  con.startAutoRepeatTimer = function(tid, interval) {
    if(con.timers[tid]) return;
    con.timers[tid] = engine.beginTimer(interval);
  }

  HIDDebug("xbc mapping "+id+" initialized");
};

xbc.shutdown = function () {
  xbc.controller.close();
  HIDDebug("shutting down controller");
};

xbc.incomingData = function (data, length) {
  // Remap certain bytes to circumvent "one-bit-bitmask" limitation of HIDPacketParser
  remapHex(data, 11, 0x04, 10, 0x03);
  remapHex(data, 11, 0x14, 10, 0x05);
  remapHex(data, 11, 0x1C, 10, 0x06);
  remapHex(data, 11, 0x0C, 10, 0x07);
  HIDDebug(_.join(data, " "));
  xbc.controller.parsePacket(data, length);
};

xbc.registerCallbacks = function(id) {
  var con = xbc.controller;
  var packet = con.getInputPacket("control");

  if(packet == undefined) {
    HIDDebug("no input packet "+con+" defined");
    return;
  }

  if(con == undefined) {
    HIDDebug("controller is undefined");
    return;
  }

  con.linkControl("hid","a","deck1","beats_adjust_slower");
  con.linkControl("hid","b","deck1","eject");
  con.linkControl("hid","x","deck1","play");
  con.linkControl("hid","y","deck1","beats_adjust_faster");
  con.linkControl("hid","pov_u","deck2","beats_adjust_slower");
  con.linkControl("hid","pov_r","deck2","eject");
  con.linkControl("hid","pov_l","deck2","play");
  con.linkControl("hid","pov_d","deck2","beats_adjust_faster");
  con.linkControl("hid","sel","mixer","crossfader_down");
  con.linkControl("hid","sta","Master","crossfader_up");
/*   con.setCallback("control","hid","lsy",1,"H",0x00);
  con.setCallback("control","hid","ls",12,"H",0x01);
  con.setCallback("control","hid","rsx",5,"H",0xFF);
  con.setCallback("control","hid","rsx",7,"H",0x00);
  con.setCallback("control","hid","rs",12,"H",0x02);
  con.linkControl("hid","lb","deck2","loop_enable");
  con.setCallback("control","hid","rb",0x0b,"H",0x20);
  con.setCallback("control","hid","lt",0x0a,"H",0xFF);
  con.setCallback("control","hid","rt",0x0a,"H",0x00);
  con.setCallback("control","hid","t_min",0x0a,"H",0x80);
  con.setCallback("control","hid","lsy",xbc.left_jog);
  con.setCallback("control","hid","rsy",xbc.right_jog);

  con.setCallback("control","hid","lsx",xbc.left_jog);
  con.setCallback("control","hid","rsx",xbc.right_jog);*/
};