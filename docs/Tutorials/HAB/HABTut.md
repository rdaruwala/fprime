# Introduction

The following example shows the steps to implement a simple pair of components connected by a pair of ports. The first, `MathSender`, will invoke the second, `MathReceiver`, via  a `MathOp` port to perform a math operation and return the result via a `MathResult` port. 

The following example shows the steps to implement a high altitude balloon payload running on a Raspberry Pi using several inexpensive, commercially available sensors and a radio. Please note that this tutorial serves as an example - To date, this payload has not been tested or flown on a high-altitude balloon.

TODO picture

All the code in this tutorial can be found in this directory. The actual location of the components will be in the `Ref` directory, where a demonstration reference application is located. 

The following hardware is required in addition to breadboards and jumper wires:

|Item|Price|
|---|---|
|2x Raspberry Pi|$35 each|
|1x BME280 |$20| 
|1x UART GPS |$60| 
|2x LoRa RFM96 Radio |$20 each| 

The prerequisite skills to understand this totorial are as follows:

1) Working knowledge of Linux; how to navigate in a shell and execute programs
2) An understanding of C++, including class declarations and inheritance
3) An understanding of how XML is structured

Before beginning, please make sure that all the dependencies are installed. Please read the [User Guide](/docs/UsersGuide/FprimeUserGuide.pdf) for help installing these packages.

Here is a description of the components:

# 1 Component Descriptions

## 1.1 BME280
`BME280` implements an active component for the BME280 sensor and must do the following:

### 1.1.1 Commands
`BME280` has no commands.

### 1.1.2 Events
`BME280` should emit events in the error cases that the I2C bus cannot be opened, the device address cannot be set, or if the software is unable to correctly read from a device register.

### 1.1.3 Telemetry Channels
BME280 should have four channels:
1) The recorded pressure (hPa)
2) The recorded temperature (deg C)
3) The humidity (%)
4) The altitude dervived from the recorded pressure (m)

### 1.1.4 Parameters
1) The mean sea level pressure (hPa), used to calcaulate the altitude

## 1.2 GPS
`GPS` implements an active component for the GPS sensor and must do the following:

### 1.2.1 Commands
`GPS` has no commands.

### 1.2.2 Events
`GPS` should emit events in the error cases that the Serial bus cannot be opened, the device cannot be read from, or if the software is unable to successfully parse the GPS data. `GPS` should additionally emit events when a satellite lock is acquired or lost.

### 1.2.3 Telemetry Channels
GPS should have five channels:
1) The current UTC time
2) The recorded Latitude (decimal)
3) The recorded Longitude (decimal)
4) The number of satellites currently locked onto
5) The recorded Altitude (m)

### 1.2.4 Parameters
GPS will have no parameters

## 1.3 LoRaGndIf
`LoRaGndIf` implements a passive ground-interface component for the LoRa radio and must do the following:

### 1.3.1 Downlink
`LoRaGndIf` should be able to receive buffers from both the `downlinkPort` and `fileDownlinkBufferSendIn` ports and transmit those buffers over the air to a ground station.

### 1.3.2 Uplink
`LoRaGndIf` should be able to receive command and file uplink packets, discern between the two, and then pass the buffers to the respective handlers.


# 2 Component Implementation

## 2.1 BME280

Create a folder `BME280` within the `Ref` application directory. Create a file there called `BME280ComponentAi.xml`. The component XML is as follows:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<?xml-model href="../../Autocoders/schema/ISF/component_schema.rng" type="application/xml" schematypens="http://relaxng.org/ns/structure/1.0"?>

<component name="BME280" kind="active" namespace="Ref" modeler="true">

	<import_port_type>Fw/Log/LogPortAi.xml</import_port_type>
	<import_port_type>Svc/Sched/SchedPortAi.xml</import_port_type>
	<import_port_type>Fw/Tlm/TlmPortAi.xml</import_port_type>
	
	<import_dictionary>Ref/BME280/Telemetry.xml</import_dictionary>
	<import_dictionary>Ref/BME280/Events.xml</import_dictionary>

	
	<ports>
		<port name="eventOut" data_type="Fw::Log"  kind="output" role="LogEvent"    max_number="1">
		</port>
		
		<port name="schedIn" data_type="Svc::Sched"  kind="async_input"    max_number="1">
		</port>

		<port name="tlmOut" data_type="Fw::Tlm"  kind="output" role="Telemetry"    max_number="1">
		</port>
	 </ports>
	
	<parameters>
		<parameter id="0" name="mean_sea_level_pressure" data_type="U16" default="1013" set_opcode="10" save_opcode="11">
			<comment>
				Sea level pressure for altitude calibration
			</comment>
		</parameter>
	</parameters>

</component>
```

### 2.1.1 BME280 Port Imports

We import the following ports so that they can be used within our component:

```xml
<import_port_type>Fw/Log/LogPortAi.xml</import_port_type>
<import_port_type>Svc/Sched/SchedPortAi.xml</import_port_type>
<import_port_type>Fw/Tlm/TlmPortAi.xml</import_port_type>
	
<import_dictionary>Ref/BME280/Telemetry.xml</import_dictionary>
<import_dictionary>Ref/BME280/Events.xml</import_dictionary>
```

### 2.1.2 BME280 Dictionaries

In order to not make our XML file too large, we'll outsource our Telemetry and Events to separate files. We'll address those files in a moment - But in the meantime, we'll include them here:

```xml
<import_dictionary>Ref/BME280/Telemetry.xml</import_dictionary>
<import_dictionary>Ref/BME280/Events.xml</import_dictionary>
```

### 2.1.3 BME280 Ports

We'll include three ports:

```xml
<ports>
	<port name="eventOut" data_type="Fw::Log"  kind="output" role="LogEvent"    max_number="1">
	</port>
	
	<port name="schedIn" data_type="Svc::Sched"  kind="async_input"    max_number="1">
	</port>

	<port name="tlmOut" data_type="Fw::Tlm"  kind="output" role="Telemetry"    max_number="1">
	</port>
 </ports>
```

`eventOut` allows us to output log events. Similarily, `tlmOut` allows us to output telemetry events. Finally, the `schedIn` port allows us to connect our port to a rate group. The rate group will execute our barometer code at specific intervals.

### 2.1.4 BME280 Parameters

There's a single parameter -- The mean sea level pressure. The XML code for it is as follows:

```xml
<parameters>
	<parameter id="0" name="mean_sea_level_pressure" data_type="U16" default="1013" set_opcode="10" save_opcode="11">
		<comment>
			Sea level pressure for altitude calibration
		</comment>
	</parameter>
</parameters>
```

Note that as a part of the parameter creation, we specify the data type (unsigned 16-bit integer) as well as a default value (1013). Later on, we can opt to change this value through a dashboard command.

### 2.1.5 BME280 Telemetry

Next, create a file called `Telemetry.xml` within the `BME280` directory. The contents of that file are as follows:

```xml
<telemetry>
	<channel id="0" name="BME_Pressure" data_type="F32">
		<comment>
		Pressure Reading (hPa)
		</comment>
	</channel>
	<channel id="1" name="BME_Temperature" data_type="F32">
		<comment>
		Temperature Reading (deg C)
		</comment>
	</channel>
	<channel id="2" name="BME_Humidity" data_type="F32">
		<comment>
		Humidity (%)
		</comment>
	</channel>
	<channel id="3" name="BME_Altitude" data_type="F32">
		<comment>
		Calculated Altitude (m)
		</comment>
	</channel>
</telemetry>
```

Each telemetry channel that we want to create requires its own unique ID, a name, and a data-type. The four channels we've created are all 32-bit floating point types.


### 2.1.6 BME280 Events

For the events, create a file called `Events.xml` within the `BME280` directory. The contents of that file are as follows:


```xml
<events>
	<event id="0" name="BME280_OpenError" severity="WARNING_HI" format_string = "Unable to open I2C device: %d" >
		<comment>
		I2C Open Error
		</comment>
		<args>
			<arg name="device" type="I16">
				<comment>I2C Device</comment>
			</arg>          
		</args>
	</event>
	<event id="1" name="BME280_AddressError" severity="WARNING_HI" format_string = "Unable to set I2C address: %d" >
		<comment>
		I2C Set Address Error
		</comment>
		<args>
			<arg name="Address" type="U8">
				<comment>I2C Address</comment>
			</arg>          
		</args>
	</event>
	<event id="2" name="BME280_I2C_ReadError" severity="WARNING_HI" format_string = "Unable to Read from I2C Register: %d" >
		<comment>
		BME280 I2C Read Error
		</comment>
		<args>
			<arg name="Address" type="U8">
				<comment>I2C Register Address</comment>
			</arg>          
		</args>
	</event>
</events>
```

Similar to the Telemetry channels, each event requires an ID and name. Each event also requires a severity. Options include: `DIAGNOSTIC`, `ACTIVITY_LO`, `ACTIVITY_HI`, `WARNING_LO`, `WARNING_HI`, and `FATAL`. Since any of these errors prevent the sensor from functioning correctly but don't fatally damage our payload, we'll categorize these events as `WARNING_HI`. 

Events also include options format_string and args. This allows customizable messages to be displayed on the GSE or GDS, including relevant data, when an event is triggered.

### 2.1.7 Creating mod.mk and Makefiles 

In order to generate files for this component, we'll need to add `mod.mk` and `Makefile` files to the `BME280` directory. First, create a file called `mod.mk`. Its contents are:

```make
SRC = BME280ComponentAi.xml
```

We'll make edits to this file in a bit.

Next, create a `Makefile`:

```make
# derive module name from directory

MODULE_DIR = Ref/BME280
MODULE = $(subst /,,$(MODULE_DIR))

BUILD_ROOT ?= $(subst /$(MODULE_DIR),,$(CURDIR))
export BUILD_ROOT

include $(BUILD_ROOT)/mk/makefiles/module_targets.mk
```

For each component, the `Makefile` is typically identical excluding `MODULE_DIR` pointing to the directory it's currently located in.

Finally, the build system needs to be made aware of the port XML. To do this, edit the file `/mk/configs/modules/modules.mk`. 

Find the `REF_MODULES` variable and add the new directory:

```make
REF_MODULES := \
	Ref/Top \
	Ref/RecvBuffApp \
	Ref/SendBuffApp \
	Ref/SignalGen \
	Ref/PingReceiver \
	Ref/BME280
