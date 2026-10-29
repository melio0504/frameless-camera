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
  void resizeEvent(QResizeEvent *event) override {
    // let qt resize the view first, then refit the video to the new dimensions
    QGraphicsView::resizeEvent(event);
    updateVideoTransform();
  }

  void mousePressEvent(QMouseEvent *event) override {
    // only the left mouse button starts window movement or resizing
    if (event->button() != Qt::LeftButton) {
      QGraphicsView::mousePressEvent(event);
      return;
    }

    const Qt::Edges edges = resizeEdgesAt(event->position().toPoint());
    QWindow *nativeWindow = windowHandle();

    // ask the desktop compositor to resize when an edge or corner is pressed
    if (nativeWindow != nullptr && edges != Qt::Edges{} &&
        nativeWindow->startSystemResize(edges)) {
      event->accept();
      return;
    }

    // otherwise, ask the compositor to move the entire window
    if (nativeWindow != nullptr && nativeWindow->startSystemMove()) {
      event->accept();
      return;
    }

    // fall back to manual movement when the window manager cannot move it
    dragging_ = true;
    dragOffset_ = event->globalPosition().toPoint() - frameGeometry().topLeft();
    event->accept();
  }

  void mouseMoveEvent(QMouseEvent *event) override {
    // manual dragging is only used when native system movement is unavailable
    if (dragging_ && (event->buttons() & Qt::LeftButton)) {
      move(event->globalPosition().toPoint() - dragOffset_);
      event->accept();
      return;
    }

    // show a directional cursor when the pointer is near a resize edge
    updateResizeCursor(resizeEdgesAt(event->position().toPoint()));
    event->accept();
  }

  void mouseReleaseEvent(QMouseEvent *event) override {
    if (event->button() == Qt::LeftButton) {
      dragging_ = false;
      event->accept();
      return;
    }

    QGraphicsView::mouseReleaseEvent(event);
  }

  void leaveEvent(QEvent *event) override {
    if (!dragging_) {
      viewport()->unsetCursor();
    }
    QGraphicsView::leaveEvent(event);
  }

private:
  void updateVideoTransform() {
    // there is nothing to scale until both the video and viewport have a size
    if (videoItem_->boundingRect().isEmpty() || viewport()->size().isEmpty()) {
      return;
    }

    resetTransform();
    fitInView(videoItem_, Qt::KeepAspectRatioByExpanding);

    // mirror horizontally so the preview behaves like a mirror
    scale(-1.0, 1.0);
    centerOn(videoItem_);
  }

  [[nodiscard]] Qt::Edges resizeEdgesAt(const QPoint &position) const {
    // combine horizontal and vertical edges to identify sides and corners
    Qt::Edges edges;

    if (position.x() <= kResizeMargin) {
      edges |= Qt::LeftEdge;
    } else if (position.x() >= viewport()->width() - kResizeMargin) {
      edges |= Qt::RightEdge;
    }

    if (position.y() <= kResizeMargin) {
      edges |= Qt::TopEdge;
    } else if (position.y() >= viewport()->height() - kResizeMargin) {
      edges |= Qt::BottomEdge;
    }

    return edges;
  }

  void updateResizeCursor(Qt::Edges edges) {
    // use the cursor shape that matches the detected edge or corner
    if (edges == (Qt::TopEdge | Qt::LeftEdge) ||
        edges == (Qt::BottomEdge | Qt::RightEdge)) {
      viewport()->setCursor(Qt::SizeFDiagCursor);
    } else if (edges == (Qt::TopEdge | Qt::RightEdge) ||
               edges == (Qt::BottomEdge | Qt::LeftEdge)) {
      viewport()->setCursor(Qt::SizeBDiagCursor);
    } else if (edges.testFlag(Qt::LeftEdge) || edges.testFlag(Qt::RightEdge)) {
      viewport()->setCursor(Qt::SizeHorCursor);
    } else if (edges.testFlag(Qt::TopEdge) || edges.testFlag(Qt::BottomEdge)) {
      viewport()->setCursor(Qt::SizeVerCursor);
    } else {
      viewport()->unsetCursor();
    }
  }

  // these members keep the camera pipeline and graphics scene alive
  QCamera camera_;
  QGraphicsScene scene_;
  QMediaCaptureSession captureSession_;
  QGraphicsVideoItem *videoItem_;
  bool dragging_ = false;
  QPoint dragOffset_;
};

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
