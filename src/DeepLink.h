#pragma once
#include <QString>
#include <QStringList>

struct DeepLink { enum Page { Home, Discover, Updates, Installed, Sources, Search, PackageDetails, Category } page = Home; QString value; };
DeepLink parseDeepLink(const QStringList &arguments);
