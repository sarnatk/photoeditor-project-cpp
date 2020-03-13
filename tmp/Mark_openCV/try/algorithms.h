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

cv::Mat brighten(const cv::Mat &img, int brightness = 0, int contrast = 0);

cv::Mat gray(const cv::Mat &img);

cv::Mat pink(const cv::Mat &img);

cv::Mat blend(const cv::Mat &img1, const cv::Mat &img2, double alpha);

#endif //OPENCVTEST_ALGORITHMS_H
