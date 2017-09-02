//
//  ElmoStateMachine.hpp
//  am2b
//
//  Created by Felix Sygulla on 2015-11-18.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//


#ifndef ELMOSTATEMACHINE_HPP_E58F152B
#define ELMOSTATEMACHINE_HPP_E58F152B

#include <string.h>
#include <xstdio.h>

//! For ElmoState enum
#include "iface_ec_lola.hpp"

//! Async States
#include "SDOAsyncState.hpp"

//! For the logging macros
#include "BusSlave.hpp"
#include "LogRateLimiter.hpp"

namespace ec {

  //! Macro for activating the verbose mode (print to stdout)
  //#define EL_STM_VERBOSE

  //! Maximum messages per error (log rate limit)
  #define EL_STM_MAX_MSGS_PER_ERROR 10
  
  //! log rate reduction factor after maximum error msgs
  #define EL_STM_REDUCED_LOG_RATE 1000

  //! Be CAREFUL. If this macro is activated, homeing is not required for switching to OPERATIONAL state
  #undef EL_ASSUME_HOMING_DONE

  //! Defines the mode of operation for normal operation
  #define ELMO_MOO_OPERATIONAL 8

  //! Defines the mode of operation for homing 
  #define ELMO_MOO_HOMING 6
  
  //! Defines the mod of operation for profiled moves to Zero after Limit-Switch Homing
  #define ELMO_MOO_PROFILE_POSITION 1
  
  //! Defines the homing speed for limit switches (counts/sec)
  #define ELMO_HOMING_SPEED 10000
  
  //! Defines the low homing speed for searching zero position (counts/sec)
  #define ELMO_HOMING_SPEED_LOW 500

  //! Homing methods
  enum ElmoHomingType: int8_t  {
    ABS_ENCODER = 35,     //!< Use the absolute encoder for homing (can be done in READY)
    REVERSE_LIMIT_SWITCH = 17,  //!< Move in reverse direction until the limit switch fires. -> reaches homing position.
    FORWARD_LIMIT_SWITCH = 18  //!< Move in forward direction until the limit switch fires. -> reaches homing position.
  };


  /*! Elmo state machine representation (internal) */
  class ElmoStateMachine {

  public:
  
    /*! Default constructor for the elmo state machine. Initialize all members and states
  
     \param homingMethod Type of homing procedure
     \param no_motor_motion Do not allow motion if true
     */
    ElmoStateMachine(const ElmoHomingType& homingMethod, const bool& no_motor_motion);
  
    /*!
      Input: current elmo status word, mode of operation, current velocity (ticks/s)
      Calculates the state of the elmo from the status word and 
      handles the necessary transitions to get to the requested state
    */
    void updateState(const uint16_t& statusWord, const int8_t& modeOfOperation, const int32_t& currentVelocity) {
    
      m_statusWord = statusWord;
      m_modeOfOperation.setState(modeOfOperation);
      m_currentVelocity = currentVelocity;

      // update state and calculate control word
      updateState();
      dispatchTransitions();
    }
  
    /*! Returns the control word to be sent to the elmo */
    uint16_t getControlWord() {
      return m_controlWord;
    }
  
    /*! Return SDOAsyncState object for mode of operation (send only, receive is PDO) */
    SDOAsyncState<int8_t>* getAsyncModeOfOperation() {
      return &m_modeOfOperation;
    }
  
    /*! Return SDOAsyncState object for homing method */
    SDOAsyncState<int8_t>* getAsyncHomingMethod() {
      return &m_homingMethod;
    }
  
    /*! Return SDOAsyncState object for homing offset */
    SDOAsyncState<int32_t>* getAsyncHomingOffset() {
      return &m_homingOffset;
    }
    
    /*! Returns SDOAsyncState Object for profile velocity (profiled mode) */
    SDOAsyncState<uint32_t>* getAsyncProfileVelocity() {
      return &m_profileVelocity;
    }
    
    /*! Returns SDOAsyncState Object for homing speed */
    SDOAsyncState<uint32_t>* getAsyncHomingSpeed() {
      return &m_homingSpeed;
    }
    
    /*! Returns ElmoAsynState Object for low homing speed */
    SDOAsyncState<uint32_t>* getAsyncHomingSpeedLow() {
      return &m_homingSpeedLow;
    }
    
