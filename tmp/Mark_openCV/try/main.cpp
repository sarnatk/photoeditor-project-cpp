#include <opencv2/opencv.hpp>
#include "algorithms.h"
//#include "opencv2/highgui/highgui.hpp"

int main() {
    //cv::Mat img = cv::imread(fin, CV_8UC1);
    cv::Mat img = cv::imread("om.jpg");


    double saturation = -300;
    double scale = 4;
    int angle = 0;

    for (int i = 0; i < 15000; i++) {
        // what it does here is dst = (uchar) ((double)src*scale+saturation);
        /*

        cv::Mat saturated = saturate(img, 0, scale);
        scale -= 0.005;


        cv::Mat dst;

        // horizontal
        cv::hconcat(img, saturated, dst);


        cv::imshow("both", dst);
*/
        cv::Mat dst2, dst3, dst4, flipped;

        auto rotated = rotate_in_frame(img, angle++);
        auto rotated2 = rotate_in_frame(img, -angle + 1);
        auto rotated3 = rotate_in_frame(img, angle++);
        auto rotated4 = rotate_in_frame(img, -angle + 1);


        cv::hconcat(rotated, rotated2, dst2);

        cv::flip(dst2, flipped, 0);

        cv::vconcat(dst2, flipped, dst4);


        cv::imshow("4", dst4);


        char k = cv::waitKey(1) & 0XFF;
        if (k == 27)
            break;
    }
    return 0;
}