```

### 2.1.8 Generating Code

From the `BME280` directory, we can begin generating code for this component. Run the following command:


```shell
make gen_make
Generating templates
...
Regenerating global Makefile
Generating Makefiles in .../fprime/mk/makefiles
Makefile generation complete.
Build Time: 0:00.51
```

Now we can generate autocoded files:

```shell
make
Building module RefMathPorts code generation (level 4)
...
make[1]: Leaving directory '.../fprime/Ref/BME280'
Build Time: 0:03.49
```

The code generation from the XML produces two files:

```
BME280ComponentAc.cpp
BME280ComponentAc.hpp
```
These contain the C++ classes that implement the port functionality. The build system will automatically compile them when it is aware of the port XML file. Typically the developer will never need to edit these autocoded files.

### 2.1.9 Stub Generation

We'll now generate the Component Implementation files and add functionality to them. From within the `BME280` directory, generate the stub files using `make impl`.

This will create the following two files:

```
BME280ComponentImpl.cpp-template
BME280ComponentImpl.hpp-template
```

Rename them and remove the `-template` suffixes:

```
BME280ComponentImpl.cpp
BME280ComponentImpl.hpp
```

Since the code we implement for the Raspberry Pi won't run on standard Linux or MacOS computers, we'll also create a simple "stub" file outputting fake data so that the program can be built and run on all platforms. Create a copy of the `.cpp`. In the end, you should have the following files in your `BME280` directory:

```
BME280ComponentImplStub.cpp
BME280ComponentImpl.cpp
BME280ComponentImpl.hpp
```

We can now append the `mod.mk` file to include these new files:


```make
SRC = BME280ComponentAi.xml BME280ComponentImplStub.cpp

SRC_RASPIAN = BME280ComponentImpl.cpp

HDR = BME280ComponentImpl.hpp
```

Since we're using a separate `.cpp` file for the `RASPIAN` make target, we'll specify it by adding a specific `SRC_RASPIAN` flag. 


Make the build system aware of the new files and build:

```
make rebuild
```

The stub files should sucessfully compile.


### 2.1.10 Component Implementation

We'll start with the Stub as it's fairly quick. Open `BME280ComponentImplStub.cpp` and find the following function:

```c++
 void BME280ComponentImpl ::
		schedIn_handler(
				const NATIVE_INT_TYPE portNum,
				NATIVE_UINT_TYPE context
		)
	{
		// TODO
	}
```

This function represents the `schedIn` port handler which will be connected to a rate group. Within here, we can simply write out dummy data to the telemetry channels:


```c++
 void BME280ComponentImpl ::
		schedIn_handler(
				const NATIVE_INT_TYPE portNum,
				NATIVE_UINT_TYPE context
		)
	{
		tlmWrite_BME_Pressure(964.57);
		tlmWrite_BME_Humidity(49.62);
		tlmWrite_BME_Temperature(25.39);
		tlmWrite_BME_Altitude(411.64);
		
		return;
	}
```

If you want, you could optionally enhance this with random number generators or some other feature!

Next, we'll begin working on the `BME280ComponentImpl.cpp` file.

The first thing we'll need to do is add includes for the Linux I2C drivers. Add the following code:


```c++
extern "C" {
	#include <linux/i2c-dev.h>
	#include <sys/ioctl.h>
}
```

Next, we'll want to add code to read and write from the various I2C registers on the device. The code is as follows:

```c++
void BME280ComponentImpl ::
	writeI2CValue(
			U8 reg, 
			U8 data
		)
	{
		if(data == 0){
			i2c_smbus_access(this->m_fd, I2C_SMBUS_WRITE, reg, I2C_SMBUS_BYTE, NULL);
		}
		else{
			union i2c_smbus_data i2cdata;
			i2cdata.byte = data;
			i2c_smbus_access(this->m_fd, I2C_SMBUS_WRITE, reg, I2C_SMBUS_BYTE_DATA, &i2cdata);
		}
		
	}
	
U8 BME280ComponentImpl ::
		readI2CValue(void)
	{
		union i2c_smbus_data data;
		
		if(i2c_smbus_access(this->m_fd, I2C_SMBUS_READ, 0, I2C_SMBUS_BYTE, &data)){
			this->log_WARNING_HI_BME280_I2C_ReadError(0);
			return 0;
		}
		
		return data.byte & 0xFF;
		
	}
	
U16 BME280ComponentImpl ::
		readI2CValue16(
			U8 reg
		)
	{
		union i2c_smbus_data data;
		
		if(i2c_smbus_access(this->m_fd, I2C_SMBUS_READ, reg, I2C_SMBUS_WORD_DATA, &data)){
			this->log_WARNING_HI_BME280_I2C_ReadError(reg);
			return 0;
		}
		
		return data.word & 0xFFFF;
	}
	
U8 BME280ComponentImpl ::
		readI2CValue8(
			U8 reg
		)
	{
		union i2c_smbus_data data;
		
		if(i2c_smbus_access(this->m_fd, I2C_SMBUS_READ, reg, I2C_SMBUS_BYTE_DATA, &data)){
			this->log_WARNING_HI_BME280_I2C_ReadError(reg);
			return 0;
		}
		
		return data.byte & 0xFF;
	}
```

By reading the BME280 datasheet, you can derive the following functions used to calibrate and compensate for the various data values:

```c++
 void BME280ComponentImpl ::
			read_cal(void)
		{
			this->cal.dig_T1 = readI2CValue16(BME280_REGISTER_DIG_T1);
			this->cal.dig_T2 = readI2CValue16(BME280_REGISTER_DIG_T2);
			this->cal.dig_T3 = readI2CValue16(BME280_REGISTER_DIG_T3);
			
			this->cal.dig_P1 = readI2CValue16(BME280_REGISTER_DIG_P1);
			this->cal.dig_P2 = readI2CValue16(BME280_REGISTER_DIG_P2);
			this->cal.dig_P3 = readI2CValue16(BME280_REGISTER_DIG_P3);
			this->cal.dig_P4 = readI2CValue16(BME280_REGISTER_DIG_P4);
			this->cal.dig_P5 = readI2CValue16(BME280_REGISTER_DIG_P5);
			this->cal.dig_P6 = readI2CValue16(BME280_REGISTER_DIG_P6);
			this->cal.dig_P7 = readI2CValue16(BME280_REGISTER_DIG_P7);
			this->cal.dig_P8 = readI2CValue16(BME280_REGISTER_DIG_P8);
			
			this->cal.dig_H1 = readI2CValue8(BME280_REGISTER_DIG_H1);
			this->cal.dig_H2 = readI2CValue16(BME280_REGISTER_DIG_H2);
			this->cal.dig_H3 = readI2CValue8(BME280_REGISTER_DIG_H3);
			this->cal.dig_H4 = (readI2CValue8(BME280_REGISTER_DIG_H4) << 4) | (readI2CValue8(BME280_REGISTER_DIG_H4+1) & 0xF);
			this->cal.dig_H5 = (readI2CValue8(BME280_REGISTER_DIG_H5+1) << 4) | (readI2CValue8(BME280_REGISTER_DIG_H5) >> 4);
			this->cal.dig_H6 = readI2CValue8(BME280_REGISTER_DIG_H6);
		}
		
	I32 BME280ComponentImpl ::
			getTemperatureCalibration(
				I32 temp
			)
		{
			I32 var1  = ((((temp>>3) - ((I32)this->cal.dig_T1 <<1))) *
					((I32)this->cal.dig_T2)) >> 11;

			I32 var2  = (((((temp>>4) - ((I32)this->cal.dig_T1)) *
						((temp>>4) - ((I32)this->cal.dig_T1))) >> 12) *
					((I32)this->cal.dig_T3)) >> 14;

			return var1 + var2;
		}
		
	F32 BME280ComponentImpl ::
			compensatePressure(
				I32 adc_P,
				I32 t_fine
			)
		{
			I64 var1, var2, p;

			var1 = ((I64)t_fine) - 128000;
			var2 = var1 * var1 * (I64)this->cal.dig_P6;
			var2 = var2 + ((var1*(I64)this->cal.dig_P5)<<17);
			var2 = var2 + (((I64)this->cal.dig_P4)<<35);
			var1 = ((var1 * var1 * (I64)this->cal.dig_P3)>>8) +
				((var1 * (I64)this->cal.dig_P2)<<12);
			var1 = (((((I64)1)<<47)+var1))*((I64)this->cal.dig_P1)>>33;

			if (var1 == 0) {
				return 0;  // avoid exception caused by division by zero
			}
			p = 1048576 - adc_P;
			p = (((p<<31) - var2)*3125) / var1;
			var1 = (((I64)this->cal.dig_P9) * (p>>13) * (p>>13)) >> 25;
			var2 = (((I64)this->cal.dig_P8) * p) >> 19;

			p = ((p + var1 + var2) >> 8) + (((I64)this->cal.dig_P7)<<4);
			return (F32) p/256;
		}
		
	F32 BME280ComponentImpl ::
			compensateHumidity(
				I32 adc_H,
				I32 t_fine
			)
		{
			I32 v_x1_u32r;

			v_x1_u32r = (t_fine - ((I32)76800));

			v_x1_u32r = (((((adc_H << 14) - (((I32)this->cal.dig_H4) << 20) -
					(((I32)this->cal.dig_H5) * v_x1_u32r)) + ((I32)16384)) >> 15) *
							(((((((v_x1_u32r * ((I32)this->cal.dig_H6)) >> 10) *
						(((v_x1_u32r * ((I32)this->cal.dig_H3)) >> 11) + ((I32)32768))) >> 10) +
					((I32)2097152)) * ((I32)this->cal.dig_H2) + 8192) >> 14));

			v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
								((I32)this->cal.dig_H1)) >> 4));

			v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
			v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
			F32 h = (v_x1_u32r>>12);
			
			return  h / 1024.0;
		}
		
	F32 BME280ComponentImpl ::
			compensateTemperature(
				I32 t_fine
			)
		{
			F32 T  = (t_fine * 5 + 128) >> 8;
			return T/100;
		}
```

We can also add a function to utilize the `mean_sea_level` parameter to calculate altitude:


```c++
F32 BME280ComponentImpl ::
		getAltitude(
			F32 pressure
		)
	{
		Fw::ParamValid valid;
		return 44330.0 * (1.0 - pow(pressure / paramGet_mean_sea_level_pressure(valid), 0.190294957));
	}
```


Next, we can add a setup function to initialize and setup the I2C interface:

```c++
void BME280ComponentImpl ::
		setup(void)
	{
		//Create I2C fd. todo for later - Allow customizable I2C bus
		char filename[20];
		sprintf(filename, "/dev/i2c-1");
		
		this->m_fd = open(filename, O_RDWR);
		
		if (this->m_fd < 0) {
			this->log_WARNING_HI_BME280_OpenError(1);
		}
		
		if (ioctl(this->m_fd, I2C_SLAVE, 0x77) < 0) {
			this->log_WARNING_HI_BME280_AddressError(0x77);
		}
		
		read_cal();
		
		// Setup sampling
		writeI2CValue(0xf2, 0x01);
		writeI2CValue(0xf4, 0x25);
		
		this->m_setup = true;
	}
