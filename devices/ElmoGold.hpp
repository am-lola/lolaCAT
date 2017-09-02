//
//  ElmoGold.hpp
//  am2b
//
//  Created by Felix Sygulla on 2015-11-03.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//

#ifndef ELMOGOLD_HPP_C36227A8
#define ELMOGOLD_HPP_C36227A8

#include <math.h>

#include "BusVar.hpp"
#include "ElmoStateMachine.hpp"
#include "SDOQueue.hpp"
#include "BusException.hpp"
#include "ElmoErrorCodes.hpp"

// Delay between consecutive SDO transfers for one Elmo
// Unit: Milliseconds
#define HWL_EC_ELMO_SDO_DELAY 160

#define HWL_EC_ELMO_FIR_FILTER_LENGTH 8

#undef HWL_EC_ELMO_DISABLE_ACC_FF
#undef HWL_EC_ELMO_DISABLE_VEL_FF

namespace ec {

  /* Elmo Gold Device implementation */
  template<class PipedInterface>
  class ElmoGold : public PipedInterface {


  public:
  
    /*! ElmoGold constructor
  
        \param homingMethod Type of homing
        \param countsIncEncoder Number of counts/rev for the incremental encoder
        \param countsAbsEncoder Number of counts/rev for the absolute encoder
        \param ratedCurrent Rated current of the motor in mA. Used for torque feedforward.
        \param no_motor_motion Do not allow motion of the motors, but simulate elmo states (for testing)
    */
    ElmoGold(const ElmoHomingType& homingMethod,
             unsigned int countsIncEncoder,
             unsigned int countsAbsEncoder,
             const uint32_t& ratedCurrent,
             const bool& no_motor_motion = false,
             const int32_t& posLimitMin = 0,
             const int32_t& posLimitMax = 0)
                                    : m_stm(homingMethod, no_motor_motion), 
                                      m_sdoQueue((void*) this, &ElmoGold<PipedInterface>::asyncSendSDO, &ElmoGold<PipedInterface>::asyncReceiveSDO),
                                      m_ratedCurrent(true,false) {
      m_desPosition = 0;
      m_controlWord = 0;
      m_velocityOffset = 0;
      m_torqueOffset = 0;
      m_countsIncEncoder = countsIncEncoder;
      m_countsAbsEncoder = countsAbsEncoder;

      // Set initial values for pos limits, but do not send them right away
      m_posLimitMin.setInitialRequestedState(posLimitMin);
      m_posLimitMax.setInitialRequestedState(posLimitMax);

      // Set initial value for rated current to check consistency with Elmo value
      m_ratedCurrent.setInitialRequestedState(ratedCurrent);
    }
    
    /*! Set the requested state of the elmo 
       The request of a FAULT-state (e.g. if other devices on the bus fail)
       leads to internal transition to IDLE mode. getState() always reflects the
       "true" state of the elmo.
    */
    void setRequestedState(const ElmoState& state) {
      
      // Ensure smooth transitions
      if (state == ElmoState::OPERATIONAL && m_stm.getRequestedState() != ElmoState::OPERATIONAL) {
        
        // read position and set as desired
        m_desPosition = m_position;
        
      }

      // If homing takes place, set the desired position to zero for limit switch homing
      if(state == ElmoState::HOMED && m_stm.getRequestedState() != ElmoState::HOMED && m_stm.getAsyncHomingMethod()->getRequestedState() != ElmoHomingType::ABS_ENCODER) {
        m_desPosition = 0;
      }

      
      if (state == ElmoState::FAULT) {
        // fault request -> go back to IDLE internally
        m_stm.setRequestedState(ElmoState::IDLE);

      } else {
        m_stm.setRequestedState(state);
      }

    }

    /*! Returns the simplified state of the Elmo device */
    ElmoState getState() {
      return m_stm.getState();
    }
    
    /*! Returns true if the requested state has been reached */
    bool stateReached() {
      return m_stm.getState() == m_stm.getRequestedState();
    }
    
    /*! Return the raw actual position of the inc. encoder (ticks) */
    int32_t getPositionRaw() {
      return m_position;
    }

