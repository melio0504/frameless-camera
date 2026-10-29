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
protected:
private:
}
