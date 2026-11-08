#include "core/AppSettings.h"

#include <QLocale>
#include <QStandardPaths>

namespace {
constexpr auto japaneseLanguage = "ja";
constexpr auto englishLanguage = "en";

// Picks the UI language from the user's preferred UI languages.
//
// QLocale::system().language() follows the Windows regional format, which can
// differ from the display language. uiLanguages() reflects the languages the
// user actually wants to see in applications.
QString detectUiLanguage()
{
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &localeName : uiLanguages) {
        const QLocale locale(localeName);
        if (locale.language() == QLocale::Japanese) {
            return QString::fromLatin1(japaneseLanguage);
        }
        if (locale.language() == QLocale::English) {
            return QString::fromLatin1(englishLanguage);
        }
    }

    return QString::fromLatin1(englishLanguage);
}
} // namespace

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
    , m_language(detectUiLanguage())
{
}

QString AppSettings::language() const
{
    return m_language;
}

void AppSettings::setLanguage(const QString &language)
{
    // Only the two supported UI languages are accepted.
    if (language != QLatin1String(japaneseLanguage)
        && language != QLatin1String(englishLanguage)) {
        return;
    }

    if (m_language == language) {
        return;
    }

    m_language = language;
    emit languageChanged();
}

QString AppSettings::theme() const
{
    // The MVP uses a fixed dark theme.
    return QStringLiteral("dark");
}

QString AppSettings::dataLocation() const
{
    // Depends on the organization and application names set in main.cpp.
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString AppSettings::translationFile() const
{
    // Compiled by qt_add_translations and embedded at this resource path.
    return QStringLiteral(":/i18n/desktophub_%1.qm").arg(m_language);
}
