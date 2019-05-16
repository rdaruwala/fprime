// ====================================================================== 
// \title  GPSImpl.hpp
// \author rdaruwala
// \brief  hpp file for GPS component implementation class
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
