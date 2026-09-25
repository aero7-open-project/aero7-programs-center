#pragma once

#include "DeepLink.h"
#include "backend/AlpmBackend.h"
#include "metadata/Metadata.h"
#include "sources/SourceCatalog.h"

#include <QMainWindow>

class QCloseEvent;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QStackedWidget;
class QTreeWidget;

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
    QLabel *m_breadcrumb = nullptr;
    QLabel *m_status = nullptr;
    QWidget *m_home = nullptr;
    QWidget *m_results = nullptr;
    QWidget *m_settings = nullptr;
    QTreeWidget *m_table = nullptr;

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
    const Package *findPackage(const QString &id) const;
    QWidget *makeHome();
    QWidget *makeListPage();
    QWidget *makeSettingsPage();
    QWidget *makeRightPane();
};
