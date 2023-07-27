#ifndef SAMPLETABLEWGT_H
#define SAMPLETABLEWGT_H

#include <QTableWidget>
#include <QStyledItemDelegate>
#include "global.h"

class SampleTableWidget: public QTableWidget
{
    Q_OBJECT
public:
    explicit SampleTableWidget(QWidget *parent = 0);
    ~SampleTableWidget();

signals:
    void sigViewBtnClicked(int i);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
    virtual void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible) override;

private:
    void initFrozenHeaderH();
    void initFrozenHeaderV();
    void updateFrozenTableGeometry();
    void initFrame();

private:
    QTableWidget *m_frozenHeaderWgtH;
    QTableWidget *m_frozenHeaderWgtV;

    static constexpr int ROW_HEIGHT = 20;
    static constexpr int COLUMN_WIDTH = 65;
    static constexpr int TYPES = 3, DEGREES = 360 / 5, PHASES = CHANNEL_NUM - 1;
    static constexpr int COLUMNS = TYPES * DEGREES * PHASES;
    static constexpr int ROWS = MAX_FREQ - MIN_FREQ + 1;
};

class ItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ItemDelegate(int type, QObject *parent=0);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    enum DelegateType {
        HorizentalHeader = 0,
        VerticalHeader,
        DATA
    };

private:
    int m_type;
};

#endif // TDMSUMMARYTABLEWGT_H
