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

        if (this->port_number == 0){
        	return;
        }
        else {

                // Spawn read task:
        	Os::Task::TaskStatus stat = this->socketTask.start(name,0,priority,stackSize,LoRaGndIfImpl::socketReadTask, (void*) this, cpuAffinity);
        	FW_ASSERT(Os::Task::TASK_OK == stat,static_cast<NATIVE_INT_TYPE>(stat));
        }
    }
    
    void LoRaGndIfImpl::socketReadTask(void* ptr) {
        FW_ASSERT(ptr);
        bool acceptConnections;
        uint32_t packetDelimiter;
        // Gotta love VxWorks...
        uint32_t packetSize;
        uint32_t packetDesc;
        U8 buf[FW_COM_BUFFER_MAX_SIZE];
        ssize_t bytesRead;

        // cast pointer to component type
        LoRaGndIfImpl* comp = (LoRaGndIfImpl*) ptr;


        acceptConnections = true;

        // loop until magic "kill" packet
        while (acceptConnections) {
              if(rf95.available()){
                uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
                uint8_t len = sizeof(buf);
                
                
                if (rf95.recv(buf, &len))
                {
                  printf("Packet received: %s\n", buf);
                  // Check if it's a Command Packet
                  if(!strncmp((char*) buf, "*0xC", 4)){
                    // Do command things
                    if (comp->isConnected_uplinkPort_OutputPort(0)) {
                         Fw::ComBuffer cmdBuffer(buf+4, strlen((char*) buf+4));
                         comp->uplinkPort_out(0,cmdBuffer,0);
                    }
                  }
                  else if(!strncmp((char*) buf, "*0xF", 4)){
                    // Do file things
                    Fw::Buffer packet_buffer = comp->fileUplinkBufferGet_out(0, packetSize);
                    U8* data_ptr = (U8*)packet_buffer.getdata();
                    
                    memcpy(data_ptr, buf+4, strlen((char*)buf+4));
                    
                    if (comp->isConnected_fileUplinkBufferSendOut_OutputPort(0)) {
                        comp->fileUplinkBufferSendOut_out(0, packet_buffer);
                    }
                  }
                }
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
        }
    }

    void LoRaGndIfImpl::setUseDefaultHeader(bool useDefault)
    {
        this->useDefaultHeader = useDefault;
    }

    Svc::ConnectionStatus LoRaGndIfImpl::isConnected_handler(NATIVE_INT_TYPE portNum)
    {
        return (this->m_connectionFd != -1) ? Svc::SOCKET_CONNECTED :
                                              Svc::SOCKET_NOT_CONNECTED;
    }

}