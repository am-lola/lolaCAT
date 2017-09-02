//
//  SDOQueue.hpp
//  am2b
//
//  Created by Felix Sygulla on 2016-06-16.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//

#ifndef SDOQUEUE_HPP_414AB011
#define SDOQUEUE_HPP_414AB011

#include <vector>

#include "SDOAsyncState.hpp"

namespace ec {

  class SDOQueue {

  public:
    
    //! Construct a SDO Queue with given slave and callbacks
    SDOQueue(void* slavePtr, bool (*sendSDO)(void*, BusVarType* const), bool (*receiveSDO)(void*, BusVarType* const)) {
      m_sendSDO = sendSDO;
      m_receiveSDO = receiveSDO;
      m_slavePtr = slavePtr;
    }
    
    //! Sets the delay between SDO requests (in Nr of process() calls)
    void setDelay(unsigned int delay) {
      m_delay = delay;
      m_counter = m_delay;
      //pdbg("Initialized SDO Queue with %d ms delay\n", delay);
    }
    
    //! Add a state element
    void addAsyncState(SDOAsyncStateType* sdo) {
      m_states.push_back(sdo);
    }
    
  
    //! Process the queue elements
    void process() {

      // increment the counter if below the delay threshold
      if (m_counter < m_delay) {
        m_counter++;
      }
      
      // we have an active transfer, which is (no longer) in progress
      if (m_curTransfer && !m_curTransfer->transferInProgress()) {
        
        // call process of this transfer state
        m_curTransferState->process();
        m_curTransfer = 0;
      }

      // Is a new transfer requested?
      if (m_curTransfer == 0 && m_counter == m_delay) {
        
        // Search for the next transfer to execute
        for (stateIterator it = m_states.begin(); it != m_states.end(); ++it) {
          
          if ((*it)->requestedStateChanged()) {
            
            // new async send necessary
            if (m_sendSDO(m_slavePtr, (*it)->getBusVarDesired())) {
              perr("SDOQueue: Error sending queued SDO\n");
            } else {
              m_curTransfer = (*it)->getBusVarDesired();
              m_curTransferState = (*it);
              m_counter = 0;
              return;
            }

          } else if ((*it)->updateState()) {
            
            // new async receive necessary
            if (m_receiveSDO(m_slavePtr, (*it)->getBusVarActual())) {
              perr("SDOQueue: Error receiving queued SDO\n"); 
            } else {
              m_curTransfer = (*it)->getBusVarActual();
              m_curTransferState = (*it);
              m_counter = 0;
              return;
            }
            
          }
        }
        
        m_curTransfer = 0;
      }
    }
  
  private:
    
    // Definition of the iterator type
    typedef typename std::vector<SDOAsyncStateType*>::iterator  stateIterator;
    
    //! wait time between SDO requests (in Nr of process() calls)
    unsigned int                                                m_delay = 0;
    
    //! BusSlave used to send the SDOs
    void*                                                       m_slavePtr;
    
    //! wait counter
    unsigned int                                                m_counter = 0;

    /*! List of Pointers to the asyncronous SDO states */
    std::vector<SDOAsyncStateType* >                            m_states;
    
    //! BusVarType for the current transfer
    BusVarType*                                                 m_curTransfer = 0;
    
    //! SDOAsyncStateType for the current transfer
    SDOAsyncStateType*                                          m_curTransferState = 0;
    
    //! Function ptr to sendSDO function
    bool (*m_sendSDO)(void*, BusVarType* const);
    
    //! Function ptr to receiveSDO function
    bool (*m_receiveSDO)(void*, BusVarType* const);

  };
  
}

#endif /* end of include guard: SDOQUEUE_HPP_414AB011 */
