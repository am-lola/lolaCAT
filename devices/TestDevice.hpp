//
//  TestDevice.hpp
//  am2b
//
//  Created by Felix Sygulla on 2015-12-08.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//


#ifndef TESTDEVICE_HPP_02276189
#define TESTDEVICE_HPP_02276189

#include "BusVar.hpp"
#include "BusException.hpp"

namespace ec {

  /*! Test Device Implementation, mainly for testing purposes */
  template<class PipedInterface>
  class TestDevice : public PipedInterface {

  public:
  
    TestDevice(int linkAdr) {
    
      m_adr = linkAdr;
    
    }
  
    /*! Set dummy var via SDO */
    void setByteVarSDOSync(char value) {
  
      // send synchronuously
      if (this->syncSendSDO(m_adr, 0, &value, 1)) {
      
        throw BusException("Device: Error sending sync SDO!");
      }
    }
  
    /*! get dummy var via SDO */
    char getByteVarSDOSync() {
    
      char tmp = 0;
    
      // read synchronuously
      if (this->syncReceiveSDO(m_adr, 0, &tmp, 1, 0)) {
        throw BusException("Device: Error receiving sync SDO!");
      }
   
      return tmp;
    }
  
    /*! async send of the given adr via SDO */
    void sendVarAsync(char value) {
      m_outputSDOByte = value;
      if (this->asyncSendSDO(&m_outputSDOByte)) {
        throw BusException("Device: Error sending async SDO!");
      }
    }
  
    /*! async receive of the given adr via SDO */
    void receiveVarAsync() {
      if (this->asyncReceiveSDO(&m_inputSDOByte)) {
        throw BusException("Device: Error receiving async SDO!");
      }
    }
  
    /*! Send in progress? */
    bool asyncSendInProgress() {
      return m_outputSDOByte.transferInProgress();
    }
  
    /*! Receive in progress? */
    bool asyncReceiveInProgress() {
      return m_inputSDOByte.transferInProgress();
    }
  
    /*! new async data? */
    bool newAsyncDataAvailable() {
      return m_inputSDOByte.newTransferDone();
    }
  
    /*! SDO send transfer done? */
    bool doneAsyncDataTransfer() {
      return m_outputSDOByte.newTransferDone();
    }
  
    int8_t getInputVar() {
    
      return m_inputSDOByte;
    
    }
  
  private:
    
    //!init in bus operational state
    void initOp(){
    }
  
    //! process new data
    void process(){
    }
    
    //!init before bus operation
    void init(){
    }
  
    
    void link() {
    
      this->linkSDOVar(m_adr, 0, &m_inputSDOByte);
      this->linkSDOVar(m_adr, 0, &m_outputSDOByte);
    
    }
  
    int                       m_adr;
  
    BusInt8<BusInputSDO>      m_inputSDOByte;
    BusInt8<BusOutputSDO>     m_outputSDOByte;

  };

}

#endif /* end of include guard: TESTDEVICE_HPP_02276189 */
