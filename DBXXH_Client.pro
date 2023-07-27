QT += widgets core gui multimedia charts network sql printsupport opengl

CONFIG += c++20

DEFINES += QCUSTOMPLOT_USE_LIBRARY
DEFINES += QCUSTOMPLOT_USE_OPENGL

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CAudioMonitorThread.cpp \
    ChartNB.cpp \
    ChartViewCustom.cpp \
    ChartViewSpectrum.cpp \
    ChartViewWaterfall.cpp \
    ChartWB.cpp \
    ChartWidget.cpp \
    CombineWidget.cpp \
    DataManager.cpp \
    DataProcess.cpp \
    HeapMap.cpp \
    Model.cpp \
    PointTableWidget.cpp \
    SampleRecv.cpp \
    SampleTableWidget.cpp \
    SideWidget.cpp \
    SqlData.cpp \
    SqlWidget.cpp \
    TableInterference.cpp \
    TableNoise.cpp \
    TableSignals.cpp \
    TcpSocket.cpp \
    TinyConfig.cpp \
    ZCWidget.cpp \
    global.cpp \
    main.cpp

HEADERS += \
    CAudioMonitorThread.h \
    ChartNB.h \
    ChartViewCustom.h \
    ChartViewSpectrum.h \
    ChartViewWaterfall.h \
    ChartWB.h \
    ChartWidget.h \
    CombineWidget.h \
    DataManager.h \
    DataProcess.h \
    FixedThreadPool.hpp \
    HeapMap.h \
    Model.h \
    PointTableWidget.h \
    SampleRecv.h \
    SampleTableWidget.h \
    SideWidget.h \
    SqlData.h \
    SqlWidget.h \
    StructNetData.h \
    StructSample.h \
    TableInterference.h \
    TableNoise.h \
    TableSignals.h \
    TcpSocket.h \
    ThreadSafeQueue.h \
    TinyConfig.h \
    ZCWidget.h \
    global.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -L$$PWD/fftw-3.3.5-dll64/ -llibfftw3-3
LIBS += -lOpengl32

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/QCustomPlot/ -lqcustomplot2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/QCustomPlot/ -lqcustomplotd2

TRANSLATIONS += \
    client_cn.ts

RESOURCES += \
    Resource.qrc
