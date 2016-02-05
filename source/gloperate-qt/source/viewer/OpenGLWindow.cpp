
#include "gloperate-qt/viewer/OpenGLWindow.h"

#include <QApplication>
#include <QDebug>
#include <QResizeEvent>
#include <QOpenGLContext>


namespace gloperate_qt
{


QSurfaceFormat OpenGLWindow::defaultFormat()
{
    QSurfaceFormat format;
    format.setProfile(QSurfaceFormat::CoreProfile);
    #ifndef NDEBUG
        format.setOption(QSurfaceFormat::DebugContext);
    #endif
    return format;
}

OpenGLWindow::OpenGLWindow()
: OpenGLWindow(defaultFormat())
{
}

OpenGLWindow::OpenGLWindow(const QSurfaceFormat & format)
: m_context(new QOpenGLContext)
, m_initialized(false)
, m_updatePending(false)
{
    QSurfaceFormat f(format);
    f.setRenderableType(QSurfaceFormat::OpenGL);

    setSurfaceType(OpenGLSurface);
    create();

    if (f.version().first < 3)
    {
        m_context->setFormat(f);
        if (!m_context->create()) {
            qDebug() << "Could not create intermediate OpenGL context.";
            QApplication::quit();
        } else {
            QSurfaceFormat intermediateFormat = m_context->format();
            qDebug().nospace() << "Created intermediate OpenGL context " << intermediateFormat.version().first << "." << intermediateFormat.version().second;

            if ((intermediateFormat.version().first == 3 && intermediateFormat.version().second == 0) || intermediateFormat.version().first < 3)
            {
                f.setMajorVersion(3);
                f.setMinorVersion(2);
                f.setProfile(QSurfaceFormat::CoreProfile);
            }
        }
    }

    m_context->setFormat(f);
    if (!m_context->create()) {
        qDebug() << "Could not create OpenGL context.";
        QApplication::quit();
    } else {
        qDebug().nospace() << "Created OpenGL context " << m_context->format().version().first << "." << m_context->format().version().second;
    }
}

OpenGLWindow::~OpenGLWindow()
{
}

QOpenGLContext * OpenGLWindow::context() const
{
    return m_context.data();
}

void OpenGLWindow::updateGL()
{
    if (!m_updatePending) {
        m_updatePending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

void OpenGLWindow::initialize()
{
    m_context->makeCurrent(this);

    onInitialize();

    m_context->doneCurrent();

    m_initialized = true;
}

void OpenGLWindow::resize(QResizeEvent * event)
{
    if (!m_initialized) {
        initialize();
    }

    m_context->makeCurrent(this);

    QResizeEvent deviceSpecificResizeEvent(event->size() * devicePixelRatio(), event->oldSize() * devicePixelRatio());

    onResize(&deviceSpecificResizeEvent);

    m_context->doneCurrent();
}

void OpenGLWindow::paint()
{
    if (!m_initialized) {
        initialize();
    }

    if (!isExposed()) {
        return;
    }

    m_updatePending = false;

    m_context->makeCurrent(this);

    onPaint();

    m_context->swapBuffers(this);

    m_context->doneCurrent();
}

void OpenGLWindow::onInitialize()
{
}

void OpenGLWindow::onResize(QResizeEvent * )
{
}

void OpenGLWindow::onPaint()
{
}

bool OpenGLWindow::event(QEvent * event)
{
    switch (event->type()) {
        case QEvent::UpdateRequest:
            paint();
            return true;
        case QEvent::Enter:
            enterEvent(event);
            return true;
        case QEvent::Leave:
            leaveEvent(event);
            return true;
        default:
            return QWindow::event(event);
    }
}

void OpenGLWindow::resizeEvent(QResizeEvent * event)
{
    resize(event);
    paint();
}

void OpenGLWindow::exposeEvent(QExposeEvent * )
{
    paint();
}

void OpenGLWindow::enterEvent(QEvent *)
{
}

void OpenGLWindow::leaveEvent(QEvent *)
{
}

void OpenGLWindow::makeCurrent()
{
    m_context->makeCurrent(this);
}

void OpenGLWindow::doneCurrent()
{
    m_context->doneCurrent();
}


} // namespace gloperate-qt