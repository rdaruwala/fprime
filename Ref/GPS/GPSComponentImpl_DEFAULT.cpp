// ====================================================================== 
// \title  GPSImpl.cpp
// \author rdaruwala
// \brief  cpp file for GPS component implementation class
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

// Acknowledgements to GitHub user LeStarch's GPS Tutorial as it was used as a reference for this class

#include <Ref/GPS/GPSComponentImpl.hpp>
#include "Fw/Types/BasicTypes.hpp"

namespace Ref {

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

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

  void GPSComponentImpl ::
    schedIn_handler(
        const NATIVE_INT_TYPE portNum,
        NATIVE_UINT_TYPE context
    )
  {
    return;
  }

} // end namespace Ref
