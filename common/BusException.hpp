//
//  BusException.hpp
//  am2b
//
//  Created by Felix Sygulla on 2015-08-13.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//

#ifndef BUSEXCEPTION_HPP_24BBE8A4
#define BUSEXCEPTION_HPP_24BBE8A4

#include <exception>
#include <string>

namespace ec {

  /*! Simple exception class for BusExceptions */
  class BusException : public std::exception {
  
  public:
    /*! Constructor
  
        \param str The error message related to the exception
    */
    BusException(const std::string& str) {
      m_msg = "Bus Exception: ";
      m_msg.append(str);
    }
  
    ~BusException() throw() {};

    virtual const char* what() const throw() {
      return m_msg.c_str();
    }
  
  private:
  
    //! Message related to the exception
    std::string   m_msg;

  };

}

#endif /* end of include guard: BUSEXCEPTION_HPP_24BBE8A4 */
