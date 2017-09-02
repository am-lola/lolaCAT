/*
  @file iface_ec_lola.hpp

  Ethercat device control ids

  Copyright  Institute of Applied Mechanics, TU-Muenchen
  All rights reserved.
*/

#ifndef __IFACE_EC_LOLA_HPP__
#define __IFACE_EC_LOLA_HPP__


//! Elmo states
/* The assigned integer value matches the importance of the states for
   comparison. (lower = more important)
 */
enum class ElmoState : uint16_t {
  
  /*! fault occured - depending on error handling the high voltage may be off */
  FAULT = 0,
  
  /*! INIT - This is the initial state before any state is read from the device - high voltage should be off! */
  INIT = 1,

  /*! idle state, which is restored after a fault reset or power up - high voltage should be off! */
  IDLE = 8,

  /*! Elmo in "On" State, which means the power amplifier is working, but the drive mode is still disabled */
  READY = 7,
  
  /*! Motor without hall sensors perform the commutation or alignment in this state */
  COMMUTATION = 2,

  /* Drive module activated, commands are executed */
  OPERATIONAL = 10,

  // ------------------------------------------------------------------------
  // The following states are special states for homing. They basically represent one of the 3 main states 
  // with additional state information about successful homing / and the state of the drive
  // ------------------------------------------------------------------------

  /*! In homing IDLE state - basically IDLE with activated homing mode.
    This is an (internal) intermediate state to reflect the Elmo modes */
  HOMING_IDLE = 3,

  /*! In homing READY state - Basically READY with activated homing mode. 
     This mode is an intermediate state before switching to HOMING_OPERATIONAL.*/
  HOMING_READY = 4,

  /*! In homing state. This is basically OPERATIONAL with the intention to achieve homing / alignment of the drive.
      During this first change to OPERATIONAL, the elmo might also execute the parametrized commutation search (alignment). The preHoming flag
      is set if this state is reached. Before actual homing command is executed, the user program has to call ackHoming() on the device
  */
  HOMING_OPERATIONAL = 5,
  
  /*! Homing occured, the joint moves to zero position before state changes to HOMED */
  HOMING_ZEROING = 6,

  /*! Homing acquired state. Elmo returns to this state, if homing is acquired. This corresponds to a READY state of the device */
  HOMED = 9
};

//! Function to get a string representation of an Elmo State
inline std::string convertElmoStateToStr(const ElmoState& state) {

  switch(state) {
  
    case ElmoState::FAULT:
    return std::string("FAULT");
    
    case ElmoState::INIT:
    return std::string("INIT");
  
    case ElmoState::IDLE:
    return std::string("IDLE");
  
    case ElmoState::READY:
    return std::string("READY");
    
    case ElmoState::COMMUTATION:
    return std::string("COMMUTATION");
        
    case ElmoState::OPERATIONAL:
    return std::string("OPERATIONAL");
  
    case ElmoState::HOMING_READY:
    return std::string("HOMING_READY");
  
    case ElmoState::HOMING_OPERATIONAL:
    return std::string("HOMING_OPERATIONAL");
    
    case ElmoState::HOMING_ZEROING:
    return std::string("HOMING_ZEROING");

    case ElmoState::HOMING_IDLE:
    return std::string("HOMING_IDLE");
  
    case ElmoState::HOMED:
    return std::string("HOMED");
  
    default:
    return std::string("UNKNOWN");
  }
}


#endif
