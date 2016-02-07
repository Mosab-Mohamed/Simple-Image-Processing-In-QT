#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMouseEvent>
#include <QMainWindow>
#ifndef QT_NO_PRINTER
#include <QPrinter>
#endif


class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;

class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    ImageViewer();
    bool loadFile(const QString &);
    bool saveImage(const QString &);
    void mousePressEvent( QMouseEvent* ev );
    void mouseReleaseEvent( QMouseEvent* ev );

private slots:
    void open();
    void save();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();
    void rotate();
    void undo();
    void redo();
    void crop();

private:
    //crop operation's variables
    bool cropping ;
    QPoint startPoint ;
    QPoint endPoint ;
    int imageWidth ;
    int imageHeight  ;

    void createActions();
    void createMenus();
    void updateActions();
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    bool saveFile(const QString &);

    QLabel *imageLabel;
    QScrollArea *scrollArea;
    double scaleFactor;
    QImage image;
    QImage cropped;
    QList <QPixmap> undo_stack ;
    QList <QPixmap> redo_stack ;
    QList <double> redo_scale_stack ;
    QList <double> undo_scale_stack ;
    QImage original,original_rotated;
    bool isRotated;

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

    QAction *openAct;
    QAction *saveAct;
    QAction *exitAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
    QAction *undoAct;
    QAction *redoAct;
    QAction *rotateAct;
    QAction *cropAct;

    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *editMenu;

};

#endif
