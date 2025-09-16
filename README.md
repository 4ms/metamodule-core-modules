## 4ms CoreModules

This repo contains the modules from 4ms Company, which are in the CoreModule format.

Documentation on the CoreModule format and the CoreProcess base class is in the [metamodule-plugin-sdk README](https://github.com/4ms/metamodule-plugin-sdk/blob/main/README.md).

## User Manual

[User Manual for modules](doc/4ms-coremodules-manual.md)

## Building

This is a library intended to be a included in a larger framework such as the
MetaModule firmware or a VCV Rack plugin.

If you want to build these modules into a different framework, you should study these examples:

- MetaModule: https://github.com/4ms/metamodule/tree/main/firmware

- 4ms VCV Rack plugin: https://github.com/4ms/4ms-vcv

This requires the [4ms cpputil library](https://github.com/4ms/cpputil), and
the [metamodule-plugin-sdk](https://github.com/4ms/metamodule-plugin-sdk)
(just the metamodule-core-interface library).
