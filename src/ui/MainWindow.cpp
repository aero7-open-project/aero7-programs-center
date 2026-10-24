#include "MainWindow.h"

#include "Aero7Icon.h"
#include "PackageIcon.h"
#include "metadata/BrowseCategories.h"
#include "ProgramCard.h"
#include "SourceBadge.h"
#include "Win7Style.h"
#include "WrappingGridLayout.h"
#include "backend/PackageTransaction.h"
#include "sources/SourceRegistration.h"

#include <QCheckBox>
#include <QColor>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QEventLoop>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QProgressDialog>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSettings>
#include <QSignalBlocker>
#include <QShortcut>
#include <QStackedWidget>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QTimer>
#include <QTemporaryFile>
#include <QUrl>

#include <algorithm>

namespace {
QPushButton *taskLink(const QString &text, const QIcon &icon)
{
    auto *button = new QPushButton(icon, text);
    button->setObjectName(QStringLiteral("taskLink"));
    button->setFlat(true);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

QPushButton *taskLink(const QString &text, const QString &icon)
{
    return taskLink(text, Aero7Icons::icon(icon));
}

QLabel *sectionHeading(const QString &text)
{
    auto *label = new QLabel(text);
    label->setObjectName(QStringLiteral("sectionHeading"));
    return label;
}

QToolButton *commandButton(const QString &text, const QString &icon, const QString &description)
{
    auto *button = new QToolButton;
    button->setText(text);
    button->setIcon(Aero7Icons::icon(icon));
    button->setIconSize(QSize(18, 18));
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setToolTip(description);
    button->setAccessibleName(description);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

QString statusText(const Package &package)
{
    if (!package.blockedReason.isEmpty()) return QStringLiteral("Unavailable");
    if (package.state == PackageState::UpdateAvailable) return QStringLiteral("Update available");
    if (package.state == PackageState::Installed) return QStringLiteral("Installed");
    return QStringLiteral("Available");
}

void styleProgramRow(QTreeWidgetItem *item, const Package &package)
{
    QFont title = item->font(0);
    title.setPointSize(10);
    item->setFont(0, title);
    item->setForeground(0, QColor(QStringLiteral("#1e7b1e")));
    item->setToolTip(0, package.displayName);
    item->setToolTip(3, SourcePolicy::descriptor(package.source).name);
}
}

MainWindow::MainWindow(const DeepLink &link, QWidget *parent)
    : QMainWindow(parent), m_backend(this)
{
    setWindowTitle(QStringLiteral("Aero7 Programs Center"));
    setWindowIcon(Aero7Icons::resource(QStringLiteral(":/icons/aero7-programs-center.png"), QStringLiteral("programs center")));
    buildUi();
    loadData();
    QSettings settings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter"));
    restoreGeometry(settings.value(QStringLiteral("geometry")).toByteArray());
    if (width() < 900) resize(1180, 760);
    if (link.page == DeepLink::Updates) showUpdates();
    else if (link.page == DeepLink::Installed) showInstalled();
    else if (link.page == DeepLink::Sources) {
        showHome();
        QTimer::singleShot(0, this, &MainWindow::showSettings);
    }
    else if (link.page == DeepLink::Search) showResults(link.value);
    else if (link.page == DeepLink::PackageDetails) showDetails(link.value);
    else if (link.page == DeepLink::Category) showResults({}, link.value);
    else showHome();
}

MainWindow::~MainWindow() = default;

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter")).setValue(QStringLiteral("geometry"), saveGeometry());
    QMainWindow::closeEvent(event);
}

void MainWindow::buildUi()
{
    auto *navigation = new QToolBar;
    navigation->setObjectName(QStringLiteral("navigationBar"));
    navigation->setMovable(false);
    navigation->setIconSize(QSize(22, 22));
    addToolBar(navigation);
    auto *back = navigation->addAction(Aero7Icons::icon(QStringLiteral("back")), QStringLiteral("Back"));
    back->setShortcut(QKeySequence(QStringLiteral("Alt+Left")));
    auto *forward = navigation->addAction(Aero7Icons::icon(QStringLiteral("forward")), QStringLiteral("Forward"));
    forward->setEnabled(false);
    m_breadcrumb = new QLabel(QStringLiteral("Programs Center"));
    m_breadcrumb->setObjectName(QStringLiteral("breadcrumb"));
    m_breadcrumb->setMinimumWidth(250);
    m_breadcrumb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    navigation->addWidget(m_breadcrumb);
    m_search = new QLineEdit;
    m_search->setObjectName(QStringLiteral("searchBox"));
    m_search->setPlaceholderText(QStringLiteral("Search Programs Center"));
    m_search->setClearButtonEnabled(true);
    m_search->setMinimumWidth(175);
    m_search->setMaximumWidth(285);
    m_search->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    navigation->addWidget(m_search);

    auto *shell = new QWidget;
    auto *shellLayout = new QVBoxLayout(shell);
    shellLayout->setContentsMargins(0, 0, 0, 0);
    shellLayout->setSpacing(0);
    QHBoxLayout *commands = nullptr;
    shellLayout->addWidget(Win7Style::commandBar(&commands));
    auto *home = commandButton(QStringLiteral("Home"), QStringLiteral("home"), QStringLiteral("Home"));
    auto *discover = commandButton(QStringLiteral("Find Program"), QStringLiteral("search"), QStringLiteral("Find a program"));
    auto *updates = commandButton(QStringLiteral("Updates"), QStringLiteral("update"), QStringLiteral("Check for updates"));
    auto *installed = commandButton(QStringLiteral("Installed"), QStringLiteral("package"), QStringLiteral("Installed programs"));
    auto *settings = commandButton(QStringLiteral("Settings"), QStringLiteral("settings"), QStringLiteral("Settings"));
    for (auto *button : {home, discover, updates, installed, settings}) {
        button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        commands->addWidget(button);
    }
    commands->addStretch();
    auto *body = new QWidget;
    auto *bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);
    m_sidebar = new QListWidget;
    m_sidebar->setObjectName(QStringLiteral("sidebar"));
    m_sidebar->setFixedWidth(205);
    m_sidebar->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_sidebar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_sidebar->setTextElideMode(Qt::ElideRight);
    auto *categoryHeading = new QListWidgetItem(QStringLiteral("CATEGORIES"));
    categoryHeading->setFlags(Qt::NoItemFlags);
    m_sidebar->addItem(categoryHeading);
    for (const auto &category : BrowseCategories::all()) {
        auto *item = new QListWidgetItem(Aero7Icons::categoryIcon(category.icon), category.label);
        item->setData(Qt::UserRole, category.id);
        item->setToolTip(category.label);
        m_sidebar->addItem(item);
    }
    bodyLayout->addWidget(m_sidebar);
    m_pages = new QStackedWidget;
    m_home = makeHome();
    m_results = makeListPage();
    m_pages->addWidget(m_home);
    m_pages->addWidget(m_results);
    bodyLayout->addWidget(m_pages, 1);
    shellLayout->addWidget(body, 1);
    setCentralWidget(shell);

