QT += widgets core axcontainer gui multimedia printsupport opengl

CONFIG += c++20

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QCUSTOMPLOT_USE_LIBRARY
DEFINES += QCUSTOMPLOT_USE_OPENGL
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    'C:/Program Files (x86)/Intel/oneAPI/ipp/2021.9.0/include'

SOURCES += \
    ConductivityDialog.cpp \
    Demodulate/IPPFir.cpp \
    Demodulate/FSKModule.cpp \
    ChannelWidget.cpp \
    ChartViewAfterglow.cpp \
    ChartViewCustom.cpp \
    ChartViewHeatmap.cpp \
    ChartViewScatter.cpp \
    ChartViewSpectrum.cpp \
    ChartViewSpectrumNB.cpp \
    ChartViewSpectrumWB.cpp \
    ChartViewWaterfall.cpp \
    ChartViewWave.cpp \
    ChartWidgetCombine.cpp \
    ChartWidgetNB.cpp \
    ChartWidgetWB.cpp \
    DataManager.cpp \
    DataProcess.cpp \
    DemodulationDetect.cpp \
    MainWidget.cpp \
    Model.cpp \
    PulseDetectModel.cpp \
    PulseDetectTableView.cpp \
    ResistivityDialog.cpp \
    TcpSocket.cpp \
    ThreadAudio.cpp \
    TinyConfig.cpp \
    global.cpp \
    main.cpp \
    src/CommonInfoDialog.cpp \
    src/ManMadeNoiseModel.cpp \
    src/PopupParamDialog.cpp \
    src/SignalNoiseModel.cpp \
    src/DisturbNoiseTableView.cpp \
    src/ManMadeNoiseTableView.cpp \
    src/SignalDetectTableView.cpp \
    src/TypicalFreqDialog.cpp \
    src/WBSignalDetectModel.cpp \
    src/WBSignalDetectWidget.cpp

HEADERS += \
    ConductivityDialog.h \
    Demodulate/Export.h \
    Demodulate/FSKModule.h \
    ChannelWidget.h \
    ChartViewAfterglow.h \
    ChartViewCustom.h \
    ChartViewHeatmap.h \
    ChartViewScatter.h \
    ChartViewSpectrum.h \
    ChartViewSpectrumNB.h \
    ChartViewSpectrumWB.h \
    ChartViewWaterfall.h \
    ChartViewWave.h \
    ChartWidgetCombine.h \
    ChartWidgetNB.h \
    ChartWidgetWB.h \
    DataManager.h \
    DataProcess.h \
    DemodulationDetect.h \
    MainWidget.h \
    Model.h \
    PulseDetectModel.h \
    PulseDetectTableView.h \
    ResistivityDialog.h \
    StructNetData.h \
    StructSample.h \
    TcpSocket.h \
    ThreadAudio.h \
    ThreadSafeQueue.h \
    TinyConfig.h \
    global.h \
    inc/CommonInfoDialog.h \
    inc/ManMadeNoiseModel.h \
    inc/PopupParamDialog.h \
    inc/SignalNoiseModel.h \
    inc/DisturbNoiseTableView.h \
    inc/ManMadeNoiseTableView.h \
    inc/SignalDetectTableView.h \
    inc/TypicalFreqDialog.h \
    inc/WBSignalDetectModel.h \
    inc/WBSignalDetectWidget.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -L'C:/Program Files (x86)/Intel/oneAPI/ipp/2021.9.0/lib/intel64/' -lippcore -lipps -lippvm
LIBS += -L$$PWD/fftw-3.3.5-dll64/ -llibfftw3-3
LIBS += -lOpengl32
LIBS += -lws2_32

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/QCustomPlot/ -lqcustomplot2
#-lqcustomplot2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/QCustomPlot/ -lqcustomplotd2
#-lqcustomplotd2

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/QXlsx/ -lQXlsx
#-llibQXlsx
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/QXlsx/ -lQXlsxd
#-llibQXlsxd

TRANSLATIONS += \
    client_cn.ts

TR_EXCLUDE += $$PWD/boost/*
TR_EXCLUDE += $$PWD/QCustomPlot/*
TR_EXCLUDE += $$PWD/QXlsx/*

RESOURCES += \
    Resource.qrc
