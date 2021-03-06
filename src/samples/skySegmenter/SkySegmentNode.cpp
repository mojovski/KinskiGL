//
//  SkySegmentNode.cpp
//  gl
//
//  Created by Fabian Schmidt on 6/25/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "SkySegmentNode.h"

using namespace cv;
using namespace std;
using namespace kinski;

SkySegmentNode::SkySegmentNode():
m_cannyLow(RangedProperty<int>::create("Canny Low", 85, 0, 255)),
m_cannyHigh(RangedProperty<int>::create("Cany High", 208, 0, 255)),
m_morphKernSize(RangedProperty<int>::create("Morph kernel size", 3, 3, 15)),
m_threshVal(RangedProperty<int>::create("Intesity threshold", 140, 0, 255)),
m_minPathLength(RangedProperty<int>::create("Minimal path length", 200, 0, 1024))
{
    registerProperty(m_cannyLow);
    registerProperty(m_cannyHigh);
    registerProperty(m_morphKernSize);
    registerProperty(m_threshVal);
    registerProperty(m_minPathLength);
}

SkySegmentNode::~SkySegmentNode()
{

}

vector<Mat> SkySegmentNode::doProcessing(const Mat &img)
{
    Mat scaledImg, grayImg, threshImg,
    edgeImg, skyMask;
    
    float scale = 0.2f;
    
    // scale down input image
    cv::resize(img, scaledImg, Size(0,0), scale, scale);
    
    cv::cvtColor(scaledImg, grayImg, cv::COLOR_BGR2GRAY);
    
    // compute dynamic and static thresholds
    Mat tmpThresh;
    cv::threshold(grayImg, threshImg, *m_threshVal, 255,
                  THRESH_BINARY_INV | THRESH_OTSU);
    cv::threshold(grayImg, tmpThresh, *m_threshVal, 255,
                  THRESH_BINARY_INV);
    
    threshImg &= tmpThresh;
    
    // edge detection
    cv::blur(grayImg, grayImg, cv::Size(3, 3));
    cv::Canny(grayImg, edgeImg, *m_cannyLow, *m_cannyHigh);
    
    Mat workImg;
    Size kernSize = getValidKernSize(*m_morphKernSize);
    *m_morphKernSize = kernSize.width;
    
    // for all columns. when a value != 0 is encountered,
    // all values from the current row down to the bottom are set to 255
    for (int i = 0; i < edgeImg.cols; i++)
    {
        Mat col = edgeImg.col(i);
        
        int hitCount = 0;
        
        for (int j = 0; j < col.rows; j++)
        {
            // iterate pixels in column
            if( col.at<uint8_t>(j, 0) )
            {
                if(!hitCount)
                {
                    col.rowRange(0, j) = 0.0;
                }
                
                ++hitCount;
                
                if(hitCount > 5)
                {
                    
                    col.rowRange(j, col.rows) = 255.0;
                    break;
                }
            }
        }
    }
    cv::dilate(edgeImg,
               workImg,
               cv::getStructuringElement(MORPH_ELLIPSE, kernSize ),
               Point(-1,-1),
               2);
    
    morphologyEx(threshImg,
                 threshImg,
                 MORPH_CLOSE,
                 cv::getStructuringElement(MORPH_ELLIPSE,kernSize),
                 Point(-1,-1),
                 2);
    
    workImg |= threshImg;
    
    
    
//    vector<vector<Point> > contours;
//    vector<Vec4i> hierarchy;
//    vector<vector<Point> > contours0;
//    
//    // find contours on sky image
//    Mat contourImg = workImg.clone();
//    findContours( contourImg, contours0, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
//    
//    // find contour description ?
//    std::vector<cv::Point> poly;
//    //approxPolyDP(contours0, poly, 5, true);
//    
//    int minBound = 250;
//    vector<vector<Point> >::iterator contIt = contours0.begin();
//    for(;contIt != contours0.end();contIt++)
//    {
//        Rect bound = boundingRect(*contIt);
//        if(bound.area() > minBound * minBound)
//        {
//            contours.push_back(*contIt);
//        
//            //rectangle(outMat, bound, Scalar(255, 0, 0), 3);
//        }
//        else
//        {
//            // fill area with black
//            //workImg(bound) = 0.f;
//        }
//    }
    
    //drawContours(outMat, contours, 0, Scalar(128, 255, 255), 3);
    vector<Mat> outMats;
    
    outMats.push_back(edgeImg);
    outMats.push_back(workImg);
    
    return outMats;
}

cv::Mat SkySegmentNode::combineImages(const cv::Mat &img1, const cv::Mat &img2)
{
    Mat outMat = Mat(img1.size(), CV_8UC1);
    
    Rect left(0, 0, img1.cols / 2, img1.rows);
    Rect right(img1.cols / 2 , 0, img1.cols / 2, img1.rows);
    
    Mat tmpLeft = img1(left), tmpRight = img2(right);
    
    if(tmpLeft.channels() == 1) cvtColor(tmpLeft, tmpLeft, CV_GRAY2BGR);
    
    if(tmpRight.channels() == 1) cvtColor(tmpRight, tmpRight, CV_GRAY2BGR);
    
    outMat(left) = tmpLeft;
    outMat(right) = tmpRight;
    
    return outMat;
}

cv::Size SkySegmentNode::getValidKernSize(const uint32_t sz)
{
    int dim = sz;
    
    if(! (dim%2))
        dim--;
    dim = std::max(3, dim);
    
    return Size(dim, dim);
}
