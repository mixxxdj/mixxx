#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This script sends the Serato SysEx messages to a Serato MIDI controller, then
continuously sends a special MIDI heartbeat message while presenting an
interactive prompt that allows sending custom MIDI messages. This is useful for
controllers like the Roland DJ-505, that will fall back into a generic mode
with a reduced feature set if that's not the case.

When you're going to create a mapping for such a controller, you can use this
script to send custom different MIDI messages and see how the controller reacts
to it while that mode is active. Please make sure that the actual mapping must
not depend on this script being running in the background. Instead, you can
just add somethink like this to your JavaScript controller mapping:

    // Send Serato SysEx messages to request initial state and unlock pads
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x00, 0xF7], 6);
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x01, 0xF7], 6);
    // Send "keep-alive" message to keep controller in Serato mode
    engine.beginTimer(500, function() {
        midi.sendShortMsg(0xBF, 0x64, 0x00);
    });

Optionally, you can also supply a Mixxx XML mapping file to this script. It
will use that information to present information about available MIDI messages.
"""
import argparse
import queue
import sys
import threading
import time
import xml.etree.ElementTree
import mido

try:
    import prompt_toolkit
except ImportError:
    prompt_session = None
else:
    prompt_session = prompt_toolkit.PromptSession(
        history=prompt_toolkit.history.InMemoryHistory(),
        auto_suggest=prompt_toolkit.auto_suggest.AutoSuggestFromHistory(),
    )


def prompt(text):
    if prompt_session:
        return prompt_session.prompt(text)
    return input(text)


MSG_SYSEX1 = mido.Message.from_bytes([0xF0, 0x00, 0x20, 0x7F, 0x00, 0xF7])
MSG_SYSEX2 = mido.Message.from_bytes([0xF0, 0x00, 0x20, 0x7F, 0x01, 0xF7])
MSG_SERATO_KEEPALIVE = mido.Message.from_bytes([0xBF, 0x64, 0x00])


def load_descriptions(filename):
    tree = xml.etree.ElementTree.parse(filename)  # nosec
    for control in tree.findall("controller/controls/control"):
        description = control.find("description")
        status = control.find("status")
        midino = control.find("midino")
        if not any((description is None, status is None, midino is None)):
            yield (
                (int(status.text, 16), int(midino.text, 16)),
                description.text,
            )


def serato_keepalive(q, stop_event):
    while not stop_event.isSet():
        q.put(MSG_SERATO_KEEPALIVE)
        time.sleep(0.25)


def send_midi_messages(q, port):
    while True:
        item = q.get()
        if item is None:
            break
        port.send(item)


def print_help(descriptions):
    if descriptions:
        print("MIDI   DESCRIPTION")
        print("-----  -----------")
        for midi, desc in descriptions.items():
            print("{:02x} {:02x}  {}".format(*midi, desc))
        print("-----  -----------")
        print("")
    print("Please input hex bytes (without leading 0x) or type help/exit.")


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Interactive MIDI prompt for Serato mode controllers.",
        epilog=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("port", metavar="PORT", help="MIDI output port name")
    parser.add_argument("-f", "--xmlfile", help="Mixxx XML file")
    args = parser.parse_args(argv)

    portname = None
    for portname in mido.get_output_names():
        if args.port in portname:
            break
    else:
        print("Did not find output!")
        return 1
    port = mido.open_output(portname)

    q = queue.Queue()
    q.put(MSG_SYSEX1)
    q.put(MSG_SYSEX2)

    stop_event = threading.Event()
    t1 = threading.Thread(
        target=serato_keepalive, args=(q, stop_event), daemon=True,
    )
    t2 = threading.Thread(
        target=send_midi_messages, args=(q, port), daemon=True,
    )
    t1.start()
    t2.start()

    if args.xmlfile:
        try:
            descriptions = dict(load_descriptions(args.xmlfile))
        except Exception:
            print("Failed to parse XML file: {}".format(args.xmlfile))
            descriptions = {}
    else:
        descriptions = {}

    print_help(descriptions)
    while True:
        try:
            text = prompt("> ").strip()
        except EOFError:
            text = "exit"

        if not text:
            continue

        if text == "exit":
            break

        if text == "help":
            print_help(descriptions)
            continue

        try:
            message = mido.Message.from_hex(text.replace(" ", ""))
        except Exception as e:
            print("Failed to parse Message: %r" % e)
            continue

        description = descriptions.get(tuple(message.bytes()[:2]))
        print(
            "{:30s} {}{}".format(
                " ".join("0x{:02x}".format(b) for b in message.bytes()),
                str(message),
                (" // " + description) if description else "",
            )
        )

        q.put(message)

    stop_event.set()
    q.put(None)

    return 0


if __name__ == "__main__":
    sys.exit(main())