```

Note how warnings are logged in the case of I2C device failures.

Finally, with all the groundwork laid, we can implement the `schedIn` handler like we did with the stub:

```c++
 void BME280ComponentImpl ::
		schedIn_handler(
				const NATIVE_INT_TYPE portNum,
				NATIVE_UINT_TYPE context
		)
	{

		if(!this->m_setup){
			 setup();
		}
		
		if(!this->m_setup){
			return;
		}
		
		// Setup sampling
		writeI2CValue(0xf2, 0x01);
		writeI2CValue(0xf4, 0x25);
		
		// Read data
		writeI2CValue(0xF7, 0);
		
		BME280_raw raw_data;
		
		raw_data.pmsb = readI2CValue();
		raw_data.plsb = readI2CValue();
		raw_data.pxsb = readI2CValue();
		
		raw_data.tmsb = readI2CValue();
		raw_data.tlsb = readI2CValue();
		raw_data.txsb = readI2CValue();
		
		raw_data.hmsb = readI2CValue();
		raw_data.hlsb = readI2CValue();
		
		raw_data.temperature = 0;
		raw_data.temperature = (raw_data.temperature | raw_data.tmsb) << 8;
		raw_data.temperature = (raw_data.temperature | raw_data.tlsb) << 8;
		raw_data.temperature = (raw_data.temperature | raw_data.txsb) >> 4;
		
		raw_data.pressure = 0;
		raw_data.pressure = (raw_data.pressure | raw_data.pmsb) << 8;
		raw_data.pressure = (raw_data.pressure | raw_data.plsb) << 8;
		raw_data.pressure = (raw_data.pressure | raw_data.pxsb) >> 4;
		
		raw_data.humidity = 0;
		raw_data.humidity = (raw_data.humidity | raw_data.hmsb) << 8;
		raw_data.humidity = (raw_data.humidity | raw_data.hlsb);
		
		I32 t_fine = getTemperatureCalibration(raw_data.temperature);
		F32 pressure = compensatePressure(raw_data.pressure, t_fine) / 100;
		F32 humidity = compensateHumidity(raw_data.humidity, t_fine);
		F32 temperature = compensateTemperature(t_fine);
		F32 altitude = getAltitude(pressure);
		
		tlmWrite_BME_Pressure(pressure);
		tlmWrite_BME_Humidity(humidity);
		tlmWrite_BME_Temperature(temperature);
		tlmWrite_BME_Altitude(altitude);
	}
```

The function checks to make sure the device is setup, and if not, returns without sending anything. For more details on how the sensor data is put together, check out the BME280 datasheet.


Overall, the file should look like this:

```c++
#include <Ref/BME280/BME280ComponentImpl.hpp>
#include "Fw/Types/BasicTypes.hpp"

#include <math.h>
#include <fcntl.h>
#include <unistd.h>

namespace Ref {
	
	extern "C" {
		#include <linux/i2c-dev.h>
		#include <sys/ioctl.h>
	}

	// ----------------------------------------------------------------------
	// Construction, initialization, and destruction 
	// ----------------------------------------------------------------------

	BME280ComponentImpl ::
#if FW_OBJECT_NAMES == 1
		BME280ComponentImpl(
				const char *const compName
		) :
			BME280ComponentBase(compName)
#else
		BME280Impl(void)
#endif
	{

	}

	void BME280ComponentImpl ::
		init(
				const NATIVE_INT_TYPE queueDepth,
				const NATIVE_INT_TYPE instance
		) 
	{
		BME280ComponentBase::init(queueDepth, instance);
	}

	BME280ComponentImpl ::
		~BME280ComponentImpl(void)
	{

	}
	
	void BME280ComponentImpl ::
		writeI2CValue(
				U8 reg, 
				U8 data
			)
		{
			if(data == 0){
				i2c_smbus_access(this->m_fd, I2C_SMBUS_WRITE, reg, I2C_SMBUS_BYTE, NULL);
			}
			else{
				union i2c_smbus_data i2cdata;
				i2cdata.byte = data;
				i2c_smbus_access(this->m_fd, I2C_SMBUS_WRITE, reg, I2C_SMBUS_BYTE_DATA, &i2cdata);
			}
			
		}
		
	U8 BME280ComponentImpl ::
			readI2CValue(void)
		{
			union i2c_smbus_data data;
			
			if(i2c_smbus_access(this->m_fd, I2C_SMBUS_READ, 0, I2C_SMBUS_BYTE, &data)){
				this->log_WARNING_HI_BME280_I2C_ReadError(0);
				return 0;
			}
			
			return data.byte & 0xFF;
			
		}
		
	U16 BME280ComponentImpl ::
			readI2CValue16(
				U8 reg
			)
		{
			union i2c_smbus_data data;
			
			if(i2c_smbus_access(this->m_fd, I2C_SMBUS_READ, reg, I2C_SMBUS_WORD_DATA, &data)){
				this->log_WARNING_HI_BME280_I2C_ReadError(reg);
				return 0;
			}
			
			return data.word & 0xFFFF;
		}
		
	U8 BME280ComponentImpl ::
			readI2CValue8(
				U8 reg
			)
		{
			union i2c_smbus_data data;
			
			if(i2c_smbus_access(this->m_fd, I2C_SMBUS_READ, reg, I2C_SMBUS_BYTE_DATA, &data)){
				this->log_WARNING_HI_BME280_I2C_ReadError(reg);
				return 0;
			}
			
			return data.byte & 0xFF;
		}
		
	void BME280ComponentImpl ::
			read_cal(void)
		{
			this->cal.dig_T1 = readI2CValue16(BME280_REGISTER_DIG_T1);
			this->cal.dig_T2 = readI2CValue16(BME280_REGISTER_DIG_T2);
			this->cal.dig_T3 = readI2CValue16(BME280_REGISTER_DIG_T3);
			
			this->cal.dig_P1 = readI2CValue16(BME280_REGISTER_DIG_P1);
			this->cal.dig_P2 = readI2CValue16(BME280_REGISTER_DIG_P2);
			this->cal.dig_P3 = readI2CValue16(BME280_REGISTER_DIG_P3);
			this->cal.dig_P4 = readI2CValue16(BME280_REGISTER_DIG_P4);
			this->cal.dig_P5 = readI2CValue16(BME280_REGISTER_DIG_P5);
			this->cal.dig_P6 = readI2CValue16(BME280_REGISTER_DIG_P6);
			this->cal.dig_P7 = readI2CValue16(BME280_REGISTER_DIG_P7);
			this->cal.dig_P8 = readI2CValue16(BME280_REGISTER_DIG_P8);
			
			this->cal.dig_H1 = readI2CValue8(BME280_REGISTER_DIG_H1);
			this->cal.dig_H2 = readI2CValue16(BME280_REGISTER_DIG_H2);
			this->cal.dig_H3 = readI2CValue8(BME280_REGISTER_DIG_H3);
			this->cal.dig_H4 = (readI2CValue8(BME280_REGISTER_DIG_H4) << 4) | (readI2CValue8(BME280_REGISTER_DIG_H4+1) & 0xF);
			this->cal.dig_H5 = (readI2CValue8(BME280_REGISTER_DIG_H5+1) << 4) | (readI2CValue8(BME280_REGISTER_DIG_H5) >> 4);
			this->cal.dig_H6 = readI2CValue8(BME280_REGISTER_DIG_H6);
		}
		
	I32 BME280ComponentImpl ::
			getTemperatureCalibration(
				I32 temp
			)
		{
			I32 var1  = ((((temp>>3) - ((I32)this->cal.dig_T1 <<1))) *
					((I32)this->cal.dig_T2)) >> 11;

			I32 var2  = (((((temp>>4) - ((I32)this->cal.dig_T1)) *
						((temp>>4) - ((I32)this->cal.dig_T1))) >> 12) *
					((I32)this->cal.dig_T3)) >> 14;

			return var1 + var2;
		}
		
	F32 BME280ComponentImpl ::
			compensatePressure(
				I32 adc_P,
				I32 t_fine
			)
		{
			I64 var1, var2, p;

			var1 = ((I64)t_fine) - 128000;
			var2 = var1 * var1 * (I64)this->cal.dig_P6;
			var2 = var2 + ((var1*(I64)this->cal.dig_P5)<<17);
			var2 = var2 + (((I64)this->cal.dig_P4)<<35);
			var1 = ((var1 * var1 * (I64)this->cal.dig_P3)>>8) +
				((var1 * (I64)this->cal.dig_P2)<<12);
			var1 = (((((I64)1)<<47)+var1))*((I64)this->cal.dig_P1)>>33;

			if (var1 == 0) {
				return 0;  // avoid exception caused by division by zero
			}
			p = 1048576 - adc_P;
			p = (((p<<31) - var2)*3125) / var1;
			var1 = (((I64)this->cal.dig_P9) * (p>>13) * (p>>13)) >> 25;
			var2 = (((I64)this->cal.dig_P8) * p) >> 19;

			p = ((p + var1 + var2) >> 8) + (((I64)this->cal.dig_P7)<<4);
			return (F32) p/256;
		}
		
