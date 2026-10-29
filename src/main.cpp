// frameless camera
// author: romelio teodoro
// created as a movable webcam preview for recording with obs
// initial version: october 29, 2026

#include <QApplication>
#include <QCamera>
#include <QCameraDevice>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QGraphicsView>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMouseEvent>
#include <QWindow>

namespace {

// this margin defines the draggable resize area along each window edge
constexpr int kResizeMargin = 8;

// inheriting from qgraphicsview lets the window display a graphics scene
// while also receiving the mouse and resize events needed by a frameless window
class CameraWindow final : public QGraphicsView {
public:
  explicit CameraWindow(const QCameraDevice &device)
      : camera_(device), videoItem_(new QGraphicsVideoItem) {
    // configure the top-level window without decorations and keep it above others
    setWindowTitle(QStringLiteral("Frameless Camera"));
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint);
    setMinimumSize(160, 120);
    resize(440, 280);

    // place the video item inside a scene and remove every visible view decoration
    setScene(&scene_);
    scene_.addItem(videoItem_);
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setBackgroundBrush(Qt::black);
    setAlignment(Qt::AlignCenter);
    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    // fill the whole window while preserving the camera's aspect ratio
    videoItem_->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);

    // route frames from the selected camera into the graphics video item
    captureSession_.setCamera(&camera_);
    captureSession_.setVideoOutput(videoItem_);

    // update the scene after the camera reports its actual frame dimensions
    connect(videoItem_, &QGraphicsVideoItem::nativeSizeChanged, this,
            [this](const QSizeF &size) {
              if (size.isEmpty()) {
                return;
              }

              videoItem_->setSize(size);
              scene_.setSceneRect(videoItem_->boundingRect());
              updateVideoTransform();
            });

    // write camera failures to the terminal because the app has no visible ui
    connect(&camera_, &QCamera::errorOccurred, this,
            [](QCamera::Error, const QString &message) {
              qCritical().noquote() << "Camera error:" << message;
            });

    camera_.start();
  }

protected:
private:

} // namespace

int main(int argc, char *argv[]) {
  // every qt widgets program needs one application object
  QApplication application(argc, argv);
  QApplication::setApplicationName(QStringLiteral("frameless-camera"));

  // select the default camera and stop early if the system has no camera
  const QCameraDevice device = QMediaDevices::defaultVideoInput();
  if (device.isNull()) {
    qCritical("No camera was found.");
    return 1;
  }

  // create the camera window and enter qt's event loop
  CameraWindow window(device);
  window.show();

  return application.exec();
}
