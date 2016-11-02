# OSC-Client

## What is OSC?
"Open Sound Control (OSC) is a protocol for networking sound synthesizers, computers, and other multimedia devices for purposes such as musical performance or show control. OSC's advantages include interoperability, accuracy, flexibility and enhanced organization and documentation." [https://en.wikipedia.org/wiki/Open_Sound_Control]

## What is implemented in Mixxx
This branch can send for each deck:
* if it is playing
* the title,
* the volume,
* the (relative) play-position
* the duration of the loaded song

Additionally the number of decks is transmitted

## How to use it?
Mixxx works as a osc-client. In order to get the data, you need an osc-server.

If you want to use the data in your own application you can set up your own server. An example implementation can be found here: http://liblo.sourceforge.net/examples/

### OSC-Sceme:

| osc path        | description | parameters
| -------------   |:------------- |----------|
|/mixxx/numDecks | the number of Decks| numDecks (int)
|/mixxx/deck/playing | Is the deck playing? |deckNr (int)<br>isPlaying (int)
|/mixxx/deck/volume  | The real volume of the deck. Including crossfader. <br>Zero if not playing|deckNr (int)<br>volume (float)
|/mixxx/deck/pos | The relative play-position | deckNr (int)<br>position (float)
|/mixxx/deck/duration | The absolute duration | deckNr (int)<br>duration (float)
|/mixxx/deck/title  | The title of the playing song | deckNr (int)<br> title (sting)

 The messages are sent every half second and on ui changes (e.g. if play was pressed)
