#include "cropdialog.h"
#include "ui_cropdialog.h"

CropDialog::CropDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CropDialog)
{
    ui->setupUi(this);
    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setParent(this);

}

void CropDialog::setImage(QImage img)
{
    image = img ;
    QPixmap pix(QPixmap::fromImage(image));
    imageLabel->resize(pix.size());
    imageLabel->setPixmap(pix);
    QList<QPushButton *> buttons = this->findChildren<QPushButton *>();
    foreach (QPushButton * b, buttons) {
        b->setParent(imageLabel);
    }
}

CropDialog::~CropDialog()
{
    delete ui;
}


void CropDialog::on_saveButton_clicked()
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
    this->close();
}

void CropDialog::on_CancelButton_clicked()
{
    this->close();
}
