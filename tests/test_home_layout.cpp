#include "ui/MainWindow.h"
#include "ui/ProgramCard.h"
#include "ui/WrappingGridLayout.h"
#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QStackedWidget>
#include <QTemporaryDir>
#include <QToolButton>
#include <QWheelEvent>
#include <QtTest>

class HomeLayoutTest : public QObject {
    Q_OBJECT
    QTemporaryDir settings;
    static void fill(QWidget *host) {
        while (auto *item = host->layout()->takeAt(0)) { delete item->widget(); delete item; }
        for (int i = 0; i < 8; ++i) {
            Package package;
            package.name = QString("test-%1").arg(i);
            package.displayName = "Computer Management and File Utilities";
            package.summary = "A longer description that must wrap without hiding the install button or overlapping another card.";
            package.repository = "aero7";
            host->layout()->addWidget(new ProgramCard(package));
        }
    }
private slots:
    void initTestCase() {
        QVERIFY(settings.isValid());
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
    }
    void homeRemainsInsideViewport() {
        MainWindow window(parseDeepLink({"app", "--home"}));
        auto *scroll = window.findChild<QScrollArea *>("homeScroll");
        QVERIFY(scroll);
        auto *host = scroll->findChild<QWidget *>("recommendedGrid");
        QVERIFY(host);
        fill(host);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        for (int width : {1600, 1200, 900, 1200}) {
            window.resize(width, 780);
            // Wayland commits compositor-approved sizes asynchronously. Wait
            // for the requested size, not a fixed delay that can test the old
            // geometry or catch the scroll area's intermediate resize.
            QTRY_COMPARE(window.size(), QSize(width, 780));
            QTRY_COMPARE(scroll->widget()->width(), scroll->viewport()->width());
            qInfo() << "Home geometry" << "requested" << width << "window" << window.size()
                    << "content" << scroll->widget()->size() << "viewport" << scroll->viewport()->size()
                    << "minimum" << scroll->widget()->minimumSizeHint();
            QCOMPARE(scroll->horizontalScrollBar()->maximum(), 0);
            QVERIFY(scroll->widget()->width() <= scroll->viewport()->width());
            const auto cards = host->findChildren<ProgramCard *>(QString(), Qt::FindDirectChildrenOnly);
            QCOMPARE(cards.size(), 8);
            for (auto *card : cards) {
                QVERIFY2(host->rect().contains(card->geometry()), qPrintable(QString("Card outside host at width %1").arg(width)));
                for (auto *other : cards) if (other != card)
                    QVERIFY(!other->geometry().intersects(card->geometry()));
                for (auto *label : card->findChildren<QLabel *>()) {
                    const QRect geometry(label->mapTo(card, QPoint()), label->size());
                    QVERIFY(card->rect().contains(geometry));
                    if (label->wordWrap()) QVERIFY(label->height() >= label->heightForWidth(label->width()));
                }
            }
        }
    }
    void emptyAndSingleCard() {
        QWidget host;
        auto *layout = new WrappingGridLayout(&host);
        QCOMPARE(layout->heightForWidth(400), 0);
        Package package;
        package.displayName = "Paint";
        layout->addWidget(new ProgramCard(package));
        host.resize(400, layout->heightForWidth(400));
        host.show();
        QTest::qWait(50);
        QCOMPARE(layout->count(), 1);
        QVERIFY(host.rect().contains(layout->itemAt(0)->geometry()));
    }
    void settingsOpensSeparateWindow() {
        MainWindow window(parseDeepLink({"app", "--home"}));
        auto *sidebar = window.findChild<QListWidget *>("sidebar");
        auto *pages = window.findChild<QStackedWidget *>();
        QVERIFY(sidebar);
        QVERIFY(pages);
        QToolButton *settingsButton = nullptr;
        for (auto *button : window.findChildren<QToolButton *>()) {
            if (button->text() == "Settings") settingsButton = button;
            QVERIFY(button->text() != "Control Panel");
        }
        QVERIFY(settingsButton);
        for (int row = 0; row < sidebar->count(); ++row)
            QVERIFY(sidebar->item(row)->text() != "Software Sources");
        settingsButton->click();
        auto *settingsWindow = window.findChild<QDialog *>("settingsWindow");
        QVERIFY(settingsWindow);
        QVERIFY(settingsWindow->isWindow());
        QVERIFY(settingsWindow->isVisible());
        QCOMPARE(pages->currentWidget()->objectName(), QString("homeScroll"));
        for (const QString source : {"Chaotic-AUR", "CachyOS Applications", "Flathub"}) {
            QGroupBox *box = nullptr;
            for (auto *candidate : settingsWindow->findChildren<QGroupBox *>())
                if (candidate->title() == source) box = candidate;
            QVERIFY(box);
            auto *toggle = box->findChild<QCheckBox *>();
            QVERIFY(toggle);
            QVERIFY(!toggle->isChecked());
        }
        QVERIFY(settingsWindow->findChildren<QPushButton *>().size() >= 2);
        settingsWindow->close();
        QVERIFY(!settingsWindow->isVisible());
        settingsButton->click();
        QVERIFY(settingsWindow->isVisible());
    }
    void scrollbarUsesBundledStyleAndStillScrolls() {
        MainWindow window(parseDeepLink({"app", "--home"}));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        auto *home = window.findChild<QScrollArea *>("homeScroll");
        QVERIFY(home);
        QToolButton *button = nullptr;
        for (auto *candidate : window.findChildren<QToolButton *>())
            if (candidate->text() == QStringLiteral("Settings")) button = candidate;
        QVERIFY(button);
        button->click();
        auto *dialog = window.findChild<QDialog *>("settingsWindow");
        QVERIFY(dialog);
        auto *settingsScroll = dialog->findChild<QScrollArea *>("settingsScroll");
        QVERIFY(settingsScroll);
        QVERIFY(window.styleSheet().contains(QStringLiteral("QScrollBar::handle")));
        QVERIFY(dialog->styleSheet().contains(QStringLiteral("QScrollBar::handle")));
        QCOMPARE(settingsScroll->verticalScrollBar()->width(), 17);
        for (const QSize size : {QSize(900, 720), QSize(720, 560)}) {
            dialog->resize(size);
            QTRY_COMPARE(dialog->size(), size);
            auto *bar = settingsScroll->verticalScrollBar();
            QVERIFY(bar->maximum() > 0);
            bar->setValue(0);
            QWheelEvent wheel(QPointF(10, 10), QPointF(10, 10), QPoint(), QPoint(0, -120),
                              Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
            QApplication::sendEvent(settingsScroll->viewport(), &wheel);
            QVERIFY(bar->value() > 0);
            bar->setValue(0);
            bar->setFocus();
            QTest::keyClick(bar, Qt::Key_Down);
            QVERIFY(bar->value() > 0);
        }
        QVERIFY(home->horizontalScrollBar()->maximum() == 0);
    }
};
QTEST_MAIN(HomeLayoutTest)
#include "test_home_layout.moc"
