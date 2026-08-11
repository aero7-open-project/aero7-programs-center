#include "DeepLink.h"
#include "ui/MainWindow.h"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc,char**argv){QApplication app(argc,argv);app.setApplicationName("Programs Center");app.setApplicationVersion("0.1.0-alpha");app.setOrganizationName("Aero7");QCommandLineParser p;p.setApplicationDescription("Discover, install, remove and update software on Aero7.");p.addHelpOption();p.addVersionOption();for(const auto&o:{QCommandLineOption("home","Open Home"),QCommandLineOption("updates","Open Updates"),QCommandLineOption("installed","Open Installed Programs")})p.addOption(o);p.addOption({"search","Search software","query"});p.addOption({"package","Open package details","name"});p.addOption({"category","Open category","name"});p.process(app);MainWindow w(parseDeepLink(app.arguments()));w.show();return app.exec();}
