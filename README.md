# open (S)ecure (I)nstant (M)essaging (P)rotocol
![license](https://img.shields.io/github/license/Spixa/openSIMP)
![issues](https://img.shields.io/github/issues-raw/Spixa/openSIMP)
![stargazers](https://img.shields.io/github/stars/Spixa/openSIMP)
![languuages](https://img.shields.io/github/languages/count/Spixa/openSIMP)

[![forthebadge](https://forthebadge.com/images/badges/made-with-rust.svg)](https://forthebadge.com)
[![forthebadge](https://forthebadge.com/images/badges/made-with-c-plus-plus.svg)](https://forthebadge.com)

The open-source, secure instant messaging protocol, developed by 3 hobbyists as a way to practice their chosen language, in order:
- [Spixa](https://github.com/Spixa), working on c++ components (server, debug client & indev TUI client)
- [phnixir (a.k.a phoenix_ir_)](https://github.com/phnixir), actively working on a minimalistic Rust gui client and maintaining a debugging utility also written in rust.
- [KasraIDK](https://github.com/KasraIDK), planning to create an android client with Java.

openSIMP is an indev protocol built from the ground up with openness and security in mind, openness as in being able to implement your own servers and make your own clients with ease, after stabilizing the main server implementation (c++), a specification will be released as to what packets a server should be able to parse and send and the same thing for clients as well, encryption will be built in on both sides and is enforced.

This protocol was also created as a way to be based, knowing what every software that you're running is doing in the background and as such all of the officials implementations (the ones inside this very repository) are all licensed under an open source license and will stay that way forever.

## Roadmap
- [ ] (General) Make clients in C++, Rust and Java
    - [x] C++
    - [x] Rust
    - [ ] Java
- [ ] (Rust client) Divide the client into a `bin+lib` crate and refactor
- [ ] (Server) Add a console and commands
- [x] (Server) Add a name-system
- [ ] (Server) Add more packets

## Licensing
The openSIMP server, the clients and the debug utilities are all licensed under the [Mozilla Public License, version 2.0](https://www.mozilla.org/en-US/MPL/2.0/).

## Contributing
Please open an issue or merge request to contibute. Code contributions submitted for inclusion in the work by you, as defined in the MPL2.0 license, shall be licensed as the above without any additional terms or conditions.
