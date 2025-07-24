#include "ThemeManager.h"
#include <QApplication>

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent), currentTheme(LightTheme)
{
    settings = new QSettings(this);
    loadThemeFromSettings();
}

ThemeManager::~ThemeManager()
{
    saveThemeToSettings();
}

void ThemeManager::setTheme(Theme theme)
{
    if (currentTheme != theme) {
        currentTheme = theme;
        emit themeChanged(theme);
        saveThemeToSettings();
    }
}

ThemeManager::Theme ThemeManager::getCurrentThemeEnum() const
{
    return currentTheme;
}

QString ThemeManager::getCurrentTheme() const
{
    switch (currentTheme) {
    case LightTheme: return "Light";
    case DarkTheme: return "Dark";
    case BlueTheme: return "Blue";
    case GreenTheme: return "Green";
    case RedTheme: return "Red";
    default: return "Light";
    }
}

void ThemeManager::switchTheme()
{
    Theme nextTheme = static_cast<Theme>((currentTheme + 1) % 5);
    setTheme(nextTheme);
}

void ThemeManager::loadThemeFromSettings()
{
    int themeValue = settings->value("theme", static_cast<int>(LightTheme)).toInt();
    currentTheme = static_cast<Theme>(themeValue);
}

void ThemeManager::saveThemeToSettings()
{
    settings->setValue("theme", static_cast<int>(currentTheme));
}

QString ThemeManager::getBackgroundColor() const
{
    return getThemeColors(currentTheme).background;
}

QString ThemeManager::getPrimaryColor() const
{
    return getThemeColors(currentTheme).primary;
}

QString ThemeManager::getSecondaryColor() const
{
    return getThemeColors(currentTheme).secondary;
}

QString ThemeManager::getAccentColor() const
{
    return getThemeColors(currentTheme).accent;
}

QString ThemeManager::getTextColor() const
{
    return getThemeColors(currentTheme).text;
}

QString ThemeManager::getBorderColor() const
{
    return getThemeColors(currentTheme).border;
}

QString ThemeManager::getFrameColor() const
{
    return getThemeColors(currentTheme).frame;
}

QString ThemeManager::getHoverColor() const
{
    return getThemeColors(currentTheme).hover;
}

QString ThemeManager::getPressedColor() const
{
    return getThemeColors(currentTheme).pressed;
}

QString ThemeManager::getDisabledColor() const
{
    return getThemeColors(currentTheme).disabled;
}

ThemeManager::ThemeColors ThemeManager::getThemeColors(Theme theme) const
{
    ThemeColors colors;

    switch (theme) {
    case LightTheme:
        colors.background = "#FFFFFF";
        colors.primary = "#2E86AB";
        colors.secondary = "#A23B72";
        colors.accent = "#F18F01";
        colors.text = "#333333";
        colors.border = "#CCCCCC";
        colors.frame = "#F8F9FA";
        colors.hover = "#70C1E7";
        colors.pressed = "#1E5F7A";
        colors.disabled = "#AAAAAA";
        break;

    case DarkTheme:
        colors.background = "#2B2B2B";
        colors.primary = "#4A9EFF";
        colors.secondary = "#FF6B9D";
        colors.accent = "#FFB347";
        colors.text = "#FFFFFF";
        colors.border = "#555555";
        colors.frame = "#3C3C3C";
        colors.hover = "#6BB6FF";
        colors.pressed = "#2E7BD6";
        colors.disabled = "#666666";
        break;

    case BlueTheme:
        colors.background = "#E3F2FD";
        colors.primary = "#1976D2";
        colors.secondary = "#0D47A1";
        colors.accent = "#42A5F5";
        colors.text = "#0D47A1";
        colors.border = "#90CAF9";
        colors.frame = "#BBDEFB";
        colors.hover = "#2196F3";
        colors.pressed = "#0D47A1";
        colors.disabled = "#9E9E9E";
        break;

    case GreenTheme:
        colors.background = "#E8F5E8";
        colors.primary = "#4CAF50";
        colors.secondary = "#2E7D32";
        colors.accent = "#81C784";
        colors.text = "#1B5E20";
        colors.border = "#A5D6A7";
        colors.frame = "#C8E6C9";
        colors.hover = "#66BB6A";
        colors.pressed = "#2E7D32";
        colors.disabled = "#9E9E9E";
        break;

    case RedTheme:
        colors.background = "#FFEBEE";
        colors.primary = "#F44336";
        colors.secondary = "#C62828";
        colors.accent = "#EF5350";
        colors.text = "#B71C1C";
        colors.border = "#FFCDD2";
        colors.frame = "#FFCDD2";
        colors.hover = "#E57373";
        colors.pressed = "#C62828";
        colors.disabled = "#9E9E9E";
        break;
    }

    return colors;
}

QString ThemeManager::getMainWindowStyle() const
{
    ThemeColors colors = getThemeColors(currentTheme);

    return QString(
        "QMainWindow {"
        "background-color: %1;"
        "color: %2;"
        "}"
        "QStatusBar {"
        "background-color: %3;"
        "color: %2;"
        "border-top: 1px solid %4;"
        "}"
    ).arg(colors.background, colors.text, colors.frame, colors.border);
}

QString ThemeManager::getButtonStyle() const
{
    ThemeColors colors = getThemeColors(currentTheme);

    return QString(
        "QPushButton {"
        "background-color: %1;"
        "border: 2px solid %2;"
        "border-radius: 8px;"
        "color: white;"
        "font-weight: bold;"
        "font-size: 12px;"
        "padding: 8px 16px;"
        "}"
        "QPushButton:hover {"
        "background-color: %3;"
        "}"
        "QPushButton:pressed {"
        "background-color: %4;"
        "}"
        "QPushButton:disabled {"
        "background-color: %5;"
        "color: #888888;"
        "}"
    ).arg(colors.primary, colors.border, colors.hover, colors.pressed, colors.disabled);
}

QString ThemeManager::getFrameStyle() const
{
    ThemeColors colors = getThemeColors(currentTheme);

    return QString(
        "QFrame {"
        "background-color: %1;"
        "border: 1px solid %2;"
        "border-radius: 5px;"
        "}"
    ).arg(colors.frame, colors.border);
}

QString ThemeManager::getTableStyle() const
{
    ThemeColors colors = getThemeColors(currentTheme);

    return QString(
        "QTableWidget {"
        "background-color: %1;"
        "alternate-background-color: %2;"
        "color: %3;"
        "gridline-color: %4;"
        "selection-background-color: %5;"
        "}"
        "QHeaderView::section {"
        "background-color: %6;"
        "color: %3;"
        "border: 1px solid %4;"
        "padding: 8px;"
        "font-weight: bold;"
        "}"
    ).arg(colors.background, colors.frame, colors.text,
        colors.border, colors.accent, colors.secondary);
}

QString ThemeManager::getInputStyle() const
{
    ThemeColors colors = getThemeColors(currentTheme);

    return QString(
        "QLineEdit, QTextEdit, QComboBox {"
        "background-color: %1;"
        "border: 2px solid %2;"
        "border-radius: 4px;"
        "color: %3;"
        "padding: 6px;"
        "}"
        "QLineEdit:focus, QTextEdit:focus, QComboBox:focus {"
        "border-color: %4;"
        "}"
    ).arg(colors.background, colors.border, colors.text, colors.primary);
}