#include "DeepLink.h"

DeepLink parseDeepLink(const QStringList &a) {
    DeepLink out;
    for (int i=1; i<a.size(); ++i) {
        const QString v=a[i];
        if (v=="--home") out.page=DeepLink::Home;
        else if (v=="--updates") out.page=DeepLink::Updates;
        else if (v=="--installed") out.page=DeepLink::Installed;
        else if (v=="--sources") out.page=DeepLink::Sources;
        else if ((v=="--search" || v=="--package" || v=="--category") && i+1<a.size()) {
            out.page = v=="--search" ? DeepLink::Search : v=="--package" ? DeepLink::PackageDetails : DeepLink::Category;
            out.value=a[++i];
        }
    }
    return out;
}