    /*! Approval to reset a fault on the elmo */
    void resetFault() {
      m_resetFault = true;
    }

    /*! Set the requested state for the Elmo */
    void setRequestedState(const ElmoState& state) {

      // If the requested state changed to HOMED
      if (m_reqState != state && state == ElmoState::HOMED) {
        // reset the homingDone flag, as homing is currently in progress
        m_homingDone = false;
        m_zeroingDone = false;
      }

      m_reqState = state;
    }
    
    /*! Returns the current requested state for the Elmo */
    ElmoState getRequestedState() {
      return m_reqState;
    }
  
    /*! Return the current state of the Elmo */
    ElmoState getState() {
      return m_state;
    }
  
    /*! Homing complete? */
    bool homingComplete() {
      return m_homingDone && m_zeroingDone;
    }
  
    /*! Pre-Homing flag
       This flag goes true right before the transition to the HOMING_OPERATION state, where the drive is moved.
       Should trigger an user interface output to apply high voltage on the drive before ackHoming() is called to
       resume with the homing / alignment process.
      */
    bool preHoming() {
      return (m_preHoming && !m_ackHoming);
    }
  
    /*! acknowledge operation of homing / alignment (see Pre-Homing flag) */
    void ackHoming() {
      m_ackHoming = true;
    
      // reset preHoming...
      m_preHoming = false;
      
      // Homing offset should be set now
      m_homingOffset.renew();
    }

    /*! 
      Sets the homing offset (start value) for the encoder registered at the "homing socket"
      This must be done before ackHoming() is called!
     */
    void setHomingOffset(const int32_t& calibOffset) {
      m_homingOffset.setInitialRequestedState(calibOffset);
    }
  
    /*! Set slave name for logging */
    void setSlaveName(std::string name) {
      m_slaveName = name;
    }
    
    /*! Print the state of the Elmo to stdout. Debugging only, blocking method! */
    void printState() {
      pdbgSlave("--------------------------------------------\n");
      pdbgSlave("STM state (act, req): %s, %s\n", convertElmoStateToStr(m_state).c_str(), convertElmoStateToStr(m_reqState).c_str());
      pdbgSlave("STM controlWord: %i\n", m_controlWord);
      pdbgSlave("STM statusWord: %i\n", m_statusWord);
      pdbgSlave("STM modeOfOperation: (act, req): %i, %i\n", m_modeOfOperation.getState(), m_modeOfOperation.getRequestedState());
      pdbgSlave("STM homingMethod: (act, req): %i, %i\n", m_homingMethod.getState(), m_homingMethod.getRequestedState());
      pdbgSlave("STM homingOffset: (act, req): %i, %i\n", m_homingOffset.getState(), m_homingOffset.getRequestedState());
      pdbgSlave("STM homingSpeed: (act, req): %i, %i\n", m_homingSpeed.getState(), m_homingSpeed.getRequestedState());
      pdbgSlave("STM homingSpeedLow: (act, req): %i, %i\n", m_homingSpeedLow.getState(), m_homingSpeedLow.getRequestedState());
      pdbgSlave("STM profileVelocity: (act, req): %i, %i\n", m_profileVelocity.getState(), m_profileVelocity.getRequestedState());
      fflush(stdout);
    }
  
  private:
  
    /*! slave name for perrSlave, pmsgSlave... */
    std::string             m_slaveName;
  
    ElmoState         m_state;    //!< Actual elmo state
    ElmoState         m_reqState; //!< Requested elmo state
  
    //! Is the homing already acquired?
    bool                    m_homingDone;
    
    //! Is the zeroing process already done?
    bool                    m_zeroingDone;
  
    //! pre-homing flag
    bool                    m_preHoming;
  
    //! homing ack flag
    bool                    m_ackHoming;
  
    /*! store the old state */
    ElmoState         m_stateOld;
  
    //! flag to reset the fault state
    bool                    m_resetFault;
  
    //! Elmo status word
    uint16_t                m_statusWord;
  
    //! Elmo control word
    uint16_t                m_controlWord;
    
    //! Current velocity of the joint (motor side, tics/s)
    uint32_t                m_currentVelocity;
  
    //! Mode of operation
    SDOAsyncState<int8_t>  m_modeOfOperation;
  
    //! Homing method
    SDOAsyncState<int8_t>  m_homingMethod;
  
