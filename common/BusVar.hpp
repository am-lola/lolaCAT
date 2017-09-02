//
//  BusVar.hpp
//  am2b
//
//  Created by Felix Sygulla on 2015-12-03.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//

#ifndef BUSVAR_HPP_9ABB3A02
#define BUSVAR_HPP_9ABB3A02

#include <mutex>
#include <typeinfo>

#include "BusVarType.hpp"
#include <xdebug.h>

namespace ec {

  /*! Provides interface for user data access to variables on the bus.
      
      Implements locking and abstraction of low-level data exchange with the BusMaster.
   */
  template<typename T, class BusVarDirection>
  class BusVar : public BusVarDirection {
  
  public:
  
    //! Constructor
    BusVar() {
    
      // set pointer address to member of parent
      this->m_dataPtr = (void*) &m_data;
    
      // set the typeid
      this->m_typeid = &typeid(T);
    
      // zero is always defined
      this->m_data = 0;
    }
  
    //! copy constructor
    BusVar (const BusVar& other) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      // copy data
      this->m_data = (T) const_cast<BusVar&>(other);
    
    }
  
    //! type based constructor
    BusVar(const T& value) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      this->m_data = value;
    
    }
  
    /*! Returns the size of the variable in bits */
    const unsigned int getSize() const {
      return BusVarTypeSize<T>();
    }
  
    //! assignment from other
    BusVar& operator=(const BusVar& other) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      if (this != &other) {
      
        // scoped lock
        std::lock_guard<std::timed_mutex> lock(this->m_mutex);
      
        // copy data
        this->m_data = (T) other;
      
      }
    
      return *this;
    }
  
    //! assignment operator from type T
    BusVar& operator=(const T& value) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
      this->m_data = value;

      return *this;
    }
  
    //! type assignment operator
    operator T() {
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
      return this->m_data;
    }
  
    BusVar& operator+=(const T& value) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      this->m_data += value;
      return *this;
    }
  
    BusVar& operator-=(const T& value) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      this->m_data -= value;
      return *this;
    
    }
  
    BusVar& operator*=(const T& value) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      this->m_data *= value;
      return *this;
    }
  
    BusVar& operator/=(const T& value) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      this->m_data /= value;
      return *this;
    }
  
    BusVar& operator%=(const T& value) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      this->m_data %= value;
      return *this;
    }
  
    BusVar& operator&=(const T& value) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      this->m_data &= value;
      return *this;
    }
  
    BusVar& operator|=(const T& value) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      this->m_data |= value;
      return *this;
    }
  
    BusVar& operator^=(const T& value) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      this->m_data ^= value;
      return *this;
    }
  
    BusVar& operator<<=(const T& value) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      this->m_data <<= value;
      return *this;
    }
  
    BusVar& operator>>=(const T& value) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      this->m_data >>= value;
      return *this;
    }
  
    T operator++() {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // pre-increment
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      return ++this->m_data;
    }
  
    T operator++(int) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // post-increment
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);

      return this->m_data++;
    }
  
    T operator--() {
      // pre-decrement
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      return --this->m_data;
    }
  
    T operator--(int) {
      // post-decrement
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      return this->m_data--;
    }

    //! get value method
    const T getValue() {
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      return this->m_data;
    }
  
  private:
  
    //! The internal representation of the bus variable data
    T     m_data;
  
  };


  /*! Emergency Object type for CoE */
  class BusVarEmergency : public BusInputSDO {
    public:
      
      //! Constructor
      BusVarEmergency() {
        // set pointer address to member of parent
        this->m_dataPtr = (void*) &m_data;
        this->m_offset = BUSVAR_COE_EMERGENCY;
        
        m_data.errorCode = 0;
        m_data.errorRegister = 0;
        this->m_typeid = &typeid(CoEEmergency);
      }
      
      /*! Returns the size of the emergency object in bits */
      const unsigned int getSize() const {
        return 8*8;
      }
      
      //! Return the error code
      const uint16_t getErrorCode() {
        
        // scoped lock
        std::lock_guard<std::timed_mutex> lock(this->m_mutex);
        
        return m_data.errorCode;
      }
      
      //! Return the error register
      const uint8_t getErrorRegister() {
        
        // scoped lock
        std::lock_guard<std::timed_mutex> lock(this->m_mutex);
        
        return m_data.errorRegister;
      }
      
      //! Return the error data byte at given index
      const uint8_t getErrorDataAt(const unsigned int& index) {
        
        if (index > 4) {
          return 0;
        }
        
        // scoped lock
        std::lock_guard<std::timed_mutex> lock(this->m_mutex);
        
        return m_data.errorData[index];
      }
    
    private:
      
      //! Internal struct to store the CoE emergency data
      #include <begin_pack.h>
      struct CoEEmergency {
        uint16_t  errorCode;
        uint8_t   errorRegister;
        uint8_t   errorData[5];
      };
      #include <end_pack.h>
      
      //! stores the CoE emergency data
      CoEEmergency m_data;
      
  };

  /* Specific type aliases */
  template <class BusVarDir>
  using BusBool = BusVar< bool, BusVarDir >;

  template <class BusVarDir>
  using BusInt8 = BusVar< int8_t, BusVarDir >;

  template <class BusVarDir>
  using BusSint = BusInt8<BusVarDir>;

  template <class BusVarDir>
  using BusUInt8 = BusVar< uint8_t, BusVarDir >;

  template <class BusVarDir>
  using BusUint = BusUInt8<BusVarDir>;

  template <class BusVarDir>
  using BusInt16 = BusVar< int16_t, BusVarDir >;

  template <class BusVarDir>
  using BusInt = BusInt16<BusVarDir>;

  template <class BusVarDir>
  using BusUInt16 = BusVar< uint16_t, BusVarDir >;

  template <class BusVarDir>
  using BusUInt = BusUInt16<BusVarDir>;

  template <class BusVarDir>
  using BusInt32 = BusVar< int32_t, BusVarDir >;

  template <class BusVarDir>
  using BusDInt = BusInt32<BusVarDir>;

  template <class BusVarDir>
  using BusUInt32 = BusVar< uint32_t, BusVarDir >;

  template <class BusVarDir>
  using BusUDInt = BusUInt32<BusVarDir>;

  template <class BusVarDir>
  using BusInt64 = BusVar< int64_t, BusVarDir >;

  template <class BusVarDir>
  using BusUInt64 = BusVar< uint64_t, BusVarDir >;

  template <class BusVarDir>
  using BusReal32 = BusVar< float, BusVarDir >;

  template <class BusVarDir>
  using BusReal64 = BusVar< double, BusVarDir >;

}

#endif /* end of include guard: BUSVAR_HPP_9ABB3A02 */
