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

namespace Ref {
  

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction 
  // ----------------------------------------------------------------------

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

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

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

} // end namespace Ref
