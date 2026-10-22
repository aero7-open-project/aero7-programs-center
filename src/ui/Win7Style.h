#pragma once

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QTreeWidget>
#include <QHeaderView>

namespace Win7Style {

inline QLabel *pageTitle(const QString &text)
{
    auto *label = new QLabel(text);
    label->setObjectName(QStringLiteral("pageTitle"));
    return label;
}

inline QLabel *instruction(const QString &text)
{
    auto *label = new QLabel(text);
    label->setObjectName(QStringLiteral("instructionText"));
    label->setWordWrap(true);
    return label;
}

inline QFrame *separator()
{
    auto *line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setObjectName(QStringLiteral("hairline"));
    return line;
}

inline QFrame *commandBar(QHBoxLayout **layoutOut = nullptr)
{
    auto *bar = new QFrame;
    bar->setObjectName(QStringLiteral("commandBar"));
    bar->setFixedHeight(29);
    auto *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(12, 0, 10, 0);
    layout->setSpacing(16);
    if (layoutOut) *layoutOut = layout;
    return bar;
}

inline void configureTree(QTreeWidget *tree)
{
    tree->setRootIsDecorated(false);
    tree->setIndentation(0);
    tree->setAlternatingRowColors(false);
    tree->setFrameShape(QFrame::NoFrame);
    tree->setSelectionMode(QAbstractItemView::SingleSelection);
    tree->header()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    tree->header()->setStretchLastSection(true);
    tree->header()->setFixedHeight(26);
}

inline QString applicationStyleSheet()
{
    return QStringLiteral(R"(
QMainWindow, QDialog, QWidget { font-family: "Segoe UI", "Noto Sans"; font-size: 9pt; color: #1f1f1f; }
QMainWindow, QDialog { background: #ffffff; }
QToolBar#navigationBar { min-height: 38px; max-height: 38px; padding: 2px 8px; spacing: 3px;
  border: 0; border-bottom: 1px solid #9fb6ca;
  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #f9fcff,stop:.45 #e9f3fb,stop:.48 #d5e6f3,stop:1 #edf5fb); }
QToolBar#navigationBar QToolButton { min-width: 28px; min-height: 28px; padding: 0 3px; border: 1px solid transparent; background: transparent; }
QToolBar#navigationBar QToolButton:hover { border: 1px solid #a7c7df; background: #e6f3fc; }
QToolBar#navigationBar QToolButton:pressed { border: 1px solid #6b9cc2; background: #d5eaf8; }
QLabel#breadcrumb { min-height: 25px; padding: 0 10px; border: 1px solid #a8bdce; background: white; color: #1b1b1b; }
QLineEdit#searchBox { min-height: 25px; max-height: 25px; padding: 0 8px; border: 1px solid #a8bdce; background: white; selection-background-color: #3399ff; }
QFrame#commandBar { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #f7fbfe,stop:1 #e7f0f7); border: 0; border-bottom: 1px solid #c7d5df; }
QFrame#commandBar QToolButton { color: #1f4e79; padding: 2px 7px; border: 1px solid transparent; background: transparent; }
QFrame#commandBar QToolButton:hover { border: 1px solid #b8cbd9; background: #f9fcff; }
QListWidget#sidebar { border: 0; border-right: 1px solid #c7d5df; background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #f8fbfe,stop:1 #e6f0f8); padding: 9px 5px; outline: 0; }
QListWidget#sidebar::item { min-height: 23px; padding: 1px 6px; border: 1px solid transparent; color: #1f4e79; }
QListWidget#sidebar::item:selected { color: #1f1f1f; border: 1px solid #b8cedf; background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #fdfefe,stop:1 #dbeaf5); }
QListWidget#sidebar::item:disabled { color: #6d6d6d; font-weight: bold; background: transparent; }
QWidget#page, QScrollArea#homeScroll, QScrollArea#settingsScroll, QScrollArea#homeScroll > QWidget > QWidget, QScrollArea#settingsScroll > QWidget > QWidget { background: #ffffff; }
QLabel#pageTitle { color: #1a5dab; font-size: 13pt; background: transparent; }
QLabel#instructionText { color: #555555; background: transparent; }
QFrame#hairline { color: #d9d9d9; background: #d9d9d9; max-height: 1px; border: 0; }
QLabel#sectionHeading { color: #555555; font-weight: normal; background: transparent; }
QFrame#programCard { background: #ffffff; border: 1px solid #d6dce1; border-radius: 0; }
QFrame#programCard:hover { border: 1px solid #7eb4df; background: #f4f9fd; }
QLabel#cardName { color: #1a5dab; font-weight: normal; }
QLabel#cardSummary, QLabel#cardSource { color: #555555; font-size: 8pt; }
QLabel#cardWarning { color: #7b5c00; font-size: 8pt; }
QFrame#trustedSourceBadge, QFrame#thirdPartySourceBadge { border: 0; background: transparent; }
QLabel#sourceBadgeText { color: #555555; font-size: 8pt; }
QPushButton { min-height: 22px; padding: 1px 12px; border: 1px solid #8e9ca8; border-radius: 2px;
  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #ffffff,stop:.48 #f6f6f6,stop:.52 #e5e5e5,stop:1 #f5f5f5); }
QPushButton:hover { border-color: #3c7fb1; background: #edf7fd; }
QPushButton:pressed { background: #d9edf9; }
QPushButton:disabled { color: #888; border-color: #c8c8c8; background: #f2f2f2; }
QPushButton#taskLink { min-height: 19px; border: 0; padding: 2px 2px; text-align: left; color: #1f4e99; background: transparent; }
QPushButton#taskLink:hover { color: #0033aa; text-decoration: underline; }
QFrame#taskBox { background: transparent; border: 0; }
QTreeWidget { border: 0; background: white; alternate-background-color: white; outline: 0; }
QTreeWidget::item { min-height: 30px; border-bottom: 1px solid #eeeeee; }
QTreeWidget::item:selected { color: #000000; background: #dcecff; border: 1px solid #7da2ce; }
QHeaderView::section { background: #f0f0f0; border: 0; border-right: 1px solid #cccccc; border-bottom: 1px solid #cccccc; padding: 4px; }
QStatusBar { min-height: 25px; background: #f4f7fb; border-top: 1px solid #d2d9df; }
QGroupBox { margin-top: 14px; border: 1px solid #c9d2db; background: #ffffff; }
QGroupBox::title { subcontrol-origin: margin; left: 9px; padding: 0 4px; color: #1a5dab; }
QCheckBox { spacing: 7px; }
QComboBox { min-height: 22px; border: 1px solid #8e9ca8; background: white; padding: 0 5px; }
/* Shared Aero scrollbar treatment for Home, lists, Settings and dialogs. */
QScrollBar:vertical { width: 17px; margin: 17px 0; border: 1px solid #b7c4cf;
  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #e7eef4,stop:.48 #f7f9fb,stop:1 #d9e5ef); }
QScrollBar:horizontal { height: 17px; margin: 0 17px; border: 1px solid #b7c4cf;
  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #e7eef4,stop:.48 #f7f9fb,stop:1 #d9e5ef); }
QScrollBar::handle:vertical, QScrollBar::handle:horizontal { min-height: 28px; min-width: 28px;
  border: 1px solid #8fa7ba; border-radius: 2px;
  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #f9fcff,stop:.45 #dcecf7,stop:.52 #c0d7e8,stop:1 #eaf4fb); }
QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover { border-color: #4b8fbd;
  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #f9fdff,stop:.45 #c5e8fa,stop:.52 #93c9e9,stop:1 #d5eefb); }
QScrollBar::handle:vertical:pressed, QScrollBar::handle:horizontal:pressed { border-color: #3579a6; background: #9ccce9; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { border: 1px solid #9eafbf;
  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #ffffff,stop:.5 #edf5fa,stop:1 #d7e5ef); }
QScrollBar::add-line:vertical:hover, QScrollBar::sub-line:vertical:hover,
QScrollBar::add-line:horizontal:hover, QScrollBar::sub-line:horizontal:hover { border-color: #4b8fbd; background: #d8effc; }
QScrollBar::add-line:vertical { subcontrol-position: bottom; subcontrol-origin: margin; height: 17px; }
QScrollBar::sub-line:vertical { subcontrol-position: top; subcontrol-origin: margin; height: 17px; }
QScrollBar::add-line:horizontal { subcontrol-position: right; subcontrol-origin: margin; width: 17px; }
QScrollBar::sub-line:horizontal { subcontrol-position: left; subcontrol-origin: margin; width: 17px; }
QScrollBar::up-arrow:vertical { image: url(:/scrollbar/up.svg); width: 8px; height: 8px; }
QScrollBar::down-arrow:vertical { image: url(:/scrollbar/down.svg); width: 8px; height: 8px; }
QScrollBar::left-arrow:horizontal { image: url(:/scrollbar/left.svg); width: 8px; height: 8px; }
QScrollBar::right-arrow:horizontal { image: url(:/scrollbar/right.svg); width: 8px; height: 8px; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }
QMessageBox, QProgressDialog { background: #f5f5f5; }
)");
}

} // namespace Win7Style
