#!/usr/bin/env bash

##############################################################################
#
# File: ci.sh
#
# Function:
#     Travis script for test-building this library.
#
# Copyright Notice:
#     See LICENSE file accompanying this project.
#
# Author:
#     frazar     October 2020
#
##############################################################################

# Treat unset variables and parameters as an error
set -o nounset

# Exit immediately if a command fails
set -o errexit

# If set, the return value of a pipeline is the value of the last (rightmost)
# command to exit with a non-zero status, or zero if all commands in the
# pipeline exit successfully
set -o pipefail

# Configure bash to output debug information
set -o verbose

##############################################################################
# Run tests

# Install the Arduino-LMIC library
arduino-cli lib install "MCCI LoRaWAN LMIC library"

# Compile a simple example sketch from the Arduino-LMIC library
arduino-cli compile --verbose --libraries .. --fqbn 'mcci:stm32:mcci_catena_4551:lorawan_region=us915,opt=osstd,xserial=generic,upload_method=STLink,sysclk=pll32m' --build-properties recipe.hooks.objcopy.postobjcopy.1.pattern=true tests/raw-feather/raw-feather.ino
