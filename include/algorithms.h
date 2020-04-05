//
// Created by mark on 27.02.2020.
//

#ifndef OPENCVTEST_ALGORITHMS_H
#define OPENCVTEST_ALGORITHMS_H

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

/*
 *
 * first argument -- src image
 *
 * result -- return value
 *
 */

/*
 *
 * take picture using camera
 *
 */
cv::Mat takePicture(cv::VideoCapture cam);


/*
 * crops image
 *
 * result with width = w, height = h
 *
*/
cv::Mat crop(const cv::Mat &img, int w, int h, int x = 0, int y = 0);

/*
 *
 * rotates image by given angle without crop
 *
 */
cv::Mat rotate_in_frame(const cv::Mat &img, double angle);

/*
 *
 * saturates image
 *
 */
cv::Mat saturate(const cv::Mat &img, int saturation, double scale = 1);


/*
 *
 * brightens image
 *
 */
cv::Mat brighten(const cv::Mat &img, int brightness = 0, int contrast = 0);

/*
 *
 * black and white filter
 *
 */
cv::Mat gray(const cv::Mat &img);

/*
 *
 * pink filter
 *
 */
cv::Mat pink(const cv::Mat &img);

/*
 *
 * blue filter
 *
 */
cv::Mat blue(const cv::Mat &img);

/*
 *
 * green filter
 *
 */
cv::Mat green(const cv::Mat &img);

/*
 *
 * blends two images with given ratio
 *
 */
cv::Mat blend(const cv::Mat &img1, const cv::Mat &img2, double alpha);


/*
 *
 * sharpens image
 *
 */
cv::Mat Sharpen(const cv::Mat& myImage);

#endif //OPENCVTEST_ALGORITHMS_H
