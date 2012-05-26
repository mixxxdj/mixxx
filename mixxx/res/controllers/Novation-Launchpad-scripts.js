/****************************************************************/
/*      Novation Launchpad Mapping                              */
/*      For Mixxx version 1.11                                  */
/*      Author: zestoi                                          */
/****************************************************************/

NovationLaunchpad = {

	init: function() {

		//
		// setup variables and methods
		//

		this.page = 1;
		this.shift = 0;
		this.shift2 = 0;
		this.callbacks = {};
		this.feedbacks = {};
		this.cache = [{}, {}, {}, {}];
		this.feedback_cache = {};
		this.toggle_cache = [{}, {}, {}, {}];
		this.name2control = {};
		this.control2name = {};
		this.vumeters = [];

		var self = NovationLaunchpad;
		this.colors = self.colors();
		this.capture = self.capture;
		this.feedback = self.feedback;
		this.send = self.send;
		this.button = self.button;
		this.toggle = self.toggle;
		this.hotcue = self.hotcue;
		this.flanger = self.flanger;
		this.jog = self.jog;
		this.get = self.get;
		this.loop = self.loop;
		this.gator = self.gator;
		this.set_page = self.set_page;
		this.vfader = self.vfader;
		this.vumeter = self.vumeter;
		this.vumeter_toggle = self.vumeter_toggle;
		this.playlist = self.playlist;

		//
		// map the midi config into something more useful
		//

		var buttons = self.buttons();
		for (name in buttons) {
			var type = buttons[name][0];
			var chan = buttons[name][1];
			var value = buttons[name][2];
			var status = (type == 'cc') ? 0xb0 + chan - 1 : 0x90 + chan - 1;
			this.name2control[name]= [ status, value ];
			this.control2name["" + status + value] = name; // stringify it
		}

		//
		// reset device, enable flashing colors
		//

		midi.sendShortMsg(0xb0, 0x0, 0x0);
		midi.sendShortMsg(0xb0, 0x0, 0x28);

		/////////////////////////////////////////////////////////////////////////
		// button layout mapping starts here
		/////////////////////////////////////////////////////////////////////////

		// shift buttons

		this.button("arm", "all", 1, 'hi_yellow', 'lo_yellow', '', '', function(g, n, v) { this.shift = v > 0 ? 1 : 0; });
		this.button("solo", "all", 1, 'hi_yellow', 'lo_yellow', '', '', function(g, n, v) { this.shift2 = v > 0 ? 1 : 0; });
		this.toggle("mixer", "all", 0, 'hi_red', 'lo_red', '', '', function(g, n, v) { this.set_page(v > 0 ? 2 : 1); });
		this.toggle("vol", "all", 2, 'hi_red', 'lo_red', '', '', function(g, n, v) { this.vumeter_toggle(v); });

		//// MAIN PAGE ////

		// track navigation

		this.playlist("up", 0, "SelectPrevTrack");
		this.playlist("down", 0, "SelectNextTrack");
		this.button("left", "press", 0, 'hi_yellow', 'lo_yellow', "[Playlist]", "SelectPrevPlaylist");
		this.button("right", "press", 0, 'hi_yellow', 'lo_yellow', "[Playlist]", "SelectNextPlaylist");

		// deck mappings

		for (deck=1; deck<=2; deck++) {
			var offset = deck == 1 ? 0 : 4;
			var group = "[Channel" + deck + "]";

			// tracks

			this.toggle("0," + (offset + 0), "all", 1, 'hi_red', 'lo_red', group, "quantize");
			this.toggle("0," + (offset + 1), "all", 1, 'hi_red', 'lo_red', group, "keylock");
			this.toggle("0," + (offset + 2), "all", 1, 'hi_red', 'lo_red', group, "pfl");
			this.button("0," + (offset + 3), "all", 1, 'hi_yellow', 'lo_amber', group, "LoadSelectedTrack");

			// flanger

			this.flanger("1," + (offset + 0), 1, group, 0.5, 1500000, 333);
			this.flanger("1," + (offset + 1), 1, group, 1, 500000, 666);

			// spinback effect

			this.button("1," + (offset + 2), "all", 1, 'hi_red', 'lo_red', group, "", function(g, n, v) {
				script.spinback(g, v > 0);
			});

			// brake effect

			this.button("1," + (offset + 3), "all", 1, 'hi_red', 'lo_red', group, "", function(g, n, v) {
				script.brake(g, v > 0);
			});

			// instant loops 

			this.loop("2," + (offset + 0), 1, group, 1);
			this.loop("2," + (offset + 1), 1, group, 0.5);
			this.loop("2," + (offset + 2), 1, group, 0.25);
			this.loop("2," + (offset + 3), 1, group, 0.125);

			// loop in or loop half when active

			this.button("3," + (offset + 0), "press", 1, 'hi_green', 'lo_green', group, "", function(g, n, v) {
				if (engine.getValue(g, "loop_enabled")) {
					engine.setValue(g, "loop_halve", 1);
				}
				else {
					engine.setValue(g, "loop_in", 1);
				}
			});

			// loop out or loop double when active

			this.button("3," + (offset + 1), "press", 1, 'hi_green', 'lo_green', group, "", function(g, n, v) {
				if (engine.getValue(g, "loop_enabled")) {
					engine.setValue(g, "loop_double", 1);
				}
				else {
					engine.setValue(g, "loop_out", 1);
				}
			});

			// reloop or exit loop

			this.button("3," + (offset + 2), "all", 1, 'hi_green', 'lo_green', group, "reloop_exit");

			// gator effect

			this.gator("3," + (offset + 3),  1, group, 8, 0.7);

			// led feedback for loop in/out buttons to show loop status

			this.feedback(group, "loop_enabled", function(self, g, e, value) {
				var offset = g == "[Channel1]" ? 0 : 4; // ????
				self.send("3," + (offset + 0), self.colors[value > 0 ? 'hi_green' : 'lo_green'], page);
				self.send("3," + (offset + 1), self.colors[value > 0 ? 'hi_green' : 'lo_green'], page);
			});

			// hotcues or needle drop with shift2 pressed

			this.hotcue("4," + (offset + 0), 1, group, 1);
			this.hotcue("4," + (offset + 1), 1, group, 2);
			this.hotcue("4," + (offset + 2), 1, group, 3);
			this.hotcue("4," + (offset + 3), 1, group, 4);
			this.hotcue("5," + (offset + 0), 1, group, 5);
			this.hotcue("5," + (offset + 1), 1, group, 6);
			this.hotcue("5," + (offset + 2), 1, group, 7);
			this.hotcue("5," + (offset + 3), 1, group, 8);

			// transport

			this.button("6," + (offset + 0), "all", 1, 'hi_yellow', 'lo_red', group, "cue_default");

			this.button("6," + (offset + 1), "press", 1, 'hi_yellow', 'lo_yellow', group, "rate", function(g, n, v) { 
				engine.setValue(g, n, 0); 
			});

			this.button("6," + (offset + 2), "press", 1, 'hi_yellow', 'lo_yellow', group, "rate_perm_down_small");
			this.button("6," + (offset + 3), "press", 1, 'hi_yellow', 'lo_yellow', group, "rate_perm_up_small");

			// play button
			this.toggle("7," + (offset + 0), "press", 1, 'hi_yellow', 'lo_red', group, "play");

			// flash play button when near end of track
			this.feedback(group, "playposition", function(self, g, e, value) {
				if (value > 0.9 && engine.getValue(g, "play") > 0) {
					self.send(g == "[Channel1]" ? "7,0" : "7,4", self.colors['flash_hi_red'], 1);
				}
			});
			
			// sync or move beatgrid when shift is pressed

			this.button("7," + (offset + 1), "all", 1, 'hi_yellow', 'lo_amber', group, "beatsync", function(g, n, v) { 
				if (this.shift > 0)
					engine.setValue(g, "beats_translate_curpos", v > 0 ? 1 : 0); 
				else
					engine.setValue(g, n, v > 0 ? 1 : 0); 
			});

			// fwd/rev when not playing unless shift and then fine jog movements for beat gridding, jog when playing, jog more when shift+playing

			this.jog("7," + (offset + 2), 1, group, "back");
			this.jog("7," + (offset + 3), 1, group, "fwd");
		}

		//// MIXER PAGE ////

		this.toggle("0,0", "all", 2, 'flash_hi_red', 'lo_red', "[Channel1]", "filterHighKill");
		this.toggle("0,1", "all", 2, 'flash_hi_red', 'lo_red', "[Channel1]", "filterMidKill");
		this.toggle("0,2", "all", 2, 'flash_hi_red', 'lo_red', "[Channel1]", "filterLowKill");
		this.toggle("0,5", "all", 2, 'flash_hi_red', 'lo_red', "[Channel2]", "filterHighKill");
		this.toggle("0,6", "all", 2, 'flash_hi_red', 'lo_red', "[Channel2]", "filterMidKill");
		this.toggle("0,7", "all", 2, 'flash_hi_red', 'lo_red', "[Channel2]", "filterLowKill");

		this.vfader(7, 0, 2, 7, 'hi_orange', 'lo_green', "[Channel1]", "filterLow");
		this.vfader(7, 1, 2, 7, 'hi_orange', 'lo_green', "[Channel1]", "filterMid");
		this.vfader(7, 2, 2, 7, 'hi_orange', 'lo_green', "[Channel1]", "filterHigh");
		this.vfader(7, 5, 2, 7, 'hi_orange', 'lo_green', "[Channel2]", "filterLow");
		this.vfader(7, 6, 2, 7, 'hi_orange', 'lo_green', "[Channel2]", "filterMid");
		this.vfader(7, 7, 2, 7, 'hi_orange', 'lo_green', "[Channel2]", "filterHigh");

		this.vfader(7, 3, 2, 8, 'hi_yellow', 'lo_red', "[Channel1]", "volume");
		this.vfader(7, 4, 2, 8, 'hi_yellow', 'lo_red', "[Channel2]", "volume");
		this.vumeter(7, 3, 2, 8, 'hi_yellow', 'lo_red', "[Channel1]", "VuMeter");
		this.vumeter(7, 4, 2, 8, 'hi_yellow', 'lo_red', "[Channel2]", "VuMeter");

		/////////////////////////////////////////////////////////////////////////
		// button layout mapping ends here
		/////////////////////////////////////////////////////////////////////////
	},

	//
	// convert incoming midi to a 'name' and call callbacks (if any)
	//

	incomingData: function(channel, control, value, status, group) {
		if ((name = this.control2name["" + status + control]) != undefined) {
			if (this.callbacks[name] != undefined) {
				var callbacks = this.callbacks[name];
				for (var i=0; i<callbacks.length; i++) {
					if ((callbacks[i][1] == 0 || callbacks[i][1] == this.page) && typeof(callbacks[i][2]) == 'function') {

						//
						// check we need to call for this value change: all, press, release
						//

						if (callbacks[i][0] == "all" ||
							(callbacks[i][0] == "press" && value > 0) ||
							(callbacks[i][0] == "release" && value == 0)) {

							//
							// call a callback function for this control
							//

							callbacks[i][2](this, group, name, value);
						}
					}
				}
			}
		}
	},

	//
	// gator effect using high eq kill
	//

	gator: function(name, page, group, rate, depth) {
		this.button(name, "all", page, 'hi_red', 'lo_red', group, "", function(g, n, v) {
			var self = NovationLaunchpad;
			if (typeof(self.gator_timer) != undefined && self.gator_timer != null) {
				engine.stopTimer(self.gator_timer);
				self.gator_timer = null;
			}

			if (v > 0) {
				if ((bpm = engine.getValue(g, 'bpm')) > 0) {
					var interval = parseInt(1000 / bpm * 60 / rate);
					self.gator_direction = false;
					self.gator_depth = depth;
					self.gator_timer = engine.beginTimer(interval, 'NovationLaunchpad.process_gator("' + g + '")');
				}
			}
			else {
				engine.setValue(group, 'filterHighKill', 0);
			}
		});
	},

	//
	// gator
	//

	process_gator: function(group) {
		var self = NovationLaunchpad;
		self.gator_direction = !self.gator_direction;
		engine.setValue(group, 'filterHighKill', self.gator_direction ? 1 : 0);
	},

	//
	// flanger button
	//

	flanger: function(name, page, group, depth, period, delay) {
		this.button(name, "all", page, 'hi_amber', 'lo_amber', group, "flanger", function(g, name, v) {
			if (v > 0) {
				engine.setValue("[Flanger]", "lfoDepth", depth);
				engine.setValue("[Flanger]", "lfoPeriod", period);
				engine.setValue("[Flanger]", "lfoDelay", delay);
			}
			engine.setValue(group, "flanger", v > 0 ? 1 : 0);
		});
	},

	loop: function(name, page, group, size) {
		this.button(name, "all", page, 'hi_yellow', 'lo_yellow', group, "", function(g, name, v) {
			if (v > 0) {
				engine.setValue(g, "beatloop_" + size + "_activate", 1);
			}
			else {
				if (engine.getValue(g, "beatloop_" + size + "_enabled")) {
					engine.setValue(g, "beatloop_" + size + "_toggle", 1);
				}
			}
		});
	},

	//
	// track scrolling
	//

	playlist: function(name, page, action) {
		this.button(name, "all", page, 'hi_yellow', 'lo_yellow', "[Playlist]", action, function(g, n, v) {
			var self = NovationLaunchpad;
			if (typeof(self.playlist_timer) != undefined && self.playlist_timer != null) {
				engine.stopTimer(self.playlist_timer);
				self.playlist_timer = null;
			}

			if (v > 0) {
				engine.setValue("[Playlist]", action, 1);
				self.playlist_timer = engine.beginTimer(this.shift > 0 ? 30 : 150, 'NovationLaunchpad.process_playlist("' + action + '")');
			}
		});
	},

	process_playlist: function(name) {
		engine.setValue("[Playlist]", name, 1);
	},

	//
	// map a callback to an event from mixxx
	//

	feedback: function(g, e, f) {
		if (g != "" && e != "") {
			engine.connectControl(g, e, "NovationLaunchpad.feedbackData");
			if (this.feedbacks[g + e] == undefined) {
				this.feedbacks[g + e] = [];
			}
			this.feedbacks[g + e].push(f);
		}
	},

	//
	// call callbacks from mixxx events
	//

	feedbackData: function(v, g, e) {
		this.feedback_cache[g + e] = v;
		if (this.feedbacks[g + e] != undefined) {
			for (func in this.feedbacks[g + e]) {
				if (typeof(this.feedbacks[g + e][func]) == "function") {
					this.feedbacks[g + e][func](this, g, e, v);
				}
			}
		}
	},

	//
	// map a callback to a launchpad button name
	//

	capture: function(name, values, page, func) {
		if (this.callbacks[name] == undefined) {
			this.callbacks[name] = [ [ values, page, func ] ];
		}
		else {
			this.callbacks[name].push([ values, page, func ]);
		}
	},

	//
	// send back to the launchpad for leds by name
	//

	send: function(name, value, page) {
		if (page == 0 || this.page == page) {
			if ((control = this.name2control[name]) != undefined) {
				if (this.cache[page][name] == value) return;
				midi.sendShortMsg(control[0], control[1], value);
			}
		}
		this.cache[page][name] = value;
	},

	//
	// hold button
	//

	button: function(name, values, page, on_color, off_color, group, event, callback) {

		// launchpad => mixxx

		this.capture(name, "all", page, function(self, g, name, value) {
			if (callback == undefined) {
				engine.setValue(group, event, value);
			}
			else if (typeof(callback) == "function") {
				if (values == "all" || (values == "press" && value > 0) || (values == "release" && value == 0)) {
					callback(group, event, value);
				}
			}

			if (values == "all" || (values == "press" && value > 0) || (values == "release" && value == 0)) {
				self.send(name, self.colors[value > 0 ? on_color : off_color], page);
			}
		});

		// mixxx => launchpad

		this.feedback(group, event, function(self, g, e, value) {
			self.send(name, self.colors[value > 0 ? on_color : off_color], page);
		});

		// init led

		this.send(name, this.colors[off_color], page);
	},

	//
	// toggle
	//

	toggle:  function(name, values, page, on_color, off_color, group, event, callback) {
		this.capture(name, "press", page, function(self, g, name, value) {
			if (typeof(self.toggle_cache[page][name]) == "undefined") {
				self.toggle_cache[page][name] = 0;
			}
			self.toggle_cache[page][name] = self.toggle_cache[page][name] == 0 ? 1 : 0;

			if (callback == undefined) {
				engine.setValue(group, event, self.toggle_cache[page][name]);
			}
			else if (typeof(callback) == "function") {
				callback(group, event, self.toggle_cache[page][name]);
			}

			self.send(name, self.colors[self.toggle_cache[page][name] > 0 ? on_color : off_color], page);
		});

		// mixxx => launchpad

		this.feedback(group, event, function(self, g, e, value) {
			self.send(name, self.colors[value > 0 ? on_color : off_color], page);
			self.toggle_cache[page][name] = value > 0 ? 1 : 0;
		});

		// init led

		this.send(name, this.colors[off_color], page);
	},

	//
	// hotcues
	//

	hotcue: function(name, page, group, num) {
		this.capture(name, "press", page, function(self, g, name, value) {
			if (self.shift2) {
				engine.setValue(group, "playposition", (num-1)/8);
			}
			else if (self.shift) {
				engine.setValue(group, "hotcue_" + num + "_clear", 1);
			}
			else {
				engine.setValue(group, "hotcue_" + num + "_activate", 1);
			}
		});

		this.feedback(group, "hotcue_" + num + "_enabled", function(self, g, e, value) { 
			self.send(name, self.colors[value > 0 ? 'hi_red' : 'black'], page);
		});
	},

	//
	// jog
	//

	jog: function(name, page, group, dir) {
		this.button(name, "all", page, 'hi_yellow', 'lo_amber', group, "", function(g, n, v) { 

			if (dir == "fwd") {
				mult = 1;
				rate = "rate_temp_up";
			}
			else {
				mult = -1;
				rate = "rate_temp_down";
			}

			if (engine.getValue(g, "play") > 0) {
				if (this.shift > 0) {
					engine.setValue(g, rate, v > 0 ? 1 : 0);
				}
				else {
					engine.setValue(g, rate + "_small", v > 0 ? 1 : 0);
				}
			}
			else if (this.shift > 0) {
				if (v > 0) {
					engine.setValue(g, 'jog', 0.2 * mult); 
				}
			}
			else engine.setValue(g, dir, v); 
		});
	},

	//
	// turn a column of pads into a virtual fader
	//

	vfader: function(y, x, page, nbtns, on_color, off_color, group, action) {
		var incr = 1 / nbtns;

		// launchpad => mixxx

		for (var btn=0; btn<nbtns; btn++) {
			this.capture((y-btn)+","+x, "press", page, function(self, g, name, value) {
				var cap = name.match(/^(\d+),\d+/);
				var num = y - cap[1] + 1;
				engine.setValue(group, action, incr * num);
			});
			this.send((y-btn)+","+x, this.colors[on_color], page);
		}

		// mixxx => launchpad

		this.feedback(group, action, function(self, g, e, value) { 
			for (btn=0; btn<nbtns; btn++) {
				if (value > btn*incr) {
					self.send((y-btn)+","+x, self.colors[on_color], page);
				}
				else {
					self.send((y-btn)+","+x, self.colors[off_color], page);
				}
			}
		});
	},

	//
	// turn a column of pads into a vumeter
	//

	vumeter: function(y, x, page, nbtns, on_color, off_color, group, action) {
		var incr = 1 / nbtns;
		this.vumeters.push([ y, x, page, nbtns, on_color, off_color, group, action ]);
		this.feedback(group, action, function(self, g, e, value) { 
			if (self.vumeter_shift > 0) {
				for (btn=0; btn<nbtns; btn++) {
					if (value > btn*incr) {
						self.send((y-btn)+","+x, self.colors[on_color], page);
					}
					else {
						self.send((y-btn)+","+x, self.colors[off_color], page);
					}
				}
			}
		});
	},

	vumeter_toggle: function(v) {
		this.vumeter_shift = v > 0 ? 1 : 0;

		//
		// clear fader leds when enabling vumeter and set back the leds for the volume if disabling
		//

		for (i in this.vumeters) {

			var value = this.vumeter_shift > 0 ? 0 : this.feedback_cache[ this.vumeters[i][6] + 'volume' ];

			if (value != undefined) { 
				var y = this.vumeters[i][0];
				var x = this.vumeters[i][1];
				var page = this.vumeters[i][2];
				var nbtns = this.vumeters[i][3];
				var on_color = this.vumeters[i][4];
				var off_color = this.vumeters[i][5];
				var incr = 1 / nbtns;

				for (btn=0; btn<nbtns; btn++) {
					if (value > btn*incr) {
						this.send((y-btn)+","+x, this.colors[on_color], page);
					}
					else {
						this.send((y-btn)+","+x, this.colors[off_color], page);
					}
				}
			}
		}
	},

	//
	// get the last value sent to a launchpad led
	//

	get: function(name, page) {
		if (typeof(this.cache[page][name]) == undefined) {
			return 0;
		}
		else {
			return this.cache[page][name];
		}
	},

	//
	// set page
	//

	set_page: function(page) {
		if (page == this.page) return;

		var updates = {};
		var flashing = [];

		for (i in this.cache[page]) {
			if (this.cache[this.page][i] == undefined || this.cache[this.page][i] != this.cache[page][i]) {
				updates[i] = this.cache[page][i];
			}
		}

		for (i in this.cache[this.page]) {
			if (this.cache[page][i] == undefined) {
				updates[i] = 0x4; // black with copy bit set
			}
			else if (this.cache[this.page][i] != this.cache[page][i] && updates[i] == undefined) {
				updates[i] = this.cache[page][i];
			}
		}

		// select buffer 1
		midi.sendShortMsg(0xb0, 0x0, 0x31);

		for (i in updates) {
			if ((control = this.name2control[i]) != undefined) {

				// 0 makes no sense - need 0x4 for black/off
				if (updates[i] == 0) {
					updates[i] = 0x4;
				}

				// send out non-flashing colors with copy bit removed
				if (updates[i] & 0x4) {
					midi.sendShortMsg(control[0], control[1], updates[i] & 0xfb);
				}
				else {
					// send out off for this buffer for a flashing color
					midi.sendShortMsg(control[0], control[1], 0x4);
					flashing.push([ control[0], control[1], updates[i] ]);
				}
			}
		}

		// select buffer 0
		midi.sendShortMsg(0xb0, 0x0, 0x34);

		// send out any flashing updates
		for (i in flashing) {
			midi.sendShortMsg(flashing[i][0], flashing[i][1], flashing[i][2]);
		}

		// re-enable internal buffer cycling for flashing colors
		midi.sendShortMsg(0xb0, 0x0, 0x28);

		this.page = page;
	},

	//
	// define colors
	//

	colors: function() {
		return {
			black: 4,

			lo_red: 1 + 4,
			mi_red: 2 + 4,
			hi_red: 3 + 4,
			lo_green: 16 + 4,
			mi_green: 32 + 4,
			hi_green: 48 + 4,
			lo_amber: 17 + 4,
			mi_amber: 34 + 4,
			hi_amber: 51 + 4,
			hi_orange: 35 + 4,
			lo_orange: 18 + 4,
			hi_yellow: 50 + 4,
			lo_yellow: 33 + 4,
			
			flash_lo_red: 1,
			flash_mi_red: 2,
			flash_hi_red: 3,
			flash_lo_green: 16,
			flash_mi_green: 32,
			flash_hi_green: 48,
			flash_lo_amber: 17,
			flash_mi_amber: 34,
			flash_hi_amber: 51,
			flash_hi_orange: 35,
			flash_lo_orange: 18,
			flash_hi_yellow: 50,
			flash_lo_yellow: 33
		}
	},

	//
	// define midi for all the buttons (as we can't define names in the xml or access that data here)
	// to create a 90 degree rotated mapping just redefine this list so "0,0" is still top left etc
	//

	buttons: function() {
		return {
			'up':      [ 'cc',   1, 0x68 ],
			'down':    [ 'cc',   1, 0x69 ],
			'left':    [ 'cc',   1, 0x6A ],
			'right':   [ 'cc',   1, 0x6B ],
			'session': [ 'cc',   1, 0x6C ],
			'user1':   [ 'cc',   1, 0x6D ],
			'user2':   [ 'cc',   1, 0x6E ],
			'mixer':   [ 'cc',   1, 0x6F ],
			'vol':     [ 'note', 1, 0x8 ],
			'pan':     [ 'note', 1, 0x18 ],
			'snda':    [ 'note', 1, 0x28 ],
			'sndb':    [ 'note', 1, 0x38 ],
			'stop':    [ 'note', 1, 0x48 ],
			'trkon':   [ 'note', 1, 0x58 ],
			'solo':    [ 'note', 1, 0x68 ],
			'arm':     [ 'note', 1, 0x78 ],
			'0,0':     [ 'note', 1, 0x00 ],
			'0,1':     [ 'note', 1, 0x01 ],
			'0,2':     [ 'note', 1, 0x02 ],
			'0,3':     [ 'note', 1, 0x03 ],
			'0,4':     [ 'note', 1, 0x04 ],
			'0,5':     [ 'note', 1, 0x05 ],
			'0,6':     [ 'note', 1, 0x06 ],
			'0,7':     [ 'note', 1, 0x07 ],
			'1,0':     [ 'note', 1, 0x10 ],
			'1,1':     [ 'note', 1, 0x11 ],
			'1,2':     [ 'note', 1, 0x12 ],
			'1,3':     [ 'note', 1, 0x13 ],
			'1,4':     [ 'note', 1, 0x14 ],
			'1,5':     [ 'note', 1, 0x15 ],
			'1,6':     [ 'note', 1, 0x16 ],
			'1,7':     [ 'note', 1, 0x17 ],
			'2,0':     [ 'note', 1, 0x20 ],
			'2,1':     [ 'note', 1, 0x21 ],
			'2,2':     [ 'note', 1, 0x22 ],
			'2,3':     [ 'note', 1, 0x23 ],
			'2,4':     [ 'note', 1, 0x24 ],
			'2,5':     [ 'note', 1, 0x25 ],
			'2,6':     [ 'note', 1, 0x26 ],
			'2,7':     [ 'note', 1, 0x27 ],
			'3,0':     [ 'note', 1, 0x30 ],
			'3,1':     [ 'note', 1, 0x31 ],
			'3,2':     [ 'note', 1, 0x32 ],
			'3,3':     [ 'note', 1, 0x33 ],
			'3,4':     [ 'note', 1, 0x34 ],
			'3,5':     [ 'note', 1, 0x35 ],
			'3,6':     [ 'note', 1, 0x36 ],
			'3,7':     [ 'note', 1, 0x37 ],
			'4,0':     [ 'note', 1, 0x40 ],
			'4,1':     [ 'note', 1, 0x41 ],
			'4,2':     [ 'note', 1, 0x42 ],
			'4,3':     [ 'note', 1, 0x43 ],
			'4,4':     [ 'note', 1, 0x44 ],
			'4,5':     [ 'note', 1, 0x45 ],
			'4,6':     [ 'note', 1, 0x46 ],
			'4,7':     [ 'note', 1, 0x47 ],
			'5,0':     [ 'note', 1, 0x50 ],
			'5,1':     [ 'note', 1, 0x51 ],
			'5,2':     [ 'note', 1, 0x52 ],
			'5,3':     [ 'note', 1, 0x53 ],
			'5,4':     [ 'note', 1, 0x54 ],
			'5,5':     [ 'note', 1, 0x55 ],
			'5,6':     [ 'note', 1, 0x56 ],
			'5,7':     [ 'note', 1, 0x57 ],
			'6,0':     [ 'note', 1, 0x60 ],
			'6,1':     [ 'note', 1, 0x61 ],
			'6,2':     [ 'note', 1, 0x62 ],
			'6,3':     [ 'note', 1, 0x63 ],
			'6,4':     [ 'note', 1, 0x64 ],
			'6,5':     [ 'note', 1, 0x65 ],
			'6,6':     [ 'note', 1, 0x66 ],
			'6,7':     [ 'note', 1, 0x67 ],
			'7,0':     [ 'note', 1, 0x70 ],
			'7,1':     [ 'note', 1, 0x71 ],
			'7,2':     [ 'note', 1, 0x72 ],
			'7,3':     [ 'note', 1, 0x73 ],
			'7,4':     [ 'note', 1, 0x74 ],
			'7,5':     [ 'note', 1, 0x75 ],
			'7,6':     [ 'note', 1, 0x76 ],
			'7,7':     [ 'note', 1, 0x77 ]
		};
	}
};

