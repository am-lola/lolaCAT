//
//  EL3104Device.hpp
//  am2b
//
//  Created by Felix Sygulla on 2015-08-13.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//

#ifndef EL3104DEVICE_HPP_ACDDF772
#define EL3104DEVICE_HPP_ACDDF772

#include "BusVar.hpp"

namespace ec {

  template<class PipedInterface>
  class EL3104Device : public PipedInterface {

  public:
    

  
    /*! Get the input with 
      \param index */
    int16_t getInputRaw(int index) {
    
      // invalid index
      if (index > 3 || index < 0) {
        pwrn_ffl("Index out of range\n");
        return false;
      }
    
      // assign the value to the Bus data type
      return m_in[index];
    
    }


    /*! Get the input in the range [-10V,+10V] with 
      \param index */
    float getInput(int index) {
    
      // invalid index
      if (index > 3 || index < 0) {
        pwrn_ffl("Index out of range\n");
        return false;
      }

      if (m_in[index] > 0) {
          // 0...32767
          return 10.0*((float) m_in[index])/(32768-1);
      } else {
          // -32768...0
          return 10.0*((float) m_in[index])/(32768);  
          
      }
    
    }
  
  
  private:
    
    //!init in bus operational state
    void initOp(){}
    
    //!process new data
    void process(){
    }
    
    void link() {
    
      // link the pdo vars to the Bus var types
      this->linkPDOVar("AI Standard Channel 1.Value", &m_in[0]);
      this->linkPDOVar("AI Standard Channel 2.Value", &m_in[1]);
      this->linkPDOVar("AI Standard Channel 3.Value", &m_in[2]);
      this->linkPDOVar("AI Standard Channel 4.Value", &m_in[3]);
    }
    
    //!init before Bus operation
    void init(){}
  
    BusInt16<BusInput>      m_in[4];

  };

}

#endif /* end of include guard: EL2004DEVICE_HPP_ACDDF772 */
