//
//  CVNodes.h
//  kinskiGL
//
//  Created by Fabian Schmidt on 6/12/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "opencv2/opencv.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/thread.hpp"

namespace kinski{
    
class CVNode
{
public:
    typedef boost::shared_ptr<CVNode> Ptr;
    virtual std::string getName() = 0;
    virtual std::string getDescription() = 0;
};
    

class CVSourceNode : public CVNode
{
public:
    typedef boost::shared_ptr<CVSourceNode> Ptr;
    
    // inherited from INode
    virtual std::string getName(){return "Instance of CVSourceNode";};
    virtual std::string getDescription(){return "Generic Input-source";};
    
    virtual bool getNextImage(cv::Mat &img) = 0;
};

class CVBufferedSourceNode : public CVSourceNode 
{    
public:
    CVBufferedSourceNode(const CVSourceNode::Ptr srcNode):
    m_sourceNode(srcNode)
    {};
    
private:
    
    boost::thread m_thread;
    boost::mutex m_mutex;
    CVSourceNode::Ptr m_sourceNode;
};

class CVProcessNode : public CVNode
{
public:
    typedef boost::shared_ptr<CVProcessNode> Ptr;
    
    // inherited from INode
    virtual std::string getName(){return "Instance of CVProcessNode";};
    virtual std::string getDescription(){return "Generic processing node";};
    
    virtual cv::Mat doProcessing(const cv::Mat &img) = 0;
};
    
class CvCaptureNode : public CVSourceNode
{
public:
    typedef boost::shared_ptr<CvCaptureNode> Ptr;
    
    CvCaptureNode(const int camId);
    CvCaptureNode(const std::string &movieFile);
    virtual ~CvCaptureNode();
    
    virtual std::string getName();
    virtual std::string getDescription();
    
    bool getNextImage(cv::Mat &img);
    
    // capture interface
    int getNumFrames();
    void jumpToFrame(const unsigned int newIndex);
    void setLoop(bool b);
    
    float getFPS();
    
private:
    cv::VideoCapture m_capture;
    
    std::string m_videoSource;
    std::string m_description;

    int m_numFrames;
    float m_captureFPS;
    bool m_loop;
};
    
}// namespace kinski