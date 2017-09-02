//
//  ElmoStateMachine_impl.hpp
//  am2b
//
//  Created by Felix Sygulla on 2016-03-31.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//

// ===============
// = Constructor =
// ===============
ElmoStateMachine::ElmoStateMachine(const ElmoHomingType& homingMethod, const bool& no_motor_motion)
    : m_modeOfOperation(false, true) {
  
    // start with INIT as the initial state
    m_state = ElmoState::INIT;
    m_stateOld = m_state;
  
    // request the IDLE state
    m_reqState = ElmoState::IDLE;
    m_controlWord = 0;
    m_currentVelocity = 0;
    m_resetFault = false;
    m_modeOfOperation.setInitialRequestedState(ELMO_MOO_OPERATIONAL);
    m_homingMethod.setInitialRequestedState(homingMethod);
    m_profileVelocity.setInitialRequestedState(ELMO_HOMING_SPEED);
    m_homingSpeed.setInitialRequestedState(ELMO_HOMING_SPEED);
    m_homingSpeedLow.setInitialRequestedState(ELMO_HOMING_SPEED_LOW);
    
    m_homingOffset.setInitialRequestedState(0);
    m_preHoming = false;
    m_ackHoming = false;
    
    m_noMotorMotion = no_motor_motion;

#ifdef EL_ASSUME_HOMING_DONE
    m_homingDone = true;
    m_zeroingDone = true;
#else
    m_homingDone = false;
    m_zeroingDone = false;
#endif
    
    // In case no_motion flag is set, assume homing for homing types, which require
    // operation
    if (m_noMotorMotion && (m_homingMethod.getRequestedState() != ElmoHomingType::ABS_ENCODER)) {
      m_homingDone = true;
      m_zeroingDone = true;
    }
}

