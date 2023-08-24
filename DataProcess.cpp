#include "DataProcess.h"

#include "global.h"
#include "Model.h"
#include "StructNetData.h"

DataProcess::DataProcess(QString connectionName, ChartWidget* m_cxWidget): m_cxWidget(m_cxWidget), sqlData(connectionName)
{
}

DataProcess::~DataProcess()
{

}
