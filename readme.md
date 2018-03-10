LoraDeviceLib
=============

A LoRaWAN device implementation still in an early stage of development.

[![Build Status](https://travis-ci.org/cjhdev/lora_device_lib.svg?branch=master)](https://travis-ci.org/cjhdev/lora_device_lib)

## Highlights

- Portable design
- Region may be selected at run-time
- Support for SX1272
- Build-time options for optimisation and/or conformance to your project requirements
- Tests utilising the Cmocka framework
- Ruby binding for accelerated testing, experimentation, tool making, etc.
- Linted to MISRA 2012

## Porting Guide

See [doc/porting_guide.md](doc/porting_guide.md).

## Technical Overview

See [doc/technical_overview.md](doc/technical_overview.md).

## Examples

- [AVR ATMega328p target](examples/mega_demo)
- [Ruby port](bindings/ruby/ext/ldl/ext_mac/ext_mac.c)

## License

LoraDeviceLib has an MIT license. 