    /*! Returns the actual inc. encoder position of the drive (rad) */
    double getPosition() {
      return ((double) m_position) / (double)m_countsIncEncoder * 2.0 * M_PI;
    }
    
    /*! Return the actual velocity (inc. encoder, tics/s) */
    int32_t getVelocityRaw() {
      return m_velocity;
    }

    /*! Return the actual velocity (inc. encoder, rad/s) */
    double getVelocity() {
      return ((double) m_velocity) / (double)m_countsIncEncoder * 2.0 * M_PI;
    }
    
    /*! Return the actual current for the drive in A */
    double getCurrent() {
      return ((double)(m_current)*(double)(m_ratedCurrent.getState())*0.000001);
    }
    
    /*! Return the raw absolute encoder position (ticks) */
    int32_t getAbsPositionRaw() {
      return m_absPosition;
    }
    
    /*! Return the absolute encoder position in rad */
    double getAbsPosition() {
      return ((double) m_absPosition) / (double)m_countsAbsEncoder * 2.0 * M_PI;
    }

    /*! Set the demanded position of the drive (inc. encoder, rad) */
    void setDesiredPosition(double pos) {
      m_desPosition = (int32_t) (pos * ((double) m_countsIncEncoder) / 2.0 / M_PI);
    }
    
    /*! Set the demanded position of the drive (inc. encoder, ticks) */
    void setDesiredPositionRaw(int32_t pos) {
      m_desPosition = pos;
    }
    
    /*! Set a velocity offset for position control loop (feedforward, ticks/s) */
    void setVelocityOffset(double velOffset) {
      m_velocityOffset = (int32_t) (velOffset * ((double) m_countsIncEncoder) / 2.0 / M_PI);
    }

    /*! Set a velocity offset for position control loop (feedforward, ticks/s) */
    void setVelocityOffsetRaw(int32_t velOffset) {
      m_velocityOffset = velOffset;
    }
    
    /*! Set a current offset for the position control loop (feedforward)
        Unit: Current in A
     */
    void setCurrentOffset(double currOffset) {
      m_torqueOffset = (int16_t) (m_useTorqueOffset*currOffset*1000000.0/(double)(m_ratedCurrent.getState()));
    }

    int16_t getCurrentOffset()
    {
      return m_torqueOffset;
    }

    /*! Return true if a (fatal) fault occured.
      Safety reactions need to be taken in this case */
    bool onFault() {
      #ifdef HWL_EC_DISABLE_SLAVE_FAULT_REACTION
        perrSlave("Fault Reaction disabled!\n");
        return false;
      #else
        return (PipedInterface::onFault() || m_stm.getState() == ElmoState::FAULT);
      #endif
    }

    /*! Reset fault */
    void resetFault() {
      m_waitFault = 0;
      m_errorMsgShown = false;
      m_stm.resetFault();
      PipedInterface::resetFault();
    }

    /*! Homing done? */
    bool homingDone() {
      return m_stm.homingComplete();
    }

    /*! Pre-Homing flag
       This flag goes true right before the transition to the HOMING state.
       The state machine waits until the device implementation has acknowledged (ackHoming()) the setting of
       required variables for the homing process until it resumes the transition.
      */
    bool preHoming() {
      return m_stm.preHoming();
    }

    /*! acknowledge homing (see Pre-Homing flag) */
    void ackHoming() {
      m_stm.ackHoming();
    }

    /*! 
      Sets the homing offset (start value) for the encoder registered at the "homing socket".
      This must be done before ackHoming() is called!
      Unit: rad
     */
    void setHomingOffset(const double& calibOffset) {
      // The Elmo inverts the offset!!
      m_stm.setHomingOffset((int32_t) (-calibOffset * ((double) m_countsIncEncoder) / 2.0 / M_PI));
    }

    /*!
      Sets the homing offset (start value) for the encoder registered at the "homing socket".
      This must be done before ackHoming() is called!
      Unit: Ticks
    */
    void setHomingOffsetRaw(const int32_t& ticksOffset) {
      // The Elmo inverts the offset
      m_stm.setHomingOffset(-ticksOffset);
    }