    //! Homing offset
    SDOAsyncState<int32_t> m_homingOffset;
    
    //! Homing Speed
    SDOAsyncState<uint32_t> m_homingSpeed;
    
    //! Homing Speed Low
    SDOAsyncState<uint32_t> m_homingSpeedLow;

    //! Profile Velocity (profiled mode)
    SDOAsyncState<uint32_t> m_profileVelocity;
    
    //! no_motor_motion allowed if true
    bool                     m_noMotorMotion;

    /*! Returns the slave Name as string
       This is needed for the perrSlave, pmsgSlave,... macros */
    std::string getName() {
      return m_slaveName;
    }
  
    /*! Calculates the required bits in the control word / mode of operation / ... for the 
      transition to the mapped simplified state of the elmo
     */
    void dispatchTransitions();

    /*! Calculates the simplified state of the elmo from status word bits,
       mode of operation,...
     */
    void updateState();
  
  
    /* CONTROL WORD HELPERS */
    
    //! shutdown drive
    inline uint16_t shutdownCmd() {
      return 0x06;
    }
  
    //! switch on command
    inline uint16_t switchOnCmd() {
      return 0x07;
    }
  
    //! switch on and enable operation cmd
    inline uint16_t switchOnEnableCmd() {
      return 0x0f;
    }
  
    //! disable voltage cmd
    inline uint16_t disableVoltageCmd() {
      return 0;
    }
  
    //! disable operation cmd
    inline uint16_t disableOperationCmd() {
      return 0x07;
    }
  
    //! enable operation cmd
    inline uint16_t enableOperationCmd() {
      return 0x0f;
    }
  
    //! fault reset
    inline uint16_t faultReset() {
      return 0x80;
    }
  
    //! homing request
    inline uint16_t homingCmd() {
      return 0x10;
    }
    
    //! Change Set point immediately
    inline uint16_t immediateSetPoint() {
      return 0x20;
    }
    
    //! New set point
    inline uint16_t newSetPoint() {
      return 0x10;
    }

    /* STATUS WORD HELPERS */
    /* mutually excluded states of the elmo */
  
    //! NOT READY TO SWITCH ON?
    inline bool notReadyToSwitchOn() {
      return ((0x4f & m_statusWord) == 0);
    }
  
    //! SWITCH ON DISABLED ?
    inline bool switchOnDisabled() {
      return ((0x4f & m_statusWord) == 0x40);
    }
  
    //! READY TO SWITCH ON ?
    inline bool readyToSwitchOn() {
      return ((0x6f & m_statusWord) == 0x21);
    }
  
    //! SWITCHED ON ?
    inline bool switchedOn() {
      return ((0x6f & m_statusWord) == 0x23);
    }
  
    //! OPERATION ENABLED ?
    inline bool operationEnabled() {
      return ((0x6f & m_statusWord) == 0x27);
    }
  
    //! QUICK STOP ACTIVE ?
    inline bool quickStopActive() {
      return ((0x6f & m_statusWord) == 0x07);
    }
  
    //! FAULT REACTION ACTIVE ?
    inline bool faultReactionActive() {
      return ((0x4f & m_statusWord) == 0x0f);
    }
  
    //! FAULT ?
    inline bool fault() {
      return ((0x4f & m_statusWord) == 0x08);
    }

    /* Additional flags */
  
    //! This becomes true if the drive is enabled (high voltage on motor)?
    inline bool voltageEnabled() {
      return (0x10 & m_statusWord);
    }
  
    //! Warning flag
    inline bool warning() {
      return (0x80 & m_statusWord);
    }
  
    //! target reached flag
    inline bool targetReached() {
      return (0x400 & m_statusWord);
    }
  
    //! Internal limit flag
    inline bool limitActive() {
      return (0x800 & m_statusWord);
    }
  
    //! Specialized bit flag 12. may be ack, homing attained, etc..
    inline bool bit12() {
      return (0x1000 & m_statusWord);
    }

    //! Specialized bit flag 13. may be tracking error, slippage error or homing error
    inline bool bit13() {
      return (0x2000 & m_statusWord);
    }
  
  };
  

#include "ElmoStateMachine_impl.hpp"

}

#endif /* end of include guard: ELMOSTATEMACHINE_HPP_E58F152B */
