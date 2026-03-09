#!/usr/bin/env python
import pylibremidi as lm

api = lm.midi2_default_api()
print(api)
observer_config = lm.ObserverConfiguration()
observer = lm.Observer(observer_config, api)

pi = observer.get_input_ports()
if len(pi) == 0:
    print("No input available")
    exit(1)
else:
    print(f"Found input: {pi[0].display_name}")

po = observer.get_output_ports()
if len(po) == 0:
    print("No output available")
    exit(1)
else:
    print(f"Found output: {po[0].display_name}")

out_config = lm.OutputConfiguration()
midi_out = lm.MidiOut(out_config, api)
err = midi_out.open_port(po[0])
if err:
    print("Error when opening output port: ", err)
    exit(1)

in_config = lm.UmpInputConfiguration()
in_config.on_message = lambda msg: print(f"{msg}")

midi_in = lm.MidiIn(in_config, api)
err = midi_in.open_port(pi[0])
if err:
    print("Error when opening input port: ", err)
    exit(1)

while True:
    midi_in.poll()
