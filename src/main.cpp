#include <QApplication>
#include <QCommandLineParser>

#include "../include/imageviewer.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QGuiApplication::setApplicationDisplayName(ImageViewer::tr("Photoeditor"));
    QCommandLineParser commandLineParser;
    commandLineParser.addHelpOption();
    commandLineParser.addPositionalArgument(ImageViewer::tr("[file]"), ImageViewer::tr("Image file to open."));
    commandLineParser.process(QCoreApplication::arguments());
    ImageViewer imageViewer;
    imageViewer.setWindowState(Qt::WindowMaximized);
    if (!commandLineParser.positionalArguments().isEmpty()
        && !imageViewer.loadFile(commandLineParser.positionalArguments().front())) {
        return -1;
    }
    imageViewer.show();
    return app.exec();
}