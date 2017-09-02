//
//  BusVarType.hpp
//  am2b
//
//  Created by Felix Sygulla on 2015-08-16.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//

#ifndef BUSVARTYPE_HPP_6F8FF699
#define BUSVARTYPE_HPP_6F8FF699

#include <mutex>
#include <typeinfo>

namespace ec {
  
// This is the address of the emergency object for a slave
// to link against the async CoE emergency SDO
#define BUSVAR_COE_EMERGENCY 0xFFFFFF

  // ==============================================================
  // = Parent class BusVarType,                                   =
  // = which stores basic information for all variable types      =
  // ==============================================================
  /*! Base class for the Bus variables / arrays

      Stores all basic information about bus variables / arrays and
      is the interface description to the BusMaster.
   */
  class BusVarType : public BusVarTraits {

  public:
  
    //! Default constructor
    BusVarType() {
      m_SDOTransferDone = false;
      m_SDOTransferInProgress = false;
    }
  
    /*! Returns the size of the variable in bits */
    virtual const unsigned int getSize() const = 0;
  
    /*! Returns ptr to data */
    void* getPointer() {
      return m_dataPtr;
    }
  
    /*! Returns the mutex to lock access to the data area */
    std::timed_mutex& getMutex() {
      return m_mutex;
    }
  
    /*! Returns true, if an SDO transfer is in progress */
    bool transferInProgress() {
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);

      return m_SDOTransferInProgress;
    
    }
  
    /*! Returns true if a new SDO transfer happened
       since the last call to this function */
    bool newTransferDone() {
    
      bool ret;
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    

      if (!m_SDOTransferInProgress) {

        ret = m_SDOTransferDone;
        m_SDOTransferDone = false;
        return ret;
      
      } else {
      
        m_SDOTransferDone = false;
        return false;
      }
    }
    
    /*! Returns true if the last SDO transfer failed
     (since last call to this function) */
    bool newTransferFailed() {
      
      bool ret;
      
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
      
      if (!m_SDOTransferInProgress) {

        ret = m_SDOTransferFailed;
        m_SDOTransferFailed = false;
        return ret;
      
      } else {
      
        m_SDOTransferFailed = false;
        return false;
      }
      
    }
  
    /*! Is the variable an output? */
    virtual const bool isOutput() const = 0;
  
    /*! Returns the typeid of the stored variable */
    const std::type_info* getTypeId() const {
      return m_typeid;
    }
  
    /*! Is this an array? */
    virtual const bool isArray() const {
      return false;
    }
  
    /*! PDO? (SDO otherwise) */
    virtual const bool isPDO() const {
      return true;
    }
  
    //! True if there has been a successful SDO mailbox transfer since last call to newTransferDone()
    bool                    m_SDOTransferDone;
  
    //! True if there is currently a SDO mailbox transfer in progress
    bool                    m_SDOTransferInProgress;
    
    //! True if there has been a failed SDO mailbox transfer since last call to newTransferFailed()
    bool                    m_SDOTransferFailed;
  
  protected:
  
    //! ptr to the data assigned to the variable
    void*                   m_dataPtr;
  
    //! for thread safety (PDO data is accessed from within JobTask thread)         
    std::timed_mutex        m_mutex;
  
    //! to identify type after cast to this parent class           
    const std::type_info*   m_typeid;
  };


  /*! PDO BusOutput Abstraction class */
  class BusOutput : public BusVarType {
  
  public:
  
    const bool isOutput() const {
      return true;
    }

  };

  /*! PDO BusInput Abstraction class */
  class BusInput : public BusVarType {
  
  public:
  
    const bool isOutput() const {
      return false;
    }

  };

  /*! SDO BusOutput Abstraction class */
  class BusOutputSDO : public BusVarType {
  
  public:
  
    const bool isOutput() const {
      return true;
    }
  
    // SDO
    const bool isPDO() const {
      return false;
    }

  };

  /*! SDO BusInput Abstraction class */
  class BusInputSDO : public BusVarType {
  
  public:
  
    const bool isOutput() const {
      return false;
    }
  
    // SDO
    const bool isPDO() const {
      return false;
    }

  };


  /*! template function to determine size of type T */
  template <typename T> unsigned int BusVarTypeSize() {
    return 8*sizeof(T);
  }

  /*! BusVarTypeSize specialization for bool */
  template <> unsigned int BusVarTypeSize<bool>() {
    return 1;
  }

}

#endif /* end of include guard: BusVarType_HPP_6F8FF699 */