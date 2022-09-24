# MPMCQueue.h

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/rigtorp/MPMCQueue/master/LICENSE)

A fork of [Erik Rigtorp's MPMCQueue](https://github.com/rigtorp/MPMCQueue), a bounded multi-producer multi-consumer concurrent queue written in C++11.

This fork offers an additional `try_consume_until_current_head` method that consumes all[1] elements currently on the queue. Elements concurrently added while the method is being executed won't be consumed.

[1] This method consumes **all** the elements currently on the queue only when it is the only when there are no other concurrent consumers. If there are other concurrent consumers, this method might **not** consume all the elements currently on the queue because other consumers might consume some of them.

## About
[This fork](https://github.com/ferranpujolcamins/MPMCQueue) is mantained by Ferran Pujol Camins, based on [the original project](https://github.com/rigtorp/MPMCQueue) by Erik Rigtorp.
