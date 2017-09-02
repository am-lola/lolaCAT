//
//  BusArray.hpp
//  am2b
//
//  Created by Felix Sygulla on 2015-12-03.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//

#ifndef BUSARRAY_HPP_6DF4422B
#define BUSARRAY_HPP_6DF4422B

#include <mutex>
#include <typeinfo>

#include "BusVarType.hpp"
#include <xdebug.h>

namespace ec {

  /*! Provides interface for user data access to arrays on the bus.
      
      Implements locking and abstraction of low-level data exchange with the BusMaster.
   */
  template<typename T, std::size_t Size, class BusVarDirection>
  class BusArray : public BusVarDirection {
  
  public:
  
    /*! Implements isArray() method from BusVarType */
    const bool isArray() const {
      return true;
    }
  
    //! Constructor
    BusArray() {
    
      // set pointer address to member of parent
      this->m_dataPtr = (void*) &m_data;
    
      // set the typeid
      this->m_typeid = &typeid(T);

      // zero is always defined as initial value
      for(uint16_t idx=0;idx<Size;idx++)
        this->m_data[idx] = 0;

      this->sz = Size;
    }
  
    //! copy constructor
    BusArray (const BusArray& other) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      // copy data 
      other.copyTo(this->m_data, Size);
    
    }
  
    //! type based constructor (thread-safe)
    BusArray(const T& value) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      // assign to whole array
      for (int i = 0; i < Size; i++) {
        this->m_data[i] = value;
      }
    }
  
    /*! Returns the size of the variable in bits */
    const unsigned int getSize() const {
      return Size * BusVarTypeSize<T>();
    }
  
    //! Assignment from other (thread-safe)
    BusArray& operator=(const BusArray& other) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      if (this != &other) {
      
        // scoped lock
        std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
        // copy data
        other.copyTo(this->m_data, Size);
      
      }
    
      return *this;
    }
  
    //! assignment operator from type T (thread-safe)
    BusArray& operator=(const T& value) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      // assign to whole array
      for (int i = 0; i < Size; i++) {
        this->m_data[i] = value;
      }

      return *this;
    }

    /*! subscript operator, read only! (thread-safe) */
    const T operator[](unsigned long idx) {
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
      return this->m_data[idx];
    }
  
    /*! Set value for given index (thread-safe) */
    void setValue(unsigned long idx, const T& value) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
      this->m_data[idx] = value;
    }
  
    /*! Copy data to given buffer (thread-safe) */
    void copyTo(uint8_t* dest, unsigned int size) {
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      int sizeMin = Size;
      if (size < Size) {
        sizeMin = size;
      }
    
      for (int i=0; i < sizeMin; i++) {
        dest[i] = m_data[i];
      }
    
    }
  
    /*! Copy data from given buffer (thread-safe) */
    void copyFrom(uint8_t* src, unsigned int size) {
    
  #ifdef DEBUG
      // write not allowed on input
      XASSERT(this->isOutput());
  #endif
    
      // scoped lock
      std::lock_guard<std::timed_mutex> lock(this->m_mutex);
    
      int sizeMin = Size;
      if (size < Size) {
        sizeMin = size;
      }
    
      for (int i=0; i < sizeMin; i++) {
        m_data[i] = src[i];
      }
    
    }

  private:
  
    T     m_data[Size];
    std::size_t sz;

  };


  /* Specific type aliases */
  template <class BusVarDir, size_t N >
  using BusArrayBool = BusArray< bool, N, BusVarDir >;

  template <class BusVarDir, size_t N >
  using BusArrayUInt8 = BusArray< uint8_t, N, BusVarDir >;

  template <class BusVarDir, size_t N >
  using BusArrayInt8 = BusArray< int8_t, N, BusVarDir >;

  template <class BusVarDir, size_t N >
  using BusArrayInt16 = BusArray< int16_t, N, BusVarDir >;

  template <class BusVarDir, size_t N >
  using BusArrayUInt16 = BusArray< uint16_t, N, BusVarDir >;

  template <class BusVarDir, size_t N >
  using BusArrayInt32 = BusArray< int32_t, N, BusVarDir >;

  template <class BusVarDir, size_t N >
  using BusArrayUInt32 = BusArray< uint32_t, N, BusVarDir >;

  template <class BusVarDir, size_t N >
  using BusArrayInt64 = BusArray< int64_t, N, BusVarDir >;

  template <class BusVarDir, size_t N >
  using BusArrayUInt64 = BusArray< uint64_t, N, BusVarDir >;

  template <class BusVarDir, size_t N >
  using BusArrayReal32 = BusArray< float, N, BusVarDir >;

  template <class BusVarDir, size_t N >
  using BusArrayReal64 = BusArray< double, N, BusVarDir >;

}

#endif /* end of include guard: BUSARRAY_HPP_6DF4422B */
