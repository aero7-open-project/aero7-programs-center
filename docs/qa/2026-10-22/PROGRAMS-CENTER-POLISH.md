# Programs Center host polish review — 2026-10-22

## Visual reference and decisions

- Compared the current Aero7 Control Panel's `Win7Ui.h` and category page with the supplied System and Security screenshot. Programs Center now uses the same role split: a dark blue page title, green section and item titles, blue actions, and neutral 9 pt body text.
- Kept the Home card sections, command bar, separate Settings window, source checks, warnings, and bundled scrollbars. Program details now scroll while the action buttons stay visible.
- A category opened from the sidebar or a deep link now selects its sidebar entry. Changing the source filter keeps that category and heading instead of dropping back to a general search.
- Browse categories follow the [freedesktop Desktop Menu category registry](https://specifications.freedesktop.org/menu/latest/category-registry.html). Its exact, case-sensitive tags are mapped to stable Programs Center IDs in `BrowseCategories.h`. More specific tags take precedence over broad tags; the flat menu has `parentId` data ready for a later expandable hierarchy.
- Six categories that lacked suitable distinct Aero pack art now have bundled, original vector symbols: conversation bubbles, a music note, a movie clapper, books and pencil, a file cabinet, and a shield with lock. The ten other categories retain distinct bundled pack icons. All 16 differ at sidebar sizes and remain independent of the OS icon theme.

## Host browse results

The current Aero7 PC's enabled catalog produced these visible program counts after grouping source variants:

| Category | Programs | Category | Programs |
| --- | ---: | --- | ---: |
| Internet | 82 | Communication | 44 |
| Multimedia | 40 | Music & Audio | 130 |
| Video & Movies | 31 | Graphics & Photos | 116 |
| Office | 79 | Games | 161 |
| Science & Math | 54 | Education | 19 |
| Developer Tools | 112 | File Management | 32 |
| Security & Privacy | 18 | Settings | 24 |
| System Tools | 102 | Utilities | 204 |

These are a snapshot of this machine's catalog, not fixed expected counts. Each app currently has one primary browse category. Home keeps its four existing category shortcuts.

## Validation and screenshots

- `cmake --build build-trust -j 4` succeeded.
- `ctest --test-dir build-trust --output-on-failure` passed all six suites. The Qt layout test checks Home at 1600, 1200, 900, and 720 px, the separate Settings window and wheel/keyboard scroll, stable category navigation with the sidebar scrolling at 720 px, and preserving the category while filtering sources. Icon tests compare all 16 category images at 16, 20, and 22 px under different icon themes.
- Captured live host windows: [Home](home-polish-host.png), [Communication list](communication-polish-host.png), [Settings](settings-polish-host.png), [Firefox details](details-polish-host.png). The details capture confirms its icon and title align at the left, with actions visible below the content.
- No VM or package-repository configuration was changed for this review.