	F32 BME280ComponentImpl ::
			compensateHumidity(
				I32 adc_H,
				I32 t_fine
			)
		{
			I32 v_x1_u32r;

			v_x1_u32r = (t_fine - ((I32)76800));

			v_x1_u32r = (((((adc_H << 14) - (((I32)this->cal.dig_H4) << 20) -
					(((I32)this->cal.dig_H5) * v_x1_u32r)) + ((I32)16384)) >> 15) *
							(((((((v_x1_u32r * ((I32)this->cal.dig_H6)) >> 10) *
						(((v_x1_u32r * ((I32)this->cal.dig_H3)) >> 11) + ((I32)32768))) >> 10) +
					((I32)2097152)) * ((I32)this->cal.dig_H2) + 8192) >> 14));

			v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
								((I32)this->cal.dig_H1)) >> 4));

			v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
			v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
			F32 h = (v_x1_u32r>>12);
			
			return  h / 1024.0;
		}
		
	F32 BME280ComponentImpl ::
			compensateTemperature(
				I32 t_fine
			)
		{
			F32 T  = (t_fine * 5 + 128) >> 8;
			return T/100;
		}
		
	F32 BME280ComponentImpl ::
			getAltitude(
				F32 pressure
			)
		{
			Fw::ParamValid valid;
			return 44330.0 * (1.0 - pow(pressure / paramGet_mean_sea_level_pressure(valid), 0.190294957));
		}
	
	void BME280ComponentImpl ::
			setup(void)
		{
			//Create I2C fd. todo for later - Allow customizable I2C bus
			char filename[20];
			sprintf(filename, "/dev/i2c-1");
			
			this->m_fd = open(filename, O_RDWR);
			
			if (this->m_fd < 0) {
				this->log_WARNING_HI_BME280_OpenError(1);
			}
			
			if (ioctl(this->m_fd, I2C_SLAVE, 0x77) < 0) {
				this->log_WARNING_HI_BME280_AddressError(0x77);
			}
			
			read_cal();
			
			// Setup sampling
			writeI2CValue(0xf2, 0x01);
			writeI2CValue(0xf4, 0x25);
			
			this->m_setup = true;
		}

	// ----------------------------------------------------------------------
	// Handler implementations for user-defined typed input ports
	// ----------------------------------------------------------------------

	void BME280ComponentImpl ::
		schedIn_handler(
				const NATIVE_INT_TYPE portNum,
				NATIVE_UINT_TYPE context
		)
	{

		if(!this->m_setup){
			 setup();
		}
		
		if(!this->m_setup){
			return;
		}
		
		// Setup sampling
		writeI2CValue(0xf2, 0x01);
		writeI2CValue(0xf4, 0x25);
		
		// Read data
		writeI2CValue(0xF7, 0);
		
		BME280_raw raw_data;
		
		raw_data.pmsb = readI2CValue();
		raw_data.plsb = readI2CValue();
		raw_data.pxsb = readI2CValue();
		
		raw_data.tmsb = readI2CValue();
		raw_data.tlsb = readI2CValue();
		raw_data.txsb = readI2CValue();
		
		raw_data.hmsb = readI2CValue();
		raw_data.hlsb = readI2CValue();
		
		raw_data.temperature = 0;
		raw_data.temperature = (raw_data.temperature | raw_data.tmsb) << 8;
		raw_data.temperature = (raw_data.temperature | raw_data.tlsb) << 8;
		raw_data.temperature = (raw_data.temperature | raw_data.txsb) >> 4;
		
		raw_data.pressure = 0;
		raw_data.pressure = (raw_data.pressure | raw_data.pmsb) << 8;
		raw_data.pressure = (raw_data.pressure | raw_data.plsb) << 8;
		raw_data.pressure = (raw_data.pressure | raw_data.pxsb) >> 4;
		
		raw_data.humidity = 0;
		raw_data.humidity = (raw_data.humidity | raw_data.hmsb) << 8;
		raw_data.humidity = (raw_data.humidity | raw_data.hlsb);
		
		I32 t_fine = getTemperatureCalibration(raw_data.temperature);
		F32 pressure = compensatePressure(raw_data.pressure, t_fine) / 100;
		F32 humidity = compensateHumidity(raw_data.humidity, t_fine);
		F32 temperature = compensateTemperature(t_fine);
		F32 altitude = getAltitude(pressure);
		
		tlmWrite_BME_Pressure(pressure);
		tlmWrite_BME_Humidity(humidity);
		tlmWrite_BME_Temperature(temperature);
		tlmWrite_BME_Altitude(altitude);
	}

} // end namespace Ref
```

In addition to modifying the `cpp` files, we'll edit the `hpp` file to include register constants, function variables such as calibration and the file descriptor, and the function declarations. It should look like the following:

```c++
#ifndef BME280_HPP
#define BME280_HPP

#include "Ref/BME280/BME280ComponentAc.hpp"

#define BME280_REGISTER_DIG_T1        0x88
#define BME280_REGISTER_DIG_T2        0x8A
#define BME280_REGISTER_DIG_T3        0x8C
#define BME280_REGISTER_DIG_P1        0x8E
#define BME280_REGISTER_DIG_P2        0x90
#define BME280_REGISTER_DIG_P3        0x92
#define BME280_REGISTER_DIG_P4        0x94
#define BME280_REGISTER_DIG_P5        0x96
#define BME280_REGISTER_DIG_P6        0x98
#define BME280_REGISTER_DIG_P7        0x9A
#define BME280_REGISTER_DIG_P8        0x9C
#define BME280_REGISTER_DIG_P9        0x9E
#define BME280_REGISTER_DIG_H1        0xA1
#define BME280_REGISTER_DIG_H2        0xE1
#define BME280_REGISTER_DIG_H3        0xE3
#define BME280_REGISTER_DIG_H4        0xE4
#define BME280_REGISTER_DIG_H5        0xE5
#define BME280_REGISTER_DIG_H6        0xE7
#define BME280_REGISTER_CHIPID        0xD0
#define BME280_REGISTER_VERSION       0xD1
#define BME280_REGISTER_SOFTRESET     0xE0
#define BME280_RESET                  0xB6
#define BME280_REGISTER_CAL26         0xE1
#define BME280_REGISTER_CONTROLHUMID  0xF2
#define BME280_REGISTER_CONTROL       0xF4
#define BME280_REGISTER_CONFIG        0xF5
#define BME280_REGISTER_PRESSUREDATA  0xF7
#define BME280_REGISTER_TEMPDATA      0xFA
#define BME280_REGISTER_HUMIDDATA     0xFD

namespace Ref {

	class BME280ComponentImpl :
		public BME280ComponentBase
	{
		
		struct BME280_raw
		{
			U8 pmsb;
			U8 plsb;
			U8 pxsb;

			U8 tmsb;
			U8 tlsb;
			U8 txsb;

			U8 hmsb;
			U8 hlsb;

			U32 temperature;
			U32 pressure;
			U32 humidity;  

		};
		
		struct BME280_cali
		{
			U16 dig_T1;
			I16 dig_T2;
			I16 dig_T3;

			U16 dig_P1;
			I16 dig_P2;
			I16 dig_P3;
			I16 dig_P4;
			I16 dig_P5;
			I16 dig_P6;
			I16 dig_P7;
			I16 dig_P8;
			I16 dig_P9;

			U8 dig_H1;
			I16 dig_H2;
			U8 dig_H3;
			I16 dig_H4;
			I16 dig_H5;
			I8 dig_H6;
		};

		public:

			// ----------------------------------------------------------------------
			// Construction, initialization, and destruction
			// ----------------------------------------------------------------------

			//! Construct object BME280
			//!
			BME280ComponentImpl(
#if FW_OBJECT_NAMES == 1
					const char *const compName /*!< The component name*/
#else
					void
#endif
			);

			//! Initialize object BME280
			//!
			void init(
					const NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
					const NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
			);

			//! Destroy object BME280
			//!
			~BME280ComponentImpl(void);

		PRIVATE:
			
			void writeI2CValue(
					U8 reg, 
					U8 data
				);
			
			U8 readI2CValue(void);
			
			U16 readI2CValue16(
				U8 reg
			);
			
			U8 readI2CValue8(
				U8 reg
			);
			
			void read_cal(void);
			
			I32 getTemperatureCalibration(
				I32 temp
			);
			
			F32 compensatePressure(
				I32 adc_P,
				I32 t_fine
			);
			
			F32 compensateHumidity(
				I32 adc_H,
				I32 t_fine
			);
			
			F32 compensateTemperature(
				I32 t_fine
			);
			
			F32 getAltitude(
				F32 pressure
			);
			
			void setup(void);

			// ----------------------------------------------------------------------
			// Handler implementations for user-defined typed input ports
			// ----------------------------------------------------------------------

			//! Handler implementation for schedIn
			//!
			void schedIn_handler(
					const NATIVE_INT_TYPE portNum, /*!< The port number*/
					NATIVE_UINT_TYPE context /*!< The call order*/
			);
			
			I16 m_fd;
			bool m_setup;
			BME280_cali cal;

		};

} // end namespace Ref

#endif
```

Once complete, typing `make rebuild` should successfully build the component. 


## 2.2 GPS

### 2.2.1 XML

The GPS component structure follows quite similarily to that of the BME280 sensor. Create a directory `GPS` within `Ref` called `GPSComponentAi.xml` and add the following XML:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<?xml-model href="../../Autocoders/schema/ISF/component_schema.rng" type="application/xml" schematypens="http://relaxng.org/ns/structure/1.0"?>

<component name="GPS" kind="active" namespace="Ref" modeler="true">


	<import_port_type>Fw/Log/LogPortAi.xml</import_port_type>
	<import_port_type>Svc/Sched/SchedPortAi.xml</import_port_type>
	<import_port_type>Fw/Tlm/TlmPortAi.xml</import_port_type>
	
	<import_dictionary>Ref/GPS/Telemetry.xml</import_dictionary>
	<import_dictionary>Ref/GPS/Events.xml</import_dictionary>

	
	<ports>

		<port name="eventOut" data_type="Fw::Log"  kind="output" role="LogEvent"    max_number="1">
		</port>

		<port name="schedIn" data_type="Svc::Sched"  kind="async_input"    max_number="1">
		</port>

		<port name="tlmOut" data_type="Fw::Tlm"  kind="output" role="Telemetry"    max_number="1">
		</port>
		
	 </ports>
</component>
```

The Telemetry XML is as follows:

```xml
<telemetry>
	<channel id="0" name="GPS_Time" data_type="U32">
		<comment>
		UTC Time
		</comment>
	</channel>
	<channel id="1" name="GPS_Latitude" data_type="F32">
		<comment>
		GPS Latitude Reading
		</comment>
	</channel>
	<channel id="2" name="GPS_Longitude" data_type="F32">
		<comment>
		GPS Longitude Reading
		</comment>
	</channel>
	<channel id="3" name="GPS_Satellites" data_type="U8">
		<comment>
		Number of Satellites Acquired
		</comment>
	</channel>
	<channel id="4" name="GPS_Altitude" data_type="F32">
		<comment>
		GPS Altitude
		</comment>
	</channel>
</telemetry>
```

And for the Events:

```xml
<events>
	<event id="0" name="GPS_OpenError" severity="WARNING_HI" format_string = "Unable to open GPS device" >
		<comment>
		GPS Serial Open Error
		</comment>
	</event>
	<event id="1" name="GPS_ReadError" severity="WARNING_HI" format_string = "Unable to read GPS device" >
		<comment>
		GPS Serial Read Error
		</comment>
	</event>
	<event id="2" name="GPS_ParseError" severity="WARNING_LO" format_string = "Unable to Parse GPS data" >
		<comment>
		GPS Parse Issues
		</comment>
	</event>
	<event id="3" name="GPS_LockAcquired" severity="ACTIVITY_HI" format_string = "GPS Lock Acquired" >
		<comment>
		GPS Lock Acquired
		</comment>
	</event>
	<event id="4" name="GPS_LockLost" severity="WARNING_HI" format_string = "GPS Lock Lost" >
		<comment>
		GPS Lock Lost
		</comment>
	</event>
</events>
```

