#pragma once

#include "DeepLink.h"
#include "backend/AlpmBackend.h"
#include "metadata/Metadata.h"
#include "sources/SourceCatalog.h"

#include <QMainWindow>
#include <QPointer>
#include <functional>

class QCloseEvent;
class QComboBox;
class QCheckBox;
class QDialog;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QStackedWidget;
class QTreeWidget;
class QVBoxLayout;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(const DeepLink &, QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *) override;

private:
    AlpmBackend m_backend;
    Metadata m_metadata;
    SourceCatalog m_catalog;
    QList<Package> m_packages;
    QListWidget *m_sidebar = nullptr;
    QStackedWidget *m_pages = nullptr;
    QLineEdit *m_search = nullptr;
    QComboBox *m_sourceFilter = nullptr;
    QWidget *m_filterRow = nullptr;
    QLabel *m_breadcrumb = nullptr;
    QLabel *m_status = nullptr;
    QPushButton *m_catalogButton = nullptr;
    QCheckBox *m_passwordlessCheck = nullptr;
    QWidget *m_home = nullptr;
    QWidget *m_results = nullptr;
    QWidget *m_settings = nullptr;
    QPointer<QDialog> m_settingsWindow;
    QTreeWidget *m_table = nullptr;
    QVBoxLayout *m_customSourcesLayout = nullptr;
    QWidget *m_recommendedSection = nullptr;

    void buildUi();
    void loadData();
    void showHome();
    void showResults(const QString &query = {}, const QString &category = {});
    void showDetails(const QString &id);
    void showInstalled();
    void showUpdates();
    void showSettings();
    void runAction(const Package &, bool remove = false);
    void setLocation(const QString &location);
    bool sourceEnabled(SoftwareSource source) const;
    bool sourceEnabled(const Package &package) const;
    QString sourceLabel(const Package &package) const;
    void refreshCustomSources();
    void addSource();
    void runSourceCommand(const QString &program, const QStringList &arguments,
                          const std::function<void()> &onSuccess,
                          const QString &cleanupPath = {});
    bool trustedInstallAllowed() const;
    void changeTrustedInstallPermission(bool enabled);
    const Package *findPackage(const QString &id) const;
    QWidget *makeHome();
    QWidget *makeListPage();
    QWidget *makeSettingsPage();
};
