// ====================================================================== 
// \title  BME280Impl.hpp
// \author rdaruwala
// \brief  hpp file for BME280 component implementation class
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
