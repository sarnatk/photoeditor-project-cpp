//#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgcodecs.hpp>
#include "opencv2/objdetect.hpp"
#include "opencv2/imgproc.hpp"
#include "algorithms.h"
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

int main() {

    string source_image, command, new_name;

    while (true) {
        cout << "Выберите изображение для обработки\n";
        cin >> source_image;
        cv::Mat src = cv::imread(source_image);
        cout << "Что хотите сделать с изображением?\n";
        cout << "> Повернуть (1)\n";
        cout << "> Обрезать (2)\n";
        cout << "> Изменить тон (3)\n";
        cout << "> Изменить цвет (4)\n";
        cout << "> Наложить другое изображение (5)\n";
        cin >> command;
        if (command == "1") {
            float angle;
            new_name = "rotated_" + source_image;
            cout << "Введите угол поворота: ";
            cin >> angle;
            cv::Mat new_image = rotate_in_frame(src, angle);
            imwrite(new_name, new_image);
        } else if (command == "2") {
            int width, height, x, y;
            new_name = "cropped_" + source_image;
            cout << "Введите новую ширину: ";
            cin >> width;
            cout << "Введите новую высоту: ";
            cin >> height;
            cout << "Введите координаты левого нижнего угла: ";
            cin >> x >> y;
            cv::Mat new_image = crop(src, width, height, x, y);
            imwrite(new_name, new_image);
        } else if (command == "3") {
            cout << "> Яркость (1)\n";
            cout << "> Резкость (2)\n";
            cout << "> Насыщенность (3)\n";
            cin >> command;
            if (command == "1") {
                int brightness, contrast;
                new_name = "bright_" + source_image;
                cout << "Введите степень яркости (wtf?): ";
                cin >> brightness;
                cout << "Введите степень контрастности (wtf?): ";
                cin >> contrast;
                cv::Mat new_image = brighten(src, brightness, contrast);
                imwrite(new_name, new_image);
                cv::imshow("Linear Blend", new_image);
                waitKey(0);
            } else if (command == "2") {
                new_name = "sharp_" + source_image;
                cv::Mat new_image = Sharpen(src);
                imwrite(new_name, new_image);
                cv::imshow("Linear Blend", new_image);
                waitKey(0);
            } else if (command == "3") {
                int saturation;
                double scale;
                new_name = "saturated_" + source_image;
                cout << "Введите степень насыщенности (wtf?): ";
                cin >> saturation;
                cv::Mat new_image = saturate(src, saturation, scale = 1);
                imwrite(new_name, new_image);
            } else {
                cout << "Неизвестная команда\n";
                continue;
            }
            imwrite(new_name, new_image);
            cv::imshow("Linear Blend", new_image);
            waitKey(0);
        } else if (command == "4") {
            cout << "> Чёрно-белый фильтр (1)\n";
            cout << "> Розовый фиьтр (2)\n";
            cin >> command;
            if (command == "1") {
                new_name = "bw_" + source_image;
                cv::Mat new_image = gray(src);
            } else if (command == "2") {
                new_name = "pink_" + source_image;
                cv::Mat new_image = pink(src);
            } else {
                cout << "Неизвестная команда\n";
                continue;
            }
            imwrite(new_name, new_image);
            cv::imshow("Linear Blend", new_image);
            waitKey(0);
        } else if (command == "5") {
            double alpha = 0.5;
            string another_source_image;
            new_name = "blended_" + source_image;
            cout << "Выберите изображение для наложения\n";
            cin >> another_source_image;
            cv::Mat another_src = cv::imread(another_source_image);
            cv::Mat new_image = blend(src, another_src, alpha);
            imwrite(new_name, new_image);
            cv::imshow("Linear Blend", new_image);
            waitKey(0);
        } else {
            cout << "Неизвестная команда\n";
            continue;
        }
        cout << "Успешно сохранено в файл " << new_name << "\n";
        cout << "> Продолжить работу (1)\n";
        cout << "> Выйти из приложения (2)\n";
        cin >> command;
        if (command == "2") {
            return 0;
        }
    }
}

