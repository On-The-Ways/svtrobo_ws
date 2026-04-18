#!/usr/bin/env python3
"""SVTROBO Web Control Server - aiohttp backend for camera MJPEG streaming + static files."""

import argparse
import asyncio
import json
import logging
import os
import queue
import signal
import subprocess
import sys
import threading
import time
import traceback
from datetime import datetime
from pathlib import Path

import cv2
from aiohttp import web, WSMsgType

try:
    import aiohttp
except ImportError:
    aiohttp = None

logging.basicConfig(level=logging.INFO, format='%(asctime)s [%(levelname)s] %(message)s')
logger = logging.getLogger(__name__)

BASE_DIR = Path(__file__).parent
STATIC_DIR = BASE_DIR / 'static'

# --- Camera Configuration ---
CAMERA_CONFIG = {
    'd405_1': {'type': 'realsense', 'serial': '409122272399', 'size': (640, 480), 'fps': 15, 'depth': True},
    'd405_2': {'type': 'realsense', 'serial': '409122273344', 'size': (640, 480), 'fps': 15, 'depth': True},
    'zed':    {'type': 'zed',       'serial': None,           'size': None,       'fps': 15, 'depth': True},
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
                        color_only=not cfg.get('depth', False),
                    )
                else:
                    cam = ZEDCamera(
                        fps=cfg['fps'],
                        color_only=not cfg.get('depth', False),
                    )

                cam.start()

                stop_event = threading.Event()
                frame_queue = queue.Queue(maxsize=2)

                t = threading.Thread(
                    target=self._capture_loop,
                    args=(name, cam, frame_queue, stop_event, depth_queue),
                    daemon=True,
                )
                t.start()

                depth_queue = queue.Queue(maxsize=2) if cfg.get('depth') else None

                self.cameras[name] = {
                    'instance': cam,
                    'thread': t,
                    'stop_event': stop_event,
                    'frame_queue': frame_queue,
                    'depth_queue': depth_queue,
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
            # Clear IMU cache when ZED stops
            if name == 'zed':
                with imu_cache['lock']:
                    imu_cache['data'] = None
            logger.info(f"Camera {name} stopped")
            return True, f"Camera {name} stopped"

    def get_status(self):
        """Return status of all cameras."""
        return {
            name: {'running': name in self.cameras and self.cameras[name]['running']}
            for name in CAMERA_CONFIG
        }

    def get_frame(self, name):
        """Get the latest JPEG frame and timestamp for a camera (non-blocking).

        Returns:
            (jpeg_bytes, timestamp_us) or None
        """
        if name not in self.cameras or not self.cameras[name]['running']:
            return None
        try:
            return self.cameras[name]['frame_queue'].get_nowait()
        except queue.Empty:
            return None

    def get_depth_frame(self, name):
        """Get the latest depth JPEG frame and timestamp for a camera (non-blocking).

        Returns:
            (jpeg_bytes, timestamp_us) or None
        """
        if name not in self.cameras or not self.cameras[name]['running']:
            return None
        dq = self.cameras[name].get('depth_queue')
        if dq is None:
            return None
        try:
            return dq.get_nowait()
        except queue.Empty:
            return None

    @staticmethod
    def _capture_loop(name, cam, frame_queue, stop_event, depth_queue=None):
        """Background thread: continuously capture frames and encode as JPEG.
        
        If depth_queue is provided, depth frames are also encoded as colored JPEG
        and placed in depth_queue alongside the color frame.
        """
        # Depth range for visualization (mm)
        DEPTH_MAX_ZED = 20000.0     # ZED: 20m
        DEPTH_MAX_D405 = 1000.0     # D405: 1m (short range depth sensor)

        while not stop_event.is_set():
            try:
                result = cam.capture()
                if result is None:
                    continue
                color = result[0]  # (color, depth) or (left, depth)
                if color is None:
                    continue

                depth_raw = result[1]  # float32 mm (ZED) or uint16 (D405) or None

                _, jpeg = cv2.imencode('.jpg', color, [cv2.IMWRITE_JPEG_QUALITY, JPEG_QUALITY])
                frame_bytes = jpeg.tobytes()
                timestamp_us = int(time.time() * 1_000_000)

                # Drop old frame if queue is full
                try:
                    frame_queue.get_nowait()
                except queue.Empty:
                    pass
                frame_queue.put((frame_bytes, timestamp_us))

                # Encode depth frame as colored JPEG
                if depth_queue is not None and depth_raw is not None:
                    try:
                        depth_max = DEPTH_MAX_ZED if name == 'zed' else DEPTH_MAX_D405
                        if depth_raw.dtype == np.float32:
                            # ZED SDK returns float32 in mm
                            depth_vis = np.clip(depth_raw / depth_max, 0, 1)
                            depth_u8 = (depth_vis * 255).astype(np.uint8)
                        else:
                            # RealSense returns uint16, scale depends on device
                            depth_vis = np.clip(depth_raw.astype(np.float32) / depth_max, 0, 1)
                            depth_u8 = (depth_vis * 255).astype(np.uint8)
                        depth_colored = cv2.applyColorMap(depth_u8, cv2.COLORMAP_JET)
                        _, depth_jpeg = cv2.imencode('.jpg', depth_colored, [cv2.IMWRITE_JPEG_QUALITY, JPEG_QUALITY])
                        depth_bytes = depth_jpeg.tobytes()
                        try:
                            depth_queue.get_nowait()
                        except queue.Empty:
                            pass
                        depth_queue.put((depth_bytes, timestamp_us))
                    except Exception as e:
                        logger.debug(f"Camera {name} depth encode error: {e}")

                # Extract IMU data from ZED camera (if SDK mode)
                if name == 'zed' and hasattr(cam, 'get_imu_data'):
                    try:
                        imu_data = cam.get_imu_data()
                        if imu_data:
                            with imu_cache['lock']:
                                imu_cache['data'] = imu_data
                    except Exception:
                        pass

            except Exception as e:
                logger.warning(f"Camera {name} capture error: {e}")
                stop_event.wait(0.1)

        logger.info(f"Camera {name} capture thread exiting")

    def stop_all(self):
        """Stop all running cameras."""
        for name in list(self.cameras.keys()):
            self.stop_camera(name)


# --- Recording Manager ---


RECORDING_DIR = Path('/home/svt/svtrobo_ws/recordings')

# Topics to record via ros2 bag
RECORD_TOPICS = [
    '/svtrobot_cmd',
    '/lift_control_cmd',
    '/chassis/joint_states',
    '/chassis/diagnostics',
    '/f710/joy',
    '/zed/imu/data',
    '/zed/imu/mag',
    '/zed/imu/temperature',
]

# After recording stops, convert .db3 to JSONL and delete the original db file
DELETE_DB_AFTER_CONVERT = False


class RecordingManager:
    """Manages data collection: ros2 bag subprocess + periodic camera frame saving."""

    def __init__(self, camera_mgr: CameraManager):
        self.camera_mgr = camera_mgr
        self.running = False
        self.bag_process = None
        self.save_thread = None
        self.stop_event = threading.Event()
        self.output_dir = None
        self.start_time = None
        self._lock = threading.Lock()
        # Track cameras started by recording (so we only stop those we started)
        self._cameras_started = set()

    def start(self):
        """Start data recording. Returns (ok, message, path)."""
        with self._lock:
            if self.running:
                return False, 'Already recording', None

            timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
            self.output_dir = RECORDING_DIR / timestamp
            self.output_dir.mkdir(parents=True, exist_ok=True)

            # Start ros2 bag record
            bag_dir = self.output_dir / 'rosbag'
            try:
                cmd = (
                    'source /opt/ros/humble/setup.bash && '
                    'source /home/svt/svtrobo_ws/install/setup.bash && '
                    'exec ros2 bag record ' + ' '.join(RECORD_TOPICS) + f' -o {bag_dir}'
                )
                self.bag_process = subprocess.Popen(
                    ['bash', '-c', cmd],
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                )
                logger.info(f"ros2 bag record started, saving to {bag_dir}")
            except FileNotFoundError:
                return False, 'ros2 command not found. Is ROS2 sourced?', None
            except Exception as e:
                return False, str(e), None

            # Auto-start all cameras (skip those already running)
            self._cameras_started = set()
            cam_start_errors = []
            for cam_name in CAMERA_CONFIG:
                status = self.camera_mgr.get_status().get(cam_name, {})
                if not status.get('running', False):
                    ok, msg = self.camera_mgr.start_camera(cam_name)
                    if ok:
                        self._cameras_started.add(cam_name)
                        logger.info(f"Recording auto-started camera: {cam_name}")
                    else:
                        cam_start_errors.append(f"{cam_name}: {msg}")
                        logger.warning(f"Recording failed to start camera {cam_name}: {msg}")

            # Start camera frame saver
            self.stop_event.clear()
            self.save_thread = threading.Thread(
                target=self._save_camera_frames_loop,
                daemon=True,
            )
            self.save_thread.start()

            self.running = True
            self.start_time = time.time()
            return True, 'Recording started', str(self.output_dir)

    def stop(self):
        """Stop recording. Returns (ok, message, info_dict)."""
        with self._lock:
            if not self.running:
                return False, 'Not recording', {}

            # Stop ros2 bag
            if self.bag_process and self.bag_process.poll() is None:
                self.bag_process.terminate()
                try:
                    self.bag_process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    self.bag_process.kill()
                logger.info("ros2 bag record stopped")

            # Stop camera frame saver
            self.stop_event.set()
            if self.save_thread:
                self.save_thread.join(timeout=5)

            # Stop cameras that were auto-started by recording
            for cam_name in list(self._cameras_started):
                try:
                    self.camera_mgr.stop_camera(cam_name)
                    logger.info(f"Recording auto-stopped camera: {cam_name}")
                except Exception as e:
                    logger.warning(f"Failed to stop camera {cam_name}: {e}")
            self._cameras_started.clear()

            duration = time.time() - self.start_time if self.start_time else 0
            info = {
                'path': str(self.output_dir),
                'duration': round(duration, 1),
            }
            self.running = False
            self.start_time = None

            # Convert .db3 to JSONL in background
            if self.output_dir:
                bag_dir = self.output_dir / 'rosbag'
                if bag_dir.exists():
                    self._convert_bag(bag_dir, self.output_dir)

            return True, 'Recording stopped', info

    def _convert_bag(self, bag_dir, output_dir):
        """Run bag-to-JSONL conversion in a background thread."""
        def _do():
            try:
                script = Path(__file__).parent / 'bag_converter.py'
                cmd = [sys.executable, str(script), str(bag_dir), str(output_dir)]
                if DELETE_DB_AFTER_CONVERT:
                    cmd.append('--delete-db')
                result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
                if result.returncode == 0:
                    logger.info(f"Bag→JSONL: {result.stdout.strip()}")
                else:
                    logger.warning(f"Bag→JSONL failed: {result.stderr.strip()}")
            except Exception as e:
                logger.warning(f"Bag→JSONL error: {e}")

        threading.Thread(target=_do, daemon=True, name='bag-converter').start()

    def get_status(self):
        """Return current recording status."""
        elapsed = 0
        if self.running and self.start_time:
            elapsed = time.time() - self.start_time
        return {
            'running': self.running,
            'path': str(self.output_dir) if self.output_dir else None,
            'elapsed': round(elapsed, 1),
        }

    def _save_camera_frames_loop(self):
        """Periodically save camera frames (color + depth) from CameraManager queues."""
        cam_names = ['d405_1', 'd405_2', 'zed']

        while not self.stop_event.is_set():
            for name in cam_names:
                result = self.camera_mgr.get_frame(name)
                if result:
                    frame, timestamp_us = result
                    img_dir = self.output_dir / 'images' / name
                    img_dir.mkdir(parents=True, exist_ok=True)
                    path = img_dir / f'{timestamp_us}.jpg'
                    try:
                        path.write_bytes(frame)
                    except Exception as e:
                        logger.warning(f"Failed to save {name} frame: {e}")

                # Save depth frame if available
                depth_result = self.camera_mgr.get_depth_frame(name)
                if depth_result:
                    depth_frame, d_timestamp_us = depth_result
                    depth_dir = self.output_dir / 'depth' / name
                    depth_dir.mkdir(parents=True, exist_ok=True)
                    depth_path = depth_dir / f'{d_timestamp_us}.jpg'
                    try:
                        depth_path.write_bytes(depth_frame)
                    except Exception as e:
                        logger.warning(f"Failed to save {name} depth: {e}")

            self.stop_event.wait(0.1)  # 10 Hz

        logger.info("Camera frame saver exiting")


# --- F710 Manager ---


class F710Manager:
    """Manages the f710_teleop node lifecycle."""

    def __init__(self):
        self.process = None
        self._lock = threading.Lock()

    def start(self):
        """Start the f710_teleop node. Returns (ok, message)."""
        with self._lock:
            if self.process and self.process.poll() is None:
                return False, 'F710 node already running'

            try:
                cmd = (
                    'source /opt/ros/humble/setup.bash && '
                    'source /home/svt/svtrobo_ws/install/setup.bash && '
                    'exec ros2 launch f710_teleop f710_teleop.launch.py'
                )
                self.process = subprocess.Popen(
                    ['bash', '-c', cmd],
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                    preexec_fn=os.setpgrp,
                )
                logger.info(f"F710 node started (pid={self.process.pid})")
                return True, 'F710 node started'
            except FileNotFoundError:
                return False, 'ros2 command not found. Is ROS2 sourced?'
            except Exception as e:
                return False, str(e)

    def stop(self):
        """Stop the f710_teleop node. Returns (ok, message)."""
        with self._lock:
            if not self.process or self.process.poll() is not None:
                return False, 'F710 node not running'

            try:
                os.killpg(os.getpgid(self.process.pid), signal.SIGTERM)
                self.process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                os.killpg(os.getpgid(self.process.pid), signal.SIGKILL)
            except Exception:
                pass
            self.process = None
            logger.info("F710 node stopped")
            return True, 'F710 node stopped'

    def get_status(self):
        """Return whether the f710 node process is alive."""
        alive = self.process is not None and self.process.poll() is None
        return {'running': alive}


# --- HTTP Handlers ---

camera_mgr = CameraManager()
recording_mgr = RecordingManager(camera_mgr)
f710_mgr = F710Manager()
imu_cache = {'data': None, 'lock': threading.Lock()}


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
            result = camera_mgr.get_frame(name)
            if result is not None:
                frame, _ = result
                msg = boundary + header + frame + b'\r\n'
                await response.write(msg)
            await asyncio.sleep(1.0 / STREAM_FPS)
    except (ConnectionResetError, ConnectionError):
        pass
    return response


async def camera_depth_stream_handler(request):
    """Stream depth frames as MJPEG for a camera."""
    name = request.match_info['name']
    if name not in CAMERA_CONFIG:
        return web.Response(status=404, text="Unknown camera")

    status = camera_mgr.get_status()
    if not status.get(name, {}).get('running'):
        return web.Response(status=404, text="Camera not active")

    response = web.StreamResponse()
    response.content_type = 'multipart/x-mixed-replace; boundary=frame'
    response.headers['Cache-Control'] = 'no-cache'
    await response.prepare(request)

    boundary = b'--frame\r\n'
    header = b'Content-Type: image/jpeg\r\n\r\n'

    try:
        while True:
            result = camera_mgr.get_depth_frame(name)
            if result is not None:
                frame, _ = result
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


async def recording_start_handler(request):
    ok, msg, path = recording_mgr.start()
    return web.json_response({'ok': ok, 'message': msg, 'path': path})


async def recording_stop_handler(request):
    ok, msg, info = recording_mgr.stop()
    resp = {'ok': ok, 'message': msg}
    resp.update(info)
    return web.json_response(resp)


async def recording_status_handler(request):
    return web.json_response(recording_mgr.get_status())


async def f710_start_handler(request):
    ok, msg = f710_mgr.start()
    return web.json_response({'ok': ok, 'message': msg})


async def f710_stop_handler(request):
    ok, msg = f710_mgr.stop()
    return web.json_response({'ok': ok, 'message': msg})


async def f710_status_handler(request):
    return web.json_response(f710_mgr.get_status())


async def imu_data_handler(request):
    """Return latest IMU data from ZED camera."""
    with imu_cache['lock']:
        data = imu_cache['data']
    if data is None:
        return web.json_response({'ok': False, 'message': 'IMU data not available. Start ZED camera first.'})
    return web.json_response({'ok': True, 'data': data})


async def on_shutdown(app):
    f710_mgr.stop()
    recording_mgr.stop()
    camera_mgr.stop_all()
    # Close all ws proxy connections
    for ws in app.get('ws_proxies', set()):
        await ws.close()


async def rosbridge_proxy_handler(request):
    """Proxy WebSocket connections to rosbridge_server on localhost:9090."""
    ws_client = web.WebSocketResponse()
    await ws_client.prepare(request)

    # Connect to rosbridge
    rosbridge_url = 'http://localhost:9090'
    try:
        async with aiohttp.ClientSession() as session:
            async with session.ws_connect(rosbridge_url) as ws_ros:
                # Store for cleanup on shutdown
                ws_proxies = request.app.setdefault('ws_proxies', set())
                ws_proxies.add(ws_client)

                async def forward_to_ros():
                    """Forward messages from browser → rosbridge."""
                    try:
                        async for msg in ws_client:
                            if msg.type == WSMsgType.TEXT:
                                await ws_ros.send_str(msg.data)
                            elif msg.type == WSMsgType.BINARY:
                                await ws_ros.send_bytes(msg.data)
                            elif msg.type == WSMsgType.ERROR:
                                break
                    except Exception:
                        pass

                async def forward_to_client():
                    """Forward messages from rosbridge → browser."""
                    try:
                        async for msg in ws_ros:
                            if msg.type == WSMsgType.TEXT:
                                await ws_client.send_str(msg.data)
                            elif msg.type == WSMsgType.BINARY:
                                await ws_client.send_bytes(msg.data)
                            elif msg.type == WSMsgType.ERROR:
                                break
                    except Exception:
                        pass

                # Run both directions concurrently
                await asyncio.gather(
                    forward_to_ros(),
                    forward_to_client(),
                    return_exceptions=True,
                )

                ws_proxies.discard(ws_client)
    except Exception as e:
        logger.warning(f"rosbridge proxy connection failed: {e}")
        if not ws_client.closed:
            await ws_client.close()

    return ws_client


def create_app():
    app = web.Application()
    app.on_shutdown.append(on_shutdown)

    # Routes
    app.router.add_get('/', index_handler)
    app.router.add_static('/static', STATIC_DIR, name='static')
    app.router.add_get('/camera/{name}', camera_stream_handler)
    app.router.add_get('/camera/{name}/depth', camera_depth_stream_handler)
    app.router.add_post('/camera/start', camera_start_handler)
    app.router.add_post('/camera/stop', camera_stop_handler)
    app.router.add_get('/camera/status', camera_status_handler)
    app.router.add_post('/recording/start', recording_start_handler)
    app.router.add_post('/recording/stop', recording_stop_handler)
    app.router.add_get('/recording/status', recording_status_handler)
    app.router.add_post('/f710/start', f710_start_handler)
    app.router.add_post('/f710/stop', f710_stop_handler)
    app.router.add_get('/f710/status', f710_status_handler)
    app.router.add_get('/api/imu', imu_data_handler)
    app.router.add_get('/ws', rosbridge_proxy_handler)

    return app


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='SVTROBO Web Control Server')
    parser.add_argument('--host', default='0.0.0.0', help='Host to bind (default: 0.0.0.0)')
    parser.add_argument('--port', type=int, default=8080, help='Port to bind (default: 8080)')
    args = parser.parse_args()

    logger.info(f"Starting server at http://{args.host}:{args.port}")
    web.run_app(create_app(), host=args.host, port=args.port, print=logger.info)