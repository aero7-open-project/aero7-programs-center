#pragma once

#include <QLayout>
#include <QList>

// Equal-width columns that wrap to the available viewport, without retaining
// the minimum width of a previously wider row after the window shrinks.
class WrappingGridLayout final : public QLayout {
public:
    explicit WrappingGridLayout(QWidget *parent = nullptr) : QLayout(parent) {
        setContentsMargins(0, 0, 0, 0);
        setSpacing(8);
    }
    ~WrappingGridLayout() override {
        while (auto *item = takeAt(0)) delete item;
    }
    void addItem(QLayoutItem *item) override { m_items.append(item); invalidate(); }
    int count() const override { return m_items.size(); }
    QLayoutItem *itemAt(int index) const override { return m_items.value(index); }
    QLayoutItem *takeAt(int index) override {
        if (index < 0 || index >= count()) return nullptr;
        auto *item = m_items.takeAt(index);
        invalidate();
        return item;
    }
    Qt::Orientations expandingDirections() const override { return Qt::Horizontal; }
    bool hasHeightForWidth() const override { return true; }
    int heightForWidth(int width) const override { return arrange(QRect(0, 0, width, 0), false); }
    QSize sizeHint() const override { return minimumSize(); }
    QSize minimumSize() const override {
        QSize size;
        for (auto *item : m_items) size = size.expandedTo(item->minimumSize());
        const auto margins = contentsMargins();
        return size + QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    }
    void setGeometry(const QRect &rect) override {
        QLayout::setGeometry(rect);
        arrange(rect, true);
    }
private:
    int arrange(const QRect &rect, bool apply) const {
        const auto margins = contentsMargins();
        const QRect area = rect.marginsRemoved(margins);
        const int gap = qMax(0, spacing());
        int minimumWidth = 1;
        for (auto *item : m_items) minimumWidth = qMax(minimumWidth, item->minimumSize().width());
        const int columns = qMax(1, qMin(count(), (area.width() + gap) / (minimumWidth + gap)));
        const int width = qMax(1, (area.width() - gap * (columns - 1)) / columns);
        int y = area.y();
        for (int start = 0; start < count(); start += columns) {
            int height = 0;
            for (int i = start; i < qMin(start + columns, count()); ++i) {
                auto *item = m_items.at(i);
                height = qMax(height, qMax(item->minimumSize().height(),
                    item->hasHeightForWidth() ? item->heightForWidth(width) : item->sizeHint().height()));
            }
            if (apply) for (int i = start; i < qMin(start + columns, count()); ++i)
                m_items.at(i)->setGeometry(QRect(area.x() + (i - start) * (width + gap), y, width, height));
            y += height + gap;
        }
        return margins.top() + margins.bottom() + (count() ? y - area.y() - gap : 0);
    }
    QList<QLayoutItem *> m_items;
};
