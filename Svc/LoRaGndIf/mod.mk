#
#   Copyright 2004-2008, by the California Institute of Technology.
#   ALL RIGHTS RESERVED. United States Government Sponsorship
#   acknowledged.
#
#

# This is a template for the mod.mk file that goes in each module
# and each module's subdirectories.
# With a fresh checkout, "make gen_make" should be invoked. It should also be
# run if any of the variables are updated. Any unused variables can 
# be deleted from the file.

# There are some standard files that are included for reference

SRC =

HDR =				LoRaGndIfImpl.hpp
					
HDR_RASPIAN =		LoRaGndIfImpl.hpp \
					LoRaLibs_RasPi/RadioHead.h \
					LoRaLibs_RasPi/RasPi.h \
					LoRaLibs_RasPi/RH_RF95.h \
					LoRaLibs_RasPi/RHDatagram.h \
					LoRaLibs_RasPi/RHGenericDriver.h \
					LoRaLibs_RasPi/RHGenericSPI.h \
					LoRaLibs_RasPi/RHHardwareSPI.h \
					LoRaLibs_RasPi/RHSPIDriver.h

SRC_LINUX = 		SocketGndIfImpl.cpp

SRC_CYGWIN =		SocketGndIfImpl.cpp

SRC_DARWIN =        SocketGndIfImpl.cpp

SRC_RASPIAN = 		LoRaGndIfImpl.cpp  \
					LoRaLibs_RasPi/RasPi.cpp \
					LoRaLibs_RasPi/RH_RF95.cpp \
					LoRaLibs_RasPi/RHDatagram.cpp \
					LoRaLibs_RasPi/RHGenericDriver.cpp \
					LoRaLibs_RasPi/RHGenericSPI.cpp \
					LoRaLibs_RasPi/RHHardwareSPI.cpp \
					LoRaLibs_RasPi/RHSPIDriver.cpp