    m_status = new QLabel(QStringLiteral("Loading software information…"));
    statusBar()->addWidget(m_status, 1);
    statusBar()->addPermanentWidget(new QLabel(QStringLiteral("Aero7 Programs Center — Beta")));
    connect(home, &QToolButton::clicked, this, &MainWindow::showHome);
    connect(discover, &QToolButton::clicked, this, [this] { showResults(); m_search->setFocus(); });
    connect(updates, &QToolButton::clicked, this, &MainWindow::showUpdates);
    connect(installed, &QToolButton::clicked, this, &MainWindow::showInstalled);
    connect(settings, &QToolButton::clicked, this, &MainWindow::showSettings);
    connect(back, &QAction::triggered, this, &MainWindow::showHome);
    auto *searchDelay = new QTimer(this);
    searchDelay->setSingleShot(true);
    searchDelay->setInterval(180);
    connect(m_search, &QLineEdit::textChanged, searchDelay, qOverload<>(&QTimer::start));
    connect(searchDelay, &QTimer::timeout, this, [this] { showResults(m_search->text()); });
    connect(m_search, &QLineEdit::returnPressed, this, [this, searchDelay] { searchDelay->stop(); showResults(m_search->text()); });
    connect(m_sourceFilter, &QComboBox::currentIndexChanged, this, [this] {
        if (m_pages->currentWidget() == m_results)
            showResults(m_activeQuery, m_activeCategoryId);
    });
    connect(m_sidebar, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (item->flags() != Qt::NoItemFlags)
            showResults({}, item->data(Qt::UserRole).toString());
    });
    new QShortcut(QKeySequence::Find, this, [this] { m_search->setFocus(); m_search->selectAll(); });
    new QShortcut(QKeySequence::Refresh, this, [this] { loadData(); showHome(); });
    setStyleSheet(Win7Style::applicationStyleSheet());
}

QWidget *MainWindow::makeHome()
{
    auto *scroll = new QScrollArea;
    scroll->setObjectName(QStringLiteral("homeScroll"));
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *content = new QWidget;
    content->setObjectName(QStringLiteral("page"));
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(32, 23, 34, 28);
    layout->setSpacing(12);
    layout->addWidget(Win7Style::pageTitle(QStringLiteral("Get programs for Aero7")));
    layout->addWidget(Win7Style::instruction(QStringLiteral("Find trusted desktop programs, install updates, and manage software sources.")));
    layout->addWidget(Win7Style::separator());
    auto *notice = new QLabel;
    notice->setObjectName(QStringLiteral("catalogNotice"));
    notice->setWordWrap(true);
    layout->addWidget(notice);
    m_catalogButton = taskLink(QStringLiteral("Install the application catalog"), QStringLiteral("install"));
    layout->addWidget(m_catalogButton);
    connect(m_catalogButton, &QPushButton::clicked, this, [this] {
        if (const auto *package = findPackage(QStringLiteral("archlinux-appstream-data"))) runAction(*package);
    });
    layout->addWidget(sectionHeading(QStringLiteral("Featured programs")));
    auto *featured = new QWidget;
    featured->setObjectName(QStringLiteral("featuredGrid"));
    featured->setLayout(new WrappingGridLayout);
    layout->addWidget(featured);
    layout->addWidget(sectionHeading(QStringLiteral("Browse by category")));
    auto *categories = new WrappingGridLayout;
    for (const auto &category : BrowseCategories::all()) {
        if (!category.homeShortcut) continue;
        auto *button = taskLink(category.label, Aero7Icons::categoryIcon(category.icon));
        connect(button, &QPushButton::clicked, this, [this, id = category.id] { showResults({}, id); });
        categories->addWidget(button);
    }
    layout->addLayout(categories);
    m_recommendedSection = new QWidget;
    auto *recommendedLayout = new QVBoxLayout(m_recommendedSection);
    recommendedLayout->setContentsMargins(0, 0, 0, 0);
    recommendedLayout->addWidget(sectionHeading(QStringLiteral("Recommended programs")));
    auto *recommended = new QWidget;
    recommended->setObjectName(QStringLiteral("recommendedGrid"));
    recommended->setLayout(new WrappingGridLayout);
    recommendedLayout->addWidget(recommended);
    layout->addWidget(m_recommendedSection);
    layout->addStretch();
    scroll->setWidget(content);
    return scroll;
}

QWidget *MainWindow::makeListPage()
{
    auto *page = new QWidget;
    page->setObjectName(QStringLiteral("page"));
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 18, 28, 18);
    layout->setSpacing(8);
    auto *title = Win7Style::pageTitle(QStringLiteral("Software"));
    title->setObjectName(QStringLiteral("listTitle"));
    layout->addWidget(title);
    layout->addWidget(Win7Style::instruction(QStringLiteral("Select a program to view details or double-click to open it.")));
    layout->addWidget(Win7Style::separator());
    m_filterRow = new QWidget;
    auto *filterRow = new QHBoxLayout(m_filterRow);
    filterRow->setContentsMargins(0, 0, 0, 0);
    filterRow->addStretch();
    filterRow->addWidget(new QLabel(QStringLiteral("Source:")));
    m_sourceFilter = new QComboBox;
    m_sourceFilter->addItem(QStringLiteral("All sources"), QStringLiteral("all"));
    for (const auto &source : SourcePolicy::descriptors())
        if (source.source != SoftwareSource::Local && source.source != SoftwareSource::Unknown)
            m_sourceFilter->addItem(source.name, source.id);
    m_sourceFilter->addItem(QStringLiteral("External sources"), QStringLiteral("unknown"));
    filterRow->addWidget(m_sourceFilter);
    layout->addWidget(m_filterRow);
    m_table = new QTreeWidget;
    m_table->setIconSize(QSize(22, 22));
    m_table->setColumnCount(5);
    m_table->setHeaderLabels({QStringLiteral("Name"), QStringLiteral("Version"), QStringLiteral("Size"), QStringLiteral("Source"), QStringLiteral("Status")});
    Win7Style::configureTree(m_table);
    m_table->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    layout->addWidget(m_table, 1);
    connect(m_table, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item) { showDetails(item->data(0, Qt::UserRole).toString()); });
    return page;
}