    /*! Set the control gains for this Elmo.
      
      The parameters saved on the drive are used if this method is never called.
    */
    void setControlGains(float currentGainP, float currentGainI, 
                         float velocityGainP, float velocityGainI, float positionGainP) {
      
      // Tell SDO Queue to update via SDOs
      m_currentGainP.setRequestedState(currentGainP);
      m_currentGainI.setRequestedState(currentGainI);
      m_velocityGainP.setRequestedState(velocityGainP);
      m_velocityGainI.setRequestedState(velocityGainI);
      m_positionGainP.setRequestedState(positionGainP);
      
    }
    void setPContKP(float val_) {
      // Tell SDO Queue to update via SDOs
      pdbg("setting pos kp = %e for joint %s\n",val_,this->getName().c_str());
      m_positionGainP.setRequestedState(val_);
    }
    void setVContKP(float val_) {
      // Tell SDO Queue to update via SDOs
      pdbg("setting vel kp = %e for joint %s\n",val_,this->getName().c_str());
      m_velocityGainP.setRequestedState(val_);
    }
    void setVContKI(float val_) {
      // Tell SDO Queue to update via SDOs
      pdbg("setting vel ki = %e for joint %s\n",val_,this->getName().c_str());
      m_velocityGainI.setRequestedState(val_);
    }
    void setIContKP(float val_) {
      // Tell SDO Queue to update via SDOs
      pdbg("setting cur kp = %e for joint %s\n",val_,this->getName().c_str());
      m_currentGainP.setRequestedState(val_);
    }
    void setIContKI(float val_) {
      // Tell SDO Queue to update via SDOs
      pdbg("setting cur ki = %e for joint %s\n",val_,this->getName().c_str());
      m_currentGainI.setRequestedState(val_);
    }
    void setIContFF(float val_) {
      pdbg("setting cur ff = %e for joint %s\n",val_,this->getName().c_str());
      m_useTorqueOffset=(bool)(val_);
    }
    
    /*! Returns true if the gains set via setControlGains() have been
        successfully sent to the Elmo */
    bool controlGainsUpdated() {
      return (m_currentGainP.requestedStateReached() && m_currentGainI.requestedStateReached() 
              && m_velocityGainP.requestedStateReached() && m_velocityGainI.requestedStateReached() 
              && m_positionGainP.requestedStateReached());
    }
    
    /*! Sets hardware position limits on the elmo
        Unit: rads on motor side
    */
    void setPositionLimitsRad(const double& radMin, const double& radMax) {
      this->setPositionLimits( (int32_t) (radMin) * ((double) m_countsIncEncoder) / 2.0 / M_PI,
                                  (int32_t) (radMax) * ((double) m_countsIncEncoder) / 2.0 / M_PI);
    }
    
    /*! Sets hardware position limits on the Elmo
        Unit: Motor encoder ticks
    */
    void setPositionLimits(const int32_t& ticksMin, const int32_t& ticksMax) {
      m_posLimitMin.setRequestedState(ticksMin);
      m_posLimitMax.setRequestedState(ticksMax);
    }

    /*! Return dc link voltage
     */
    double getDCLinkVoltage() {
      return ((double) m_voltage) / 1000;
    }
  
  private:
    
