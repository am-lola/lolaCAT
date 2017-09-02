//
//  SDOAsyncState.hpp
//  am2b
//
//  Created by Felix Sygulla on 2016-06-16.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//

#ifndef SDOASYNCSTATE_HPP_84ABE9A5
#define SDOASYNCSTATE_HPP_84ABE9A5

#include "BusVar.hpp"

namespace ec {

  /*! Base type for asynchronous SDO states */
  class SDOAsyncStateType {

  public:
    
    //! Construct base type
    // If the pointers to the actual or desired BusVarType are NULL,
    // the corresponding types are not used and an update event for
    // those types is not produced.
    SDOAsyncStateType(BusVarType* actual, BusVarType* desired) {
      m_actual = actual;
      m_desired = desired;
    }
    
    //! Returns the BusVarType for the desired state (async send)
    BusVarType* getBusVarDesired() {
      return m_desired;
    }
    
    //! Returns the BusVarType for the actual state (async receive)
    BusVarType* getBusVarActual() {
      return m_actual;
    }
    
    /*! Is a writing update on the state necessary? (triggers SDO send) */
    bool requestedStateChanged() {
      return (m_reqStateChanged && !m_reqStateUpdated && m_desired);
    }
    
    /*! Is a reading update on the state necessary? (triggers SDO receive) */
    bool updateState() {
      return (m_reqStateUpdated && !m_stateUpdated && m_actual);
    }
    
    //! Pure virtual method process(), called by SDOQueue
    virtual void process() = 0;


  protected:
    
    //! Has the state been updated since the last change of the req. state?
    bool    m_stateUpdated = false;
  
    //! Has the requested state been changed since last state update?
    bool    m_reqStateChanged = false;
  
    //! Has the requested state been updated since the change of the req. state?
    bool    m_reqStateUpdated = false;

  private:


    // Ptr to BusVarType object for desired state
    BusVarType*   m_desired = 0;


    // Ptr to BusVarType object for actual state
    BusVarType*   m_actual = 0;

  };

  /*! Defines a template structure for an asynchronously updated state variable.
  
    This holds two BusVar Async SDO objects, which correspond to the given data type.
    (One for each direction)

    The SDOQueue may read flags on this objects to actually trigger corresponding
    asynchronous transfers.
  
    If the setRequestedState() or renew() methods on the object are not called,
    the asynchronous state does not send any SDO requests to the device and the
    state stays at it's initial default setting for the device

    NOT thread-safe! */
  template <typename T>
  class SDOAsyncState : public SDOAsyncStateType {

  public:
    
    //! Constructor
    // Optional: The SDO updates for either actual
    // or desired state can be deactivated
    // e.g. if one of the variables is already read via PDO
    SDOAsyncState(bool usesActualSDO=true, bool usesDesiredSDO=true) : 
          SDOAsyncStateType(usesActualSDO ? &m_actual : NULL, usesDesiredSDO ? &m_desired : NULL) {
      m_usesDesiredSDO = usesDesiredSDO;
    }

    /*! Returns the requested state */
    T getRequestedState() {

      return m_reqValue;
    }

    /*! Get the state */
    T getState() {
      return m_value;
    }

    /* Methods for the STM-Side of the code: */

    //! Re-run requested state setting with current value
    void renew() {
      m_reqStateChanged = true;
      this->setRequestedState(m_reqValue);
    }

    /*! Set initial requested state. This is the "default" value.
        Setting this won't trigger sending an update to/from the device.
     */
    void setInitialRequestedState(const T& val) {
      m_reqValue = val;
    }

    /*! Set a new requested state */
    void setRequestedState(const T& val) {
    

      if ((m_reqValue == val && !m_reqStateChanged) || !m_usesDesiredSDO) {
        // This is the same request as before, just read out
        // if there is no request pending
        // OR: just read out if desired state is set via PDO/otherwise

        m_reqStateUpdated=true;
        m_reqStateChanged=false;
      
      } else {
      
        m_reqStateUpdated=false;
        m_reqStateChanged=true;
      
      }
  
      m_stateUpdated=false;
      m_reqValue = val;
      m_desired = val;

    }
  
    /*! updated and req + actual value identical? */
    bool requestedStateReached() {
      return ((m_reqValue == m_value) && m_stateUpdated && m_reqStateUpdated);
    }
    
    /*! Set the new, updated state */
    void setState(const T& val) {
      m_value = val;
      m_stateUpdated = true;
    
      // Did we reach the desired state?
      if (m_reqStateUpdated && m_value != m_reqValue) {
        // do it again...
        this->renew();
      }
    }

  private:
  
    //! Process method called from SDOQueue, runs in hwio main thread
    void process() {
      
      // Check if a transfer for the desired state
      // is complete
      if (m_desired.newTransferDone()) {
        
        m_reqStateUpdated = true;
        m_reqStateChanged = false;
        
      }
      
      // Check if a transfer for the actual state
      // is complete
      if (m_actual.newTransferDone()) {
        this->setState(m_actual);
      }

    }

    //! The value of the state variable
    T                       m_value = 0;

    //! Requested value of the state variable
    T                       m_reqValue = 0;
    
    //! BusVar for the desired state
    BusVar<T,BusOutputSDO>  m_desired;
    
    //! BusVar for the actual state
    BusVar<T,BusInputSDO>   m_actual;

    //! Desired state used?
    bool                    m_usesDesiredSDO;

  };

}

#endif /* end of include guard: SDOASYNCSTATE_HPP_84ABE9A5 */
