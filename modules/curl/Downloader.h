//
//  GOMTalking.h
//  MrProximity
//
//  Created by Fabian Schmidt on 11/20/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef kinski__Downloader_h_INCLUDED
#define kinski__Downloader_h_INCLUDED

#include "kinskiCore/Definitions.h"
#include <thread>
#include <mutex>
#include <condition_variable>


namespace kinski{ namespace net{
    
    struct ConnectionInfo
    {
        std::string url;
        double dl_total, dl_now, ul_total, ul_now;
    };
    
    class Downloader
    {
    public:
        
        static const long DEFAULT_TIMEOUT;
        
        typedef std::function<void(ConnectionInfo)> ProgressHandler;
        typedef std::function<void(ConnectionInfo, const std::vector<uint8_t>&)> CompletionHandler;
        
        Downloader();
        virtual ~Downloader();
        
        /*!
         * Download the resource at the given url (blocking) 
         */
        std::vector<uint8_t> getURL(const std::string &the_url);
        
        /*!
         * Download the resource at the given url (nonblocking)
         */
        void getURL_async(const std::string &the_url,
                          CompletionHandler ch,
                          ProgressHandler ph = ProgressHandler());
        
        long getTimeOut();
        void setTimeOut(long t);
        
        void run();
        
    private:
        
        typedef std::shared_ptr<class Action> ActionPtr;
        std::unique_ptr<struct Downloader_impl> m_impl;
        
        void stop(){};
        void addAction(const ActionPtr& theAction);
        
        std::deque<ActionPtr> m_actionQueue;
        unsigned int m_maxQueueSize;
        
        long m_timeout;
        
        // threading
        volatile bool m_stop;
        std::thread m_thread;
        std::mutex m_mutex;
        std::condition_variable m_conditionVar;
    };
    
}}// namespace
#endif
