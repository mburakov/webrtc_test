/*
 * libjingle
 * Copyright 2012, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "main_wnd.h"

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>

#include <stddef.h>

#include "backend/defaults.h"
#include "talk/base/common.h"
#include "talk/base/logging.h"
#include "talk/base/stringutils.h"

using talk_base::sprintfn;

void QtMainWnd::HandleUIThreadCallback(int msg_id, void* data)
{
  callback_->UIThreadCallback(msg_id, data);
}

//
// GtkMainWnd implementation.
//
QtMainWnd::QtMainWnd(const char* server, int port, bool autoconnect,
                       bool autocall)
    : server_(server), autoconnect_(autoconnect), autocall_(autocall), state_(CONNECT_TO_SERVER), closing_(false)
{
  char buffer[10];
  sprintfn(buffer, sizeof(buffer), "%i", port);
  port_ = buffer;
}

QtMainWnd::~QtMainWnd() {
}

void QtMainWnd::RegisterObserver(MainWndCallback* callback) {
  callback_ = callback;
}

bool QtMainWnd::IsWindow() {
  return !closing_;
}

void QtMainWnd::MessageBox(const char* caption, const char* text, bool is_error) {
  if (is_error) QMessageBox::critical(this, caption, text);
  else  QMessageBox::information(this, caption, text);
}

MainWindow::UI QtMainWnd::current_ui() {
  return state_;
}

void QtMainWnd::StartLocalRenderer(webrtc::VideoTrackInterface* local_video) {
  local_renderer_.reset(new VideoRenderer(this, local_video));
}

void QtMainWnd::StopLocalRenderer() {
  local_renderer_.reset();
}

void QtMainWnd::StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video) {
  remote_renderer_.reset(new VideoRenderer(this, remote_video));
}

void QtMainWnd::StopRemoteRenderer() {
  remote_renderer_.reset();
}

void QtMainWnd::QueueUIThreadCallback(int msg_id, void* data) {
  QMetaObject::invokeMethod(this, "HandleUIThreadCallback", Qt::QueuedConnection, Q_ARG(int, msg_id), Q_ARG(void*, data));
}

QSharedPointer<QMutexLocker> QtMainWnd::Synchronized()
{
  static QMutex mutex;
  return QSharedPointer<QMutexLocker>(new QMutexLocker(&mutex));
}

bool QtMainWnd::Create() {
  show();
  SwitchToConnectUI();
  return true;
}

bool QtMainWnd::Destroy() {
  return true;
}

void QtMainWnd::SwitchToConnectUI() {
  if (layout()) delete layout();
  qDeleteAll(children());

  QHBoxLayout* hbox = new QHBoxLayout();
  hbox->addWidget(new QLabel("Server"));
  hbox->addWidget(server_edit_ = new QLineEdit("localhost"));
  hbox->addWidget(port_edit_ = new QLineEdit("8888"));
  hbox->addWidget(connect_button_ = new QPushButton("Connect"));

  port_edit_->setFixedWidth(70);
  connect(connect_button_, SIGNAL(clicked()), this, SLOT(OnConnect()));

  setLayout(hbox);
  state_ = CONNECT_TO_SERVER;
}

void QtMainWnd::SwitchToPeerList(const Peers& peers) {
  if (layout()) delete layout();
  qDeleteAll(children());

  peer_list_ = new QListWidget(this);
  peer_list_->addItem("List of currently connected peers:");
  for (const auto& it: peers)
  {
    std::string item = it.second + " / " + std::to_string(it.first);
    peer_list_->addItem(item.c_str());
  }
  connect(peer_list_, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(OnPeerActivated(QListWidgetItem*)));
  peer_list_->show();

  state_ = LIST_PEERS;
}

void QtMainWnd::SwitchToStreamingUI() {
  if (layout()) delete layout();
  qDeleteAll(children());

  state_ = STREAMING;
}

void QtMainWnd::closeEvent(QCloseEvent* event)
{
  callback_->Close();
  closing_ = true;
}

void QtMainWnd::OnConnect()
{
  connect_button_->setEnabled(false);
  server_ = server_edit_->text().toStdString();
  port_ = port_edit_->text().toStdString();
  int port = port_.length() ? atoi(port_.c_str()) : 0;
  callback_->StartLogin(server_, port);
}

void QtMainWnd::OnPeerActivated(QListWidgetItem* peer)
{
  // TODO: Sanity checks
  const std::string& text = peer->text().toStdString();
  auto idx = text.rfind('/') + 2;
  int peer_num = atoi(text.c_str() + idx);
  callback_->ConnectToPeer(peer_num);
}

void QtMainWnd::paintEvent(QPaintEvent* event)
{
  QWidget::paintEvent(event);
  if (!local_renderer_ && !remote_renderer_)
    return;

  // Synchronized with frame size change
  const auto& sync = Synchronized();

  QPainter painter(this);

  if (remote_renderer_)
  {
    const QImage remote(remote_renderer_->image(), remote_renderer_->width(), remote_renderer_->height(), QImage::Format_ARGB32);
    painter.drawImage(QPoint(0, 0), remote.scaled(width(), height()));
  }

  if (local_renderer_)
  {
    const QImage local(local_renderer_->image(), local_renderer_->width(), local_renderer_->height(), QImage::Format_ARGB32);
    painter.drawImage(QPoint(0, 0), local.scaled(width() / 4, height() / 4));
  }
}

QtMainWnd::VideoRenderer::VideoRenderer(
    QtMainWnd* main_wnd,
    webrtc::VideoTrackInterface* track_to_render)
    : width_(0),
      height_(0),
      main_wnd_(main_wnd),
      rendered_track_(track_to_render) {
  rendered_track_->AddRenderer(this);
}

QtMainWnd::VideoRenderer::~VideoRenderer() {
  rendered_track_->RemoveRenderer(this);
}

void QtMainWnd::VideoRenderer::SetSize(int width, int height) {
  // Must be synchronized with draw function
  const auto& sync = main_wnd_->Synchronized();

  width_ = width;
  height_ = height;
  image_.reset(new uint8[width * height * 4]);
}

void QtMainWnd::VideoRenderer::RenderFrame(const cricket::VideoFrame* frame) {
  int size = width_ * height_ * 4;
  // TODO: Convert directly to RGBA
  frame->ConvertToRgbBuffer(cricket::FOURCC_ARGB,
                            image_.get(),
                            size,
                            width_ * 4);
  // Convert the B,G,R,A frame to R,G,B,A, which is accepted by GTK.
  // The 'A' is just padding for GTK, so we can use it as temp.
/*  uint8* pix = image_.get();
  uint8* end = image_.get() + size;
  while (pix < end) {
    pix[3] = pix[0];     // Save B to A.
    pix[0] = pix[2];  // Set Red.
    pix[2] = pix[3];  // Set Blue.
    pix[3] = 0xFF;     // Fixed Alpha.
    pix += 4;
  }*/

  QMetaObject::invokeMethod(main_wnd_, "repaint", Qt::QueuedConnection);
}
