/*
 * Copyright (C) 2026 Matthias Klumpp <matthias@tenstral.net>
 *
 * SPDX-License-Identifier: MIT
 */

#include <QApplication>
#include <QClipboard>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDesktopServices>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QFontDatabase>
#include <QHeaderView>
#include <QLineEdit>
#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QSplitter>
#include <QTabWidget>
#include <QTreeView>
#include <QUrl>
#include <QVBoxLayout>

#include <functional>
#include <memory>
#include <unistd.h>

class GLView : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    GLView()
    {
        setMinimumSize(240, 240);
        connect(this, &QOpenGLWidget::frameSwapped, this, qOverload<>(&QWidget::update));
    }

protected:
    void initializeGL() override
    {
        initializeOpenGLFunctions();
        m_renderer = QString::fromLatin1(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
        m_glVersion = QString::fromLatin1(reinterpret_cast<const char *>(glGetString(GL_VERSION)));

        m_program.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                          "attribute vec2 pos;\n"
                                          "attribute vec3 color;\n"
                                          "varying vec3 vColor;\n"
                                          "uniform mat4 matrix;\n"
                                          "void main() {\n"
                                          "    vColor = color;\n"
                                          "    gl_Position = matrix * vec4(pos, 0.0, 1.0);\n"
                                          "}\n");
        m_program.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                          "varying vec3 vColor;\n"
                                          "void main() {\n"
                                          "    gl_FragColor = vec4(vColor, 1.0);\n"
                                          "}\n");
        m_program.bindAttributeLocation("pos", 0);
        m_program.bindAttributeLocation("color", 1);
        if (!m_program.link())
            qFatal("Shader program failed to link: %s", qPrintable(m_program.log()));

        m_clock.start();
        m_fpsTimer.start();
    }

    void paintGL() override
    {
        static const GLfloat vertices[] = {0.0f, 0.7f, -0.6f, -0.5f, 0.6f, -0.5f};
        static const GLfloat colors[] = {0.88f, 0.11f, 0.14f, 0.2f, 0.82f, 0.48f, 0.21f, 0.52f, 0.89f};

        glClearColor(0.14f, 0.12f, 0.19f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        QMatrix4x4 matrix;
        const float aspect = float(width()) / float(qMax(height(), 1));
        matrix.ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
        matrix.rotate(m_clock.elapsed() * 0.09f, 0.0f, 0.0f, 1.0f); // 90°/s

        m_program.bind();
        m_program.setUniformValue("matrix", matrix);
        m_program.enableAttributeArray(0);
        m_program.enableAttributeArray(1);
        m_program.setAttributeArray(0, vertices, 2);
        m_program.setAttributeArray(1, colors, 3);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        m_program.disableAttributeArray(0);
        m_program.disableAttributeArray(1);
        m_program.release();

        // FPS, averaged over half a second
        ++m_frames;
        if (const qint64 ms = m_fpsTimer.elapsed(); ms >= 500) {
            m_fps = m_frames * 1000.0 / ms;
            m_frames = 0;
            m_fpsTimer.restart();
        }

        QPainter painter(this);
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPointSize(14);
        painter.setFont(font);
        painter.drawText(rect().adjusted(12, 10, -12, -10),
                         Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
                         QStringLiteral("%1 FPS\n%2\n%3").arg(m_fps, 0, 'f', 1).arg(m_renderer, m_glVersion));
    }

private:
    QOpenGLShaderProgram m_program;
    QElapsedTimer m_clock;
    QElapsedTimer m_fpsTimer;
    int m_frames = 0;
    double m_fps = 0.0;
    QString m_renderer;
    QString m_glVersion;
};