Note that some of the events use different severity, especially for less critical ones such as acquring a GPS lock or being unable to successfully parse the data.

### 2.2.2 Stub Implementation

Similarily to the BME280 component, add a `mod.mk` and `Makefile`, and generate the autocoded port definitions. Next, run `make impl` to generate the component implementation files. Create a Stub copy of the `cpp` file. We should now see the following files within our `GPS` directory:

```
GPSComponentImpl.cpp
GPSComponentImplStub.cpp
GPSComponentImpl.hpp
```

The Stub implementation is relatively simple, we'll send dummy data through:

```c++
void GPSComponentImpl ::
	schedIn_handler(
			const NATIVE_INT_TYPE portNum,
			NATIVE_UINT_TYPE context
	)
{
	
	tlmWrite_GPS_Time(123456);
	tlmWrite_GPS_Latitude(34.127967);
	tlmWrite_GPS_Longitude(-118.128176);
	tlmWrite_GPS_Altitude(433);
	tlmWrite_GPS_Satellites(24);
	return;
}
```

### 2.2.3 Component Implementation

Now, let's edit the `GPSComponentImpl.cpp` file.

GPS receivers report position data in a degrees format. We'll first add a helper function to convert it to decimal:


```c++
float GPSComponentImpl ::
	gps_deg_to_dec(
			float deg 
		)
{
	double dub_deg;
	float second = modf(deg, &dub_deg)*60;
	int degree = (int)(dub_deg/100);
	int minute = (int)(deg-(degree*100));

	float absDeg = round(degree * 1000000.);
	float absMin = round(minute * 1000000.);
	float absSec = round(second * 1000000.);

	return round(absDeg + (absMin/60) + (absSec/3600)) /1000000;
}
```

We'll next add a setup function to initialize the GPS over the serial bus:

```c++
void GPSComponentImpl ::
	setup(void)
{
	if(this->m_setup){
		return;
	}
	
	this->m_fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
	
	if(this->m_fd == -1){
		this->log_WARNING_HI_GPS_OpenError();
		this->m_setup = false;
		return;
	}
	
	struct termios options;
	tcgetattr(this->m_fd, &options);
	options.c_cflag = B57600 | CS8 | CLOCAL | CREAD;
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(this->m_fd, TCIFLUSH);
	tcsetattr(this->m_fd, TCSANOW, &options);
	
	this->m_setup = true;
	this->m_lock = false;
}
```

Finally, we can complete our `schedIn` handler:


```c++
void GPSComponentImpl ::
	schedIn_handler(
			const NATIVE_INT_TYPE portNum,
			NATIVE_UINT_TYPE context
	)
{
	if(!this->m_setup){
		setup();
	}
	
	if(!this->m_setup){
		return;
	}
	
	ioctl(this->m_fd, TCFLSH, 0);
	Os::Task::delay(1500);
	
	char buffer[1024];
	char *ptr = buffer;
	
	ssize_t rd = read(this->m_fd, buffer, sizeof(buffer));
	
	if(rd <= 0){
		this->log_WARNING_HI_GPS_ReadError();
		return;
	}
	
	U8 status;
	GPS_Data data;
	for(U16 i = 0; i < sizeof(buffer) - 6; i++){
		status = sscanf(ptr, "$GPGGA,%f,%f,%c,%f,%c,%u,%u,%f,%f",
							&data.utcTime, &data.latitude, &data.northSouth,
							&data.longitude, &data.eastWest, &data.lock,
							&data.count, &data.extra, &data.altitude);
		//Break when all GPS items are found
		if (status == 9) {
				break;
		}
		ptr++;
	}
	
	if(status != 9){
		this->log_WARNING_LO_GPS_ParseError();
		return;
	}
	
	F32 lat, lon;
	
	lat = gps_deg_to_dec(data.latitude);
	lat = lat * ((data.northSouth == 'N') ? 1 : -1);
	
	lon = gps_deg_to_dec(data.longitude);
	lon = lon * ((data.eastWest == 'E') ? 1 : -1);
	
	tlmWrite_GPS_Time(data.utcTime);
	tlmWrite_GPS_Latitude(lat);
	tlmWrite_GPS_Longitude(lon);
	tlmWrite_GPS_Altitude(data.altitude);
	tlmWrite_GPS_Satellites(data.count);
	
	if(data.lock == 0 && this->m_lock){
			this->m_lock = false;
			log_WARNING_HI_GPS_LockLost();
	} 
	else if(data.lock == 1 && !this->m_lock){
			this->m_lock = true;
			log_ACTIVITY_HI_GPS_LockAcquired();
	}
	
}
```

Overall, the file should look something like this:


```c++
#include <Ref/GPS/GPSComponentImpl.hpp>
#include "Fw/Types/BasicTypes.hpp"

#include <cmath>

namespace Ref {
	
	extern "C" {
		#include <unistd.h>
		#include <fcntl.h>
		#include <termios.h>
		#include <sys/ioctl.h>
	}

	// ----------------------------------------------------------------------
	// Construction, initialization, and destruction 
	// ----------------------------------------------------------------------

	GPSComponentImpl ::
#if FW_OBJECT_NAMES == 1
		GPSComponentImpl(
				const char *const compName
		) :
			GPSComponentBase(compName)
#else
		GPSImpl(void)
#endif
	{

	}

	void GPSComponentImpl ::
		init(
				const NATIVE_INT_TYPE queueDepth,
				const NATIVE_INT_TYPE instance
		) 
	{
		GPSComponentBase::init(queueDepth, instance);
	}

	GPSComponentImpl ::
		~GPSComponentImpl(void)
	{

	}
	
	void GPSComponentImpl ::
		setup(void)
	{
		if(this->m_setup){
			return;
		}
		
		this->m_fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
		
		if(this->m_fd == -1){
			this->log_WARNING_HI_GPS_OpenError();
			this->m_setup = false;
			return;
		}
		
		struct termios options;
		tcgetattr(this->m_fd, &options);
		options.c_cflag = B57600 | CS8 | CLOCAL | CREAD;
		options.c_iflag = IGNPAR;
		options.c_oflag = 0;
		options.c_lflag = 0;
		tcflush(this->m_fd, TCIFLUSH);
		tcsetattr(this->m_fd, TCSANOW, &options);
		
		this->m_setup = true;
		this->m_lock = false;
	}
	
	
	float GPSComponentImpl ::
		gps_deg_to_dec(
				float deg 
			)
	{
		double dub_deg;
		float second = modf(deg, &dub_deg)*60;
		int degree = (int)(dub_deg/100);
		int minute = (int)(deg-(degree*100));

		float absDeg = round(degree * 1000000.);
		float absMin = round(minute * 1000000.);
		float absSec = round(second * 1000000.);

		return round(absDeg + (absMin/60) + (absSec/3600)) /1000000;
	}

	// ----------------------------------------------------------------------
	// Handler implementations for user-defined typed input ports
	// ----------------------------------------------------------------------

	void GPSComponentImpl ::
		schedIn_handler(
				const NATIVE_INT_TYPE portNum,
				NATIVE_UINT_TYPE context
		)
	{
		if(!this->m_setup){
			setup();
		}
		
		if(!this->m_setup){
			return;
		}
		
		ioctl(this->m_fd, TCFLSH, 0);
		Os::Task::delay(1500);
		
		char buffer[1024];
		char *ptr = buffer;
		
		ssize_t rd = read(this->m_fd, buffer, sizeof(buffer));
		
		if(rd <= 0){
			this->log_WARNING_HI_GPS_ReadError();
			return;
		}
		
		U8 status;
		GPS_Data data;
		for(U16 i = 0; i < sizeof(buffer) - 6; i++){
			status = sscanf(ptr, "$GPGGA,%f,%f,%c,%f,%c,%u,%u,%f,%f",
								&data.utcTime, &data.latitude, &data.northSouth,
								&data.longitude, &data.eastWest, &data.lock,
								&data.count, &data.extra, &data.altitude);
			//Break when all GPS items are found
			if (status == 9) {
					break;
			}
			ptr++;
		}
		
		if(status != 9){
			this->log_WARNING_LO_GPS_ParseError();
			return;
		}
		
		F32 lat, lon;
		
		lat = gps_deg_to_dec(data.latitude);
		lat = lat * ((data.northSouth == 'N') ? 1 : -1);
		
		lon = gps_deg_to_dec(data.longitude);
		lon = lon * ((data.eastWest == 'E') ? 1 : -1);
		
		tlmWrite_GPS_Time(data.utcTime);
		tlmWrite_GPS_Latitude(lat);
		tlmWrite_GPS_Longitude(lon);
		tlmWrite_GPS_Altitude(data.altitude);
		tlmWrite_GPS_Satellites(data.count);
		
		if(data.lock == 0 && this->m_lock){
				this->m_lock = false;
				log_WARNING_HI_GPS_LockLost();
		} 
		else if(data.lock == 1 && !this->m_lock){
				this->m_lock = true;
				log_ACTIVITY_HI_GPS_LockAcquired();
		}
		
	}

} // end namespace Ref
```

We'll add a GPS struct, function declarations, and a few variables to the header file. In the end, it should look like this:

```c++
#ifndef GPS_HPP
#define GPS_HPP

#include "Ref/GPS/GPSComponentAc.hpp"

namespace Ref {

	class GPSComponentImpl :
		public GPSComponentBase
	{
		
		struct GPS_Data {
				float utcTime;
				float latitude;
				char northSouth;
				float longitude;
				char eastWest;
				unsigned int lock;
				unsigned int count;
				float extra;
				float altitude;
		};

		public:

			// ----------------------------------------------------------------------
			// Construction, initialization, and destruction
			// ----------------------------------------------------------------------

			//! Construct object GPS
			//!
			GPSComponentImpl(
#if FW_OBJECT_NAMES == 1
					const char *const compName /*!< The component name*/
#else
					void
#endif
			);

			//! Initialize object GPS
			//!
			void init(
					const NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
					const NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
			);

			//! Destroy object GPS
			//!
			~GPSComponentImpl(void);

		PRIVATE:

			// ----------------------------------------------------------------------
			// Handler implementations for user-defined typed input ports
			// ----------------------------------------------------------------------
			
			void setup(void);
			
			float gps_deg_to_dec(
						float deg 
					);

			//! Handler implementation for schedIn
			//!
			void schedIn_handler(
					const NATIVE_INT_TYPE portNum, /*!< The port number*/
					NATIVE_UINT_TYPE context /*!< The call order*/
			);
			
			I8 m_fd;
			bool m_setup;
			bool m_lock;


		};

} // end namespace Ref

#endif
```