QWidget *MainWindow::makeSettingsPage()
{
    auto *scroll = new QScrollArea;
    scroll->setObjectName(QStringLiteral("settingsScroll"));
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *content = new QWidget;
    content->setObjectName(QStringLiteral("page"));
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(32, 23, 38, 28);
    layout->setSpacing(9);
    layout->addWidget(Win7Style::pageTitle(QStringLiteral("Settings")));
    layout->addWidget(Win7Style::instruction(QStringLiteral("Manage software sources, install permissions and browsing preferences.")));
    layout->addWidget(Win7Style::separator());
    layout->addWidget(sectionHeading(QStringLiteral("Software sources")));
    layout->addWidget(Win7Style::instruction(QStringLiteral("Official sources are managed by the system. Optional sources show available software only after you opt in.")));
    auto addSource = [this, layout](SoftwareSource source, bool fixed) {
        const auto info = SourcePolicy::descriptor(source);
        auto *group = new QGroupBox(info.name);
        auto *row = new QHBoxLayout(group);
        row->setContentsMargins(11, 7, 11, 7);
        row->setSpacing(10);
        auto *icon = new QLabel;
        const bool available = fixed && officialSourceConfigured(source);
        icon->setPixmap(Aero7Icons::sourceIcon(source, available).pixmap(68, 38));
        if (available) icon->setToolTip(QStringLiteral("Official source configured on this system"));
        icon->setFixedSize(70, 42);
        icon->setAlignment(Qt::AlignCenter);
        row->addWidget(icon, 0, Qt::AlignTop);
        auto *copy = new QVBoxLayout;
        copy->setSpacing(3);
        auto *description = Win7Style::instruction(info.description);
        copy->addWidget(description);
        copy->addWidget(Win7Style::instruction(fixed ? QStringLiteral("Managed by the system")
            : QStringLiteral("External source · Apps and updates may be maintained by another project.")));
        row->addLayout(copy, 1);
        if (fixed) {
            auto *status = new QWidget;
            auto *statusLayout = new QHBoxLayout(status);
            statusLayout->setContentsMargins(0, 0, 0, 0);
            statusLayout->setSpacing(7);
            auto *label = new QLabel(available ? QStringLiteral("Official source\nAvailable")
                                                 : QStringLiteral("Official source\nNot configured"));
            label->setObjectName(QStringLiteral("officialSourceStatus"));
            statusLayout->addWidget(label);
            row->addWidget(status, 0, Qt::AlignRight | Qt::AlignVCenter);
        } else {
            auto *toggle = new QCheckBox(QStringLiteral("Show apps"));
            toggle->setChecked(sourceEnabled(source));
            row->addWidget(toggle, 0, Qt::AlignRight | Qt::AlignVCenter);
            connect(toggle, &QCheckBox::toggled, this, [this, info, toggle](bool enabled) {
            if (enabled && QMessageBox::warning(this, QStringLiteral("Enable external source?"),
                QStringLiteral("%1 is maintained outside Aero7 and Arch Linux. Its packages may change system software. Programs Center will show its available apps only if the source is already configured; pacman and Flatpak signature rules still apply.\n\nShow apps from this source?").arg(info.name),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
                QSignalBlocker block(toggle); toggle->setChecked(false); return;
            }
            QSettings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter")).setValue(QStringLiteral("sources/%1").arg(info.id), enabled);
            loadData();
            });
        }
        layout->addWidget(group);
    };
    addSource(SoftwareSource::Aero7, true);
    addSource(SoftwareSource::Arch, true);
    addSource(SoftwareSource::ChaoticAur, false);
    addSource(SoftwareSource::CachyOs, false);
    addSource(SoftwareSource::Flathub, false);
    layout->addWidget(Win7Style::instruction(QStringLiteral("Additional native repositories require administrator approval and existing trusted signing keys. CachyOS Applications stays within the Aero7 allowlist. Installed programs remain manageable when a source is hidden.")));
    auto *custom = new QWidget;
    m_customSourcesLayout = new QVBoxLayout(custom);
    m_customSourcesLayout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(custom);
    auto *addButton = new QPushButton(Aero7Icons::icon(QStringLiteral("install")), QStringLiteral("Add source…"));
    layout->addWidget(addButton, 0, Qt::AlignLeft);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addSource);
    refreshCustomSources();
    layout->addWidget(Win7Style::separator());
    layout->addWidget(sectionHeading(QStringLiteral("Install permissions")));
    m_passwordlessCheck = new QCheckBox(QStringLiteral("Install trusted Aero7 and Arch applications without asking for a password"));
    m_passwordlessCheck->setObjectName(QStringLiteral("trustedInstallCheck"));
    m_passwordlessCheck->setChecked(trustedInstallAllowed());
    m_passwordlessCheck->setEnabled(QFileInfo::exists(QStringLiteral("/usr/lib/aero7-programs-center/aero7-programs-center-permissions")));
    layout->addWidget(m_passwordlessCheck);
    layout->addWidget(Win7Style::instruction(QStringLiteral("An administrator must approve this setting once. It applies only to your active local session and to Aero7 or official Arch packages whose dependencies also come from those sources. Removing software, system updates and third-party native packages still require authentication. Flatpak apps install in your user account.")));
    if (!m_passwordlessCheck->isEnabled())
        layout->addWidget(Win7Style::instruction(QStringLiteral("Install the packaged Programs Center to enable this setting.")));
    connect(m_passwordlessCheck, &QCheckBox::toggled, this, &MainWindow::changeTrustedInstallPermission);
    layout->addWidget(Win7Style::separator());
    layout->addWidget(sectionHeading(QStringLiteral("Browsing")));
    auto *recommendedCheck = new QCheckBox(QStringLiteral("Show recommended programs on Home"));
    recommendedCheck->setChecked(QSettings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter"))
        .value(QStringLiteral("browsing/showRecommended"), true).toBool());
    layout->addWidget(recommendedCheck);
    connect(recommendedCheck, &QCheckBox::toggled, this, [this](bool checked) {
        QSettings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter"))
            .setValue(QStringLiteral("browsing/showRecommended"), checked);
        m_recommendedSection->setVisible(checked);
    });
    m_recommendedSection->setVisible(recommendedCheck->isChecked());
    layout->addWidget(Win7Style::separator());
    layout->addWidget(sectionHeading(QStringLiteral("Safety notices")));
    layout->addWidget(Win7Style::instruction(QStringLiteral("Show the first-install explanation again for every external source.")));
    auto *resetNotices = new QPushButton(QStringLiteral("Reset dismissed notices"));
    layout->addWidget(resetNotices, 0, Qt::AlignLeft);
    connect(resetNotices, &QPushButton::clicked, this, [this] {
        QSettings settings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter"));
        settings.remove(QStringLiteral("notices"));
        QMessageBox::information(this, QStringLiteral("Programs Center"), QStringLiteral("External-source notices will appear again before installs."));
    });
    layout->addStretch();
    scroll->setWidget(content);
    return scroll;
}

bool MainWindow::sourceEnabled(SoftwareSource source) const
{
    if (source == SoftwareSource::Aero7 || source == SoftwareSource::Arch || source == SoftwareSource::Local) return true;
    if (source == SoftwareSource::Unknown) return false;
    const auto info = SourcePolicy::descriptor(source);
    return QSettings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter")).value(QStringLiteral("sources/%1").arg(info.id), false).toBool();
}

bool MainWindow::sourceEnabled(const Package &package) const
{
    if (package.source != SoftwareSource::Unknown) return sourceEnabled(package.source);
    if (!SourceRegistration::validName(package.repository)) return false;
    const QString group = package.provider == PackageProvider::Flatpak
        ? QStringLiteral("customFlatpak") : QStringLiteral("customNative");
    return QSettings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter"))
        .value(QStringLiteral("%1/%2").arg(group, package.repository), false).toBool();
}

bool MainWindow::officialSourceConfigured(SoftwareSource source) const
{
    if (source != SoftwareSource::Aero7 && source != SoftwareSource::Arch) return false;
    for (const QString &repository : m_backend.repositories())
        if (SourcePolicy::fromRepository(repository) == source) return true;
    return false;
}

bool MainWindow::officialSourceConfigured(const Package &package) const
{
    return officialSourceConfigured(package.source)
        && m_backend.repositories().contains(package.repository)
        && SourcePolicy::fromRepository(package.repository) == package.source;
}

QString MainWindow::sourceLabel(const Package &package) const
{
    return package.source == SoftwareSource::Unknown
        ? QStringLiteral("External: %1").arg(package.repository)
        : SourcePolicy::descriptor(package.source).name;
}

