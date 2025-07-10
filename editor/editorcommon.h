#pragma once

inline bool isDarkMode() {
    // https://stackoverflow.com/a/78854851/16255372
    #if defined(_WIN32)
        // Never read dark theme on Windows as the app currently renders with white color palette regardless
        return false;
    #elif QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        const auto scheme = QGuiApplication::styleHints()->colorScheme();
        return scheme == Qt::ColorScheme::Dark;
    #else
        const QPalette defaultPalette;
        const auto text = defaultPalette.color(QPalette::WindowText);
        const auto window = defaultPalette.color(QPalette::Window);
        return text.lightness() > window.lightness();
    #endif // QT_VERSION
}
