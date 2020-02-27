//
// Created by mark on 27.02.2020.
//

#ifndef OPENCVTEST_ALGORITHMS_H
#define OPENCVTEST_ALGORITHMS_H

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"


// boring crop
cv::Mat crop(const cv::Mat &img, int w, int h, int x = 0, int y = 0);

// superfun rotate
cv::Mat rotate_in_frame(const cv::Mat &img, int angle);

cv::Mat saturate(const cv::Mat &img, int saturation, double scale = 1);

#endif //OPENCVTEST_ALGORITHMS_H
