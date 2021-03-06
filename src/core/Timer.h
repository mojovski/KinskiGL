//
//  Timer.h
//  gl
//
//  Created by Croc Dialer on 27/07/14.
//
//

#ifndef __gl__Timer__
#define __gl__Timer__

#include "Definitions.h"

namespace kinski
{
    class Stopwatch
    {
    public:
        Stopwatch();
        
        void start();
        void stop();
        bool running();
        float time_elapsed();
        float time_elapsed_for_lap();
        void reset();
        void new_lap();
        const std::vector<float>& laps();

    private:
        struct stopwatch_impl;
        std::shared_ptr<stopwatch_impl> m_impl;
    };
    
    class Timer
    {
    public:
        typedef std::function<void(void)> Callback;
        
        Timer();
        Timer(boost::asio::io_service &io, Callback cb = Callback());
        Timer(float secs, boost::asio::io_service &io, Callback cb = Callback());
        
        /*!
         * set expiration date from now in seconds
         */
        void expires_from_now(float secs);
        
        /*!
         * get expiration date from now in seconds
         */
        float expires_from_now() const;
        
        /*!
         * returns true if the timer has expired
         */
        bool has_expired() const;
        
        /*!
         * cancel a currently running timer
         */
        void cancel();
        
        /*!
         * returns true if the timer is set to fire periodicly
         */
        bool periodic() const;
        
        /*!
         * set if the timer should fire periodicly
         */
        void set_periodic(bool b);
        
        /*!
         * set the function object to call when the timer fires
         */
        void set_callback(Callback cb);
        
    private:
        struct timer_impl;
        std::shared_ptr<timer_impl> m_impl;
    };
}


#endif /* defined(__gl__Timer__) */
