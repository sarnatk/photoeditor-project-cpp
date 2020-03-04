//#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgcodecs.hpp>
#include "opencv2/objdetect.hpp"
#include "opencv2/imgproc.hpp"
#include "algorithms.h"

using namespace std;
using namespace cv;



cv::Mat takePicture() {
    cv::VideoCapture cam(0);
    cv::Mat pic;
    while (!cam.isOpened()) {
        std::cout << "Failed to make connection to cam" << std::endl;
        cam.open(0);
    }
    cam >> pic;
    return pic;
}


void detectAndDraw(Mat &img, CascadeClassifier &cascade,
                   CascadeClassifier &nestedCascade,
                   double scale) {
    vector<Rect> faces, faces2;
    Mat gray, smallImg;

    cvtColor(img, gray, COLOR_BGR2GRAY); // Convert to Gray Scale
    double fx = 1 / scale;

    // Resize the Grayscale Image
    resize(gray, smallImg, Size(), fx, fx, INTER_LINEAR);
    equalizeHist(smallImg, smallImg);

    // Detect faces of different sizes using cascade classifier
    cascade.detectMultiScale(smallImg, faces, 1.1,
                             2, false | CASCADE_SCALE_IMAGE, Size(30, 30));

    // Draw circles around the faces
    for (const auto& r : faces) {
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = Scalar(20, 48, 100); // Color for Drawing tool
        int radius;

        double aspect_ratio = (double) r.width / r.height;
        if (0.75 < aspect_ratio && aspect_ratio < 1.3) {
            center.x = cvRound((r.x + r.width * 0.5) * scale);
            center.y = cvRound((r.y + r.height * 0.5) * scale);
            radius = cvRound((r.width + r.height) * 0.25 * scale);
            circle(img, center, radius, color, 3, 8, 0);
        } else
            rectangle(img, cv::Point(cvRound(r.x * scale), cvRound(r.y * scale)),
                      cv::Point(cvRound((r.x + r.width - 1) * scale),
                                cvRound((r.y + r.height - 1) * scale)), color, 3, 8, 0);
        if (nestedCascade.empty())
            continue;
        smallImgROI = smallImg(r);

        // Detection of eyes int the input image
        nestedCascade.detectMultiScale(smallImgROI, nestedObjects, 1.1, 2,
                                       false | cv::CASCADE_SCALE_IMAGE, Size(30, 30));

        // Draw circles around eyes
        for (const auto& nr : nestedObjects) {
            center.x = cvRound((r.x + nr.x + nr.width * 0.5) * scale);
            center.y = cvRound((r.y + nr.y + nr.height * 0.5) * scale);
            radius = cvRound((nr.width + nr.height) * 0.25 * scale);
            circle(img, center, radius, color, 3, 8, 0);
        }
    }
    imshow("Face Detection", img);
}

int main() {

    // PreDefined trained XML classifiers with facial features
    CascadeClassifier cascade, nestedCascade;
    double scale = 1;

    // Load classifiers from "opencv/data/haarcascades" directory
    assert(nestedCascade.load("/opencv/share/opencv4/haarcascades/haarcascade_eye_tree_eyeglasses.xml"));


    //assert(nestedCascade.load("/home/mark/haarcascade_eye_tree_eyeglasses.xml"));

    assert(!nestedCascade.empty());

    // Change path before execution
    cascade.load("/opencv/share/opencv4/haarcascades/haarcascade_frontalcatface.xml");

    while (true) {
        cv::waitKey(1);
        cv::Mat pic1;
        pic1 = takePicture();

        //imshow("camera", rotate_in_frame(pic1, i++));


        for (int j = 0; j < 1; j++)
            detectAndDraw(pic1, cascade, nestedCascade, scale);

        char c = (char) cv::waitKey(1);

        // Press q to exit from window
        if (c == 27 || c == 'q' || c == 'Q')
            break;
    }
}