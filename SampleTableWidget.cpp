#include "SampleTableWidget.h"

#include <QTextOption>
#include <QPainter>
#include <QPushButton>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QHeaderView>

SampleTableWidget::SampleTableWidget(QWidget *parent): QTableWidget(parent)
{
    initFrozenHeaderH();
    initFrozenHeaderV();
    initFrame();
}

SampleTableWidget::~SampleTableWidget()
{
    delete m_frozenHeaderWgtH;
    delete m_frozenHeaderWgtV;
}

void SampleTableWidget::initFrame()
{
    clearContents();
    setColumnCount(COLUMNS);
    setRowCount(ROWS);
    for (int r = 0; r < ROWS; ++r)
    {
        setRowHeight(r, ROW_HEIGHT);
    }
    for (int c = 0; c < COLUMNS; ++c)
    {
        setColumnWidth(c, COLUMN_WIDTH);
    }
    horizontalHeader()->setVisible(true);
    horizontalHeader()->setFixedHeight(m_frozenHeaderWgtH->rowHeight(0) + m_frozenHeaderWgtH->rowHeight(1) + m_frozenHeaderWgtH->rowHeight(2));
    verticalHeader()->setVisible(true);
    verticalHeader()->setFixedWidth(m_frozenHeaderWgtV->columnWidth(0) + m_frozenHeaderWgtV->columnWidth(1));
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setFocusPolicy(Qt::NoFocus);
    setFrameShape(QFrame::NoFrame);
    setVerticalScrollMode(ScrollPerPixel);
    setHorizontalScrollMode(ScrollPerPixel);
    setItemDelegate(new ItemDelegate(ItemDelegate::DATA));
}

void SampleTableWidget::initFrozenHeaderH()
{
    m_frozenHeaderWgtH = new QTableWidget(this);
    m_frozenHeaderWgtH->horizontalHeader()->hide();
    m_frozenHeaderWgtH->verticalHeader()->hide();
    m_frozenHeaderWgtH->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_frozenHeaderWgtH->setFocusPolicy(Qt::NoFocus);
    m_frozenHeaderWgtH->setFrameShape(QFrame::NoFrame);
    m_frozenHeaderWgtH->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_frozenHeaderWgtH->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_frozenHeaderWgtH->setHorizontalScrollMode(ScrollPerPixel);
    m_frozenHeaderWgtH->setItemDelegate(new ItemDelegate(ItemDelegate::HorizentalHeader));
    viewport()->stackUnder(m_frozenHeaderWgtH);
    m_frozenHeaderWgtH->setColumnCount(COLUMNS);
    m_frozenHeaderWgtH->setRowCount(3);
    for (int r = 0; r < 3; ++r)
        m_frozenHeaderWgtH->setRowHeight(r, ROW_HEIGHT);
    m_frozenHeaderWgtH->setColumnWidth(0, COLUMN_WIDTH);
    m_frozenHeaderWgtH->setColumnWidth(1, COLUMN_WIDTH);

    for (int i = 0, column = 0; i < DEGREES; ++i)
    {
        static constexpr int COLUMN_DEGREE = TYPES * PHASES;
        m_frozenHeaderWgtH->setSpan(0, column, 1, COLUMN_DEGREE);
        m_frozenHeaderWgtH->setItem(0, column, new QTableWidgetItem(QString("%1°").arg(5 * i)));
        static QString CHANNEL_TEXT[PHASES] = {tr("CH1-2"),
                                               tr("CH1-3"),
                                               tr("CH1-4")};
        for (int j = 0; j < PHASES; ++j)
        {
            static constexpr int COLUMN_PHASE = TYPES;
            m_frozenHeaderWgtH->setSpan(1, column, 1, COLUMN_PHASE);
            m_frozenHeaderWgtH->setItem(1, column, new QTableWidgetItem(CHANNEL_TEXT[j]));
            static QString TYPE_TEXT[TYPES] = {tr("Theory"), tr("Real"), tr("Sub")};
            for (int k = 0; k < TYPES; ++k)
            {
                static constexpr int COLUMN_TYPE = 1;
                m_frozenHeaderWgtH->setSpan(2, column, 1, COLUMN_TYPE);
                m_frozenHeaderWgtH->setItem(2, column, new QTableWidgetItem(TYPE_TEXT[k]));
                m_frozenHeaderWgtH->setColumnWidth(column, COLUMN_WIDTH);
                ++column;
            }
        }
    }
    connect(m_frozenHeaderWgtH->horizontalScrollBar(), &QAbstractSlider::valueChanged, horizontalScrollBar(), &QAbstractSlider::setValue);
    connect(horizontalScrollBar(), &QAbstractSlider::valueChanged, m_frozenHeaderWgtH->horizontalScrollBar(), &QAbstractSlider::setValue);
}