Don't forget to update your `mod.mk` and `modules.mk` files to include your new component! Next, `make rebuild` and you should be good to go!

# Check-In

At this point, if you `make RASPIAN` on a Raspberry Pi and run the `Ref/scripts/run_ref.sh` script, you should see telemetry values popping in for both sensors. If you do, everything is working properly! Next, we'll go over the LoRa radio and set that up to be a Ground Interface.

# LoRa Radio

LoRaGndIf.cpp

```c++
#include <Svc/LoRaGndIf/LoRaGndIfImpl.hpp>
#include <Fw/Com/ComPacket.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <Fw/Types/Assert.hpp>
#include <Fw/Types/EightyCharString.hpp>
#include <Os/Task.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <bcm2835.h>

#include "LoRaLibs_RasPi/RH_RF95.h"

#define ERROR (-1)


//#define DEBUG_PRINT(x,...) printf(x,##__VA_ARGS__); fflush(stdout)
#define DEBUG_PRINT(x,...)

namespace Svc {
	
	// Radio variable
	RH_RF95 rf95(8, 25);

		/////////////////////////////////////////////////////////////////////
		// Helper functions
		/////////////////////////////////////////////////////////////////////
		namespace {

				NATIVE_INT_TYPE socketWrite(RH_RF95 rf95, U8* buf, U32 size) {
						printf("Sending socketWrite...\n");
						rf95.send(buf, size);
						rf95.waitPacketSent();
						printf("Sent socketWrite!\n");
						return size;
				}
				
				NATIVE_INT_TYPE socketRead(NATIVE_INT_TYPE fd, U8* buf, U32 size) {
						NATIVE_INT_TYPE total=0;
						while(size > 0) {
								NATIVE_INT_TYPE bytesRead = read(fd, (char*)buf, size);
								if (bytesRead == -1) {
									if (errno == EINTR) continue;
									return (total == 0) ? -1 : total;
								}
								buf += bytesRead;
								size -= bytesRead;
								total += bytesRead;
						}
						return total;
				}
				
		}

		/////////////////////////////////////////////////////////////////////
		// Class implementation
		/////////////////////////////////////////////////////////////////////

#if FW_OBJECT_NAMES == 1    
		LoRaGndIfImpl::LoRaGndIfImpl(const char* name) : GndIfComponentBase(name)
#else
		LoRaGndIfImpl::LoRaGndIfImpl() : GndIfComponentBase()
#endif
		,m_socketFd(-1)
		,m_connectionFd(-1)
		,m_udpFd(-1)
		,m_udpConnectionFd(-1)
		,m_priority(1)
		,m_stackSize(1024)
		,port_number(0)
		,hostname(NULL)
		,m_cpuAffinity(-1)
		,useDefaultHeader(true)
		,m_prot(SEND_UDP)
		,m_portConfigured(false)
		,m_setup(0)
		{
		}

		void LoRaGndIfImpl::init(NATIVE_INT_TYPE instance) {
				GndIfComponentBase::init(instance);
		}
		
		LoRaGndIfImpl::~LoRaGndIfImpl() {
		}

		void LoRaGndIfImpl ::
				GNDIF_ENABLE_INTERFACE_cmdHandler(
						const FwOpcodeType opCode,
						const U32 cmdSeq
				)
		{
				if (m_portConfigured) {

						this->startSocketTask(this->m_priority,
																	this->m_stackSize,
																	this->port_number,
																	this->hostname,
																	this->m_prot,
																	this->m_cpuAffinity);

						this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
				} else {
						this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
				}
		}

		void LoRaGndIfImpl ::
				setSocketTaskProperties(I32 priority, NATIVE_INT_TYPE stackSize, U32 portNumber, char* hostname, DownlinkProt prot, NATIVE_INT_TYPE cpuAffinity)
		{

				this->m_priority = priority;
				this->m_stackSize = stackSize;
				this->port_number = portNumber;
				this->hostname = hostname;
				this->m_prot = prot;
				this->m_cpuAffinity = cpuAffinity;

				this->m_portConfigured = true;
		}

		void LoRaGndIfImpl::startSocketTask(I32 priority, NATIVE_INT_TYPE stackSize, U32 port_number, char* hostname,  DownlinkProt prot, NATIVE_INT_TYPE cpuAffinity) {
				Fw::EightyCharString name("ScktRead");
				this->port_number = port_number;
				this->hostname = hostname;
				
				
				
				if (!bcm2835_init()) {
					printf(" bcm2835_init() Failed\n\n" );
				}
				
				printf("bcm init\n");
				
				if (!rf95.init()) {
					printf("\nRF95 module init failed, Please verify wiring/module\n" );
				}
				
				// Set maximum power
				rf95.setTxPower(23, false);
					
				// Set frequency
				rf95.setFrequency(434.12);
				
				//rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128);
				
				sem_init(&this->radio, 0, 1);
				
				printf("Setup complete!\n");
				
				this->m_setup = 1;

				Os::Task::TaskStatus stat = this->socketTask.start(name,0,priority,stackSize,LoRaGndIfImpl::socketReadTask, (void*) this, cpuAffinity);
				FW_ASSERT(Os::Task::TASK_OK == stat,static_cast<NATIVE_INT_TYPE>(stat));

		}
		
		void LoRaGndIfImpl::socketReadTask(void* ptr) {
				FW_ASSERT(ptr);
				bool acceptConnections;
				uint32_t packetDelimiter;
				// Gotta love VxWorks...
				uint32_t packetSize;
				uint32_t packetDesc;
				U8 buf[FW_COM_BUFFER_MAX_SIZE];
				char buf2[256];
				ssize_t bytesRead;
				
				int connectionFd = -1;
				int udpFd = -1;
				int socketFd = -1;
				
				char ip[100];
				int sockAddrSize;
				strncpy(ip, "192.168.2.1", sizeof(ip));
				
				int port = 50000;
					struct sockaddr_in servaddr; // Internet socket address struct
					struct sockaddr_in servAddr; // !< UDP server
					
					if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
						printf("Socket error: %s\n",strerror(errno));
						return;
					}
					
					//if (SEND_UDP == prot) {
					if(1){

							udpFd = socket(AF_INET, SOCK_DGRAM, 0);
							if (-1 == udpFd) {
									//Was already open
									close(socketFd);
									printf("UDP Socket error: %s\n",strerror(errno));
									return;
							}

							/* fill in the server's address and data */
							memset((char*)&servAddr, 0, sizeof(servAddr));
							servAddr.sin_family = AF_INET;
							servAddr.sin_port = htons(port);
							inet_aton(ip , &servAddr.sin_addr);
					}
					
					struct timeval timeout;
					timeout.tv_sec = 1;
					timeout.tv_usec = 0;
					// set socket write to timeout after 1 sec
					if (setsockopt (socketFd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
													sizeof(timeout)) < 0) {
						printf("setsockopt error: %s\n",strerror(errno));
					}
					
					// Fill in data structure with server information
					sockAddrSize = sizeof(struct sockaddr_in);
					memset((char*) &servaddr, 0, sizeof(servaddr)); // set the entire stucture to zero
					servaddr.sin_family = AF_INET; // set address family to AF_INET (2)
					servaddr.sin_port = htons(port);

					servaddr.sin_addr.s_addr = inet_addr(ip);
					if (connect(socketFd, (struct sockaddr *) &servaddr, sockAddrSize) < 0) {
							//Force connect to close
							close(udpFd);
							close(socketFd);
							socketFd = -1;
							printf("Unable to connect\n");
							return;
					}
					
					strncpy(buf2, "Register FSW\n", sizeof(buf2));
						
						int tmpBytesWritten = socketWrite(socketFd, (uint8_t*)buf2, strnlen(buf2, 13));
						if (tmpBytesWritten < 0) {
							printf("write error on port %d \n", port);
							return;
						}
					
					connectionFd = socketFd;

				// cast pointer to component type
				LoRaGndIfImpl* comp = (LoRaGndIfImpl*) ptr;


				acceptConnections = true;

				// loop until magic "kill" packet
				while (acceptConnections) {
					
							// first, read packet delimiter
							printf("Socket fd: %i \n", socketFd);
							bytesRead = socketRead(socketFd,(U8*)&packetDelimiter,sizeof(packetDelimiter));
							printf("DONE WITH THE READING BTW\n");
							if( -1 == bytesRead ) {
									DEBUG_PRINT("Delim read error: %s",strerror(errno));
									break;
							}

							if(0 == bytesRead) {
								connectionFd = -1;
								break;
							}

							if (bytesRead != sizeof(packetDelimiter)) {
									DEBUG_PRINT("Didn't get right pd size: %ld\n",(long int)bytesRead);
							}
							// correct for network order
							packetDelimiter = ntohl(packetDelimiter);

							// if magic number to quit, exit loop
							if (packetDelimiter == 0xA5A5A5A5) {
									DEBUG_PRINT("packetDelimiter = 0x%x\n", packetDelimiter);
									break;
							} else if (packetDelimiter != LoRaGndIfImpl::PKT_DELIM) {
									DEBUG_PRINT("Unexpected delimiter 0x%08X\n",packetDelimiter);
									// just keep reading until a delimiter is found
									continue;
							}

							// now read packet size
							bytesRead = socketRead(connectionFd,(U8*)&packetSize,sizeof(packetSize));
							if( -1 == bytesRead ) {
									DEBUG_PRINT("Size read error: %s",strerror(errno));
									break;
							}

							if(0 == bytesRead) {
								connectionFd = -1;
								break;
							}

							if (bytesRead != sizeof(packetSize)) {
									DEBUG_PRINT("Didn't get right ps size!\n");
							}

							// correct for network order
							packetSize = ntohl(packetSize);


							// get packet description
							bytesRead = socketRead(socketFd,(U8*)&packetDesc,sizeof(packetDesc));
							packetDesc = ntohl(packetDesc);


							switch(packetDesc) {
									case Fw::ComPacket::FW_PACKET_COMMAND:
											
											// check size of command
											if (packetSize > FW_COM_BUFFER_MAX_SIZE) {
			//comp->log_WARNING_HI_GNDIF_ReceiveError(GNDIF_PacketTooBig, comp->port_number);
													DEBUG_PRINT("Packet to large! :%d\n",packetSize);
													// might as well wait for the next packet
													break;
											}

											// read cmd packet minus description
											bytesRead = socketRead(socketFd,(U8*)buf+sizeof(packetDesc),packetSize-sizeof(packetDesc));
											if (-1 == bytesRead) {
			//comp->log_WARNING_HI_GNDIF_ReceiveError(GNDIF_PacketReadError, comp->port_number);
													DEBUG_PRINT("Size read error: %s\n",strerror(errno));
													break;
											}

											if(0 == bytesRead) {
													connectionFd = -1;
													break;
											}

											// Add back description
											bytesRead = bytesRead + sizeof(packetDesc);

											buf[3] = packetDesc & 0xff;
											buf[2] = (packetDesc & 0xff00) >> 8;
											buf[1] = (packetDesc & 0xff0000) >> 16;
											buf[0] = (packetDesc & 0xff000000) >> 24;

											// check read size
											if (bytesRead != (ssize_t)packetSize) {
			//comp->log_WARNING_HI_GNDIF_ReceiveError(GNDIF_ReadSizeMismatch, comp->port_number);
													DEBUG_PRINT("Read size mismatch: A: %ld E: %d\n",(long int)bytesRead,packetSize);
											}


											if (comp->isConnected_uplinkPort_OutputPort(0)) {
													 Fw::ComBuffer cmdBuffer(buf, bytesRead);
													 comp->uplinkPort_out(0,cmdBuffer,0);
											}
											break;

									case Fw::ComPacket::FW_PACKET_FILE:
									{   
											
											// Get Buffer
											Fw::Buffer packet_buffer = comp->fileUplinkBufferGet_out(0, packetSize - sizeof(packetDesc));
											U8* data_ptr = (U8*)packet_buffer.getdata();


											// Read file packet minus description
											bytesRead = socketRead(socketFd, data_ptr, packetSize - sizeof(packetDesc));
											if (-1 == bytesRead) {
			//comp->log_WARNING_HI_GNDIF_ReceiveError(GNDIF_PacketReadError, comp->port_number);
													DEBUG_PRINT("Size read error: %s\n",strerror(errno));
													break;
											}

											// for(uint32_t i =0; i < bytesRead; i++){
											//     DEBUG_PRINT("IN_DATA:%02x\n", data_ptr[i]);
											// }

											if (comp->isConnected_fileUplinkBufferSendOut_OutputPort(0)) {
													comp->fileUplinkBufferSendOut_out(0, packet_buffer);
											}

									}
											break;
									default:
											FW_ASSERT(0);
							}
			}
		}


		void LoRaGndIfImpl::fileDownlinkBufferSendIn_handler(
				NATIVE_INT_TYPE portNum, /*!< The port number*/
				Fw::Buffer &fwBuffer
		) {

				char buf[256];
				U32 header_size = this->useDefaultHeader ? 9 : sizeof(LoRaGndIfImpl::PKT_DELIM);
				U32 desc        = 3; // File Desc
				U32 packet_size = 0;
				U32 buffer_size = fwBuffer.getsize();
				U32 net_buffer_size = htonl(buffer_size+4);

				//Write message header
				if (this->useDefaultHeader) {
						strncpy(buf, "A5A5 GUI ", sizeof(buf));
				}
				else {
						U32 data = LoRaGndIfImpl::PKT_DELIM;
						memcpy(buf, (U8*)&data, header_size);
				}
				packet_size += header_size;

				memmove(buf + packet_size, &net_buffer_size, sizeof(net_buffer_size));
				packet_size += sizeof(buffer_size);

				desc = htonl(desc);
				memmove(buf + packet_size, &desc, sizeof(desc));
				packet_size += sizeof(desc);

				//Send msg header
				(void)socketWrite(rf95,(U8*)buf, packet_size);
				//Send msg
				(void)socketWrite(rf95,(U8*)fwBuffer.getdata(), buffer_size);
				//DEBUG_PRINT("PACKET BYTES SENT: %d\n", bytes_sent);

				// for(uint32_t i = 0; i < bytes_sent; i++){
				//     DEBUG_PRINT("%02x\n",((U8*)fwBuffer.getdata())[i]);
				// }
				this->fileDownlinkBufferSendOut_out(0,fwBuffer);

		}


		void LoRaGndIfImpl::downlinkPort_handler(NATIVE_INT_TYPE portNum, Fw::ComBuffer &data, U32 context) {
				char buf[256];
				U32 header_size;
				U32 data_size;
				I32 data_net_size;
				I32 bytes_sent;

				//this is the size of "A5A5 GUI "
				header_size = this->useDefaultHeader ? 9 : sizeof(LoRaGndIfImpl::PKT_DELIM);
				data_size = data.getBuffLength();
				data_net_size = htonl(data.getBuffLength());
				// check to see if someone is connected
				//DEBUG_PRINT("Trying to send %ld bytes to ground.\n",data.getBuffLength() + header_size + sizeof(data_size));
				if (this->useDefaultHeader) {
						strncpy(buf, "A5A5 GUI ", sizeof(buf));
				}
				else {
						U32 data = LoRaGndIfImpl::PKT_DELIM;
						memcpy(buf, (U8*)&data, header_size);
				}
				
				memmove(buf + header_size, &data_net_size, sizeof(data_net_size));
				memmove(buf + header_size + sizeof(data_size), (char *)data.getBuffAddr(), data_size);
								
				if(this->m_setup){
					
					uint8_t data2Send[256];
					int size = header_size + data_size + sizeof(data_size);
					for(uint i = 0; i < size; i++){
						data2Send[i] = (uint8_t) buf[i];
						printf("%02x ", data2Send[i]);
					}
					printf("\n");
					
					sem_wait(&this->radio); 
					printf("Sending buf, size: %i...\n", size);
					rf95.send(data2Send, size);
					rf95.waitPacketSent();
					printf("Sent buffer!\n");
					sem_post(&this->radio); 
					Os::Task::delay(100);
				}
		}
		
		Svc::ConnectionStatus LoRaGndIfImpl::isConnected_handler(NATIVE_INT_TYPE portNum)
		{
				return (this->m_connectionFd != -1) ? Svc::SOCKET_CONNECTED :
																							Svc::SOCKET_NOT_CONNECTED;
		}

}
```

