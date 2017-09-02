//
//  LogRateLimiter.hpp
//  am2b
//
//  Created by Felix Sygulla on 2016-01-26.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//

#ifndef LOGRATELIMITER_HPP_8064D404
#define LOGRATELIMITER_HPP_8064D404

namespace ec {

  /*! Helper to limit the number of logged messages
      per event. Each instance of this class represents one event.
   */
  class LogRateLimiter {

  public:
  
    /*! 
    \param threshold If this nr of messages is exceeded, the log rate is reduced
    \param reducedRateFactor factor, by which log rate is reduced (10 -> every 10th report)
    */
    LogRateLimiter(uint32_t threshold, uint32_t reducedRateFactor) : m_threshold(threshold), m_reducedRateFactor(reducedRateFactor) 
    {}
    
    //! count the event
    void count() {
      m_counter++;
    
      if (m_counter == m_threshold + 1) {
        m_reachedThreshold = true;
        m_log = false;
        return;
      }
      
      // reached threshold should be true just for one
      // call
      m_reachedThreshold = false;
    
      if (m_counter > (m_threshold + 1) && (m_counter % m_reducedRateFactor != 0)) {
        m_log = false;
        return;
      }
      
      m_log = true;
    }
  
    //! Reset counter
    void reset() {
      m_counter = 0;
      m_reachedThreshold = false;
      m_log = true;
    }
    
    //! Reached threshold
    bool onLimit() {
      return m_reachedThreshold;
    }
    
    //! do log?
    bool log() {
      return m_log;
    }


  private:
  
    uint32_t  m_counter = 0;
    uint32_t  m_threshold;
    uint32_t  m_reducedRateFactor;

    bool      m_reachedThreshold = false;
    bool      m_log = true;

  };

}

#endif /* end of include guard: LOGRATELIMITER_HPP_8064D404 */