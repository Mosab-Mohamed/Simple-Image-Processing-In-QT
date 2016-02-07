#ifndef CROPDIALOG_H
#define CROPDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QFileDialog>
#include <QImageWriter>

namespace Ui {
class CropDialog;
}

class QLable ;

class CropDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CropDialog(QWidget *parent = 0);
    ~CropDialog();
    void setImage(QImage image);

private slots:
    void on_saveButton_clicked();

    void on_CancelButton_clicked();

private:
    Ui::CropDialog *ui;
    void createActions();

    QImage image;
    QLabel *imageLabel;

};

#endif // CROPDIALOG_H
