#include <QtWidgets>
#ifndef QT_NO_PRINTER
#include <QPrintDialog>
#endif

#include "imageviewer.h"
#include "cropdialog.h"

ImageViewer::ImageViewer()
{
    ImageViewer::cropping = false ;
    setMouseTracking(true);


    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    setCentralWidget(scrollArea);

    createActions();
    createMenus();

//    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
}


bool ImageViewer::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    image = reader.read();
    if (image.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1.").arg(QDir::toNativeSeparators(fileName)));
        setWindowFilePath(QString());
        imageLabel->setPixmap(QPixmap());
        imageLabel->adjustSize();
        return false;
    }
    undo_scale_stack.clear();
    undo_stack.clear();
    redo_scale_stack.clear();
    redo_stack.clear();
    original = image.copy();
    imageLabel->setPixmap(QPixmap::fromImage(image));
    scaleFactor = 1.0;

    imageWidth = image.width();
    imageHeight = image.height();


    fitToWindowAct->setEnabled(true);
    cropAct->setEnabled(true);
    updateActions();

    if (!fitToWindowAct->isChecked())
        imageLabel->adjustSize();

    setWindowFilePath(fileName);
    return true;
}



void ImageViewer::open()
{
    const QStringList desktopLocations = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    QFileDialog dialog(this, tr("Open File"),
                       desktopLocations.isEmpty() ? QDir::currentPath() : desktopLocations.last() ,
                       tr("Image Files (*.png *.jpg *.bmp)"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

void ImageViewer::save()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                               "/home/jana/untitled.png",
                               tr("Images (*.png *.bmp *.jpg)"));
    while ( fileName.isEmpty() )
    {
        fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                       "/home/jana/untitled.png",
                                       tr("Images (*.png *.bmp *.jpg)"));
    }
    QImageWriter writer(fileName);
    writer.write(image );

}

void ImageViewer::zoomIn()
{
    undo_stack.push_back(*(imageLabel->pixmap()));
    undo_scale_stack.push_back(scaleFactor);
    scaleImage(1.1);
    updateActions();
}

void ImageViewer::zoomOut()
{
    undo_stack.push_back(*(imageLabel->pixmap()));
    undo_scale_stack.push_back(scaleFactor);
    //
    scaleImage(0.9);

    updateActions();
}

void ImageViewer::normalSize()
{
    undo_stack.push_back(*(imageLabel->pixmap()));
    undo_scale_stack.push_back(scaleFactor);
    //
    scaleFactor = 1.0;
    scaleImage(1.0);
    imageWidth = image.width();
    imageHeight = image.height();
}

void ImageViewer::fitToWindow()
{
    undo_stack.push_back(*(imageLabel->pixmap()));
    undo_scale_stack.push_back(scaleFactor);

    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow) {
        normalSize();
    }
    updateActions();
}


void ImageViewer::updateActions()
{
    saveAct->setEnabled(!fitToWindowAct->isChecked());
    cropAct->setEnabled(!fitToWindowAct->isChecked());
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
    rotateAct->setEnabled(true);
    undoAct->setEnabled(undo_stack.size() != 0);
    redoAct->setEnabled(redo_stack.size() != 0);
}

