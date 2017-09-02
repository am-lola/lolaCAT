//
//  EL1012Device.hpp
//  am2b
//
//  Created by Felix Sygulla on 2015-08-13.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//

#ifndef EL1012DEVICE_HPP_ACDDF772
#define EL1012DEVICE_HPP_ACDDF772

#include "BusVar.hpp"

namespace ec {

  template<class PipedInterface>
  class EL1012Device : public PipedInterface {

  public:
    

  
    /*! Get the input with 
      \param index */
    bool getInput(int index) {
    
      // invalid index
      if (index > 3 || index < 0) {
        pwrn_ffl("Index out of range\n");
        return false;
      }
    
      // assign the value to the Bus data type
      return m_in[index];
    
    }
  
  
  private:
    
    //!init in bus operational state
    void initOp(){}
    
    //!process new data
    void process(){
    }
    
    void link() {
    
      // link the pdo vars to the Bus var types
      this->linkPDOVar("Channel 1.Input", &m_in[0]);
      this->linkPDOVar("Channel 2.Input", &m_in[1]);
    }
    
    //!init before Bus operation
    void init(){}
  
    BusBool<BusInput>      m_in[2];

  };

}

#endif /* end of include guard: EL2004DEVICE_HPP_ACDDF772 */
