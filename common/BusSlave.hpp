//
//  BusSlave.hpp
//  am2b
//
//  Created by Felix Sygulla on 2015-08-05.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//

#ifndef BUSSLAVE_HPP_97C0C637
#define BUSSLAVE_HPP_97C0C637

#include <stdint.h>

// prototype
namespace ec {
  template<class SlaveInstanceMapperPolicy> class BusSlave;
}

#include "BusMaster.hpp"
#include "BusVarType.hpp"

// Definition disables the fault reaction for
// all ethercat slaves! Be careful. This might be DANGEROUS
//#define HWL_EC_DISABLE_SLAVE_FAULT_REACTION
#undef HWL_EC_DISABLE_SLAVE_FAULT_REACTION

#ifdef HWL_EC_DISABLE_SLAVE_FAULT_REACTION
  #define EC_FAULT_SLAVE_WARN perr("EtherCAT Slaves Fault Reaction DISABLED in code options! Be really careful!\n")
#else
  #define EC_FAULT_SLAVE_WARN
#endif

namespace ec {

  /* Logging macros to automatically log the slave name
     Only for use within a slave class derived from this template via the template pipeline
  */
  #define pmsgSlave(format, ...) pmsg("%s: " format, this->getName().c_str(), ##__VA_ARGS__)
  #define pwrnSlave(format, ...) pwrn("%s: " format, this->getName().c_str(), ##__VA_ARGS__)
  #define perrSlave(format, ...) perr("%s: " format, this->getName().c_str(), ##__VA_ARGS__)
  #define pdbgSlave(format, ...) pdbg("%s: " format, this->getName().c_str(), ##__VA_ARGS__)

  /*! Adapter class for the EtherCat slave.

      This interfaces each Slave with the corresponding master and provides methods
      to enable communication with data on the bus.
  */
  template <class SlaveInstanceMapperPolicy>
  class BusSlave : public SlaveInstanceMapperPolicy {

  public:
  
    /*! Set the master instance. This needs to be done for proper operation of the slave */
    void setMaster(BusMaster<SlaveInstanceMapperPolicy>* master) {
      m_master = master;
      
      // register this slave at the master instance
      m_master->registerSlave(this);
      
      // Link variables
      this->link();
      
      // Init slave
      this->init();
      
      // Expands to warnings for each slave if the fault reaction for slaves is disabled...
      EC_FAULT_SLAVE_WARN;
    }
    
    /*! Returns the master instance */
    BusMaster<SlaveInstanceMapperPolicy>* getMaster() {
      return m_master;
    }
  
    /*! Return true if a (fatal) fault occured.
        Safety reactions need to be taken in this case */
    bool onFault() {
    #ifdef HWL_EC_DISABLE_SLAVE_FAULT_REACTION
      perrSlave("Fault Reaction disabled!\n");
      return false;
    #else
      return m_fault;
    #endif
    }

    /*! Reset fault flag - this resets any fault flags on the 
        devices and enables continued operation after a fault. */
    void resetFault() {
      m_fault = false;
    }
    
    /* The static methods in the following can be used for callbacks from outside the templated world */

    /*! Initiate a non-blocking CoE SDO download to slave 
      with the given BusVar. Variable must have been registered with linkSDOVar() */
    static bool asyncSendSDO(void* slavePtr, BusVarType* const ptr) {
      
      // cast to correct data type
      BusSlave<SlaveInstanceMapperPolicy>* slave = static_cast<BusSlave<SlaveInstanceMapperPolicy>* > (slavePtr);
      
      // call
      return slave->getMaster()->asyncSendSDO(slave, ptr);
    }

    /*! Initiate a non-blocking CoE SDO upload from slave 
      with the given BusVar. Variable must have been registered with linkSDOVar() */
    static bool asyncReceiveSDO(void* slavePtr, BusVarType* const ptr) {
      
      // cast to correct data type
      BusSlave<SlaveInstanceMapperPolicy>* slave = static_cast<BusSlave<SlaveInstanceMapperPolicy>* > (slavePtr);
      
      // call
      return slave->getMaster()->asyncReceiveSDO(slave, ptr);
    }
    
  
  protected:
    
    /*! Called by master.process() after
        every! successful switch to OPERATIONAL state of the Bus.  */
    virtual void initOp()=0;

    /*! Called cyclically be the master in same thread as master.process()
    
        Use this to interpret data received with the BusVars or for any other
        administrative operations on slave level. Only called if the Bus is in
        OP or SAFE_OP state.
     */
    virtual void process()=0;
    
    /*! Called after registration of slave and linking of variables.
    
        Init the slave here. The Bus is not yet functional in this state!
    */
    virtual void init()=0;
    
    /*! Called during registration of the slave at the master.
    
        Link all BusVars here.
    */
    virtual void link()=0;
  
    /*! Initiate a blocking CoE SDO download to slave 
      dataLen in bytes
    */
    bool syncSendSDO(const int& objIndex, const char& objSubIndex, const void* const data, const int& dataLen) {
      return m_master->syncSendSDO(this, objIndex, objSubIndex, data, dataLen);
    }
  
    /*! Initiate a blocking CoE SDO upload from slave
      dataLen in bytes
     */
    bool syncReceiveSDO(const int& objIndex, const char& objSubIndex, void* const data, const int& dataLen, int* const outDataLen) {
      return m_master->syncReceiveSDO(this, objIndex, objSubIndex, data, dataLen, outDataLen);
    }
  
    /*! Initiate a non-blocking CoE SDO download to slave 
       with the given BusVar. Variable must have been registered with linkSDOVar() */
    bool asyncSendSDO(BusVarType* const ptr) {
      return m_master->asyncSendSDO(this, ptr);
    }
  
    /*! Initiate a non-blocking CoE SDO upload from slave 
       with the given BusVar. Variable must have been registered with linkSDOVar() */
    bool asyncReceiveSDO(BusVarType* const ptr) {
      return m_master->asyncReceiveSDO(this, ptr);
    }
  
    /*! 
    Links the given PDO BusVar to the given slave variable.
    Returns true if an error occurs
    */
    bool linkPDOVar(const std::string& name, BusVarType* const ptr) {
      return m_master->linkPDOVar(this, name, ptr);
    }
  
    /*!
    Links the given SDO BusVar to the given slave variable.
    Returns true if an error occurs
    */
    bool linkSDOVar(const int& objIndex, const char& objSubIndex, BusVarType* const ptr) {
      return m_master->linkSDOVar(this, objIndex, objSubIndex, ptr);
    }
  
    /*! Called by the BusSlave, sets the fault flag */
    void setFaultFlag() {
      m_fault = true;
    }
  
  private:
    
    friend void BusMaster<SlaveInstanceMapperPolicy>::processOnSlave(BusSlave<SlaveInstanceMapperPolicy>* slave);
    friend void BusMaster<SlaveInstanceMapperPolicy>::initOpOnSlave(BusSlave<SlaveInstanceMapperPolicy>* slave);
  
    /*! Master instance, which is used for operation of the slave */
    BusMaster<SlaveInstanceMapperPolicy>* m_master;
  
    /*! Fault flag. May be set by the client to signal
        necessity for a safety reaction */
    volatile bool m_fault = false;

  };

}

#endif /* end of include guard: BUSSLAVE_HPP_97C0C637 */