LoRaGndIf.hpp

```c++
#ifndef SVC_CMD_SOCKET_PROVIDER_IMPL_HPP
#define SVC_CMD_SOCKET_PROVIDER_IMPL_HPP

#include <Svc/GndIf/GndIfComponentAc.hpp>
#include <Os/Task.hpp>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <semaphore.h>

namespace Svc {

	class LoRaGndIfImpl : public GndIfComponentBase {
		public:
		
#if FW_OBJECT_NAMES == 1        
			LoRaGndIfImpl(const char* name);
#else
			LoRaGndIfImpl();
#endif
			void init(NATIVE_INT_TYPE instance);
			~LoRaGndIfImpl();

			enum DownlinkProt {
				SEND_UDP,
				SEND_TCP
			};

			void setSocketTaskProperties(I32 priority, NATIVE_INT_TYPE stackSize, U32 portNumber, char* hostname, DownlinkProt prot = SEND_UDP, NATIVE_INT_TYPE cpuAffinity = -1);

			void startSocketTask(NATIVE_INT_TYPE priority, NATIVE_INT_TYPE stackSize, U32 port_number, char* hostname,  DownlinkProt prot = SEND_UDP, NATIVE_INT_TYPE cpuAffinity = -1);
			void setUseDefaultHeader(bool useDefault);
		private:

			void openSocket(NATIVE_INT_TYPE port, DownlinkProt prot = SEND_UDP);

			static const U32 PKT_DELIM = 0x5A5A5A5A;

			//! Handler for input port fileDownlinkBufferSendIn
			//
			void fileDownlinkBufferSendIn_handler(
				NATIVE_INT_TYPE portNum, /*!< The port number*/
				Fw::Buffer &fwBuffer
			);

			//! Implementation for GNDIF_ENABLE_INTERFACE command handler
			//! Enable the interface for ground communications
			void GNDIF_ENABLE_INTERFACE_cmdHandler(
				const FwOpcodeType opCode, /*!< The opcode*/
				const U32 cmdSeq /*!< The command sequence number*/
			);

			static void socketReadTask(void* ptr);
			void downlinkPort_handler(NATIVE_INT_TYPE portNum, Fw::ComBuffer &data, U32 context);
			Svc::ConnectionStatus isConnected_handler(NATIVE_INT_TYPE portNum);

			NATIVE_INT_TYPE m_socketFd; // !< socket file descriptor
			NATIVE_INT_TYPE m_connectionFd; // !< connection file descriptor
			NATIVE_INT_TYPE m_udpFd; // !< udp socket if used
			NATIVE_INT_TYPE m_udpConnectionFd; // !< connection file descriptor
			struct sockaddr_in m_servAddr; // !< UDP server

			I32 m_priority;
			NATIVE_INT_TYPE m_stackSize;
			U32 port_number;
			char* hostname;
			NATIVE_INT_TYPE m_cpuAffinity;
			Os::Task socketTask;
			bool useDefaultHeader; // Use the default header for downlink, ie "A5A5 GUI "
			DownlinkProt m_prot; // is downlink TCP or UDP

			bool m_portConfigured;
			
			NATIVE_INT_TYPE m_setup;
			sem_t radio;
	};

}

#endif
```

raspian_gnu_common.mk

```make
include $(BUILD_ROOT)/mk/configs/compiler/include_common.mk
include $(BUILD_ROOT)/mk/configs/compiler/defines_common.mk
include $(BUILD_ROOT)/mk/configs/compiler/linux_common.mk
include $(BUILD_ROOT)/mk/configs/compiler/gnu-common.mk

CC :=  $(PI_TOOLS)/tools/arm-bcm2708/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
CXX := $(PI_TOOLS)/tools/arm-bcm2708/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++
GCOV := $(PI_TOOLS)/tools/arm-bcm2708/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcov
AR := $(PI_TOOLS)/tools/arm-bcm2708/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-ar

BUILD_32BIT := -m32

CC_MUNCH := $(BUILD_ROOT)/mk/bin/empty.sh

LINK_LIB := $(AR)
LINK_LIB_FLAGS := rcs
LIBRARY_TO := 
POST_LINK_LIB := $(PI_TOOLS)/tools/arm-bcm2708/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-ranlib

LINK_BIN := $(CXX)
LINK_BIN_FLAGS := -z muldefs $(LIBS) #$(BUILD_32BIT)

FILE_SIZE := $(LS) $(LS_SIZE)
LOAD_SIZE := $(SIZE)



LINK_LIBS := -ldl -lpthread -lm -lrt -lutil -lbcm2835

OPT_SPEED := -Os
DEBUG := -g3

LORA_RASPI_FLAGS := -DRASPBERRY_PI -DBCM2835_NO_DELAY_COMPATIBILITY

LINUX_GNU_CFLAGS := $(LINUX_FLAGS_COMMON) \
					$(COMMON_DEFINES) \
					$(GNU_CFLAGS_COMMON) \
					$(LORA_RASPI_FLAGS) \
					#$(BUILD_32BIT) # Quantum framework won't build 32-bit

LINUX_GNU_CXXFLAGS :=	$(LINUX_FLAGS_COMMON) \
						$(COMMON_DEFINES) \
						$(GNU_CXXFLAGS_COMMON) \
						$(LORA_RASPI_FLAGS) \
						#$(BUILD_32BIT)

COVERAGE := -fprofile-arcs -ftest-coverage

LINUX_GNU_INCLUDES := $(LINUX_INCLUDES_COMMON) $(COMMON_INCLUDES)

DUMP = $(BUILD_ROOT)/mk/bin/empty.sh

MUNCH := $(BUILD_ROOT)/mk/bin/empty.sh
```

