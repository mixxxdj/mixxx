# Queue input

The old queued input mechanism present in RtMidi and previous versions of the library has been moved out of the code as it can be built entirely on the callback mechanism and integrated with the user application's event processing queue instead.

A basic example is provided in `qmidiin.cpp`.

We recommend if possible to instead use an asynchronous runtime in order to keep an imperative behaviour while benefiting from the non-blocking properties of async programming.

An example based on C++20 coroutines with Boost.Cobalt is provided in `coroutines.cpp`.