// =======================
// = dispatchTransitions =
// =======================
void ElmoStateMachine::dispatchTransitions() {

  // from HOMED (Still OP -> IDLE)
  if (operationEnabled() && m_state == ElmoState::HOMED) {
    
    m_controlWord = shutdownCmd();
    return;
  }

  if (m_state == ElmoState::INIT) {
#ifdef EL_STM_VERBOSE
    pdbgSlave("Reboot INIT Transition\n");
#endif
    // Reset homing
    m_homingDone = false;
    m_zeroingDone = false;
  }

  // IDLE -> IDLE (ready to switch on)
  if ((m_state == ElmoState::IDLE || m_state == ElmoState::HOMED || m_state == ElmoState::HOMING_ZEROING || m_state == ElmoState::HOMING_IDLE) && switchOnDisabled()) {
    
    // -> transition to READY TO SWITCH ON
    m_controlWord = shutdownCmd();
    
    if (m_state == ElmoState::IDLE) {
      
      // This should only occur at the first transition to READY TO SWITCH ON after Booting
      // renew async objects as drive communication is online now...
      m_modeOfOperation.renew();
      m_homingMethod.renew();
      m_homingOffset.renew();

      // Sync homing speed parameters in case of limit switch homing
      if (m_homingMethod.getRequestedState() == ElmoHomingType::REVERSE_LIMIT_SWITCH || 
          m_homingMethod.getRequestedState() == ElmoHomingType::FORWARD_LIMIT_SWITCH) {
       
        m_profileVelocity.renew();
        m_homingSpeed.renew();
        m_homingSpeedLow.renew();
      }
      
#ifdef EL_STM_VERBOSE
      pdbgSlave("STM IDLE->IDLE Initial Transition!\n");
#endif

      // Assume homing for limit switch homed elmos in noMotion mode
      if (m_noMotorMotion && (m_homingMethod.getRequestedState() != ElmoHomingType::ABS_ENCODER)) {
        m_homingDone = true;
        m_zeroingDone = true;
      }
    }

#ifdef EL_STM_VERBOSE
    printState();
    
    if (m_state == ElmoState::HOMED) {
      pdbgSlave("STM transition: %s\n", "HOMED -> HOMED (ready to switch on)");
    } else if (m_state == ElmoState::HOMING_ZEROING) {
      pdbgSlave("STM transition: %s\n", "HOMING_ZEROING -> HOMING_ZEROING (ready to switch on)");
    } else if (m_state == ElmoState::HOMING_IDLE) {
      pdbgSlave("STM transition: %s\n", "HOMING_IDLE -> HOMING_IDLE (ready to switch on)");
    }
    else {
      pdbgSlave("STM transition: %s\n", "IDLE -> IDLE (ready to switch on)");
    }
#endif
      return;
  }
  
  // this function does not allow transitions to the FAULT /INIT state
  if (m_reqState == ElmoState::FAULT || m_reqState == ElmoState::INIT) {
    return;
  }

  // we are in FAULT state...
  if (m_state == ElmoState::FAULT) {

    // no further transitions possible in FAULT state, howver we should send
    // the shutdown cmd to be safe
    m_controlWord = shutdownCmd();
    
    // Reset homing as it is not guaranteed to have correct
    // homing after a FAULT occured.
    m_homingDone = false;
    m_zeroingDone = false;
  
    // if the fault reset flag is true, enable fault reset
    // on the next iteration, the drive will change its state to "switch on disabled"
    // then the state machine resumes with normal operations to acquire desired state
    if (m_resetFault) {

      m_resetFault = false;
      m_controlWord = faultReset();
    
#ifdef EL_STM_VERBOSE
    printState();
    pdbgSlave("STM transition: %s\n", "fault reset");
#endif

    }

    return;
  }

  // fault reset is only valid for the next update cycle
  m_resetFault = false;
  
  
  // required state already reached or in INIT state?
  if (m_reqState == m_state || m_state == ElmoState::INIT) {
    return;
  }

  /* state transitions */

  // transition to IDLE is always the same and equals any transition from HOMED
  if (m_reqState == ElmoState::IDLE || m_state == ElmoState::HOMED) {
  
    // shutdown command -> READY TO SWITCH ON
    m_controlWord = shutdownCmd();
#ifdef EL_STM_VERBOSE
    printState();
    pdbgSlave("STM transition: %s\n", "-> IDLE");
#endif
    
    
    // set the mode of operation to normal
    m_modeOfOperation.setRequestedState(ELMO_MOO_OPERATIONAL);
    
    return;

  }

  // Transition to OPERATIONAL is only allowed if homing is already complete
  if (m_reqState == ElmoState::OPERATIONAL && homingComplete() == false) {
#ifdef EL_STM_VERBOSE
    perrSlave("STM: OPERATIONAL state requested before homing done!\n");
#endif
    m_state = ElmoState::FAULT;
    return;
  }
  
  // Simulate transitions to OPERATIONAL in case no motion flag is set
  if (m_reqState == ElmoState::OPERATIONAL && m_noMotorMotion) {
    m_state = ElmoState::OPERATIONAL;
    return;
  }
  
  // Ignore transitions to HOMED in case no motion flag is set and homing 
  // requires operation
  if (m_noMotorMotion && (m_homingMethod.getRequestedState() != ElmoHomingType::ABS_ENCODER) && 
         m_reqState == ElmoState::HOMED) {

    m_homingDone = true;
    m_zeroingDone = true;
    return;
  }

  // Requesting state HOMING_READY not allowed
#ifdef EL_STM_VERBOSE
  if (m_reqState == ElmoState::HOMING_READY || 
    m_reqState == ElmoState::HOMING_OPERATIONAL || 
    m_reqState == ElmoState::HOMING_ZEROING ||
    m_reqState == ElmoState::HOMING_IDLE) {
  
    perrSlave("STM: Requesting HOMING_READY, HOMING_IDLE, HOMING_ZEROING or HOMING_OPERATIONAL not allowed!");
    return;
  }
#endif
  
  //! **************************************************************  from COMMUTATION ->
  if (m_state == ElmoState::COMMUTATION) {
   
    // Explicit transition from COMMUTATION
    if (m_reqState == ElmoState::READY) {
    
      // set the mode of operation to normal
      m_modeOfOperation.setRequestedState(ELMO_MOO_OPERATIONAL);
 
      // switch to the READY state
      m_controlWord = switchOnCmd();
    
#ifdef EL_STM_VERBOSE
    printState();
    pdbgSlave("STM transition: %s\n", "COMMUTATION -> READY");
#endif
      return;
      
    }
    
    // to IDLE: See above
    // to OPERATIONAL, HOMING_OPERATIONAL, HOMED: automatically via Elmo state change
    return;
  }

  //! **************************************************************  from IDLE ->
  if (m_state == ElmoState::IDLE) {
    
    // -> READY
    if (m_reqState == ElmoState::READY) {
      
      // set the mode of operation to normal
      m_modeOfOperation.setRequestedState(ELMO_MOO_OPERATIONAL);
 
      // switch to the READY state
      m_controlWord = switchOnCmd();
    
#ifdef EL_STM_VERBOSE
    printState();
    pdbgSlave("STM transition: %s\n", "IDLE -> READY");
#endif
      return;
  
    }

    // -> HOMED
    if (m_reqState == ElmoState::HOMED) {
      
      // -> transition to READY
      m_controlWord = switchOnCmd();
    
#ifdef EL_STM_VERBOSE
    printState();
    pdbgSlave("STM transition: %s\n", "IDLE -> READY");
#endif 
      return;
        
    }

    // -> OPERATIONAL
    if (m_reqState == ElmoState::OPERATIONAL) {

      // Wait until velocity of the joint is zero
      // motor can not be turned on if the joint moves (e.g. due to alignment on other joints)
      if (m_currentVelocity == 0) {
        // switch directly to the OPERATIONAL state
        m_controlWord = switchOnEnableCmd();
        
#ifdef EL_STM_VERBOSE
        printState();
        pdbgSlave("STM transition: %s\n", "IDLE -> OPERATIONAL");
#endif
      }

      return;

    }

  }

  //! **************************************************************  from READY ->
  if (m_state == ElmoState::READY) {

    //! transition to OPERATIONAL
    if (m_reqState == ElmoState::OPERATIONAL) {
      
      // Wait until velocity of the joint is zero
      // motor can not be turned on if the joint moves (e.g. due to alignment on other joints)
      if (m_currentVelocity == 0) {
        // switch to the OPERATIONAL state
        m_controlWord = enableOperationCmd();
        
#ifdef EL_STM_VERBOSE
        printState();
        pdbgSlave("STM transition: %s\n", "READY -> OPERATIONAL");
#endif
      }
   
      return;
    }
 
    //! transition to HOMED
    // -> transit to HOMING_READY first
    if (m_reqState == ElmoState::HOMED) {
    
      m_preHoming = false;
      m_ackHoming = false;
      m_homingDone = false;
      m_zeroingDone = false;
    
      // set the mode of operation accordingly
      m_modeOfOperation.setRequestedState(ELMO_MOO_HOMING);
    
#ifdef EL_STM_VERBOSE
      printState();
      pdbgSlave("STM transition: %s\n", "READY -> HOMING_READY");
#endif
      return;
    
    }

  }


  //! **************************************************************  from HOMING_READY -> HOMING_OPERATIONAL
  if (m_state == ElmoState::HOMING_READY && m_reqState == ElmoState::HOMED) {

    // transition to HOMING_OPERATIONAL
    
    if (m_noMotorMotion) {
      // motor motion disabled, stay in READY
      m_controlWord = switchOnCmd();
    } else {
      
      // Wait until velocity of the joint is zero
      // motor can not be turned on if the joint moves (e.g. due to alignment on other joints)
      if (m_currentVelocity == 0) {
        // switch to the OPERATIONAL state
        m_controlWord = enableOperationCmd();
        
#ifdef EL_STM_VERBOSE
        printState();
        pdbgSlave("STM transition: %s\n", "HOMING_READY -> HOMING_OPERATION");
#endif
      }
    }

    return;

  }

  //! **************************************************************  from OPERATIONAL -> READY
  if (m_state == ElmoState::OPERATIONAL && m_reqState == ElmoState::READY) {
    
    if (quickStopActive()) {
      // quick stop can only transition to the switch on disabled state
      // recover from there
      
      // disable voltage -> SWITCH ON DISABLED
      m_controlWord = disableVoltageCmd();
    
#ifdef EL_STM_VERBOSE
      printState();
      pdbgSlave("STM transition: %s\n", "QUICKSTOP -> READY");
#endif
      return;
    } else {
  
      // switch to SWITCHED ON state
      m_controlWord = disableOperationCmd();
#ifdef EL_STM_VERBOSE
      printState();
      pdbgSlave("STM transition: %s\n", "OPERATIONAL -> READY");
#endif
      return;
    }

  }

  //! **************************************************************  from HOMING_OPERATIONAL -> HOMED
  if (m_state == ElmoState::HOMING_OPERATIONAL && m_reqState == ElmoState::HOMED) {
    
    // this requires acknowledgement with ackHoming() function
    // set preHoming boolean to signal device readyness for homing operation
    if (m_ackHoming) {
      
      if (m_homingOffset.requestedStateReached()) {

        if (m_noMotorMotion) {
          
          // set the homing flag, but stay in READY, if motion is not allowed
          m_controlWord = homingCmd() | switchOnCmd();
          
        } else {
          
          // set the homing flag to activate actual homing on elmo
          m_controlWord = homingCmd() | enableOperationCmd();
          
        }
  
#ifdef EL_STM_VERBOSE
        printState();
        pdbgSlave("STM transition: %s\n", "HOMING_OPERATION -> HOMED");
#endif
        
      }
      return;
      
    } else {
      
      // set pre homing flag and wait for acknowledgement of the bus slave implementation
      m_preHoming = true;

      return;
    }
  
  }
  
  //! **************************************************************  from HOMING_ZEROING -> HOMED
  if (m_state == ElmoState::HOMING_ZEROING && m_reqState == ElmoState::HOMED) {

    // stay in OPERATIONAL
    m_controlWord = enableOperationCmd();
    
    if (!m_modeOfOperation.requestedStateReached() || m_modeOfOperation.getState() != ELMO_MOO_PROFILE_POSITION) {
        
      // change mode to profile position
      m_modeOfOperation.setRequestedState(ELMO_MOO_PROFILE_POSITION);  
        
    } else if (m_profileVelocity.requestedStateReached()) {
     
      // operational, new setpoint, immediate setpoint
      m_controlWord = enableOperationCmd() | newSetPoint() | immediateSetPoint();

    }


#ifdef EL_STM_VERBOSE
        printState();
        pdbgSlave("STM transition: %s\n", "HOMING_ZEROING -> HOMED");
#endif
        
    return;

  }


  //! **************************************************************  from HOMED ->
  //! See transition to IDLE


  // In this case an invalid state transition was requested
  printState();
  perrSlave("STM: Requested invalid transition from %i -> %i\n", m_state, m_reqState);
  
  // Indicate a FAULT and send shutdownCmd()
  m_controlWord = shutdownCmd();
  m_state = ElmoState::FAULT;
  

  return;
}

