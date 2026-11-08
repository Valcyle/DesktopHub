#pragma once

#include <QHash>
#include <QImage>
#include <QMutex>
#include <QQuickImageProvider>

class ItemModel;

// Serves shell icons to QML through image://itemicon/<item-id>/<revision>.
// The revision in the URL forces QML to reload the image after an edit.
class IconProvider : public QQuickImageProvider
{
public:
    explicit IconProvider(ItemModel *model);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    QImage shellIcon(const QString &path, int size) const;
    QImage fallbackIcon(const QString &type, int size) const;

    ItemModel *m_model = nullptr;
    QHash<QString, QImage> m_cache;
    mutable QMutex m_cacheMutex;
};
