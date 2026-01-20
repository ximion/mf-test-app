// Manifold test application: a spinning OpenGL triangle with an FPS counter.

#include <QElapsedTimer>
#include <QGuiApplication>
#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWindow>
#include <QPainter>

class TestWindow : public QOpenGLWindow, protected QOpenGLFunctions
{
public:
    TestWindow()
    {
        connect(this, &QOpenGLWindow::frameSwapped, this, qOverload<>(&QOpenGLWindow::update));
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

        const qreal dpr = devicePixelRatio();
        glViewport(0, 0, int(width() * dpr), int(height() * dpr));
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
        painter.drawText(QRect(12, 10, width() - 24, height() - 20),
                         Qt::AlignTop | Qt::AlignLeft,
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

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("mftestapp"));
    QGuiApplication::setApplicationVersion(QStringLiteral(APP_VERSION));
    QGuiApplication::setDesktopFileName(QStringLiteral("com.example.mftestapp"));

    TestWindow window;
    window.setTitle(QStringLiteral("Manifold Test App"));
    window.resize(800, 600);
    window.show();

    return app.exec();
}
