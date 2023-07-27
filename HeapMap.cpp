#include "HeapMap.h"

#include <QDateTime>

//HeapMap::HeapMap()
//{
//    initHeatMaps();
//    connect(this, &HeapMap::mouseMove, this,[this] (QMouseEvent* event) {
//        int x_pos = event->pos().x();
//        int y_pos = event->pos().y();
//        float x_val = xAxis->pixelToCoord(x_pos);
//        float y_val = yAxis->pixelToCoord(y_pos);
//        float z_val = m_pColorMap->data()->data(x_val, y_val);
//        QString qDate = QString(tr("[时间:")) + QDateTime::fromTime_t(x_val).toString("yyyy-M-d h:m:s");
//        QString qHeight = QString(tr("[高度:")) + QString::number(y_val);
//        QString qValue = QString(tr("] [值:")) + QString::number(z_val)+"]";
//        QString strToolTip = qDate + qHeight + qValue;
//        QToolTip::showText(cursor().pos(), strToolTip, this);
//    });
//}

//void HeapMap::initHeatMaps()
//{
//    //uint xStartValue = time_xAxisStartTime.toTime_t();//当前时;间;转化为秒
//    //uint xStartValue = QCPAxisTickerDateTime::dateTimeToKey(sec_xAxisStartTime);//日;期传;化成秒
//    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);//交互作;用，实现;拖;拉和缩;放
//    QSharedPointer dateTimeTicker(new QCPAxisTickerDateTime);//生;成;时;间;刻;度;对*象
//    m_pTimeAxis = dateTimeTicker;
//    dateTimeTicker.clear();
//    xAxis->setTicker(m_pTimeAxis);
//    m_pTimeAxis->setTickCount(7);
//    m_pTimeAxis->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
//    m_pTimeAxis->setDateTimeFormat(time_formate);//设;置;x;轴;刻;;度;显;示;格;式（time_formate为字符串，如"yyyy-M-d hⓂ️s"）

//    int nx = sec_xAxisEndTime - sec_xAxisStartTime;//x轴;上;的;点;数（sec_...为以;秒表;示的起;始与;结;束;时;间）
//    int ny = len_maxHeight;//y轴;上;的;点;数

//    xAxis->setSubTicks(false);
//    xAxis->setRange(0, nx);//x;轴;范;围

//    xAxis->setTickLabelRotation(30);//设置;刻;度;标;签顺;时针;旋;转30度
//    yAxis->setRange(0, len_maxHeight);//len_maxHeight;为y;轴的最;大;高度，建;议将len_maxHeight设置为;实测;数据;最大;高度值+1，不然实测数据只达到len_maxHeight时;绘;制;图;会出;现indexoutofbounds;错;误;提示，严*重*影*响*加*载*速度

//    m_pColorMap = new QCPColorMap(xAxis, yAxis);//以Qcustomplot的x和y轴来;初;始;化QCPColorMap
//    /*自我;简;洁理解QCPColorMap的数据为(坐标x,坐标y,;颜;*色;值value)*/
//    m_pColorMap->data()->setSize(nx, ny);//整个图的(x,y)点数
//    /*自;我;理;解，将ColorMap划分;为包;含(nx,ny)个点，每;个;点(x1,y1)的;取;值;由setRange决;定*/
//    m_pColorMap->data()->setRange(QCPRange(sec_xAxisStartTime, sec_xAxisEndTime), QCPRange(0, len_maxHeight)); //(x,y);点;对;应;的取;值;范;围,如点(nx,ny)对应的取值为(sec_xAxisEndTime,len_maxHeight)

//    //QCPColorScale *colorScale = new QCPColorScale(this);//颜*色*范;围
//    m_pColorScale = new QCPColorScale(this);//颜*色*范;围==色*条;
//    plotLayout()->addElement(0, 1, m_pColorScale); // 右;侧;添加
//    m_pColorScale->setType(QCPAxis::atRight);
//    m_pColorScale->setDataRange(QCPRange(0, data_maxColorBar));//颜*色*范围;对应;的;取;值;范;围data_maxColorBar这里;为测;量得;到的;最;大值来;代;表;颜*色*;的;最大值
//    m_pColorScale->axis()->ticker()->setTickCount(6);
//    m_pColorScale->axis()->setLabel(name_colorBar);//;色*条;的名

//    m_pColorMap->setColorScale(m_pColorScale); //热;图;连接;色*条

//    //m_pColorMap->setGradient(QCPColorGradient::gpJet);//设置默;认渐;进*色*变;化（可在QCPColorGradient中查看）
//    //QSharedPointer<QCPColorGradient> colorLayout(new QCPColorGradient);//生;成;颜*色*渐;进;变化对;象
//    m_pColorGradient = new QCPColorGradient();//自定义;的;渐;进*色*变化对;象
//    m_pColorGradient->setColorStops(color_layout);//QMap<double, QColor> color_layout为;颜*色*布局范围(double取值为0，1)
//    m_pColorMap->setGradient(*m_pColorGradient);//设;置渐进;色;变化
//    m_pColorMap->rescaleDataRange();

//    QCPMarginGroup *marginGroup = new QCPMarginGroup(this);
//    axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
//    m_pColorScale->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);

//    rescaleAxes();
//}

//void HeapMap::addOneColData(const S_HeatMap_Data &data)
//{
//    uint sec_xAddTime = QCPAxisTickerDateTime::dateTimeToKey(data.dt_Time);
//    if (sec_xAddTime >= sec_xAxisEndTime)//平;移;图
//    {
//        int nx = sec_xAxisEndTime - sec_xAxisStartTime;//x轴上;的点;数
//        sec_xAxisEndTime += nx;//向右;平移;nx秒
//        sec_xAxisStartTime += nx;
//        clearAllDatas();//清;空之;前的数;据
//        xAxis->setRange(sec_xAxisStartTime, sec_xAxisEndTime);//x轴;范围
//        m_pColorMap->data()->setSize(nx, len_maxHeight);//重;新;设;置;范围
//        m_pColorMap->data()->setRange(QCPRange(sec_xAxisStartTime, sec_xAxisEndTime), QCPRange(0, len_maxHeight)); //(x,y)点对应;取值;范;围
//        //QCPColorMapData *existData = m_pColorMap->data();
//        //m_pColorMap->setData(existData, true);
//        //m_pColorMap->data()->recalculateDataBounds();
//        //m_pColorMap->rescaleDataRange(true);
//    }

//    uint x = sec_xAddTime - sec_xAxisStartTime;
//    for (int i = 0; i < data.vec_Height.size() - 1; i++)
//    {
//        for (int y = data.vec_Height.at(i); y <= data.vec_Height.at(i+1); y++)
//        {
//            m_pColorMap->data()->setCell(x, y, data.vec_Value.at(i));//设;置;数据
//        }
//    }
//}