    //!init before bus operation
    void init(){
      m_sdoQueue.setDelay(HWL_EC_ELMO_SDO_DELAY*1000/this->getMaster()->getBusCycleTimeUs());
      
      // Add STM states
      m_sdoQueue.addAsyncState(m_stm.getAsyncHomingMethod());
      m_sdoQueue.addAsyncState(m_stm.getAsyncModeOfOperation());
      m_sdoQueue.addAsyncState(m_stm.getAsyncHomingOffset());
      m_sdoQueue.addAsyncState(m_stm.getAsyncProfileVelocity());
      m_sdoQueue.addAsyncState(m_stm.getAsyncHomingSpeed());
      m_sdoQueue.addAsyncState(m_stm.getAsyncHomingSpeedLow());
      
      // Add controller parameter states
      m_sdoQueue.addAsyncState(&m_positionGainP);
      m_sdoQueue.addAsyncState(&m_velocityGainP);
      m_sdoQueue.addAsyncState(&m_velocityGainI);
      m_sdoQueue.addAsyncState(&m_currentGainP);
      m_sdoQueue.addAsyncState(&m_currentGainI);
      
      // Add position limit states
      //m_sdoQueue.addAsyncState(&m_posLimitMin);
      //m_sdoQueue.addAsyncState(&m_posLimitMax);

      // Add FIR filter
      m_sdoQueue.addAsyncState(&m_firFilter);

      // Add acceleration/velocity feedforward
      m_sdoQueue.addAsyncState(&m_accFeedforward);
      m_sdoQueue.addAsyncState(&m_velFeedforward);

      // Add rated current
      m_sdoQueue.addAsyncState(&m_ratedCurrent);
    }

    //!init in bus operational state
    void initOp(){
    
      // set the desired position to the actual position
      m_desPosition = m_position;
      
      if (abs(m_absPosition) < 5 && m_stm.getAsyncHomingMethod()->getRequestedState() == ElmoHomingType::ABS_ENCODER) {
        
        pwrnSlave("Possibly invalid EnDat value - check cabling!\n");
        
      }
      
      // update pos limits
      // m_posLimitMax.renew();
      // m_posLimitMin.renew();

      m_firFilter.setRequestedState(HWL_EC_ELMO_FIR_FILTER_LENGTH);
#ifdef HWL_EC_ELMO_DISABLE_ACC_FF
      m_accFeedforward.setRequestedState(0);
#endif
#ifdef HWL_EC_ELMO_DISABLE_VEL_FF
      m_velFeedforward.setRequestedState(0);
#endif

      m_ratedCurrent.renew();
    }
  
    //! process new data
    void process(){
      
      // process SDO Queue
      m_sdoQueue.process();
      
      // update control word based on 
      // current state and the used state machine implementation
      m_stm.updateState(m_statusWord, m_modeOfOperation, m_velocity);

      // Check the rated current setting on the Elmo
      // This is important for the conversion of desired current offsets
      // when inverse dynamics model is used for feedforward compensation
      if (!m_ratedCurrent.requestedStateReached() && m_ratedCurrent.getState() != 0) {
        this->setFaultFlag();// Emergency shutdown
        perrSlave("Inconsistent rated current setting! Elmo: %d, ec_config.hpp: %d\n", m_ratedCurrent.getState(), m_ratedCurrent.getRequestedState());
        xabort();
      }

      // On fault state?
      if (m_stm.getState() == ElmoState::FAULT) {

        // check the emergency object
        if (m_emergency.newTransferDone()) {

          // print emergency status
          perrSlave("Emergency, %s\n", 
                    ElmoErrorCodes::getErrorStringEmergency(m_emergency.getErrorCode(), 
                                                            m_emergency.getErrorDataAt(0)).c_str(),
                    convertElmoStateToStr(m_stm.getRequestedState()).c_str(),
                    convertElmoStateToStr(m_stm.getState()).c_str());
          perrSlave("Position (act, req): %d, %d\n", (int) m_position, (int) m_desPosition);
          // print stm debugging info
          m_stm.printState();
          m_errorMsgShown = true;
          
        } else if (!m_errorMsgShown) {
          
          m_waitFault++;

          // Try to get the error code via SDO
          if (m_waitFault > 300 && !m_errorCode.transferInProgress()) {
            
            if (m_errorCode.newTransferDone()) {
              
              // print error status
              perrSlave("Fault without Emergency Object, %s, reqState=%s, state=%s\n", 
                        ElmoErrorCodes::getElmoErrorCodeDescription(m_errorCode).c_str(),
                        convertElmoStateToStr(m_stm.getRequestedState()).c_str(),
                        convertElmoStateToStr(m_stm.getState()).c_str());
              perrSlave("Position (act, req): %d, %d\n", (int) m_position, (int) m_desPosition);
	      
              // print stm debugging info
              m_stm.printState();
              m_errorMsgShown = true;
              
            } else {
              
              // trigger asyncReceive()
              if (this->asyncReceiveSDO(&m_errorCode)) {
                perrSlave("Error receiving error code via async SDO!\n");
              }
              
            }
            
          }
          
        }
      
      }
    
      // update stm control word
      m_controlWord = m_stm.getControlWord();

      
    }
    