void MainWindow::runSourceCommand(const QString &program, const QStringList &arguments,
                                  const std::function<void()> &onSuccess,
                                  const QString &cleanupPath)
{
    auto *process = new QProcess(this);
    auto *progress = new QProgressDialog(QStringLiteral("Updating software sources…"), QString(), 0, 0, this);
    progress->setWindowTitle(QStringLiteral("Programs Center"));
    progress->setCancelButton(nullptr);
    progress->setWindowModality(Qt::WindowModal);
    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
        [this, process, progress, onSuccess, cleanupPath](int code, QProcess::ExitStatus status) {
            progress->close(); progress->deleteLater();
            const QString details = QString::fromUtf8(process->readAllStandardError()).trimmed();
            process->deleteLater();
            if (!cleanupPath.isEmpty()) QFile::remove(cleanupPath);
            if (status == QProcess::NormalExit && code == 0) onSuccess();
            else QMessageBox::warning(this, QStringLiteral("Source change failed"),
                details.isEmpty() ? QStringLiteral("The source change was not completed.") : details.left(900));
        });
    process->start(program, arguments);
    if (!process->waitForStarted(3000)) {
        if (!cleanupPath.isEmpty()) QFile::remove(cleanupPath);
        progress->deleteLater(); process->deleteLater();
        QMessageBox::warning(this, QStringLiteral("Source change failed"),
            QStringLiteral("The required source-management tool could not be started."));
        return;
    }
    progress->show();
}