void ImageViewer::scaleImage(double factor)
{
    //Q_ASSERT(imageLabel->pixmap());
    scaleFactor *= factor;
    if (!isRotated)
    {
        image = original.scaled(original.width()*scaleFactor , original.height()*scaleFactor, Qt::IgnoreAspectRatio);
    }
    else
    {
        image = original_rotated.scaled(original_rotated.width()*scaleFactor , original_rotated.height()*scaleFactor, Qt::IgnoreAspectRatio);
    }
    QPixmap pix(QPixmap::fromImage(image));
    imageLabel->resize(pix.size());
    imageLabel->setPixmap(pix);

    imageWidth = image.width();
    imageHeight = image.height();

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void ImageViewer::rotate()
{
    bool ok;
    QInputDialog In ;
    double ang = In.getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Amount:"), 180, -10000, 10000, 2, &ok);
    if (ok)
    {
        isRotated = true ;
        QPixmap pixmap(*imageLabel->pixmap());
        undo_stack.push_back(pixmap);
        QMatrix rm;
        rm.rotate(ang);
        pixmap = pixmap.transformed(rm);
        imageLabel->setPixmap(pixmap);
        original_rotated = image = pixmap.toImage();
        updateActions();
    }
    else
    {
        In.close();
    }
}


void ImageViewer::undo()
{
    if(undo_stack.size() == 0)
        return ;
    imageLabel->resize(undo_stack.back().size());
    redo_stack.push_back(*(imageLabel->pixmap()));
    //
    if (!undo_scale_stack.isEmpty()){
        scaleFactor = undo_scale_stack.back();
        redo_scale_stack.push_back(scaleFactor);
        undo_scale_stack.pop_back();
    }
    //
    imageLabel->setPixmap(undo_stack.back());
    undo_stack.pop_back();

    updateActions();

}

void ImageViewer::redo()
{
    if(redo_stack.size() == 0)
        return ;
    imageLabel->resize(redo_stack.back().size());
    undo_stack.push_back(*(imageLabel->pixmap()));
    //
    if (!redo_scale_stack.isEmpty()){
        scaleFactor = redo_scale_stack.back();
        undo_scale_stack.push_back(scaleFactor);
        redo_scale_stack.pop_back();
    }
    //
    imageLabel->setPixmap(redo_stack.back());
    redo_stack.pop_back();

    updateActions();
}

void ImageViewer::crop()
{
    cropping = true ;

}

//void ImageViewer::mouseMoveEvent(QMouseEvent *event)
//{
//    if(cropping)
//    {
//        QPainter painter(this);
//        painter.setRenderHint(QPainter::Antialiasing);
//        painter.setPen(Qt::darkGreen);
//        painter.drawRect(startPoint.x() , startPoint.y() , event->pos().x()-startPoint.x() , event->pos().y()-startPoint.y() );
//    }
//}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if(cropping)
    {
        startPoint = event->pos();
        if( startPoint.x()>imageWidth || startPoint.y()>imageHeight )
        {
            cropping = false ;
            QMessageBox::about(this, tr("Error Cropping"),
                        tr("<p>can't crop image outside its bounds.</p>"));
        }
    }
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if(cropping)
    {
        endPoint = event->pos();

        if( endPoint.x()>imageWidth || endPoint.y()>imageHeight )
        {
            cropping = false ;
            QMessageBox::about(this, tr("Error Cropping"),
                        tr("<p>can't crop image outside its bounds.</p>"));

            return ;
        }

        if( abs( endPoint.x()- startPoint.x() )< 50 || abs( endPoint.y()- startPoint.y() )< 50 )
        {
            cropping = false ;
            QMessageBox::about(this, tr("Error Cropping"),
                        tr("<p>Too small width or height.</p>"));

            return ;
        }

        QPoint start;
        QPoint end;

        start.setX(startPoint.x());
        start.setY(startPoint.y());
        end.setX(endPoint.x());
        end.setY(endPoint.y());

        if(startPoint.x()>endPoint.x() && startPoint.y()>endPoint.y())
        {
            start.setX(endPoint.x());
            start.setY(endPoint.y());
            end.setX(startPoint.x());
            end.setY(startPoint.y());
        }
        else if(startPoint.x()>endPoint.x() && startPoint.y()<endPoint.y())
        {
            start.setX(endPoint.x());
            start.setY(startPoint.y());
            end.setX(startPoint.x());
            end.setY(endPoint.y());
        }
        else if(startPoint.y()>endPoint.y() && startPoint.x()<endPoint.x())
        {
            start.setX(startPoint.x());
            start.setY( endPoint.y());
            end.setX(endPoint.x());
            end.setY(startPoint.y());
        }

        QRect rect(start,end);

        original = image.copy(rect);

        CropDialog dialog ;
        dialog.setImage(image.copy(rect));
        dialog.setModal(true);
        dialog.setFixedSize( (image.copy(rect)).size() );
        dialog.exec();

        cropping = false ;
    }
}

// initializations

void ImageViewer::createActions()
{
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(tr("&Save..."), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setEnabled(false);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    zoomInAct = new QAction(tr("Zoom &In (25%)"), this);
    zoomInAct->setShortcut(tr("Ctrl++"));
    zoomInAct->setEnabled(false);
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(tr("Zoom &Out (25%)"), this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setEnabled(false);
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    normalSizeAct = new QAction(tr("&Normal Size"), this);
    normalSizeAct->setShortcut(tr("Ctrl+N"));
    normalSizeAct->setEnabled(false);
    connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));

    fitToWindowAct = new QAction(tr("&Fit to Window"), this);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));
    connect(fitToWindowAct, SIGNAL(triggered()), this, SLOT(fitToWindow()));

    undoAct = new QAction(tr("&Undo "), this);
    undoAct->setShortcut(tr("Ctrl+Z"));
    undoAct->setEnabled(false);
    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));

    redoAct = new QAction(tr("&Redo "), this);
    redoAct->setShortcut(tr("Ctrl+Y"));
    redoAct->setEnabled(false);
    connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

    rotateAct = new QAction(tr("Rotate "), this);
    rotateAct->setShortcut(tr("Ctrl+R"));
    rotateAct->setEnabled(false);
    connect(rotateAct, SIGNAL(triggered()), this, SLOT(rotate()));

    cropAct = new QAction(tr("Crop "), this);
    cropAct->setShortcut(tr("Ctrl+P"));
    cropAct->setEnabled(false);
    connect(cropAct, SIGNAL(triggered()), this, SLOT(crop()));

}

void ImageViewer::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    viewMenu = new QMenu(tr("&View"), this);
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(normalSizeAct);
    viewMenu->addSeparator();
    viewMenu->addAction(fitToWindowAct);

    editMenu = new QMenu(tr("&Edit"), this);
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addAction(rotateAct);
    editMenu->addAction(cropAct);



    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(viewMenu);
    menuBar()->addMenu(editMenu);
}