Ground Station C++

```c++
#include <stdio.h>
#include <bcm2835.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <math.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <wiringPi.h>
#include <math.h>
#include <pthread.h> 

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>

#define PKT_DELIM 0x5A5A5A5A

/* Drivers. If a C driver, append in the extern C section. If C++, include normally */
#include "RH_RF95.h"

int connectionFd = -1;
int udpFd = -1;
int socketFd = -1;
RH_RF95 rf95(8, 25);
pthread_t thread; 

int socketWrite(int fd, uint8_t* buf, uint32_t size) {
		int total=0;
		while(size > 0) {
				int bytesWritten = write(fd, (char*)buf, size);
				if (bytesWritten == -1) {
					if (errno == EINTR) continue;
					return (total == 0) ? -1 : total;
				}
				buf += bytesWritten;
				size -= bytesWritten;
				total += bytesWritten;
		}
		return total;
}

int socketRead(int fd, uint8_t* buf, uint32_t size) {
		int total=0;
		while(size > 0) {
				int bytesRead = read(fd, (char*)buf, size);
				if (bytesRead == -1) {
					if (errno == EINTR) continue;
					return (total == 0) ? -1 : total;
				}
				buf += bytesRead;
				size -= bytesRead;
				total += bytesRead;
		}
		return total;
}

void *socketReadTask(void* ptr) {
	
		(void*) ptr;
			
		bool acceptConnections;
		uint32_t packetDelimiter;
		// Gotta love VxWorks...
		uint32_t packetSize;
		uint32_t packetDesc;
		uint8_t buf[255];
		ssize_t bytesRead;

		// cast pointer to component type
		//LoRaGndIfImpl* comp = (LoRaGndIfImpl*) ptr;

		while (socketFd == -1){
			usleep(1000000);
		}

		acceptConnections = true;

		// loop until magic "kill" packet
		while (acceptConnections) {

				// read packets
				while (true) {
						// first, read packet delimiter
						printf("SocketFd: %i\n", socketFd);
						bytesRead = socketRead(socketFd,(uint8_t*)&packetDelimiter,sizeof(packetDelimiter));
						if( -1 == bytesRead ) {
								printf("Delim read error: %s\n",strerror(errno));
								break;
						}

						if(0 == bytesRead) {
							connectionFd = -1;
							break;
						}

						if (bytesRead != sizeof(packetDelimiter)) {
								printf("Didn't get right pd size: %ld\n",(long int)bytesRead);
						}
						// correct for network order
						packetDelimiter = ntohl(packetDelimiter);

						// if magic number to quit, exit loop
						if (packetDelimiter == 0xA5A5A5A5) {
								printf("packetDelimiter = 0x%x\n", packetDelimiter);
								break;
						} else if (packetDelimiter != PKT_DELIM) {
								printf("Unexpected delimiter 0x%08X\n",packetDelimiter);
								// just keep reading until a delimiter is found
								continue;
						}
						else{
							printf("Delim got: 0x%08X\n", packetDelimiter);
						}

						printf("Now doing a read with FD: %i\n", connectionFd);
						// now read packet size
						bytesRead = socketRead(connectionFd,(uint8_t*)&packetSize,sizeof(packetSize));
						if( -1 == bytesRead ) {
								printf("Size read error: %s",strerror(errno));
								break;
						}

						if(0 == bytesRead) {
							connectionFd = -1;
							break;
						}

						if (bytesRead != sizeof(packetSize)) {
								printf("Didn't get right ps size!\n");
						}

						// correct for network order
						packetSize = ntohl(packetSize);


						// get packet description
						bytesRead = socketRead(socketFd,(uint8_t*)&packetDesc,sizeof(packetDesc));
						packetDesc = ntohl(packetDesc);
						
						printf("Desc: %0x%08X\n", packetDesc);


						switch(packetDesc) {
								case 0:
									{
										// read cmd packet minus description
										bytesRead = socketRead(socketFd,(uint8_t*)buf+sizeof(packetDesc),packetSize-sizeof(packetDesc));
										if (-1 == bytesRead) {
												printf("Goofed this one up...\n");
												printf("Size read error: %s\n",strerror(errno));
												break;
										}

										if(0 == bytesRead) {
												connectionFd = -1;
												break;
										}

										// Add back description
										bytesRead = bytesRead + sizeof(packetDesc);

										buf[3] = packetDesc & 0xff;
										buf[2] = (packetDesc & 0xff00) >> 8;
										buf[1] = (packetDesc & 0xff0000) >> 16;
										buf[0] = (packetDesc & 0xff000000) >> 24;

										// check read size
										if (bytesRead != (ssize_t)packetSize) {
												printf("Read size mismatch: A: %ld E: %d\n",(long int)bytesRead,packetSize);
										}
										
										printf("Done doing stuff with a command packet\n");
										
										uint8_t buf2Send[bytesRead+4];
										
										// *0xC header
										char *header = "*0xC";
										
										buf2Send[0] = (uint8_t) header[0];
										buf2Send[1] = (uint8_t) header[1];
										buf2Send[2] = (uint8_t) header[2];
										buf2Send[3] = (uint8_t) header[3];
										
										memcpy(buf2Send+4, buf, bytesRead);
										
										rf95.send(buf2Send, bytesRead+4);
										rf95.waitPacketSent();
										
										printf("Sent!\n");
									}
										break;
								case 3:
									{
									// Get Buffer
									uint8_t data_ptr[packetSize - sizeof(packetDesc)];


									// Read file packet minus description
									bytesRead = socketRead(socketFd, data_ptr, packetSize - sizeof(packetDesc));
									if (-1 == bytesRead) {
										printf("Size read error: %s\n",strerror(errno));
										break;
									}

									// for(uint32_t i =0; i < bytesRead; i++){
									//     DEBUG_PRINT("IN_DATA:%02x\n", data_ptr[i]);
									// }
									
									rf95.send(data_ptr, packetSize - sizeof(packetDesc));
									rf95.waitPacketSent();

									printf("Done doing file packet handling\n");
									}
									break;
									
								default:
									break;
						}

						

				} // while not done with packets
				close(socketFd);
				connectionFd = -1;
		}

}

/* Main function */
int main() {
	
	// Setup Radio
	if (!bcm2835_init()) {
		fprintf( stderr, "%s bcm2835_init() Failed\n\n", __BASEFILE__ );
		return 1;
	}
	
	printf("bcm init\n");
	
	if (!rf95.init()) {
		fprintf( stderr, "\nRF95 module init failed, Please verify wiring/module\n" );
		return 1;
	}
	
	// Set maximum power
	rf95.setTxPower(23, false);
		
	// Set frequency
	rf95.setFrequency(434.12);
	
	//rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128);
	
	// Set modem config
	// Premade modem configs: https://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html#ab9605810c11c025758ea91b2813666e3
	// You can also set parameters yourself
	
	printf("Radio init complete\n");
	
	printf("Attempting socket connection...\n");
	
	char ip[100];
	int sockAddrSize;
	strncpy(ip, "192.168.2.1", sizeof(ip));
	char buf[256];
	
	int port = 50000;
	struct sockaddr_in servaddr; // Internet socket address struct
	struct sockaddr_in servAddr; // !< UDP server
	
	if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Socket error: %s\n",strerror(errno));
		return 1;
	}
	
	//if (SEND_UDP == prot) {
	if(1){

			udpFd = socket(AF_INET, SOCK_DGRAM, 0);
			if (-1 == udpFd) {
					//Was already open
					close(socketFd);
					printf("UDP Socket error: %s\n",strerror(errno));
					return 1;
			}

			/* fill in the server's address and data */
			memset((char*)&servAddr, 0, sizeof(servAddr));
			servAddr.sin_family = AF_INET;
			servAddr.sin_port = htons(port);
			inet_aton(ip , &servAddr.sin_addr);
	}
	
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	// set socket write to timeout after 1 sec
	if (setsockopt (socketFd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
									sizeof(timeout)) < 0) {
		printf("setsockopt error: %s\n",strerror(errno));
	}
	
	// Fill in data structure with server information
	sockAddrSize = sizeof(struct sockaddr_in);
	memset((char*) &servaddr, 0, sizeof(servaddr)); // set the entire stucture to zero
	servaddr.sin_family = AF_INET; // set address family to AF_INET (2)
	servaddr.sin_port = htons(port);

	servaddr.sin_addr.s_addr = inet_addr(ip);
	if (connect(socketFd, (struct sockaddr *) &servaddr, sockAddrSize) < 0) {
			//Force connect to close
			close(udpFd);
			close(socketFd);
			socketFd = -1;
			printf("Unable to connect\n");
			return 1;
	}
	
	strncpy(buf, "Register FSW\n", sizeof(buf));
	
	int tmpBytesWritten = socketWrite(socketFd, (uint8_t*)buf, strnlen(buf, 13));
	if (tmpBytesWritten < 0) {
		printf("write error on port %d \n", port);
		return 1;
	}
	
	connectionFd = socketFd;
	
	printf("Connection success! Bytes written: %i on fd: %i\n", tmpBytesWritten, socketFd);
	
	pthread_create(&thread, NULL, socketReadTask, NULL);
	
	printf("Thread started successfully\n");
	
	while(1){
		
		
		// If uplink message is available
		if(rf95.available()){
			uint8_t buf2[100];
			uint8_t len2 = sizeof(buf2);

			if (rf95.recv(buf2, &len2))
			{
				printf("Received: %i bytes\n", len2);
				
				for(uint32_t i = 0; i < len2; i++){
					printf("%02x ", buf2[i]);
				}
				printf("\n");
					
				int bytes_sent = sendto(udpFd,
								buf2,
								len2,
								0,
								(struct sockaddr *) &servAddr,
								sizeof(servAddr));
								
				printf("Bytes sent: %i with fd: %i\n", bytes_sent, udpFd);
			}
		}	
	}
	
}
```