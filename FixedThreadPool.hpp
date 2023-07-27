#ifndef FIXED_THREAD_POOL
#define FIXED_THREAD_POOL

#include <mutex>
#include <condition_variable>
#include <queue>
#include <QThread>

#include "DataProcess.h"

using DATA_TYPE = std::vector<std::shared_ptr<char[]>>;

class FixedThreadPool: public std::enable_shared_from_this<FixedThreadPool>
{
public:
    explicit FixedThreadPool(ChartWidget* chartWidget): m_cxWidget(chartWidget)
    {
    }

    void Start()
    {
        qRegisterMetaType<QVector<std::tuple<double, double, int>>>("QVector<std::tuple<double, double, int>>");
        auto self = shared_from_this();
        for (size_t i = 0; i < POOL_SIZE; ++i)
        {
            QObject::connect(&thread[i], &QThread::started, [this, i, self] {
                std::unique_lock<std::mutex> lk(mtx_);
                auto dataProcess = std::make_unique<DataProcess>("Insert_" + QString::number(i), m_cxWidget);
                QObject::connect(dataProcess.get(), &DataProcess::UpdateFreqListTable, m_cxWidget->freqListTable, &PointTableView::UpdateItems);
                for (;;)
                {
                    if (!tasks_.empty())
                    {
                        auto ptr = std::move(tasks_.front());
                        tasks_.pop();
                        lk.unlock();
                        dataProcess->ProcessData(ptr);
                        lk.lock();
                    }
                    else if (is_shutdown_)
                    {
                        break;
                    }
                    else
                    {
                        cond_.wait(lk);
                    }
                }
            });
            thread[i].start();
        }
    }

    FixedThreadPool() = delete;
    FixedThreadPool(FixedThreadPool&&) = delete;

    ~FixedThreadPool()
    {
        {
            std::lock_guard<std::mutex> lk(mtx_);
            is_shutdown_ = true;
        }
        cond_.notify_all();
    }

    void execute(DATA_TYPE task)
    {
        {
            std::lock_guard<std::mutex> lk(mtx_);
            tasks_.emplace(std::move(task));
        }
        cond_.notify_one();
    }

private:
    static constexpr int POOL_SIZE = 8;
    QThread thread[POOL_SIZE];
    std::mutex mtx_;
    std::condition_variable cond_;
    bool is_shutdown_ = false;
    std::queue<DATA_TYPE> tasks_;
    ChartWidget* m_cxWidget = nullptr;
};

#endif