void MainWindow::refreshCustomSources()
{
    if (!m_customSourcesLayout) return;
    while (auto *item = m_customSourcesLayout->takeAt(0)) {
        delete item->widget(); delete item;
    }
    QFile managed(QStringLiteral("/etc/pacman.d/aero7-programs-center-repositories.conf"));
    QStringList native;
    if (managed.open(QIODevice::ReadOnly)) {
        const QString text = QString::fromUtf8(managed.readAll());
        const QRegularExpression heading(QStringLiteral("(?m)^\\[([a-z0-9][a-z0-9._-]*)\\]\\s*$"));
        auto match = heading.globalMatch(text);
        while (match.hasNext()) native.append(match.next().captured(1));
    }
    QSettings settings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter"));
    settings.beginGroup(QStringLiteral("customFlatpak"));
    const QStringList flatpaks = settings.childKeys();
    settings.endGroup();
    if (native.isEmpty() && flatpaks.isEmpty()) {
        m_customSourcesLayout->addWidget(Win7Style::instruction(QStringLiteral("No additional sources have been added.")));
        return;
    }
    auto addRow = [this](const QString &name, bool flatpak) {
        const QString group = flatpak ? QStringLiteral("customFlatpak") : QStringLiteral("customNative");
        auto *box = new QGroupBox(QStringLiteral("External %1: %2").arg(flatpak ? QStringLiteral("Flatpak remote") : QStringLiteral("native repository"), name));
        auto *row = new QHBoxLayout(box);
        auto *icon = new QLabel;
        icon->setPixmap(Aero7Icons::sourceIcon(SoftwareSource::Unknown).pixmap(28, 28));
        row->addWidget(icon);
        auto *description = Win7Style::instruction(flatpak
            ? QStringLiteral("User Flatpak remote · Signed apps from an external publisher")
            : QStringLiteral("System pacman repository · Signed packages from an external publisher"));
        row->addWidget(description, 1);
        auto *visible = new QCheckBox(QStringLiteral("Show apps"));
        visible->setChecked(QSettings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter"))
            .value(QStringLiteral("%1/%2").arg(group, name), false).toBool());
        row->addWidget(visible);
        connect(visible, &QCheckBox::toggled, this, [this, group, name, visible](bool checked) {
            if (checked && QMessageBox::warning(this, QStringLiteral("Enable external source?"),
                QStringLiteral("%1 is maintained outside Aero7 and Arch Linux. Its available applications and updates may affect your system. Show apps from this source?").arg(name),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
                QSignalBlocker block(visible); visible->setChecked(false); return;
            }
            QSettings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter"))
                .setValue(QStringLiteral("%1/%2").arg(group, name), checked);
            loadData();
        });
        auto *remove = new QPushButton(QStringLiteral("Remove…"));
        row->addWidget(remove);
        connect(remove, &QPushButton::clicked, this, [this, name, flatpak, group] {
            const QString message = flatpak
                ? QStringLiteral("Remove the user Flatpak remote %1? Installed apps may prevent Flatpak from removing it.").arg(name)
                : QStringLiteral("Remove the managed pacman repository %1? Installed packages will remain installed. The system's other repositories are unchanged.").arg(name);
            if (QMessageBox::question(this, QStringLiteral("Remove external source?"), message) != QMessageBox::Yes) return;
            const QString program = flatpak ? QStringLiteral("flatpak") : QStringLiteral("pkexec");
            const QStringList arguments = flatpak
                ? QStringList{QStringLiteral("remote-delete"), QStringLiteral("--user"), name}
                : QStringList{QStringLiteral("--disable-internal-agent"),
                    QStringLiteral("/usr/lib/aero7-programs-center/aero7-programs-center-sources"),
                    QStringLiteral("remove"), name};
            runSourceCommand(program, arguments, [this, group, name] {
                QSettings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter"))
                    .remove(QStringLiteral("%1/%2").arg(group, name));
                loadData(); refreshCustomSources();
            });
        });
        m_customSourcesLayout->addWidget(box);
    };
    for (const QString &name : native) addRow(name, false);
    for (const QString &name : flatpaks) if (settings.value(QStringLiteral("customFlatpak/%1").arg(name)).isValid()) addRow(name, true);
}

void MainWindow::addSource()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Add software source"));
    dialog.setMinimumWidth(560);
    dialog.setStyleSheet(Win7Style::applicationStyleSheet());
    auto *layout = new QVBoxLayout(&dialog);
    layout->addWidget(Win7Style::instruction(QStringLiteral("External sources can publish packages and updates. Check the publisher, repository address and signing key before continuing.")));
    auto *form = new QFormLayout;
    auto *kind = new QComboBox;
    kind->addItem(QStringLiteral("Signed native pacman repository"));
    kind->addItem(QStringLiteral("User Flatpak remote (.flatpakrepo)"));
    auto *name = new QLineEdit;
    name->setPlaceholderText(QStringLiteral("example-repo"));
    auto *url = new QLineEdit;
    url->setPlaceholderText(QStringLiteral("https://example.org/repo/$arch"));
    auto *fingerprint = new QLineEdit;
    fingerprint->setPlaceholderText(QStringLiteral("40 hexadecimal characters; already trusted by pacman"));
    form->addRow(QStringLiteral("Type:"), kind);
    form->addRow(QStringLiteral("Source name:"), name);
    form->addRow(QStringLiteral("Server / descriptor URL:"), url);
    form->addRow(QStringLiteral("Trusted key fingerprint:"), fingerprint);
    layout->addLayout(form);
    auto *hint = Win7Style::instruction(QStringLiteral("Native repositories require a fully trusted signing key already in pacman's keyring. Both database and package signatures are required. This dialog never imports or trusts keys. New native package listings appear after the next full system refresh."));
    layout->addWidget(hint);
    connect(kind, &QComboBox::currentIndexChanged, &dialog, [fingerprint, hint, url](int index) {
        fingerprint->setVisible(index == 0);
        hint->setText(index == 0
            ? QStringLiteral("Native repositories require a fully trusted signing key already in pacman's keyring. Both database and package signatures are required. This dialog never imports or trusts keys. New package listings appear after the next full system refresh.")
            : QStringLiteral("Choose a publisher's HTTPS .flatpakrepo file containing a GPG key. Flatpak adds the remote only for your user. Programs Center will show the key fingerprint before adding it."));
        url->setPlaceholderText(index == 0 ? QStringLiteral("https://example.org/repo/$arch")
            : QStringLiteral("https://example.org/example.flatpakrepo"));
    });
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    if (dialog.exec() != QDialog::Accepted) return;
    const QString sourceName = name->text().trimmed();
    const QString address = url->text().trimmed();
    if (kind->currentIndex() == 0) {
        const QString key = fingerprint->text().remove(QLatin1Char(' ')).toUpper();
        if (!SourceRegistration::validName(sourceName) || !QRegularExpression(QStringLiteral("^[A-F0-9]{40}$")).match(key).hasMatch()
            || !address.startsWith(QStringLiteral("https://")) || address.contains(QRegularExpression(QStringLiteral("[\\s;#\\[\\]]")))) {
            QMessageBox::warning(this, QStringLiteral("Invalid source"), QStringLiteral("Enter a unique lowercase source name, HTTPS Server URL and full 40-character signing-key fingerprint."));
            return;
        }
        if (QMessageBox::warning(this, QStringLiteral("Review native repository"),
            QStringLiteral("Add external repository %1?\n\nServer: %2\nReviewed trusted key: %3\n\nThe helper checks that this key is already fully trusted. Pacman may accept any key trusted by the system keyring; it does not pin this repository to one key. Both package and database signatures remain mandatory. Administrator authentication is required.").arg(sourceName, address, key),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;
        runSourceCommand(QStringLiteral("pkexec"), {QStringLiteral("--disable-internal-agent"),
            QStringLiteral("/usr/lib/aero7-programs-center/aero7-programs-center-sources"),
            QStringLiteral("add"), sourceName, address, key}, [this, sourceName] {
                QSettings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter"))
                    .setValue(QStringLiteral("customNative/%1").arg(sourceName), true);
                loadData(); refreshCustomSources();
                QMessageBox::information(this, QStringLiteral("Source added"),
                    QStringLiteral("The signed repository is configured. Refresh the system's package databases with a full update before browsing its applications."));
            });
        return;
    }
    const QUrl descriptorUrl(address);
    if (!SourceRegistration::validName(sourceName) || !descriptorUrl.isValid()
        || descriptorUrl.scheme() != QStringLiteral("https") || descriptorUrl.host().isEmpty()
        || !descriptorUrl.userInfo().isEmpty()
        || !descriptorUrl.path().endsWith(QStringLiteral(".flatpakrepo"))) {
        QMessageBox::warning(this, QStringLiteral("Invalid Flatpak source"),
            QStringLiteral("Enter a unique lowercase source name and direct HTTPS .flatpakrepo URL without credentials."));
        return;
    }
    QNetworkAccessManager network;
    QNetworkRequest request{QUrl(address)};
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    auto *reply = network.get(request);
    QEventLoop loop;
    QTimer timer; timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(12000); loop.exec();
    const QByteArray description = reply->readAll();
    const QUrl finalUrl = reply->url();
    const auto networkError = reply->error();
    reply->deleteLater();
    QString error;
    if (networkError != QNetworkReply::NoError || finalUrl.scheme() != QStringLiteral("https")
        || !SourceRegistration::validFlatpakRepo(sourceName, address, description, &error)) {
        QMessageBox::warning(this, QStringLiteral("Invalid Flatpak source"), error.isEmpty()
            ? QStringLiteral("The HTTPS repository description could not be downloaded.") : error);
        return;
    }
    const auto keyMatch = QRegularExpression(QStringLiteral("(?m)^GPGKey=([A-Za-z0-9+/=]+)\\s*$"))
        .match(QString::fromUtf8(description));
    const auto repositoryMatch = QRegularExpression(QStringLiteral("(?m)^Url=(https://[^\\s]+)\\s*$"))
        .match(QString::fromUtf8(description));
    QTemporaryFile keyFile;
    if (!keyFile.open()) return;
    keyFile.write(QByteArray::fromBase64(keyMatch.captured(1).toLatin1())); keyFile.flush();
    QProcess keyInfo;
    keyInfo.start(QStringLiteral("gpg"), {QStringLiteral("--batch"), QStringLiteral("--with-colons"),
        QStringLiteral("--show-keys"), QStringLiteral("--fingerprint"), keyFile.fileName()});
    if (!keyInfo.waitForFinished(5000) || keyInfo.exitCode() != 0) {
        QMessageBox::warning(this, QStringLiteral("Invalid Flatpak source"), QStringLiteral("The supplied GPG key could not be inspected."));
        return;
    }
    QStringList fingerprints;
    for (const auto &line : QString::fromUtf8(keyInfo.readAllStandardOutput()).split(QLatin1Char('\n')))
        if (line.startsWith(QStringLiteral("fpr:"))) fingerprints.append(line.split(QLatin1Char(':')).value(9));
    if (fingerprints.isEmpty() || QMessageBox::warning(this, QStringLiteral("Review Flatpak remote"),
        QStringLiteral("Add external Flatpak remote %1 for your user?\n\nDescriptor: %2\nRepository: %3\nGPG key fingerprint(s):\n%4\n\nCheck these details with the publisher before continuing.")
            .arg(sourceName, finalUrl.toString(), repositoryMatch.captured(1), fingerprints.join(QLatin1Char('\n'))),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;
    QTemporaryFile descriptorFile(QDir::tempPath() + QStringLiteral("/aero7-source-XXXXXX.flatpakrepo"));
    descriptorFile.setAutoRemove(false);
    if (!descriptorFile.open() || descriptorFile.write(description) != description.size()) {
        QMessageBox::warning(this, QStringLiteral("Source change failed"), QStringLiteral("The validated repository description could not be staged."));
        return;
    }
    descriptorFile.flush(); descriptorFile.close();
    const QString stagedPath = descriptorFile.fileName();
    runSourceCommand(QStringLiteral("flatpak"), {QStringLiteral("remote-add"), QStringLiteral("--user"),
        sourceName, stagedPath}, [this, sourceName] {
            QSettings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter"))
                .setValue(QStringLiteral("customFlatpak/%1").arg(sourceName), true);
            loadData(); refreshCustomSources();
        }, stagedPath);
}

bool MainWindow::trustedInstallAllowed() const
{
    QProcess check;
    check.start(QStringLiteral("pkcheck"), {QStringLiteral("--action-id"),
        QStringLiteral("org.aero7.programscenter.trusted-install"),
        QStringLiteral("--process"), QString::number(QCoreApplication::applicationPid())});
    return check.waitForFinished(2500) && check.exitStatus() == QProcess::NormalExit
        && check.exitCode() == 0;
}

void MainWindow::changeTrustedInstallPermission(bool enabled)
{
    m_passwordlessCheck->setEnabled(false);
    auto *process = new QProcess(this);
    connect(process, qOverload<int,QProcess::ExitStatus>(&QProcess::finished), this,
        [this, process](int code, QProcess::ExitStatus status) {
            const bool success = status == QProcess::NormalExit && code == 0;
            const QString error = QString::fromUtf8(process->readAllStandardError()).trimmed();
            process->deleteLater();
            { const QSignalBlocker block(m_passwordlessCheck);
              m_passwordlessCheck->setChecked(trustedInstallAllowed());
              m_passwordlessCheck->setEnabled(true); }
            if (!success) QMessageBox::warning(this, QStringLiteral("Install permissions"),
                error.isEmpty() ? QStringLiteral("The permission setting was not changed.") : error);
        });
    process->start(QStringLiteral("pkexec"), {QStringLiteral("--disable-internal-agent"),
        QStringLiteral("/usr/lib/aero7-programs-center/aero7-programs-center-permissions"),
        enabled ? QStringLiteral("enable") : QStringLiteral("disable")});
}

void MainWindow::loadData()
{
    m_catalog.load();
    m_metadata.load();
    QList<Package> all = m_backend.packages();
    for (auto &package : all) m_metadata.enrich(package);
    if (!m_metadata.nativeCatalogAvailable()) {
        const QStringList featured = Metadata::loadFeatured(QStringLiteral(":/catalog/featured.json"));
        for (auto &package : all) if (featured.contains(package.name)) {
            package.application = true;
            if (package.displayName == package.name) {
                const QHash<QString,QString> names = {{QStringLiteral("vlc"),QStringLiteral("VLC")},
                    {QStringLiteral("gimp"),QStringLiteral("GIMP")},
                    {QStringLiteral("krita"),QStringLiteral("Krita")},
                    {QStringLiteral("libreoffice-fresh"),QStringLiteral("LibreOffice")}};
                package.displayName = names.value(package.name, package.name);
            }
        }
    }
    all.append(m_metadata.flatpakPackages());
    QList<Package> enabled;
    for (const auto &package : all) if (sourceEnabled(package) || package.state != PackageState::Available) enabled.append(package);
    m_packages = m_catalog.groupProducts(enabled);
    const QString problem = !m_backend.lastError().isEmpty() ? m_backend.lastError() : !m_catalog.lastError().isEmpty() ? m_catalog.lastError() : m_backend.repositoryWarning();
    const auto appCount = std::count_if(m_packages.cbegin(), m_packages.cend(), [](const Package &package) { return package.application; });
    m_status->setText(!m_metadata.nativeCatalogAvailable() ? QStringLiteral("%1 programs shown; install the application catalog to discover more").arg(appCount)
        : problem.isEmpty() ? QStringLiteral("%1 programs available from enabled sources").arg(appCount)
        : QStringLiteral("Software information is incomplete: %1").arg(problem));
    if (m_catalogButton) m_catalogButton->setVisible(!m_metadata.nativeCatalogAvailable()
        && findPackage(QStringLiteral("archlinux-appstream-data")));
}

void MainWindow::setLocation(const QString &location)
{
    m_breadcrumb->setText(QStringLiteral("Programs Center  ›  %1").arg(location));
}

const Package *MainWindow::findPackage(const QString &id) const
{
    const auto found = std::find_if(m_packages.cbegin(), m_packages.cend(), [&id](const Package &package) {
        return package.name.compare(id, Qt::CaseInsensitive) == 0 || package.productId.compare(id, Qt::CaseInsensitive) == 0;
    });
    return found == m_packages.cend() ? nullptr : &*found;
}

void MainWindow::showHome()
{
    m_pages->setCurrentWidget(m_home);
    m_sidebar->clearSelection();
    m_recommendedSection->setVisible(QSettings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter"))
        .value(QStringLiteral("browsing/showRecommended"), true).toBool());
    setLocation(QStringLiteral("Home"));
    auto fill = [this](const char *name, const QStringList &wanted, int maximum) {
        auto *host = m_home->findChild<QWidget *>(QLatin1String(name));
        auto *layout = host->layout();
        while (auto *item = layout->takeAt(0)) { delete item->widget(); delete item; }
        int count = 0;
        for (const auto &package : m_packages) {
            if (!package.application) continue;
            if (!wanted.isEmpty() && !wanted.contains(package.name) && !wanted.contains(package.productId)) continue;
            auto *card = new ProgramCard(package, officialSourceConfigured(package));
            connect(card, &ProgramCard::actionRequested, this, [this](const QString &id) { if (const auto *p = findPackage(id)) runAction(*p); });
            connect(card, &ProgramCard::detailsRequested, this, &MainWindow::showDetails);
            layout->addWidget(card);
            if (++count >= maximum) break;
        }
    };
    fill("featuredGrid", {QStringLiteral("firefox"), QStringLiteral("vlc"), QStringLiteral("libreoffice-fresh"), QStringLiteral("gimp"), QStringLiteral("krita")}, 5);
    fill("recommendedGrid", {}, 8);
    const QString warning = !m_backend.lastError().isEmpty() ? m_backend.lastError() : m_backend.repositoryWarning();
    auto *notice = m_home->findChild<QLabel *>(QStringLiteral("catalogNotice"));
    const QString catalogWarning = !m_metadata.nativeCatalogAvailable()
        ? QStringLiteral("The full Arch application catalog is missing. Search still finds packages by name; install the catalog to browse all available apps.") : QString();
    notice->setText(QStringList{catalogWarning, warning}.filter(QRegularExpression(QStringLiteral(".+"))).join(QStringLiteral("\n")));
    notice->setVisible(!notice->text().isEmpty());
}

void MainWindow::showResults(const QString &query, const QString &category)
{
    m_pages->setCurrentWidget(m_results);
    m_filterRow->show();
    m_table->setRootIsDecorated(false);
    m_table->clear();
    const auto *browseCategory = BrowseCategories::find(category);
    m_activeQuery = query;
    m_activeCategoryId = browseCategory ? browseCategory->id : category;
    if (browseCategory) {
        for (int row = 1; row < m_sidebar->count(); ++row)
            if (m_sidebar->item(row)->data(Qt::UserRole).toString() == browseCategory->id) {
                m_sidebar->setCurrentRow(row);
                break;
            }
    } else m_sidebar->clearSelection();
    const QString title = browseCategory ? browseCategory->label
        : !category.isEmpty() ? category
        : query.isEmpty() ? QStringLiteral("Discover Software")
        : QStringLiteral("Search results for “%1”").arg(query);
    m_results->findChild<QLabel *>(QStringLiteral("listTitle"))->setText(title);
    setLocation(title);
    const QString sourceId = m_sourceFilter->currentData().toString();
    const QString needle = query.toCaseFolded();
    for (const auto &package : m_packages) {
        if (!category.isEmpty() && (browseCategory
            ? package.categoryId != browseCategory->id && package.category != browseCategory->label
            : package.category.compare(category, Qt::CaseInsensitive) != 0)) continue;
        if (!needle.isEmpty() && !package.searchableText().contains(needle)) continue;
        if (!package.application && (needle.isEmpty() || !package.searchableText().contains(needle))) continue;
        if (sourceId != QStringLiteral("all") && SourcePolicy::descriptor(package.source).id != sourceId) continue;
        auto *item = new QTreeWidgetItem({package.displayName, package.version, humanSize(package.installedSize), sourceLabel(package), statusText(package)});
        item->setIcon(0, packageArtwork(package));
        item->setIcon(3, Aero7Icons::sourceIcon(package.source, officialSourceConfigured(package)));
        styleProgramRow(item, package);
        item->setData(0, Qt::UserRole, package.productId);
        m_table->addTopLevelItem(item);
    }
    m_status->setText(QStringLiteral("%1 programs shown").arg(m_table->topLevelItemCount()));
}

void MainWindow::showInstalled()
{
    m_pages->setCurrentWidget(m_results);
    m_sidebar->clearSelection();
    m_filterRow->hide();
    m_table->setRootIsDecorated(false);
    m_table->clear();
    m_results->findChild<QLabel *>(QStringLiteral("listTitle"))->setText(QStringLiteral("Installed Programs"));
    setLocation(QStringLiteral("Installed Programs"));
    for (const auto &package : m_packages) if (package.state != PackageState::Available && package.application) {
        auto *item = new QTreeWidgetItem({package.displayName, package.installedVersion, humanSize(package.installedSize), sourceLabel(package), statusText(package)});
        item->setIcon(0, packageArtwork(package));
        item->setIcon(3, Aero7Icons::sourceIcon(package.source, officialSourceConfigured(package)));
        styleProgramRow(item, package);
        item->setData(0, Qt::UserRole, package.productId);
        m_table->addTopLevelItem(item);
    }
    m_status->setText(QStringLiteral("%1 installed programs").arg(m_table->topLevelItemCount()));
}

void MainWindow::showUpdates()
{
    m_pages->setCurrentWidget(m_results);
    m_sidebar->clearSelection();
    m_filterRow->hide();
    m_table->clear();
    m_table->setRootIsDecorated(true);
    m_results->findChild<QLabel *>(QStringLiteral("listTitle"))->setText(QStringLiteral("Available Updates"));
    setLocation(QStringLiteral("Available Updates"));
    const auto updates = m_backend.updates();
    auto *system = new QTreeWidgetItem(m_table, {QStringLiteral("System updates")});
    auto *applications = new QTreeWidgetItem(m_table, {QStringLiteral("Application updates")});
    auto *thirdParty = new QTreeWidgetItem(m_table, {QStringLiteral("Third-party updates")});
    for (const auto &package : updates) {
        auto *group = SourcePolicy::isThirdParty(package.source) ? thirdParty : package.application ? applications : system;
        auto *item = new QTreeWidgetItem(group, {package.displayName, package.installedVersion + QStringLiteral(" → ") + package.version, humanSize(package.downloadSize), sourceLabel(package), QStringLiteral("Update available")});
        item->setIcon(0, packageArtwork(package));
        item->setIcon(3, Aero7Icons::sourceIcon(package.source, officialSourceConfigured(package)));
        styleProgramRow(item, package);
        item->setData(0, Qt::UserRole, package.name);
    }
    for (auto *group : {system, applications, thirdParty}) {
        if (!group->childCount()) delete group; else group->setExpanded(true);
    }
    m_status->setText(updates.isEmpty() ? QStringLiteral("No updates found in the cached package data") : QStringLiteral("%1 updates available").arg(updates.size()));
}

void MainWindow::showSettings()
{
    if (!m_settingsWindow) {
        m_settingsWindow = new QDialog(this, Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        m_settingsWindow->setObjectName(QStringLiteral("settingsWindow"));
        m_settingsWindow->setWindowTitle(QStringLiteral("Programs Center Settings"));
        m_settingsWindow->setWindowIcon(windowIcon());
        m_settingsWindow->setWindowModality(Qt::NonModal);
        m_settingsWindow->resize(900, 720);
        m_settingsWindow->setMinimumSize(720, 560);
        m_settingsWindow->setStyleSheet(Win7Style::applicationStyleSheet());
        auto *layout = new QVBoxLayout(m_settingsWindow);
        layout->setContentsMargins(0, 0, 0, 0);
        m_settings = makeSettingsPage();
        layout->addWidget(m_settings);
    }
    m_settingsWindow->show();
    m_settingsWindow->raise();
    m_settingsWindow->activateWindow();
}

void MainWindow::showDetails(const QString &id)
{
    const auto *found = findPackage(id);
    if (!found) {
        QMessageBox::information(this, QStringLiteral("Programs Center"), QStringLiteral("The requested program was not found in the enabled software sources."));
        return;
    }
    Package package = *found;
    QDialog dialog(this);
    dialog.setWindowTitle(package.displayName);
    dialog.setWindowIcon(windowIcon());
    dialog.resize(590, 455);
    dialog.setStyleSheet(Win7Style::applicationStyleSheet());
    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(0, 0, 0, 12);
    layout->setSpacing(8);
    auto *scroll = new QScrollArea;
    scroll->setObjectName(QStringLiteral("detailsScroll"));
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *content = new QWidget;
    content->setObjectName(QStringLiteral("detailsContent"));
    auto *details = new QVBoxLayout(content);
    details->setContentsMargins(22, 18, 22, 12);
    details->setSpacing(10);
    auto *header = new QHBoxLayout;
    header->setSpacing(13);
    auto *icon = new QLabel;
    icon->setPixmap(packageArtwork(package).pixmap(64, 64));
    icon->setFixedSize(64, 64);
    header->addWidget(icon, 0, Qt::AlignTop);
    auto *headings = new QVBoxLayout;
    headings->setSpacing(4);
    auto *title = Win7Style::pageTitle(package.displayName);
    title->setWordWrap(true);
    headings->addWidget(title);
    auto *sourceBadge = new SourceBadge(package.source, package.repository,
                                       officialSourceConfigured(package));
    headings->addWidget(sourceBadge);
    header->addLayout(headings);
    header->addStretch();
    details->addLayout(header);
    details->addWidget(Win7Style::separator());
    auto *description = new QLabel(package.description.isEmpty() ? package.summary : package.description);
    description->setWordWrap(true);
    details->addWidget(description);
    auto *warning = new QLabel(SourcePolicy::warning(package.source));
    warning->setWordWrap(true);
    warning->setStyleSheet(QStringLiteral("color:#6f5300;background:#fff9df;border:1px solid #e1cf82;padding:7px;"));
    warning->setVisible(SourcePolicy::isThirdParty(package.source));
    details->addWidget(warning);
    auto *information = new QLabel(QStringLiteral("Program information"));
    information->setObjectName(QStringLiteral("detailSectionHeading"));
    details->addWidget(information);
    auto *form = new QFormLayout;
    form->setHorizontalSpacing(18);
    form->setVerticalSpacing(7);
    auto *version = new QLabel(package.version);
    version->setWordWrap(true);
    auto *installedVersion = new QLabel(package.installedVersion.isEmpty() ? QStringLiteral("Not installed") : package.installedVersion);
    installedVersion->setWordWrap(true);
    form->addRow(QStringLiteral("Version:"), version);
    form->addRow(QStringLiteral("Installed version:"), installedVersion);
    form->addRow(QStringLiteral("Download size:"), new QLabel(humanSize(package.downloadSize)));
    auto *publisher = new QLabel(package.developer.isEmpty() ? QStringLiteral("Not specified") : package.developer);
    publisher->setWordWrap(true);
    form->addRow(QStringLiteral("Publisher:"), publisher);
    auto *license = new QLabel(package.license.isEmpty() ? QStringLiteral("Not specified") : package.license);
    license->setWordWrap(true);
    form->addRow(QStringLiteral("License:"), license);
    QComboBox *alternatives = nullptr;
    if (package.variants.size() > 1) {
        alternatives = new QComboBox;
        alternatives->setIconSize(QSize(22, 22));
        int recommendedPriority = 0;
        for (const auto &variant : package.variants) recommendedPriority = std::max(recommendedPriority, variant.priority);
        for (int index = 0; index < package.variants.size(); ++index) {
            const auto &variant = package.variants.at(index);
            const auto source = SourcePolicy::descriptor(variant.source);
            const QString name = variant.source == SoftwareSource::Unknown
                ? QStringLiteral("External: %1").arg(variant.repository) : source.name;
            const bool configured = officialSourceConfigured(variant.source)
                && m_backend.repositories().contains(variant.repository);
            alternatives->addItem(Aero7Icons::sourceIcon(variant.source, configured), QStringLiteral("%1 — %2%3").arg(name, variant.version, variant.priority == recommendedPriority ? QStringLiteral(" (recommended)") : QString()), index);
        }
        form->addRow(QStringLiteral("Install from:"), alternatives);
    } else {
        auto *source = new QLabel(sourceLabel(package));
        source->setWordWrap(true);
        form->addRow(QStringLiteral("Source:"), source);
    }
    details->addLayout(form);
    auto *blocked = new QLabel(package.blockedReason);
    blocked->setWordWrap(true);
    blocked->setStyleSheet(QStringLiteral("color:#9b1c1c;"));
    blocked->setVisible(!package.blockedReason.isEmpty());
    details->addWidget(blocked);
    details->addStretch();
    scroll->setWidget(content);
    layout->addWidget(scroll, 1);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    auto *action = buttons->addButton(package.state == PackageState::Installed ? QStringLiteral("Remove") : QStringLiteral("Install"), QDialogButtonBox::ActionRole);
    action->setEnabled(package.blockedReason.isEmpty() && (!package.critical || package.state != PackageState::Installed));
    if (alternatives) {
        for (int index = 0; index < package.variants.size(); ++index) {
            const auto &variant = package.variants.at(index);
            if (variant.packageId == package.name && variant.repository == package.repository
                && variant.provider == package.provider) { alternatives->setCurrentIndex(index); break; }
        }
        connect(alternatives, &QComboBox::currentIndexChanged, &dialog,
            [this, &package, alternatives, sourceBadge, warning, blocked, version, installedVersion, action](int index) {
                const int selected = alternatives->itemData(index).toInt();
                if (selected >= 0 && selected < package.variants.size()) {
                    const auto &variant = package.variants.at(selected);
                    package.name = variant.packageId;
                    package.version = variant.version;
                    package.installedVersion = variant.installedVersion;
                    package.repository = variant.repository;
                    package.source = variant.source;
                    package.provider = variant.provider;
                    package.state = variant.state;
                    package.critical = variant.critical;
                    package.blockedReason = variant.blockedReason;
                    sourceBadge->setSource(package.source, package.repository,
                                           officialSourceConfigured(package));
                    warning->setText(SourcePolicy::warning(package.source));
                    warning->setVisible(SourcePolicy::isThirdParty(package.source));
                    blocked->setText(package.blockedReason);
                    blocked->setVisible(!package.blockedReason.isEmpty());
                    version->setText(package.version);
                    installedVersion->setText(package.installedVersion.isEmpty() ? QStringLiteral("Not installed") : package.installedVersion);
                    action->setText(package.state == PackageState::Installed ? QStringLiteral("Remove") : QStringLiteral("Install"));
                    action->setEnabled(package.blockedReason.isEmpty() && (!package.critical || package.state != PackageState::Installed));
                }
            });
    }
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(action, &QPushButton::clicked, &dialog, [this, &dialog, &package] { const bool remove = package.state == PackageState::Installed; dialog.accept(); runAction(package, remove); });
    auto *footer = new QHBoxLayout;
    footer->setContentsMargins(22, 0, 22, 0);
    footer->addStretch();
    footer->addWidget(buttons);
    layout->addLayout(footer);
    dialog.exec();
}

void MainWindow::runAction(const Package &package, bool remove)
{
    if (package.provider == PackageProvider::Pacman && m_backend.databaseLocked()) {
        QMessageBox::warning(this, QStringLiteral("Programs Center"), QStringLiteral("Another software installation or update is currently in progress.\n\nWait for it to finish, then try again."));
        return;
    }
    if (remove && package.critical) {
        QMessageBox::warning(this, QStringLiteral("Programs Center"), QStringLiteral("This system component is protected and cannot be removed here."));
        return;
    }
    if (!package.blockedReason.isEmpty()) { QMessageBox::warning(this, QStringLiteral("Programs Center"), package.blockedReason); return; }
    if (SourcePolicy::isThirdParty(package.source)) {
        QSettings settings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter"));
        const QString sourceId = package.source == SoftwareSource::Unknown
            ? QStringLiteral("external-%1").arg(package.repository)
            : SourcePolicy::descriptor(package.source).id;
        const QString key = QStringLiteral("notices/%1").arg(sourceId);
        if (!settings.value(key).toBool()) {
            QMessageBox notice(QMessageBox::Information, QStringLiteral("Third-party software"), SourcePolicy::warning(package.source), QMessageBox::NoButton, this);
            auto *continueButton = notice.addButton(QStringLiteral("Continue"), QMessageBox::AcceptRole);
            notice.addButton(QStringLiteral("Cancel"), QMessageBox::RejectRole);
            auto *remember = new QCheckBox(QStringLiteral("Don’t show this explanation again"), &notice);
            notice.setCheckBox(remember);
            notice.exec();
            if (notice.clickedButton() != continueButton) return;
            if (remember->isChecked()) settings.setValue(key, true);
        }
    }
    const QString verb = remove ? QStringLiteral("Remove") : QStringLiteral("Install");
    if (QMessageBox::question(this, verb + QLatin1Char(' ') + package.displayName + QLatin1Char('?'), QStringLiteral("%1 %2?\n\nSource: %3\nPackage: %4\nDownload size: %5\nDisk space: %6").arg(verb, package.displayName, sourceLabel(package), package.name, humanSize(package.downloadSize), humanSize(package.installedSize))) != QMessageBox::Yes) return;
    auto *transaction = new PackageTransaction(this);
    auto *progress = new QProgressDialog(QStringLiteral("Preparing…"), QString(), 0, 0, this);
    progress->setWindowTitle(verb + QStringLiteral(" Software"));
    progress->setCancelButton(nullptr);
    progress->setWindowModality(Qt::WindowModal);
    connect(transaction, &PackageTransaction::progress, progress, [progress](const QString &phase, int, const QString &detail) { progress->setLabelText(phase + (detail.isEmpty() ? QString() : QStringLiteral("\n") + detail.left(240))); });
    connect(transaction, &PackageTransaction::finished, this, [this, transaction, progress](bool ok, const QString &details) {
        progress->close(); transaction->deleteLater();
        if (ok) { QMessageBox::information(this, QStringLiteral("Programs Center"), QStringLiteral("The software change finished successfully.")); loadData(); showHome(); }
        else QMessageBox::critical(this, QStringLiteral("Programs Center"), QStringLiteral("The software change could not be completed.\n\n%1").arg(details.left(800)));
    });
    const auto operation = remove ? PackageTransaction::Remove : PackageTransaction::Install;
    const bool passwordless = !remove && (package.source == SoftwareSource::Aero7 || package.source == SoftwareSource::Arch)
        && QStringList{QStringLiteral("aero7"),QStringLiteral("core"),QStringLiteral("extra"),QStringLiteral("multilib")}.contains(package.repository)
        && trustedInstallAllowed();
    const QString nativeTarget = !remove && !package.repository.isEmpty() && package.repository != QStringLiteral("local")
        ? package.repository + QLatin1Char('/') + package.name : package.name;
    const bool started = package.provider == PackageProvider::Flatpak ? transaction->startFlatpak(operation, package.name, package.repository)
        : passwordless ? transaction->startTrustedInstall(package.repository, package.name)
        : transaction->start(operation, {nativeTarget});
    if (!started) {
        const QString reason=transaction->lastStartError();
        progress->close(); transaction->deleteLater();
        QMessageBox::warning(this, QStringLiteral("Programs Center"),
            QStringLiteral("The software change could not be started.%1")
                .arg(reason.isEmpty() ? QString() : QStringLiteral("\n\n")+reason));
    }
    else progress->show();
}