void SampleTableWidget::initFrozenHeaderV()
{
    m_frozenHeaderWgtV = new QTableWidget(this);
    m_frozenHeaderWgtV->horizontalHeader()->show();
    m_frozenHeaderWgtV->verticalHeader()->hide();
    m_frozenHeaderWgtV->horizontalHeader()->setFixedHeight(m_frozenHeaderWgtH->rowHeight(0) + m_frozenHeaderWgtH->rowHeight(1) + m_frozenHeaderWgtH->rowHeight(2));
    m_frozenHeaderWgtV->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_frozenHeaderWgtV->setFocusPolicy(Qt::NoFocus);
    m_frozenHeaderWgtV->setFrameShape(QFrame::NoFrame);
    m_frozenHeaderWgtV->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_frozenHeaderWgtV->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_frozenHeaderWgtV->setVerticalScrollMode(ScrollPerPixel);
    m_frozenHeaderWgtV->setItemDelegate(new ItemDelegate(ItemDelegate::VerticalHeader));
    viewport()->stackUnder(m_frozenHeaderWgtV);
    m_frozenHeaderWgtV->setColumnCount(2);
    m_frozenHeaderWgtV->setRowCount(ROWS);
    for (int r = 0; r < 2; ++r)
        m_frozenHeaderWgtV->setColumnWidth(r, COLUMN_WIDTH);
    for (int i = 0; i < ROWS; ++i)
    {
        m_frozenHeaderWgtV->setRowHeight(i, ROW_HEIGHT);
        m_frozenHeaderWgtV->setItem(i, 0, new QTableWidgetItem(QString("1")));
        m_frozenHeaderWgtV->setItem(i, 1, new QTableWidgetItem(QString("%1").arg(200 + i)));
    }
    QStringList HEADERS{tr("Layer"), tr("Freq(MHz)")};
    m_frozenHeaderWgtV->setHorizontalHeaderLabels(HEADERS);
    m_frozenHeaderWgtV->setStyleSheet("QHeaderView::section { color:white; background-color:#E7EEFB; }");
    connect(m_frozenHeaderWgtV->verticalScrollBar(), &QAbstractSlider::valueChanged, verticalScrollBar(), &QAbstractSlider::setValue);
    connect(verticalScrollBar(), &QAbstractSlider::valueChanged, m_frozenHeaderWgtV->verticalScrollBar(), &QAbstractSlider::setValue);
}

void SampleTableWidget::updateFrozenTableGeometry()
{
    m_frozenHeaderWgtH->setGeometry(frameWidth() + verticalHeader()->width(), frameWidth(), viewport()->width(), horizontalHeader()->height());
    m_frozenHeaderWgtV->setGeometry(frameWidth(), frameWidth(), verticalHeader()->width(), viewport()->height() + m_frozenHeaderWgtV->horizontalHeader()->height());
}

void SampleTableWidget::resizeEvent(QResizeEvent *event)
{
    QTableView::resizeEvent(event);
    updateFrozenTableGeometry();
}

QModelIndex SampleTableWidget::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    QModelIndex current = QTableView::moveCursor(cursorAction, modifiers);
    if (cursorAction == MoveUp && current.row() > 0 && visualRect(current).topLeft().y() < m_frozenHeaderWgtH->rowHeight(1))
    {
        const int newValue = verticalScrollBar()->value() + visualRect(current).topLeft().y() - m_frozenHeaderWgtH->rowHeight(0) - m_frozenHeaderWgtH->rowHeight(1);
        verticalScrollBar()->setValue(newValue);
    }
    return current;
}

void SampleTableWidget::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    if (index.row() > 0)
        QTableView::scrollTo(index, hint);
}

ItemDelegate::ItemDelegate(int type, QObject *parent): QStyledItemDelegate(parent), m_type(type)
{
}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    switch(m_type)
    {
    case HorizentalHeader:
    case VerticalHeader:
    {
        QColor color;
        color.setRgb(231, 238, 251);
        painter->setPen(color);
        painter->setBrush(QBrush(color));
        painter->drawRect(option.rect);
        QTextOption op;
        op.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        QFont font;
        font.setFamily("Microsoft YaHei");
        font.setPixelSize(14);
        font.setBold(true);
        painter->setFont(font);
        painter->setPen(QColor(51, 51, 51));
        painter->drawText(option.rect, index.data(Qt::DisplayRole).toString(), op);
        break;
    }
    case DATA:
    {
        painter->setPen(QColor(102, 102, 102));
        QTextOption op;
        op.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        QFont font;
        font.setFamily("Microsoft YaHei");
        font.setPixelSize(12);
        painter->setFont(font);
        if(option.state & QStyle::State_Selected)
        {
            painter->fillRect( option.rect, QBrush(QColor(239, 244, 255)));
        }
        painter->drawText(option.rect, index.data(Qt::DisplayRole).toString(), op);
        painter->setPen(QColor(248,251,255));
        painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
        break;
    }
    }
}