// ===============
// = updateState =
// ===============
void ElmoStateMachine::updateState() {
  
  // If we want to switch to OPERATIONAL but are not yet there -> COMMUTATION
  // The previously computed state must be different from OPERATIONAL
  if (m_state != ElmoState::OPERATIONAL && 
    m_controlWord == enableOperationCmd() && 
    !operationEnabled() && !faultReactionActive() 
    && !fault() && !quickStopActive()) {

    m_state = ElmoState::COMMUTATION;

  } else if (switchOnDisabled() || readyToSwitchOn()) {

    if (m_modeOfOperation.getState() == ELMO_MOO_HOMING) {
      
      if (m_homingDone) {
        // Homing attained...

        if (m_homingMethod.getRequestedState() != ElmoHomingType::ABS_ENCODER) {
    
          // We have no abs encoders, consider this state as HOMING_ZEROING
          m_state = ElmoState::HOMING_ZEROING;
      
        } else {
          // homing attained and operation switched off
          m_state = ElmoState::HOMED;
          m_zeroingDone = true;
        }

      } else {

        // Homing not attained
        m_state = ElmoState::HOMING_IDLE;
      }
      
    } else if (m_modeOfOperation.getState() == ELMO_MOO_PROFILE_POSITION) {
    
      // HOMING_IDLE
      m_state = ElmoState::HOMING_IDLE;
      
      // homing done?
      if (m_homingDone) {
        
        // We are moving to the target
        
        // target reached?
        if (m_zeroingDone) {
          m_state = ElmoState::HOMED;
        }
        
      }
      
    } else {
      
      // IDLE
      m_state = ElmoState::IDLE;
    }

  } else if (switchedOn()) {
  
    if ((m_modeOfOperation.getState() == ELMO_MOO_HOMING || 
          m_modeOfOperation.getState() == ELMO_MOO_PROFILE_POSITION) &&
           m_homingMethod.requestedStateReached()) {

      if (m_noMotorMotion) {
        
        // Motion is not allowed, switchedOn state is assumed to be OPERATIONAL
        m_state = ElmoState::HOMING_OPERATIONAL;
        
        if (bit12() && !bit13() && (m_controlWord & homingCmd())) {
          // homing attained
          m_homingDone = true;
          m_zeroingDone = true;
          m_state = ElmoState::HOMED;
        }

      } else {
        
        // HOMING READY
        m_state = ElmoState::HOMING_READY;
      }
    
    } else {

      // READY
      m_state = ElmoState::READY;
    }
  
  } else if (operationEnabled() || quickStopActive()) {
  
    if (m_modeOfOperation.getState() == ELMO_MOO_HOMING && 
            m_homingMethod.requestedStateReached()) {

      // HOMING_OPERATIONAL
      m_state = ElmoState::HOMING_OPERATIONAL;

      if ((bit12() && !bit13() && (m_controlWord & homingCmd())) || m_homingDone) {

        // The actual homing is done
        m_homingDone = true;

        if (m_homingMethod.getRequestedState() != ElmoHomingType::ABS_ENCODER) {
          // Zeroing required, state is HOMING_ZEROING 
          m_state = ElmoState::HOMING_ZEROING;

        } else {

          // homing attained
          m_state = ElmoState::HOMED;
          m_zeroingDone = true;

        }
      }

    } else if (m_modeOfOperation.getState() == ELMO_MOO_PROFILE_POSITION &&
                m_homingMethod.requestedStateReached()) {

      // HOMING_ZEROING
      m_state = ElmoState::HOMING_ZEROING;
      
      // target reached and homing acquired?
      if ((targetReached() && bit12() && !bit13() && m_homingDone) || (m_homingDone && m_zeroingDone)) {
 
        m_zeroingDone = true;
        m_state = ElmoState::HOMED;
      }

    }
    else {
    
      // OPERATIONAL
      m_state = ElmoState::OPERATIONAL;
      
      
      // If we get a following error or similar, change to
      // FAULT state
      if (bit13()) {
        perrSlave("Tracking error on drive!\n");
        m_state = ElmoState::FAULT;
      } else if (!bit12()) {
        pwrnSlave("Drive does not follow command!\n");
      }
      
    }

  } else if (notReadyToSwitchOn() && !fault()) {
    
    // INIT
    m_state = ElmoState::INIT;

  } else {
    
    // FAULT
    m_state = ElmoState::FAULT;

  }
  
  static LogRateLimiter limitWarnings(EL_STM_MAX_MSGS_PER_ERROR, EL_STM_REDUCED_LOG_RATE);
  static LogRateLimiter limitActiveLimit(EL_STM_MAX_MSGS_PER_ERROR, EL_STM_REDUCED_LOG_RATE);
  
  // check for warnings
  if (warning()) {
    limitWarnings.count();
    if (limitWarnings.log()) {
      perrSlave("Warning bit enabled! This might be caused by\n\tOvertemperature/"
                         "Over-/Undervoltage/\n\tAnalog encoder amplitude low threshold exceeded/"
                         "\n\tEnd at encoder warning is received\n");
    }
  }
  
  // check for internal limits
  if (limitActive()) {
    limitActiveLimit.count();
    if (limitActiveLimit.log()) {
      perrSlave("Internal limitation of the drive active! (Endstops / Internal Position Limits)\n");
    }
    
  }

  // state changed?
  if (m_state != m_stateOld) {
    
#ifdef EL_STM_VERBOSE
    pdbgSlave("--------------------------------------------\n");
    pdbgSlave("STM state changed: %s -> %s\n", convertElmoStateToStr(m_stateOld).c_str(), convertElmoStateToStr(m_state).c_str());
    printState();
#endif
    m_stateOld = m_state;
  }

}
