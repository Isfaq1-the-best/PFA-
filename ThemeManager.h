#pragma once
#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QSettings>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum Theme {
        LightTheme,
        DarkTheme,
        BlueTheme,
        GreenTheme,
        RedTheme
    };

    explicit ThemeManager(QObject* parent = nullptr);
    ~ThemeManager();

    // Gestion des thèmes
    void setTheme(Theme theme);
    Theme getCurrentThemeEnum() const;
    QString getCurrentTheme() const;
    void switchTheme();
    void loadThemeFromSettings();
    void saveThemeToSettings();

    // Couleurs du thème actuel
    QString getBackgroundColor() const;
    QString getPrimaryColor() const;
    QString getSecondaryColor() const;
    QString getAccentColor() const;
    QString getTextColor() const;
    QString getBorderColor() const;
    QString getFrameColor() const;
    QString getHoverColor() const;
    QString getPressedColor() const;
    QString getDisabledColor() const;

    // Styles complets
    QString getMainWindowStyle() const;
    QString getButtonStyle() const;
    QString getFrameStyle() const;
    QString getTableStyle() const;
    QString getInputStyle() const;

signals:
    void themeChanged(Theme newTheme);

private:
    Theme currentTheme;
    QSettings* settings;

    struct ThemeColors {
        QString background;
        QString primary;
        QString secondary;
        QString accent;
        QString text;
        QString border;
        QString frame;
        QString hover;
        QString pressed;
        QString disabled;
    };

    ThemeColors getThemeColors(Theme theme) const;
    void initializeThemes();
};

#endif // THEMEMANAGER_H
