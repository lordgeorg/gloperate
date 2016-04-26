
#include <QApplication>
#include <QQmlEngine>

#include <globjects/base/baselogging.h>

#include <gloperate/gloperate.h>
#include <gloperate/viewer/ViewerContext.h>
#include <gloperate/viewer/GLContextUtils.h>
#include <gloperate/viewer/RenderSurface.h>
#include <gloperate/scripting/ScriptEnvironment.h>

#include <gloperate-qt/viewer/Application.h>
#include <gloperate-qt/viewer/GLContext.h>
#include <gloperate-qt/viewer/UpdateManager.h>

#include <cppexpose/reflection/Object.h>

#include <cppexpose/scripting/ScriptContext.h>

#include <gloperate-ffmpeg/VideoTool.h>

#include <gloperate-qtquick/viewer/QmlEngine.h>
#include <gloperate-qtquick/viewer/QuickView.h>
#include <gloperate-qtquick/viewer/QmlScriptContext.h>
#include <gloperate-qtquick/viewer/RenderItem.h>



using namespace gloperate;
using namespace gloperate_qt;
using namespace gloperate_qtquick;
using namespace gloperate_ffmpeg;


int main(int argc, char * argv[])
{
    // Determine data paths
    QString qmlPath = QString::fromStdString(gloperate::dataPath()) + "/gloperate/qml";

    // Create viewer context
    ViewerContext viewerContext;

    // Initialize Qt application
    gloperate_qt::Application app(&viewerContext, argc, argv);
    UpdateManager updateManager(&viewerContext);

    // Create QML engine
    QmlEngine qmlEngine(&viewerContext);
    qmlEngine.addImportPath(qmlPath);

    // Create scripting context backend
    viewerContext.scriptEnvironment()->setupScripting(
        new gloperate_qtquick::QmlScriptContext(&qmlEngine)
    );

    // Load and show QML
    QuickView * window = new QuickView(&qmlEngine);
    window->setResizeMode(QQuickView::SizeRootObjectToView);
    window->setSource(QUrl(qmlPath + "/Viewer.qml"));
    window->setGeometry(100, 100, 1280, 720);
    window->show();


    QQuickItem * item = window->rootObject();
    QQuickItem * qmlRenderItem = item->findChild<QQuickItem*>("renderItem");
    
    RenderItem * renderItem = static_cast<RenderItem *>(qmlRenderItem);
    RenderSurface * renderSurface = static_cast<RenderSurface *>(renderItem->surface());
    
    VideoTool * videoTool = new VideoTool();
    renderSurface->setVideoTool(videoTool);


    // Run main loop
    return app.exec();
}
