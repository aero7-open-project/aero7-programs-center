#include "MainWindow.h"

#include "Aero7Icon.h"
#include "PackageIcon.h"
#include "ProgramCard.h"
#include "SourceBadge.h"
#include "Win7Style.h"
#include "WrappingGridLayout.h"
#include "backend/PackageTransaction.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileInfo>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
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

#include <algorithm>

namespace {
QPushButton *taskLink(const QString &text, const QString &icon)
{
    auto *button = new QPushButton(Aero7Icons::icon(icon), text);
    button->setObjectName(QStringLiteral("taskLink"));
    button->setFlat(true);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

QLabel *sectionHeading(const QString &text)
{
    auto *label = new QLabel(text);
    label->setObjectName(QStringLiteral("sectionHeading"));
    return label;
}

QToolButton *commandButton(const QString &text, const QString &icon = {})
{
    auto *button = new QToolButton;
    button->setText(text);
    if (!icon.isEmpty()) button->setIcon(Aero7Icons::icon(icon));
    button->setToolButtonStyle(icon.isEmpty() ? Qt::ToolButtonTextOnly : Qt::ToolButtonTextBesideIcon);
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
    else if (link.page == DeepLink::Sources) showSettings();
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
    m_breadcrumb->setMinimumWidth(360);
    m_breadcrumb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    navigation->addWidget(m_breadcrumb);
    m_search = new QLineEdit;
    m_search->setObjectName(QStringLiteral("searchBox"));
    m_search->setPlaceholderText(QStringLiteral("Search Programs Center"));
    m_search->setClearButtonEnabled(true);
    m_search->setFixedWidth(285);
    navigation->addWidget(m_search);

    auto *shell = new QWidget;
    auto *shellLayout = new QVBoxLayout(shell);
    shellLayout->setContentsMargins(0, 0, 0, 0);
    shellLayout->setSpacing(0);
    QHBoxLayout *commands = nullptr;
    shellLayout->addWidget(Win7Style::commandBar(&commands));
    auto *organize = commandButton(QStringLiteral("Organize  ▾"));
    auto *home = commandButton(QStringLiteral("Home"), QStringLiteral("home"));
    auto *discover = commandButton(QStringLiteral("Discover"), QStringLiteral("search"));
    auto *updates = commandButton(QStringLiteral("Check for updates"), QStringLiteral("update"));
    auto *settings = commandButton(QStringLiteral("Software sources"), QStringLiteral("settings"));
    commands->addWidget(organize);
    commands->addWidget(home);
    commands->addWidget(discover);
    commands->addWidget(updates);
    commands->addWidget(settings);
    commands->addStretch();
    m_sourceFilter = new QComboBox;
    m_sourceFilter->addItem(QStringLiteral("All sources"), QStringLiteral("all"));
    for (const auto &source : SourcePolicy::descriptors()) {
        if (source.source != SoftwareSource::Local && source.source != SoftwareSource::Unknown)
            m_sourceFilter->addItem(source.name, source.id);
    }
    commands->addWidget(new QLabel(QStringLiteral("Source:")));
    commands->addWidget(m_sourceFilter);

    auto *body = new QWidget;
    auto *bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);
    m_sidebar = new QListWidget;
    m_sidebar->setObjectName(QStringLiteral("sidebar"));
    m_sidebar->setFixedWidth(205);
    m_sidebar->addItem(new QListWidgetItem(Aero7Icons::icon(QStringLiteral("home")), QStringLiteral("Home")));
    auto *categoryHeading = new QListWidgetItem(QStringLiteral("CATEGORIES"));
    categoryHeading->setFlags(Qt::NoItemFlags);
    m_sidebar->addItem(categoryHeading);
    for (const auto *category : {"Internet", "Multimedia", "Graphics & Photos", "Office", "Games", "Utilities", "System Tools", "Education", "Developer Tools"})
        m_sidebar->addItem(new QListWidgetItem(Aero7Icons::icon(QString::fromLatin1(category)), QString::fromLatin1(category)));
    auto *manageHeading = new QListWidgetItem(QStringLiteral("MANAGEMENT"));
    manageHeading->setFlags(Qt::NoItemFlags);
    m_sidebar->addItem(manageHeading);
    m_sidebar->addItem(new QListWidgetItem(Aero7Icons::icon(QStringLiteral("update")), QStringLiteral("Updates")));
    m_sidebar->addItem(new QListWidgetItem(Aero7Icons::icon(QStringLiteral("package")), QStringLiteral("Installed Programs")));
    m_sidebar->addItem(new QListWidgetItem(Aero7Icons::icon(QStringLiteral("settings")), QStringLiteral("Software Sources")));
    bodyLayout->addWidget(m_sidebar);
    m_pages = new QStackedWidget;
    m_home = makeHome();
    m_results = makeListPage();
    m_settings = makeSettingsPage();
    m_pages->addWidget(m_home);
    m_pages->addWidget(m_results);
    m_pages->addWidget(m_settings);
    bodyLayout->addWidget(m_pages, 1);
    bodyLayout->addWidget(makeRightPane());
    shellLayout->addWidget(body, 1);
    setCentralWidget(shell);

    m_status = new QLabel(QStringLiteral("Loading software information…"));
    statusBar()->addWidget(m_status, 1);
    statusBar()->addPermanentWidget(new QLabel(QStringLiteral("Aero7 Programs Center — Beta")));
    connect(home, &QToolButton::clicked, this, &MainWindow::showHome);
    connect(discover, &QToolButton::clicked, this, [this] { showResults(); });
    connect(updates, &QToolButton::clicked, this, &MainWindow::showUpdates);
    connect(settings, &QToolButton::clicked, this, &MainWindow::showSettings);
    connect(back, &QAction::triggered, this, &MainWindow::showHome);
    auto *searchDelay = new QTimer(this);
    searchDelay->setSingleShot(true);
    searchDelay->setInterval(180);
    connect(m_search, &QLineEdit::textChanged, searchDelay, qOverload<>(&QTimer::start));
    connect(searchDelay, &QTimer::timeout, this, [this] { showResults(m_search->text()); });
    connect(m_search, &QLineEdit::returnPressed, this, [this, searchDelay] { searchDelay->stop(); showResults(m_search->text()); });
    connect(m_sourceFilter, &QComboBox::currentIndexChanged, this, [this] {
        if (m_pages->currentWidget() == m_results) showResults(m_search->text());
    });
    connect(m_sidebar, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        const QString text = item->text();
        if (text == QStringLiteral("Home")) showHome();
        else if (text == QStringLiteral("Updates")) showUpdates();
        else if (text == QStringLiteral("Installed Programs")) showInstalled();
        else if (text == QStringLiteral("Software Sources")) showSettings();
        else if (item->flags() != Qt::NoItemFlags) showResults({}, text);
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
    layout->setContentsMargins(28, 18, 28, 24);
    layout->setSpacing(10);
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
    for (const auto *category : {"Internet", "Multimedia", "Games", "Utilities"}) {
        auto *button = taskLink(QString::fromLatin1(category), QString::fromLatin1(category));
        connect(button, &QPushButton::clicked, this, [this, category] { showResults({}, QString::fromLatin1(category)); });
        categories->addWidget(button);
    }
    layout->addLayout(categories);
    layout->addWidget(sectionHeading(QStringLiteral("Recommended programs")));
    auto *recommended = new QWidget;
    recommended->setObjectName(QStringLiteral("recommendedGrid"));
    recommended->setLayout(new WrappingGridLayout);
    layout->addWidget(recommended);
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
    m_table = new QTreeWidget;
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
    layout->setContentsMargins(28, 18, 38, 28);
    layout->setSpacing(9);
    layout->addWidget(Win7Style::pageTitle(QStringLiteral("Software Sources")));
    layout->addWidget(Win7Style::instruction(QStringLiteral("Choose where Programs Center may find applications. Aero7 and Arch Linux stay enabled to keep the system safe and supported.")));
    layout->addWidget(Win7Style::separator());
    auto addSource = [this, layout](SoftwareSource source, bool fixed) {
        const auto info = SourcePolicy::descriptor(source);
        auto *group = new QGroupBox(info.name);
        auto *row = new QHBoxLayout(group);
        auto *icon = new QLabel;
        icon->setPixmap(Aero7Icons::icon(info.iconKey).pixmap(32, 32));
        row->addWidget(icon, 0, Qt::AlignTop);
        auto *description = new QLabel(info.description);
        description->setWordWrap(true);
        row->addWidget(description, 1);
        auto *toggle = new QCheckBox(fixed ? QStringLiteral("Always enabled") : QStringLiteral("Enabled"));
        toggle->setChecked(fixed || sourceEnabled(source));
        toggle->setEnabled(!fixed);
        row->addWidget(toggle, 0, Qt::AlignTop);
        if (!fixed) connect(toggle, &QCheckBox::toggled, this, [this, info](bool enabled) {
            QSettings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter")).setValue(QStringLiteral("sources/%1").arg(info.id), enabled);
            loadData();
        });
        layout->addWidget(group);
    };
    addSource(SoftwareSource::Aero7, true);
    addSource(SoftwareSource::Arch, true);
    addSource(SoftwareSource::ChaoticAur, false);
    addSource(SoftwareSource::CachyOs, false);
    addSource(SoftwareSource::Flathub, false);
    layout->addWidget(Win7Style::instruction(QStringLiteral("Programs Center never runs yay or paru and never enables a full CachyOS repository. CachyOS Applications is limited to Aero7's reviewed allowlist. Disabling a source hides available programs but keeps installed programs manageable.")));
    layout->addWidget(Win7Style::separator());
    layout->addWidget(Win7Style::pageTitle(QStringLiteral("Install permissions")));
    m_passwordlessCheck = new QCheckBox(QStringLiteral("Install trusted Aero7 and Arch applications without asking for a password"));
    m_passwordlessCheck->setObjectName(QStringLiteral("trustedInstallCheck"));
    m_passwordlessCheck->setChecked(trustedInstallAllowed());
    m_passwordlessCheck->setEnabled(QFileInfo::exists(QStringLiteral("/usr/lib/aero7-programs-center/aero7-programs-center-permissions")));
    layout->addWidget(m_passwordlessCheck);
    layout->addWidget(Win7Style::instruction(QStringLiteral("An administrator must approve this setting once. It applies only to your active local session and to Aero7 or official Arch packages whose dependencies also come from those sources. Removing software, system updates and third-party native packages still require authentication. Flatpak apps install in your user account.")));
    if (!m_passwordlessCheck->isEnabled())
        layout->addWidget(Win7Style::instruction(QStringLiteral("Install the packaged Programs Center to enable this setting.")));
    connect(m_passwordlessCheck, &QCheckBox::toggled, this, &MainWindow::changeTrustedInstallPermission);
    layout->addStretch();
    scroll->setWidget(content);
    return scroll;
}

QWidget *MainWindow::makeRightPane()
{
    auto *pane = new QFrame;
    pane->setObjectName(QStringLiteral("rightPane"));
    pane->setFixedWidth(230);
    auto *layout = new QVBoxLayout(pane);
    layout->setContentsMargins(15, 18, 15, 18);
    layout->setSpacing(7);
    layout->addWidget(sectionHeading(QStringLiteral("See also")));
    auto *install = taskLink(QStringLiteral("Find a program"), QStringLiteral("install"));
    auto *updates = taskLink(QStringLiteral("Check for updates"), QStringLiteral("update"));
    auto *installed = taskLink(QStringLiteral("Installed programs"), QStringLiteral("package"));
    auto *sources = taskLink(QStringLiteral("Software sources"), QStringLiteral("settings"));
    auto *control = taskLink(QStringLiteral("Control Panel"), QStringLiteral("settings"));
    for (auto *button : {install, updates, installed, sources, control}) layout->addWidget(button);
    layout->addStretch();
    connect(install, &QPushButton::clicked, this, [this] { showResults(); m_search->setFocus(); });
    connect(updates, &QPushButton::clicked, this, &MainWindow::showUpdates);
    connect(installed, &QPushButton::clicked, this, &MainWindow::showInstalled);
    connect(sources, &QPushButton::clicked, this, &MainWindow::showSettings);
    connect(control, &QPushButton::clicked, this, [] { QProcess::startDetached(QStringLiteral("aero7-control-panel"), {}); });
    return pane;
}

bool MainWindow::sourceEnabled(SoftwareSource source) const
{
    if (source == SoftwareSource::Aero7 || source == SoftwareSource::Arch || source == SoftwareSource::Local || source == SoftwareSource::Unknown) return true;
    const auto info = SourcePolicy::descriptor(source);
    const bool defaultValue = source == SoftwareSource::ChaoticAur;
    return QSettings(QStringLiteral("Aero7"), QStringLiteral("ProgramsCenter")).value(QStringLiteral("sources/%1").arg(info.id), defaultValue).toBool();
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
    for (const auto &package : all) if (sourceEnabled(package.source) || package.state != PackageState::Available) enabled.append(package);
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
    m_sidebar->setCurrentRow(0);
    setLocation(QStringLiteral("Home"));
    auto fill = [this](const char *name, const QStringList &wanted, int maximum) {
        auto *host = m_home->findChild<QWidget *>(QLatin1String(name));
        auto *layout = host->layout();
        while (auto *item = layout->takeAt(0)) { delete item->widget(); delete item; }
        int count = 0;
        for (const auto &package : m_packages) {
            if (!package.application) continue;
            if (!wanted.isEmpty() && !wanted.contains(package.name) && !wanted.contains(package.productId)) continue;
            auto *card = new ProgramCard(package);
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
    m_table->setRootIsDecorated(false);
    m_table->clear();
    const QString title = !category.isEmpty() ? category : query.isEmpty() ? QStringLiteral("Discover Software") : QStringLiteral("Search results for “%1”").arg(query);
    m_results->findChild<QLabel *>(QStringLiteral("listTitle"))->setText(title);
    setLocation(title);
    const QString sourceId = m_sourceFilter->currentData().toString();
    const QString needle = query.toCaseFolded();
    for (const auto &package : m_packages) {
        if (!category.isEmpty() && package.category.compare(category, Qt::CaseInsensitive) != 0) continue;
        if (!needle.isEmpty() && !package.searchableText().contains(needle)) continue;
        if (!package.application && (needle.isEmpty() || !package.searchableText().contains(needle))) continue;
        if (sourceId != QStringLiteral("all") && SourcePolicy::descriptor(package.source).id != sourceId) continue;
        auto *item = new QTreeWidgetItem({package.displayName, package.version, humanSize(package.installedSize), SourcePolicy::descriptor(package.source).name, statusText(package)});
        item->setIcon(0, packageArtwork(package));
        item->setIcon(3, Aero7Icons::icon(SourcePolicy::descriptor(package.source).iconKey));
        item->setData(0, Qt::UserRole, package.productId);
        m_table->addTopLevelItem(item);
    }
    m_status->setText(QStringLiteral("%1 programs shown").arg(m_table->topLevelItemCount()));
}

void MainWindow::showInstalled()
{
    m_pages->setCurrentWidget(m_results);
    m_table->setRootIsDecorated(false);
    m_table->clear();
    m_results->findChild<QLabel *>(QStringLiteral("listTitle"))->setText(QStringLiteral("Installed Programs"));
    setLocation(QStringLiteral("Installed Programs"));
    for (const auto &package : m_packages) if (package.state != PackageState::Available && package.application) {
        auto *item = new QTreeWidgetItem({package.displayName, package.installedVersion, humanSize(package.installedSize), SourcePolicy::descriptor(package.source).name, statusText(package)});
        item->setIcon(0, packageArtwork(package));
        item->setData(0, Qt::UserRole, package.productId);
        m_table->addTopLevelItem(item);
    }
    m_status->setText(QStringLiteral("%1 installed programs").arg(m_table->topLevelItemCount()));
}

void MainWindow::showUpdates()
{
    m_pages->setCurrentWidget(m_results);
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
        auto *item = new QTreeWidgetItem(group, {package.displayName, package.installedVersion + QStringLiteral(" → ") + package.version, humanSize(package.downloadSize), SourcePolicy::descriptor(package.source).name, QStringLiteral("Update available")});
        item->setIcon(0, packageArtwork(package));
        item->setData(0, Qt::UserRole, package.name);
    }
    for (auto *group : {system, applications, thirdParty}) {
        if (!group->childCount()) delete group; else group->setExpanded(true);
    }
    m_status->setText(updates.isEmpty() ? QStringLiteral("No updates found in the cached package data") : QStringLiteral("%1 updates available").arg(updates.size()));
}

void MainWindow::showSettings()
{
    m_pages->setCurrentWidget(m_settings);
    setLocation(QStringLiteral("Software Sources"));
    for (int row = 0; row < m_sidebar->count(); ++row) if (m_sidebar->item(row)->text() == QStringLiteral("Software Sources")) m_sidebar->setCurrentRow(row);
    m_status->setText(QStringLiteral("Aero7 and Arch Linux are trusted system sources"));
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
    layout->setContentsMargins(22, 18, 22, 16);
    auto *header = new QHBoxLayout;
    auto *icon = new QLabel;
    icon->setPixmap(packageArtwork(package).pixmap(64, 64));
    header->addWidget(icon, 0, Qt::AlignTop);
    auto *headings = new QVBoxLayout;
    headings->addWidget(Win7Style::pageTitle(package.displayName));
    auto *sourceBadge = new SourceBadge(package.source);
    headings->addWidget(sourceBadge);
    header->addLayout(headings, 1);
    layout->addLayout(header);
    layout->addWidget(Win7Style::separator());
    auto *description = new QLabel(package.description.isEmpty() ? package.summary : package.description);
    description->setWordWrap(true);
    layout->addWidget(description);
    auto *warning = new QLabel(SourcePolicy::warning(package.source));
    warning->setWordWrap(true);
    warning->setStyleSheet(QStringLiteral("color:#6f5300;background:#fff9df;border:1px solid #e1cf82;padding:7px;"));
    warning->setVisible(SourcePolicy::isThirdParty(package.source));
    layout->addWidget(warning);
    auto *form = new QFormLayout;
    auto *version = new QLabel(package.version);
    auto *installedVersion = new QLabel(package.installedVersion.isEmpty() ? QStringLiteral("Not installed") : package.installedVersion);
    form->addRow(QStringLiteral("Version:"), version);
    form->addRow(QStringLiteral("Installed version:"), installedVersion);
    form->addRow(QStringLiteral("Download size:"), new QLabel(humanSize(package.downloadSize)));
    form->addRow(QStringLiteral("Publisher:"), new QLabel(package.developer.isEmpty() ? QStringLiteral("Not specified") : package.developer));
    form->addRow(QStringLiteral("License:"), new QLabel(package.license.isEmpty() ? QStringLiteral("Not specified") : package.license));
    QComboBox *alternatives = nullptr;
    if (package.variants.size() > 1) {
        alternatives = new QComboBox;
        int recommendedPriority = 0;
        for (const auto &variant : package.variants) recommendedPriority = std::max(recommendedPriority, variant.priority);
        for (int index = 0; index < package.variants.size(); ++index) {
            const auto &variant = package.variants.at(index);
            const auto source = SourcePolicy::descriptor(variant.source);
            alternatives->addItem(Aero7Icons::icon(source.iconKey), QStringLiteral("%1 — %2%3").arg(source.name, variant.version, variant.priority == recommendedPriority ? QStringLiteral(" (recommended)") : QString()), index);
        }
        form->addRow(QStringLiteral("Install from:"), alternatives);
    } else form->addRow(QStringLiteral("Source:"), new QLabel(SourcePolicy::descriptor(package.source).name));
    layout->addLayout(form);
    auto *blocked = new QLabel(package.blockedReason);
    blocked->setWordWrap(true);
    blocked->setStyleSheet(QStringLiteral("color:#9b1c1c;"));
    blocked->setVisible(!package.blockedReason.isEmpty());
    layout->addWidget(blocked);
    layout->addStretch();
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
            [&package, alternatives, sourceBadge, warning, blocked, version, installedVersion, action](int index) {
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
                    sourceBadge->setSource(package.source);
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
    layout->addWidget(buttons);
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
        const QString key = QStringLiteral("notices/%1").arg(SourcePolicy::descriptor(package.source).id);
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
    if (QMessageBox::question(this, verb + QLatin1Char(' ') + package.displayName + QLatin1Char('?'), QStringLiteral("%1 %2?\n\nSource: %3\nPackage: %4\nDownload size: %5\nDisk space: %6").arg(verb, package.displayName, SourcePolicy::descriptor(package.source).name, package.name, humanSize(package.downloadSize), humanSize(package.installedSize))) != QMessageBox::Yes) return;
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
    const bool started = package.provider == PackageProvider::Flatpak ? transaction->startFlatpak(operation, package.name)
        : passwordless ? transaction->startTrustedInstall(package.repository, package.name)
        : transaction->start(operation, {nativeTarget});
    if (!started) { progress->close(); transaction->deleteLater(); QMessageBox::warning(this, QStringLiteral("Programs Center"), QStringLiteral("The software change could not be started.")); }
    else progress->show();
}