    //! Method for linking PDO / async SDO variables
    void link() {
    
      this->linkPDOVar("Inputs.Status word", &m_statusWord);
      this->linkPDOVar("Inputs.Position actual value", &m_position);
      this->linkPDOVar("Inputs.Auxiliary position actual value", &m_absPosition);
      this->linkPDOVar("Inputs.Mode of operation display", &m_modeOfOperation);
      this->linkPDOVar("Inputs.Velocity actual value", &m_velocity);
      this->linkPDOVar("Inputs.Current actual value", &m_current);

      this->linkPDOVar("Outputs.Control word", &m_controlWord);
      this->linkPDOVar("Outputs.Target Position", &m_desPosition);
      this->linkPDOVar("Outputs.Velocity Offset", &m_velocityOffset);
      //this->linkPDOVar("Outputs.Target Velocity", &m_velocityOffset);
      this->linkPDOVar("Outputs.Torque Offset", &m_torqueOffset);

      // DC link circuit voltage
      this->linkPDOVar("Inputs.DC link circuit voltage", &m_voltage);


      // Link SDOs
      this->linkSDOVar(0x6060, 0, m_stm.getAsyncModeOfOperation()->getBusVarDesired());
      this->linkSDOVar(0x607C, 0, m_stm.getAsyncHomingOffset()->getBusVarDesired());
      this->linkSDOVar(0x607C, 0, m_stm.getAsyncHomingOffset()->getBusVarActual());
      this->linkSDOVar(0x6098, 0, m_stm.getAsyncHomingMethod()->getBusVarDesired());
      this->linkSDOVar(0x6098, 0, m_stm.getAsyncHomingMethod()->getBusVarActual());
      
      this->linkSDOVar(0x6081, 0, m_stm.getAsyncProfileVelocity()->getBusVarDesired());
      this->linkSDOVar(0x6081, 0, m_stm.getAsyncProfileVelocity()->getBusVarActual());
      this->linkSDOVar(0x6099, 1, m_stm.getAsyncHomingSpeed()->getBusVarDesired());
      this->linkSDOVar(0x6099, 1, m_stm.getAsyncHomingSpeed()->getBusVarActual());
      this->linkSDOVar(0x6099, 2, m_stm.getAsyncHomingSpeedLow()->getBusVarDesired());
      this->linkSDOVar(0x6099, 2, m_stm.getAsyncHomingSpeedLow()->getBusVarActual());

      // KP
      this->linkSDOVar(0x3113, 3, m_positionGainP.getBusVarDesired());
      this->linkSDOVar(0x3113, 3, m_positionGainP.getBusVarActual());
      this->linkSDOVar(0x3113, 2, m_velocityGainP.getBusVarDesired());
      this->linkSDOVar(0x3113, 2, m_velocityGainP.getBusVarActual());
      this->linkSDOVar(0x3113, 1, m_currentGainP.getBusVarDesired());
      this->linkSDOVar(0x3113, 1, m_currentGainP.getBusVarActual());

      //KI
      this->linkSDOVar(0x310C, 2, m_velocityGainI.getBusVarDesired());
      this->linkSDOVar(0x310C, 2, m_velocityGainI.getBusVarActual());
      this->linkSDOVar(0x310C, 1, m_currentGainI.getBusVarDesired());
      this->linkSDOVar(0x310C, 1, m_currentGainI.getBusVarActual());

      // Position Limits
      this->linkSDOVar(0x607D, 1, m_posLimitMin.getBusVarDesired());
      this->linkSDOVar(0x607D, 1, m_posLimitMin.getBusVarActual());
      this->linkSDOVar(0x607D, 2, m_posLimitMax.getBusVarDesired());
      this->linkSDOVar(0x607D, 2, m_posLimitMax.getBusVarActual());

      // FIR filter
      this->linkSDOVar(0x3034, 71, m_firFilter.getBusVarDesired());
      this->linkSDOVar(0x3034, 71, m_firFilter.getBusVarActual());

      // Acceleration/Velocity feedforward
      this->linkSDOVar(0x3087, 1, m_accFeedforward.getBusVarDesired());
      this->linkSDOVar(0x3087, 1, m_accFeedforward.getBusVarActual());
      this->linkSDOVar(0x3087, 2, m_velFeedforward.getBusVarDesired());
      this->linkSDOVar(0x3087, 2, m_velFeedforward.getBusVarActual());

      // Rated Current
      this->linkSDOVar(0x6076, 0, m_ratedCurrent.getBusVarActual());
      
      // Error code
      this->linkSDOVar(0x306A, 0, &m_errorCode);
      
      // emergency object
      this->linkSDOVar(0,0, &m_emergency);

      // set the slave name to the stm for logging purposes
      m_stm.setSlaveName(this->getName());
    }
    
