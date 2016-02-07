#-------------------------------------------------
#
# Project created by QtCreator 2016-02-06T02:26:37
#
#-------------------------------------------------

QT += widgets
qtHaveModule(printsupport): QT += printsupport

HEADERS       = imageviewer.h \
    cropdialog.h
SOURCES       = imageviewer.cpp \
                main.cpp \
    cropdialog.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/widgets/imageviewer
INSTALLS += target

wince {
   DEPLOYMENT_PLUGIN += qjpeg qgif
}

FORMS += \
    cropdialog.ui
