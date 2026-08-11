#pragma once
#include "DeepLink.h"
#include "backend/AlpmBackend.h"
#include "metadata/Metadata.h"
#include <QMainWindow>
class QLineEdit;class QListWidget;class QStackedWidget;class QLabel;class QGridLayout;class QTreeWidget;

class MainWindow final:public QMainWindow{
 Q_OBJECT
public:explicit MainWindow(const DeepLink&,QWidget*parent=nullptr);~MainWindow()override;
protected:void closeEvent(QCloseEvent*)override;
private:
 AlpmBackend m_backend;Metadata m_metadata;QList<Package>m_packages;QListWidget*m_sidebar=nullptr;QStackedWidget*m_pages=nullptr;QLineEdit*m_search=nullptr;QLabel*m_status=nullptr;QWidget*m_home=nullptr;QWidget*m_results=nullptr;QTreeWidget*m_table=nullptr;
 void buildUi();void loadData();void showHome();void showResults(const QString&,const QString&category={});void showDetails(const QString&);void showInstalled();void showUpdates();void runAction(const Package&,bool remove=false);void navigate(int);QWidget*makeHome();QWidget*makeListPage();QWidget*makeRightPane();
};
