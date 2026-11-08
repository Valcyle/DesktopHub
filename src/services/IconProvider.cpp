#include "services/IconProvider.h"

#include <QPainter>
#include <QPainterPath>
#include <QPen>

#include "core/Item.h"
#include "models/ItemModel.h"

#ifdef Q_OS_WIN
#  include <windows.h>
#  include <commctrl.h>
#  include <commoncontrols.h>
#  include <objbase.h>
#  include <shellapi.h>
#  include <shlobj.h>
#endif

namespace {
#ifdef Q_OS_WIN

// Initializes COM for the thread that renders icons. Image providers may run
// on a worker thread, and SHGetImageList needs an initialized apartment.
struct ComGuard
{
    ComGuard() { result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); }
    ~ComGuard()
    {
        if (SUCCEEDED(result)) {
            CoUninitialize();
        }
    }
    HRESULT result = E_FAIL;
};

QImage imageFromHIcon(HICON icon)
{
    if (!icon) {
        return {};
    }

    const QImage image = QImage::fromHICON(icon);
    DestroyIcon(icon);
    return image;
}

QImage fromImageList(const QString &path, int size)
{
    thread_local ComGuard comGuard;
    Q_UNUSED(comGuard);

    SHFILEINFOW info{};
    if (!SHGetFileInfoW(reinterpret_cast<LPCWSTR>(path.utf16()), 0, &info, sizeof(info),
                        SHGFI_SYSICONINDEX | SHGFI_LARGEICON)) {
        return {};
    }

    // MinGW does not always export IID_IImageList, so use the documented GUID
    // directly. {46EB5926-582E-4017-9FDF-E8998DAA0950}
    static const GUID imageListIid = {
        0x46eb5926, 0x582e, 0x4017,
        { 0x9f, 0xdf, 0xe8, 0x99, 0x8d, 0xaa, 0x09, 0x50 }
    };

    IImageList *imageList = nullptr;
    const int listSize = size >= 40 ? SHIL_EXTRALARGE : SHIL_LARGE;
    if (FAILED(SHGetImageList(listSize, imageListIid, reinterpret_cast<void **>(&imageList)))
        || imageList == nullptr) {
        return {};
    }

    HICON icon = nullptr;
    imageList->GetIcon(info.iIcon, ILD_TRANSPARENT, &icon);
    imageList->Release();

    return imageFromHIcon(icon);
}

QImage fromShellFileInfo(const QString &path)
{
    SHFILEINFOW info{};
    if (!SHGetFileInfoW(reinterpret_cast<LPCWSTR>(path.utf16()), 0, &info, sizeof(info),
                        SHGFI_ICON | SHGFI_LARGEICON)) {
        return {};
    }

    return imageFromHIcon(info.hIcon);
}
#endif // Q_OS_WIN

QImage drawFallback(const QString &type, int size)
{
    QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    const qreal s = size;
    const QColor accent(0x8f, 0xb4, 0xe8);
    const QColor fill(0x8f, 0xb4, 0xe8, 40);
    const qreal penWidth = qMax(1.5, s / 16.0);

    QPen pen(accent, penWidth);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(fill);

    if (type == QLatin1String("url")) {
        // Globe: outer circle, equator, and a vertical ellipse.
        const QRectF circle(s * 0.12, s * 0.12, s * 0.76, s * 0.76);
        painter.drawEllipse(circle);
        painter.drawLine(QPointF(s * 0.12, s * 0.5), QPointF(s * 0.88, s * 0.5));
        painter.drawEllipse(QRectF(s * 0.32, s * 0.12, s * 0.36, s * 0.76));
        return image;
    }

    if (type == QLatin1String("folder")) {
        QPainterPath path;
        path.moveTo(s * 0.1, s * 0.32);
        path.lineTo(s * 0.4, s * 0.32);
        path.lineTo(s * 0.48, s * 0.42);
        path.lineTo(s * 0.9, s * 0.42);
        path.lineTo(s * 0.9, s * 0.82);
        path.lineTo(s * 0.1, s * 0.82);
        path.closeSubpath();
        painter.drawPath(path);
        return image;
    }

    if (type == QLatin1String("exe") || type == QLatin1String("lnk")) {
        // Application window with a play triangle.
        painter.drawRoundedRect(QRectF(s * 0.1, s * 0.16, s * 0.8, s * 0.68),
                                s * 0.1, s * 0.1);
        painter.setBrush(accent);
        QPainterPath triangle;
        triangle.moveTo(s * 0.42, s * 0.36);
        triangle.lineTo(s * 0.66, s * 0.5);
        triangle.lineTo(s * 0.42, s * 0.64);
        triangle.closeSubpath();
        painter.drawPath(triangle);
        return image;
    }

    // Generic document.
    painter.drawRoundedRect(QRectF(s * 0.24, s * 0.12, s * 0.52, s * 0.76),
                            s * 0.06, s * 0.06);
    painter.drawLine(QPointF(s * 0.34, s * 0.38), QPointF(s * 0.66, s * 0.38));
    painter.drawLine(QPointF(s * 0.34, s * 0.5), QPointF(s * 0.66, s * 0.5));
    painter.drawLine(QPointF(s * 0.34, s * 0.62), QPointF(s * 0.56, s * 0.62));
    return image;
}
} // namespace

IconProvider::IconProvider(ItemModel *model)
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_model(model)
{
}

QImage IconProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    const int slash = id.indexOf(QLatin1Char('/'));
    const QString itemId = slash >= 0 ? id.left(slash) : id;

    int targetSize = 32;
    if (requestedSize.isValid() && requestedSize.width() > 0) {
        targetSize = qBound(16, qMax(requestedSize.width(), requestedSize.height()), 64);
    }

    const QString cacheKey = id + QLatin1Char(':') + QString::number(targetSize);
    {
        QMutexLocker locker(&m_cacheMutex);
        const auto cached = m_cache.constFind(cacheKey);
        if (cached != m_cache.cend()) {
            if (size) {
                *size = cached->size();
            }
            return cached.value();
        }
    }

    const Item item = m_model ? m_model->itemById(itemId) : Item{};
    QImage image;
    if (item.id.isEmpty()) {
        image = fallbackIcon(QStringLiteral("file"), targetSize);
    } else if (item.type == QLatin1String("url")) {
        image = fallbackIcon(QStringLiteral("url"), targetSize);
    } else {
        image = shellIcon(item.target, targetSize);
        if (image.isNull()) {
            image = fallbackIcon(item.type, targetSize);
        }
    }

    if (image.isNull()) {
        image = fallbackIcon(QStringLiteral("file"), targetSize);
    }

    if (image.width() != targetSize || image.height() != targetSize) {
        image = image.scaled(targetSize, targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    if (size) {
        *size = image.size();
    }

    {
        QMutexLocker locker(&m_cacheMutex);
        if (m_cache.size() > 256) {
            m_cache.clear();
        }
        m_cache.insert(cacheKey, image);
    }

    return image;
}

QImage IconProvider::shellIcon(const QString &path, int size) const
{
#ifdef Q_OS_WIN
    QImage image = fromImageList(path, size);
    if (image.isNull()) {
        image = fromShellFileInfo(path);
    }
    return image;
#else
    Q_UNUSED(path)
    Q_UNUSED(size)
    return {};
#endif
}

QImage IconProvider::fallbackIcon(const QString &type, int size) const
{
    return drawFallback(type, size);
}