// QFileSystemModel with a permissions column appended
class FsModel : public QFileSystemModel
{
public:
    int columnCount(const QModelIndex &parent = {}) const override
    {
        return QFileSystemModel::columnCount(parent) + 1;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (index.column() != columnCount() - 1)
            return QFileSystemModel::data(index, role);
        if (role != Qt::DisplayRole)
            return {};
        const QFileInfo info = fileInfo(index);
        const auto perms = info.permissions();
        static const QFile::Permission bits[] = {
            QFile::ReadOwner, QFile::WriteOwner, QFile::ExeOwner,
            QFile::ReadGroup, QFile::WriteGroup, QFile::ExeGroup,
            QFile::ReadOther, QFile::WriteOther, QFile::ExeOther,
        };
        QString text;
        for (int i = 0; i < 9; ++i)
            text += perms.testFlag(bits[i]) ? QLatin1Char("rwx"[i % 3]) : QLatin1Char('-');
        return QStringLiteral("%1 %2:%3").arg(text, info.owner(), info.group());
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (section == columnCount() - 1 && orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return QStringLiteral("Permissions");
        return QFileSystemModel::headerData(section, orientation, role);
    }
};

static QWidget *makeFilesTab()
{
    auto *page = new QWidget;
    auto *pathBar = new QLineEdit(QDir::homePath());
    pathBar->setPlaceholderText(QStringLiteral("Path, then Enter"));
    auto *model = new FsModel;
    model->setParent(page);
    model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    model->setRootPath(QStringLiteral("/"));
    auto *tree = new QTreeView;
    tree->setModel(model);
    tree->setSortingEnabled(true);
    tree->sortByColumn(0, Qt::AscendingOrder);
    tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    const auto jump = [=] {
        const QModelIndex index = model->index(pathBar->text());
        if (!index.isValid()) {
            pathBar->setStyleSheet(QStringLiteral("color: red"));
            return;
        }
        tree->setCurrentIndex(index);
        tree->expand(index);
        tree->scrollTo(index, QAbstractItemView::PositionAtTop);
    };
    QObject::connect(pathBar, &QLineEdit::returnPressed, tree, jump);
    QObject::connect(pathBar, &QLineEdit::textEdited, pathBar, [=] { pathBar->setStyleSheet({}); });
    QObject::connect(tree->selectionModel(), &QItemSelectionModel::currentChanged, pathBar,
                     [=](const QModelIndex &index) { pathBar->setText(model->filePath(index)); });

    // Start in the user's home directory
    jump();
    auto connection = std::make_shared<QMetaObject::Connection>();
    *connection = QObject::connect(model, &QFileSystemModel::directoryLoaded, tree, [=](const QString &path) {
        if (path != QFileInfo(QDir::homePath()).path())
            return;
        tree->scrollTo(model->index(QDir::homePath()), QAbstractItemView::PositionAtTop);
        QObject::disconnect(*connection);
    });

    auto *layout = new QVBoxLayout(page);
    layout->addWidget(pathBar);
    layout->addWidget(tree);
    return page;
}

static QPlainTextEdit *makeTextView()
{
    auto *view = new QPlainTextEdit;
    view->setReadOnly(true);
    view->setLineWrapMode(QPlainTextEdit::NoWrap);
    view->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    return view;
}

// A read-only text view filled by `probe`, with a button to run it again
static QWidget *makeTextTab(std::function<QString()> probe)
{
    auto *page = new QWidget;
    auto *view = makeTextView();
    auto *refresh = new QPushButton(QStringLiteral("Refresh"));
    QObject::connect(refresh, &QPushButton::clicked, view, [=] { view->setPlainText(probe()); });
    view->setPlainText(probe());

    auto *layout = new QVBoxLayout(page);
    layout->addWidget(refresh, 0, Qt::AlignLeft);
    layout->addWidget(view);
    return page;
}

static QString readFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return QStringLiteral("(%1)\n").arg(file.errorString());
    return QString::fromUtf8(file.readAll());
}

