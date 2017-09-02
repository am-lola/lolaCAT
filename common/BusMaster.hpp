//
//  BusMaster.hpp
//  am2b
//
//  Created by Felix Sygulla on 2015-08-05.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//

#ifndef BUSMASTER_HPP_233200D0
#define BUSMASTER_HPP_233200D0

#include <vector>
#include <string>

// prototype
namespace ec {
  template <class SlaveInstanceMapperPolicy> class BusMaster;
}

#include "BusSlave.hpp"
#include "BusVarType.hpp"

namespace ec {

  /*! Bus States typesafe class enum */
  enum class BusState {
  
    UNKNOWN,        //!< before initialization
    INIT,           //!< after initialization of the bus
    PREOP,          //!< bus system and slaves initialized
    OP,             //!< bus running
    SAFEOP        //!< error on bus, safe operation continues
      
  };

  /*! Bus Master class */
  template <class SlaveInstanceMapperPolicy>
  class BusMaster {

  public:
  
   
  
    /*! Indicates a (fatal) fault during operation of the bus.
        A safety reaction should occur in case this flag is set to true. */
    bool onFault() {
      return m_fault;
    }

    /*! Reset fault flag */
    void resetFault() {
      m_fault = false;
    }

    /*! Returns the current cycle counter value. 
        This number increases with every frame. Overflows! */
    uint64_t getCycleCounter() {
      return m_cycleCounter;
    }
    
    /* Returns the bus cycle time in microseconds */
    virtual uint32_t getBusCycleTimeUs() = 0;

  protected:
    
    /*! pure virtual method for synchronous SDO download */
    virtual bool syncSendSDO(BusSlave<SlaveInstanceMapperPolicy>* const slave, const int& objIndex, 
                              const char& objSubIndex, const void* const data, const int& dataLen) = 0;
  
    /*! pure virtual method for synchronous SDO upload */
    virtual bool syncReceiveSDO(BusSlave<SlaveInstanceMapperPolicy>* const slave, const int& objIndex, 
                                const char& objSubIndex, void* const data, const int& dataLen, int* const outDataLen) = 0;
  
    /*! virtual method for linking to PDO variables */
    virtual bool linkPDOVar(BusSlave<SlaveInstanceMapperPolicy>* const slave, const std::string& varName,
                            BusVarType* ptr) {
                              
      if (ptr->isOutput()) {
        m_variablesOutputPDO.push_back(ptr);
      } else {
        m_variablesInputPDO.push_back(ptr);
      }
      return false;
    }
  
    /*! virtual method for linking to SDO variables */
    virtual bool linkSDOVar(BusSlave<SlaveInstanceMapperPolicy>* const slave, const int& objIndex, 
                            const char& objSubIndex, BusVarType* ptr) {
      m_variablesSDO.push_back(ptr);
      return false;
    }
  
    /*! pure virtual method for asynchronous SDO download */
    virtual bool asyncSendSDO(BusSlave<SlaveInstanceMapperPolicy>* const slave, BusVarType* const ptr) = 0;
  
    /*! pure virtual method for asynchronous SDO upload */
    virtual bool asyncReceiveSDO(BusSlave<SlaveInstanceMapperPolicy>* const slave, BusVarType* const ptr) = 0;
    
    /*! pure virtual method for getting the BusState */
    virtual BusState getState() = 0;
    
    /*! pure virtual method for setting the requested BusState */
    virtual void setRequestedState(const BusState& reqState, const bool& blocking=true) = 0;
    
    /*! Registers a slave for this master - this triggers call of process() method by the master */
    void registerSlave(BusSlave<SlaveInstanceMapperPolicy>* slave) {
      m_slaves.push_back(slave);
    }
    
    friend class BusSlave<SlaveInstanceMapperPolicy>;
    
    /*! Wrapper to call process() on a slave */
    void processOnSlave(BusSlave<SlaveInstanceMapperPolicy>* slave) {
      slave->process();
    }
    
    /*! Wrapper to call initOp() on a slave */
    void initOpOnSlave(BusSlave<SlaveInstanceMapperPolicy>* slave) {
      slave->initOp();
    }
  
    /*! pointers to registered BusOutput PDO variables for all slaves */
    std::vector<BusVarType*> m_variablesOutputPDO;
    
    /*! pointers to registered Businput PDO variables for all slaves */
    std::vector<BusVarType*> m_variablesInputPDO;
  
    /*! pointers to registered SDO variables for all slaves */
    std::vector<BusVarType*> m_variablesSDO;
    
    /*! slaves registered with this master instance */
    std::vector<BusSlave<SlaveInstanceMapperPolicy>* > m_slaves;
  
    //! fault flag
    volatile bool m_fault = false;
      
    //! Cycle counter
    volatile uint64_t               m_cycleCounter = 0;
  };

}

#endif /* end of include guard: BUSMASTER_HPP_233200D0 */
