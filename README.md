# UCIEngine

A very barebones engine in C++ to parse UCI commands and provide an interface for a custom implementation.

The idea is that you can inherit the `Engine` class, and write your own backend for the rest
(i.e. the chess game itself, finding moves, etc.). You can override some of the `Handle-` functions, which will
be called at appropriate times. Further explanation on the functions can be found in `Engine.h` or in the link below,
explaining the UCI protocol.

Written in C++20.

See [this webpage](http://wbec-ridderkerk.nl/html/UCIProtocol.html) for a description of the UCI protocol.
Snippets from this webpage will be copy pasted throughout the code.