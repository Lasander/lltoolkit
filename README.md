# lltookit - Playground for C++

This repo contains random trials, errors and works in progress exploring new concepts or ideas with C++.
There's no real output other than the unit tests trying out the code using GTest.

Also trialing using github and the services provided by it.

## Development

Toolkit uses cmake. Running cmake should automatically download and make a temporary install of GTest (and GMock).

Targets `all` and `test` can be used to build and test everything.
There's also an `install` target just for testing it out.

### vscode

Toolkit includes configuration for vscode, which I've used with the following C++ related extensions:

- christian-kohler.npm-intellisense
- ms-vscode.cpptools
- twxs.cmake
- vadimcn.vscode-lldb
- vector-of-bool.cmake-tools
- PeterJausovec.vscode-docker # to manage the docker containers
