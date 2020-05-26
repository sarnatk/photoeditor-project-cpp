#ifndef PHOTOEDITOR_SLIDERS_H
#define PHOTOEDITOR_SLIDERS_H

#include <memory>
#include <QDialog>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

class SliderWidget : public QWidget {
Q_OBJECT

public:
    SliderWidget(QWidget *parent, QLineEdit *edit);

private slots:

    void setValue();

public:
    int value;
private:
    QSlider *slider1;
    QSlider *slider2;
    QSlider *slider3;
    QPushButton *button;
    QLineEdit *edit;
    QVBoxLayout *layout;
};

class SliderWindow : public QDialog {
Q_OBJECT

public:
    SliderWindow(QWidget *parent, QLineEdit *edit);

private slots:

    void setValue();

private:
    SliderWidget *widget;
    QVBoxLayout *layout;
public:
    int value = 0;
};

#endif //PHOTOEDITOR_SLIDERS_H
