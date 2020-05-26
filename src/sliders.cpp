#include "sliders.h"

SliderWidget::SliderWidget(QWidget *parent, QLineEdit *e)
        : QWidget(parent), slider1(new QSlider(Qt::Horizontal)) //-256 256
        , slider2(new QSlider(Qt::Horizontal)) //-256 256
        , slider3(new QSlider(Qt::Horizontal)) //-256 256
        , button(new QPushButton("Apply")), edit(e), layout(new QVBoxLayout) {
    layout->addWidget(slider1);
    layout->addWidget(slider2);
    layout->addWidget(slider3);
    layout->addWidget(button);
    setLayout(layout);
    connect(slider1, &QSlider::valueChanged, this, &SliderWidget::setValue);
    connect(slider2, &QSlider::valueChanged, this, &SliderWidget::setValue);
    connect(slider3, &QSlider::valueChanged, this, &SliderWidget::setValue);
    connect(button, SIGNAL(clicked()), this, SLOT(setValue()));
}

void SliderWidget::setValue() {
    value = slider1->value();
}

SliderWindow::SliderWindow(QWidget *parent, QLineEdit *edit)
        : QDialog(parent)
        , widget(new SliderWidget(nullptr, edit))
        , layout(new QVBoxLayout) {
    layout->addWidget(widget);
    setLayout(layout);
}

void SliderWindow::setValue() {
    value = widget->value;
}
