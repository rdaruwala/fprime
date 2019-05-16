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
    
    lat = (U32)(data.latitude/100.0f);
    lat = lat + (data.latitude - (lat * 100.0f))/60.0f;
    lat = lat * ((data.northSouth == 'N') ? 1 : -1);
    
    lon = (U32)(data.longitude/100.0f);
    lon = lon + (data.longitude - (lon * 100.0f))/60.f;
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
