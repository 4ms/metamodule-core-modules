## 4ms CoreModules

This repo contains the modules from 4ms Company, which are in the CoreModule format.

## User Manual

[User Manual for modules](docs/4ms-coremodules-manual.md)

## Building

This repo is meant to be a submodule within a larger framework such as the MetaModule firmware
or a VCV Rack plugin.

This depends on 4ms cpputil, and the metamodule-plugin-sdk, which must be included in the 
project.

If you want to build these modules into a different framework, you should study these examples:

- MetaModule: https://github.com/4ms/metamodule/tree/main/firmware

- 4ms VCV Rack plugin: https://github.com/4ms/4ms-vcv