    //! Counts/rev inc. encoder
    unsigned int            m_countsIncEncoder;
    
    //! Counts/rev abs. encoder
    unsigned int            m_countsAbsEncoder;

    //! Elmo state machine
    ElmoStateMachine        m_stm;
    
    //! SDO Queue
    SDOQueue                m_sdoQueue;
    
    //! Did we already output an error message in case of a fault?
    bool                    m_errorMsgShown = false;
    
    //! wait counter for fault error prints
    int                     m_waitFault = 0;
    
    //! AsyncState Object for position control P-gain
    SDOAsyncState<float>    m_positionGainP;
    
    //! AsyncState Object for velocity P-Gain
    SDOAsyncState<float>    m_velocityGainP;
    
    //! AsyncState Object for velocity I-Gain
    SDOAsyncState<float>    m_velocityGainI;
    
    //! AsyncState Object for current P-Gain
    SDOAsyncState<float>    m_currentGainP;
    
    //! AsyncState Object for current I-Gain
    SDOAsyncState<float>    m_currentGainI;
    
    //! AsyncState Object for position limit min
    SDOAsyncState<int32_t>  m_posLimitMin;
    
    //! AsyncState Object for position limit max
    SDOAsyncState<int32_t>  m_posLimitMax;

    //! AsyncState Object for FIR filter
    SDOAsyncState<int32_t>  m_firFilter;

    //! AsyncState Object for acceleration feedforward
    SDOAsyncState<float>    m_accFeedforward;
    //! AsyncState Object for velocity feedforward
    SDOAsyncState<float>    m_velFeedforward;

    //! AsyncState Object for Motor rated current
    SDOAsyncState<uint32_t> m_ratedCurrent;

    /* PDO Variables on the bus */
  
    //! Control word
    BusUInt16<BusOutput>    m_controlWord;
  
    //! Status word
    BusUInt16<BusInput>     m_statusWord;
  
    //! Mode of operation
    BusInt8<BusInput>       m_modeOfOperation;

    //! Position actual value
    BusDInt<BusInput>       m_position;
    
    //! Velocity actual value
    BusDInt<BusInput>       m_velocity;
  
    //! Current actual value in 1/1000 rated current
    BusInt16<BusInput>      m_current;
  
    //! Position abs-encoder
    BusDInt<BusInput>       m_absPosition;
  
    //! Desired Position
    BusDInt<BusOutput>      m_desPosition;
    
    //! Velocity offset (feedforward)
    BusDInt<BusOutput>      m_velocityOffset;
    
    //! Current / (Torque) offset (feedforward) in 1/1000 rated torque/rated current
    BusInt16<BusOutput>     m_torqueOffset;
    //! enable/disable flag for current/torque offset
    bool m_useTorqueOffset=false;

    BusUInt32<BusInput>     m_voltage;
  
    /* Async SDO Variables on the bus */
    
    //! SDO emergency object
    BusVarEmergency         m_emergency;
    
    //! SDO Error code
    BusUDInt<BusInputSDO>   m_errorCode;

  };

}


#endif /* end of include guard: ELMOGOLD_HPP_C36227A8 */