static QString systemInfo()
{
    QString out;
    const auto section = [&out](const QString &title, const QString &body) {
        out += QStringLiteral("=== %1 ===\n%2%3\n").arg(title, body, body.endsWith(QLatin1Char('\n')) ? "" : "\n");
    };

    QStringList groups;
    gid_t gids[256];
    for (int i = 0, n = getgroups(256, gids); i < n; ++i)
        groups << QString::number(gids[i]);
    section(QStringLiteral("Process"),
            QStringLiteral("pid=%1 uid=%2 euid=%3 gid=%4 egid=%5\ngroups=%6\ncwd=%7\nexe=%8")
                .arg(getpid()).arg(getuid()).arg(geteuid()).arg(getgid()).arg(getegid())
                .arg(groups.join(QLatin1Char(',')), QDir::currentPath(), QCoreApplication::applicationFilePath()));

    section(QStringLiteral("/etc/os-release"), readFile(QStringLiteral("/etc/os-release")));
    // Where the image description ends up at runtime is not settled; try both
    section(QStringLiteral("/manifold.json"), readFile(QStringLiteral("/manifold.json")));
    section(QStringLiteral("/app/manifold.json"), readFile(QStringLiteral("/app/manifold.json")));

    QStringList env = QProcessEnvironment::systemEnvironment().toStringList();
    env.sort();
    section(QStringLiteral("Environment"), env.join(QLatin1Char('\n')));
    section(QStringLiteral("/proc/self/mountinfo"), readFile(QStringLiteral("/proc/self/mountinfo")));
    return out;
}

// Paths of the shared objects mapped into this process
static QString loadedLibraries()
{
    QStringList libs;
    const QStringList lines = readFile(QStringLiteral("/proc/self/maps")).split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        const int slash = line.indexOf(QLatin1Char('/'));
        if (slash < 0)
            continue;
        const QString path = line.mid(slash);
        if (path.contains(QLatin1String(".so")) && !libs.contains(path))
            libs << path;
    }
    libs.sort();
    return QStringLiteral("%1 shared objects mapped\n\n%2").arg(libs.size()).arg(libs.join(QLatin1Char('\n')));
}

static QWidget *makeDesktopTab()
{
    auto *page = new QWidget;
    auto *log = makeTextView();
    auto *layout = new QVBoxLayout(page);
    const auto addButton = [=](const QString &label, std::function<QString()> action) {
        auto *button = new QPushButton(label);
        QObject::connect(button, &QPushButton::clicked, log,
                         [=] { log->appendPlainText(QStringLiteral("%1: %2").arg(label, action())); });
        layout->addWidget(button);
    };

    addButton(QStringLiteral("Open file dialog"), [=] {
        const QString file = QFileDialog::getOpenFileName(page);
        return file.isEmpty() ? QStringLiteral("cancelled") : file;
    });
    addButton(QStringLiteral("Open https://example.com"), [] {
        return QDesktopServices::openUrl(QUrl(QStringLiteral("https://example.com")))
                   ? QStringLiteral("handed over")
                   : QStringLiteral("failed");
    });
    addButton(QStringLiteral("Copy to clipboard"), [] {
        const QString text = QStringLiteral("Hello from mftestapp, pid %1").arg(getpid());
        QGuiApplication::clipboard()->setText(text);
        return text;
    });
    addButton(QStringLiteral("Paste from clipboard"), [] { return QGuiApplication::clipboard()->text(); });
    addButton(QStringLiteral("Check D-Bus session bus"), [] {
        const QDBusConnection bus = QDBusConnection::sessionBus();
        if (!bus.isConnected())
            return QStringLiteral("not connected: %1").arg(bus.lastError().message());
        const QStringList names = bus.interface()->registeredServiceNames().value();
        return QStringLiteral("connected as %1, %2 names on the bus, portal %3")
            .arg(bus.baseService())
            .arg(names.size())
            .arg(names.contains(QLatin1String("org.freedesktop.portal.Desktop")) ? "present" : "absent");
    });

    layout->addWidget(log);
    return page;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("mftestapp"));
    QApplication::setApplicationVersion(QStringLiteral(APP_VERSION));
    QApplication::setDesktopFileName(QStringLiteral("com.example.mftestapp"));

    auto *tabs = new QTabWidget;
    tabs->addTab(makeFilesTab(), QStringLiteral("Files"));
    tabs->addTab(makeTextTab(systemInfo), QStringLiteral("System"));
    tabs->addTab(makeTextTab(loadedLibraries), QStringLiteral("Libraries"));
    tabs->addTab(makeDesktopTab(), QStringLiteral("Desktop"));

    QSplitter window;
    window.addWidget(new GLView);
    window.addWidget(tabs);
    window.setStretchFactor(0, 1);
    window.setStretchFactor(1, 2);
    window.setWindowTitle(QStringLiteral("Manifold Test App"));
    window.resize(1100, 650);
    window.show();

    return app.exec();
}
