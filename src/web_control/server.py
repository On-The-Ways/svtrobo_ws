#!/usr/bin/env python3
"""SVTROBO Web Control Server - aiohttp backend for camera MJPEG streaming + static files."""

import argparse
import asyncio
import json
import logging
import queue
import sys
import threading
import traceback
from pathlib import Path

import cv2
from aiohttp import web

logging.basicConfig(level=logging.INFO, format='%(asctime)s [%(levelname)s] %(message)s')
logger = logging.getLogger(__name__)

BASE_DIR = Path(__file__).parent
STATIC_DIR = BASE_DIR / 'static'

# --- Camera Configuration ---
CAMERA_CONFIG = {
    'd405_1': {'type': 'realsense', 'serial': '409122272399', 'size': (640, 480), 'fps': 15},
    'd405_2': {'type': 'realsense', 'serial': '409122273344', 'size': (640, 480), 'fps': 15},
    'zed':    {'type': 'zed',       'serial': None,           'size': None,       'fps': 15},
}

JPEG_QUALITY = 70
STREAM_FPS = 15


class CameraManager:
    """Manages camera instances, capture threads, and frame queues."""

    def __init__(self):
        self.cameras = {}  # name -> {instance, thread, stop_event, frame_queue, running}
        self._lock = threading.Lock()

    def start_camera(self, name):
        """Start a camera capture thread."""
        if name not in CAMERA_CONFIG:
            return False, f"Unknown camera: {name}"

        with self._lock:
            if name in self.cameras and self.cameras[name]['running']:
                return False, f"Camera {name} already running"

            cfg = CAMERA_CONFIG[name]
            try:
                # Import camera drivers
                sys.path.insert(0, str(Path(__file__).parent.parent / 'camera_driver'))
                from camera_driver import RealSenseCamera, ZEDCamera

                if cfg['type'] == 'realsense':
                    cam = RealSenseCamera(
                        serial=cfg['serial'],
                        color_size=cfg['size'],
                        depth_size=cfg['size'],
                        fps=cfg['fps'],
                    )
                else:
                    cam = ZEDCamera(fps=cfg['fps'])

                cam.start()

                stop_event = threading.Event()
                frame_queue = queue.Queue(maxsize=2)

                t = threading.Thread(
                    target=self._capture_loop,
                    args=(name, cam, frame_queue, stop_event),
                    daemon=True,
                )
                t.start()

                self.cameras[name] = {
                    'instance': cam,
                    'thread': t,
                    'stop_event': stop_event,
                    'frame_queue': frame_queue,
                    'running': True,
                }
                logger.info(f"Camera {name} started")
                return True, f"Camera {name} started"

            except Exception as e:
                logger.error(f"Failed to start camera {name}: {e}")
                traceback.print_exc()
                return False, str(e)

    def stop_camera(self, name):
        """Stop a camera and free resources."""
        with self._lock:
            if name not in self.cameras:
                return False, f"Camera {name} not found"

            info = self.cameras[name]
            if not info['running']:
                return False, f"Camera {name} not running"

            info['stop_event'].set()
            info['thread'].join(timeout=5.0)
            try:
                info['instance'].stop()
            except Exception:
                pass
            info['running'] = False
            del self.cameras[name]
            logger.info(f"Camera {name} stopped")
            return True, f"Camera {name} stopped"

    def get_status(self):
        """Return status of all cameras."""
        return {
            name: {'running': name in self.cameras and self.cameras[name]['running']}
            for name in CAMERA_CONFIG
        }

    def get_frame(self, name):
        """Get the latest JPEG frame for a camera (non-blocking)."""
        if name not in self.cameras or not self.cameras[name]['running']:
            return None
        try:
            return self.cameras[name]['frame_queue'].get_nowait()
        except queue.Empty:
            return None

    @staticmethod
    def _capture_loop(name, cam, frame_queue, stop_event):
        """Background thread: continuously capture frames and encode as JPEG."""
        while not stop_event.is_set():
            try:
                result = cam.capture()
                if result is None:
                    continue
                color = result[0]  # (color, depth) or (left, depth)
                if color is None:
                    continue

                _, jpeg = cv2.imencode('.jpg', color, [cv2.IMWRITE_JPEG_QUALITY, JPEG_QUALITY])
                frame_bytes = jpeg.tobytes()

                # Drop old frame if queue is full
                try:
                    frame_queue.get_nowait()
                except queue.Empty:
                    pass
                frame_queue.put(frame_bytes)

            except Exception as e:
                logger.warning(f"Camera {name} capture error: {e}")
                stop_event.wait(0.1)

        logger.info(f"Camera {name} capture thread exiting")

    def stop_all(self):
        """Stop all running cameras."""
        for name in list(self.cameras.keys()):
            self.stop_camera(name)


# --- HTTP Handlers ---

camera_mgr = CameraManager()


async def index_handler(request):
    return web.FileResponse(STATIC_DIR / 'index.html')


async def camera_stream_handler(request):
    name = request.match_info['name']
    if name not in CAMERA_CONFIG:
        return web.Response(status=404, text="Unknown camera")

    status = camera_mgr.get_status()
    if not status.get(name, {}).get('running'):
        return web.Response(status=404, text="Camera not active. Start it first via POST /camera/start")

    response = web.StreamResponse()
    response.content_type = 'multipart/x-mixed-replace; boundary=frame'
    response.headers['Cache-Control'] = 'no-cache'
    await response.prepare(request)

    boundary = b'--frame\r\n'
    header = b'Content-Type: image/jpeg\r\n\r\n'

    try:
        while True:
            frame = camera_mgr.get_frame(name)
            if frame is not None:
                msg = boundary + header + frame + b'\r\n'
                await response.write(msg)
            await asyncio.sleep(1.0 / STREAM_FPS)
    except (ConnectionResetError, ConnectionError):
        pass
    return response


async def camera_start_handler(request):
    data = await request.json()
    name = data.get('camera', '')
    ok, msg = camera_mgr.start_camera(name)
    return web.json_response({'ok': ok, 'message': msg})


async def camera_stop_handler(request):
    data = await request.json()
    name = data.get('camera', '')
    ok, msg = camera_mgr.stop_camera(name)
    return web.json_response({'ok': ok, 'message': msg})


async def camera_status_handler(request):
    return web.json_response(camera_mgr.get_status())


async def on_shutdown(app):
    camera_mgr.stop_all()


def create_app():
    app = web.Application()
    app.on_shutdown.append(on_shutdown)

    # Routes
    app.router.add_get('/', index_handler)
    app.router.add_static('/static', STATIC_DIR, name='static')
    app.router.add_get('/camera/{name}', camera_stream_handler)
    app.router.add_post('/camera/start', camera_start_handler)
    app.router.add_post('/camera/stop', camera_stop_handler)
    app.router.add_get('/camera/status', camera_status_handler)

    return app


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='SVTROBO Web Control Server')
    parser.add_argument('--host', default='0.0.0.0', help='Host to bind (default: 0.0.0.0)')
    parser.add_argument('--port', type=int, default=8080, help='Port to bind (default: 8080)')
    args = parser.parse_args()

    logger.info(f"Starting server at http://{args.host}:{args.port}")
    web.run_app(create_app(), host=args.host, port=args.port, print=logger.info)
