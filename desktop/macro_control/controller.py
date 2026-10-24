from __future__ import annotations

import threading
import time
from collections.abc import Callable

from . import protocol
from .transport import Transport, TransportError


StatusCallback = Callable[[str], None]


class MacroController:
    def __init__(self, transport: Transport, on_status: StatusCallback | None = None) -> None:
        self.transport = transport
        self.on_status = on_status or (lambda message: None)
        self._heartbeat_thread: threading.Thread | None = None
        self._heartbeat_stop = threading.Event()
        self._hid_enabled = False
        self._lock = threading.Lock()

    @property
    def is_connected(self) -> bool:
        return self.transport.is_connected

    @property
    def hid_enabled(self) -> bool:
        return self._hid_enabled

    def list_ports(self) -> list[str]:
        return self.transport.list_ports()

    def connect(self, port: str | None = None) -> str:
        self.transport.connect(port=port)
        try:
            response = self._send(protocol.hello())
        except Exception:
            self.transport.disconnect()
            raise
        self._start_heartbeat()
        self.on_status("Bağlandı.")
        return response

    def disconnect(self) -> None:
        try:
            self.stop()
        finally:
            self._stop_heartbeat()
            self.transport.disconnect()
            self._hid_enabled = False
            self.on_status("Bağlantı kapatıldı.")

    def enable_hid(self) -> str:
        response = self._send(protocol.enable_hid(True))
        self._hid_enabled = True
        self.on_status("HID etkin.")
        return response

    def disable_hid(self) -> str:
        response = self._send(protocol.enable_hid(False))
        self._hid_enabled = False
        self.on_status("HID devre dışı.")
        return response

    def stop(self) -> str | None:
        if not self.transport.is_connected:
            self._hid_enabled = False
            return None
        try:
            response = self._send(protocol.stop())
        except TransportError:
            self._hid_enabled = False
            raise
        self._hid_enabled = False
        self.on_status("Durduruldu; basılı tuşlar bırakıldı.")
        return response

    def press_key(self, key_token: str, hold_ms: int | str | None = None) -> str:
        if not self._hid_enabled:
            raise TransportError("HID etkin değil.")
        command = protocol.key_press(key_token, hold_ms)
        response = self._send(command)
        self.on_status(f"Tuş gönderildi: {key_token}")
        return response

    def _send(self, command: protocol.Command) -> str:
        with self._lock:
            response = self.transport.send_line(command.line)
        if response.startswith("ERR"):
            self._hid_enabled = False
            raise TransportError(response)
        expected = {
            "HELLO": "OK HELLO LEONARDO_HID_SAFE 1",
            "PING": "OK PONG",
            "ENABLE 1": "OK ENABLED",
            "ENABLE 0": "OK STOPPED DISABLED",
            "STOP": "OK STOPPED APP_STOP",
            "RELEASE": "OK RELEASED",
        }.get(command.line, "OK KEY_PRESSED" if command.line.startswith("KP ") else None)
        if response != expected:
            self._hid_enabled = False
            raise TransportError(f"Beklenmeyen Arduino yanıtı: {response}")
        return response

    def _start_heartbeat(self) -> None:
        if self._heartbeat_thread and self._heartbeat_thread.is_alive():
            return
        self._heartbeat_stop.clear()
        self._heartbeat_thread = threading.Thread(target=self._heartbeat_loop, daemon=True)
        self._heartbeat_thread.start()

    def _stop_heartbeat(self) -> None:
        self._heartbeat_stop.set()
        if self._heartbeat_thread and self._heartbeat_thread.is_alive():
            self._heartbeat_thread.join(timeout=1.5)
        self._heartbeat_thread = None

    def _heartbeat_loop(self) -> None:
        while not self._heartbeat_stop.wait(1.0):
            if not self.transport.is_connected:
                continue
            try:
                self._send(protocol.ping())
            except Exception as exc:
                self._hid_enabled = False
                self.on_status(f"Heartbeat hatası: {exc}")
                break
