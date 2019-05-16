// ====================================================================== 
// \title  BME280Impl.cpp
// \author rdaruwala
// \brief  cpp file for BME280 component implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged. Any commercial use must be negotiated with the Office
// of Technology Transfer at the California Institute of Technology.
// 
// This software may be subject to U.S. export control laws and
// regulations.  By accepting this document, the user agrees to comply
// with all U.S. export laws and regulations.  User has the
// responsibility to obtain export licenses, or other export authority
// as may be required before exporting such information to foreign
// countries or providing access to foreign persons.
// ====================================================================== 


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
