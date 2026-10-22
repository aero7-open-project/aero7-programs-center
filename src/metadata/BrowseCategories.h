#pragma once

#include <QList>
#include <QString>
#include <QStringList>

// Stable IDs keep navigation independent of display wording. parentId is
// reserved for a later expandable hierarchy; this release remains flat.
struct BrowseCategory {
    QString id;
    QString label;
    QString icon;
    QString parentId;
    QStringList desktopTags;
    int priority;
    bool homeShortcut;
};

namespace BrowseCategories {

inline const QList<BrowseCategory> &all()
{
    // Tags use the case-sensitive freedesktop Desktop Menu category registry.
    // Specific tags beat broad categories when a component lists both.
    static const QList<BrowseCategory> categories = {
        {"internet", "Internet", "internet", {}, {"Network", "WebBrowser", "Feed", "News", "P2P", "RemoteAccess", "FileTransfer"}, 20, true},
        {"communication", "Communication", ":/categories/communication.svg", "internet", {"InstantMessaging", "Chat", "IRCClient", "Email", "VideoConference", "Telephony", "TelephonyTools"}, 80, false},
        {"multimedia", "Multimedia", "multimedia", {}, {"AudioVideo", "AudioVideoEditing", "Player", "Recorder", "DiscBurning"}, 20, true},
        {"music-audio", "Music & Audio", ":/categories/music-audio.svg", "multimedia", {"Audio", "Midi", "Mixer", "Sequencer", "Tuner", "Music"}, 70, false},
        {"video", "Video & Movies", ":/categories/video.svg", "multimedia", {"Video", "TV"}, 70, false},
        {"graphics", "Graphics & Photos", "graphics and photos", {}, {"Graphics", "Photography", "2DGraphics", "3DGraphics", "RasterGraphics", "VectorGraphics", "Scanning", "OCR", "Publishing", "Viewer"}, 25, false},
        {"office", "Office", "office", {}, {"Office", "WordProcessor", "Spreadsheet", "Presentation", "Database", "Calendar", "ContactManagement", "Finance", "Dictionary"}, 25, false},
        {"games", "Games", "games", {}, {"Game", "GameTool", "Emulator"}, 60, true},
        {"science-math", "Science & Math", "education", "education", {"Science", "Math", "Astronomy", "Biology", "Chemistry", "Physics", "Geography", "Geoscience", "DataVisualization", "Engineering"}, 70, false},
        {"education", "Education", ":/categories/education.svg", {}, {"Education", "Languages", "Literature", "History", "Humanities", "Art"}, 25, false},
        {"development", "Developer Tools", "developer tools", {}, {"Development", "IDE", "Debugger", "Building", "RevisionControl", "WebDevelopment", "GUIDesigner", "Profiling"}, 65, false},
        {"files", "File Management", ":/categories/files.svg", "utilities", {"FileManager", "FileTools", "Archiving", "Compression", "Filesystem"}, 80, false},
        {"security", "Security & Privacy", ":/categories/security.svg", "system", {"Security"}, 90, false},
        {"settings", "Settings", "settings", "system", {"Settings", "DesktopSettings", "HardwareSettings", "PackageManager", "Printing"}, 60, false},
        {"system", "System Tools", "system tools", {}, {"System", "TerminalEmulator", "Monitor"}, 20, false},
        {"utilities", "Utilities", "utilities", {}, {"Utility", "TextTools", "TextEditor", "Calculator", "Clock", "Accessibility"}, 0, true},
    };
    return categories;
}

inline const BrowseCategory *find(const QString &idOrLabel)
{
    for (const auto &category : all())
        if (category.id == idOrLabel || category.label.compare(idOrLabel, Qt::CaseInsensitive) == 0)
            return &category;
    return nullptr;
}

inline QString classify(const QStringList &desktopTags)
{
    const BrowseCategory *best = nullptr;
    for (const auto &category : all()) {
        bool matched = false;
        for (const QString &tag : category.desktopTags)
            if (desktopTags.contains(tag)) { matched = true; break; }
        if (matched && (!best || category.priority > best->priority)) best = &category;
    }
    return best ? best->id : QStringLiteral("utilities");
}

inline QString label(const QString &id)
{
    const auto *category = find(id);
    return category ? category->label : QStringLiteral("Utilities");
}

} // namespace BrowseCategories
