//
//  EL1012Device.hpp
//  am2b
//
//  Created by Felix Sygulla on 2015-08-13.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//

#ifndef EL2004DEVICE_HPP_ACDDF772
#define EL2004DEVICE_HPP_ACDDF772

#include "BusVar.hpp"

namespace ec {

  template<class PipedInterface>
  class EL2004Device : public PipedInterface {

  public:
    

  
    /*! Set the output with 
        \param index 
        to value 
        \param enabled*/
    void setOutput(int index, bool enabled) {
    
      // invalid index
      if (index > 3 || index < 0) {
        return;
      }
    
      // assign the value to the Bus data type
      m_out[index] = enabled;
    
    }
  
    //! Toggle the output at \param index
    void toggleOutput(int index) {
    
      // invalid index
      if (index > 3 || index < 0) {
        return;
      }
    
      m_out[index] = !m_out[index];
    
    }
  
  private:
    
    //!init in bus operational state
    void initOp(){}
    
    //!process new data
    void process(){}
    
    void link() {
    
      // link the pdo vars to the Bus var types
      this->linkPDOVar("Channel 1.Output", &m_out[0]);
      this->linkPDOVar("Channel 2.Output", &m_out[1]);
      this->linkPDOVar("Channel 3.Output", &m_out[2]);
      this->linkPDOVar("Channel 4.Output", &m_out[3]);
    }
    
    //!init before Bus operation
    void init(){}
  
    BusBool<BusOutput>      m_out[4];

  };

}

#endif /* end of include guard: EL2004DEVICE_HPP_ACDDF772 */
