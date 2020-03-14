//#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgcodecs.hpp>
#include "opencv2/objdetect.hpp"
#include "opencv2/imgproc.hpp"
#include "algorithms.h"

using namespace std;
using namespace cv;


int main() {
    /*cv::Mat src1 = cv::imread("om.jpg");
    imwrite( "om.jpeg", src1);
*/
    double alpha = 0.5;
    double beta;

    cv::Mat src1 = cv::imread("om.jpeg");
    cv::Mat src2 = cv::imread("doge.jpeg");

    src2 = crop(src2, 508, 395, 100);

    cv::Mat dst = src1;



    //namedWindow("Linear Blend", 1);
    beta = (1.0 - alpha);
    addWeighted(src1, alpha, src2, beta, 0.0, dst);



    imwrite( "omdoge.jpeg", dst);
    imshow("Linear Blend", dst);

    waitKey(0);


}