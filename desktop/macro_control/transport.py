from __future__ import annotations

import threading
import time
from dataclasses import dataclass
from typing import Protocol


class TransportError(RuntimeError):
    """Raised when serial communication fails."""


class Transport(Protocol):
    def connect(self, port: str | None = None, baudrate: int = 115200, timeout: float = 1.0) -> None:
        ...

    def disconnect(self) -> None:
        ...

    def send_line(self, line: str, timeout: float = 1.0) -> str:
        ...

    def list_ports(self) -> list[str]:
        ...

    @property
    def is_connected(self) -> bool:
        ...


class SerialTransport:
    def __init__(self) -> None:
        self._serial = None
        self._lock = threading.Lock()

    @property
    def is_connected(self) -> bool:
        return bool(self._serial and self._serial.is_open)

    def list_ports(self) -> list[str]:
        try:
            from serial.tools import list_ports
        except ImportError as exc:
            raise TransportError("PySerial kurulu değil. requirements.txt içindeki bağımlılıkları kurun.") from exc

        ports = []
        for port in list_ports.comports():
            label = f"{port.device}"
            description = (port.description or "").lower()
            hwid = (port.hwid or "").lower()
            if "arduino" in description or "leonardo" in description or "2341" in hwid or "8036" in hwid:
                ports.insert(0, label)
            elif label not in ports:
                ports.append(label)
        return ports

    def connect(self, port: str | None = None, baudrate: int = 115200, timeout: float = 1.0) -> None:
        try:
            import serial
        except ImportError as exc:
            raise TransportError("PySerial kurulu değil. requirements.txt içindeki bağımlılıkları kurun.") from exc

        selected_port = port or self._auto_select_port()
        if not selected_port:
            raise TransportError("Uygun COM port bulunamadı.")

        try:
            self._serial = serial.Serial(selected_port, baudrate=baudrate, timeout=timeout, write_timeout=timeout)
            time.sleep(2.0)
            self._serial.reset_input_buffer()
            self._serial.reset_output_buffer()
        except serial.SerialException as exc:
            self._serial = None
            raise TransportError(f"Seri porta bağlanılamadı: {exc}") from exc

    def disconnect(self) -> None:
        with self._lock:
            if self._serial and self._serial.is_open:
                self._serial.close()
            self._serial = None

    def send_line(self, line: str, timeout: float = 1.0) -> str:
        with self._lock:
            if not self.is_connected:
                raise TransportError("Seri port bağlı değil.")
            payload = f"{line}\n".encode("ascii")
            self._serial.reset_input_buffer()
            self._serial.write(payload)
            self._serial.flush()
            old_timeout = self._serial.timeout
            self._serial.timeout = timeout
            try:
                response = self._serial.readline().decode("ascii", errors="replace").strip()
            finally:
                self._serial.timeout = old_timeout
            if not response:
                raise TransportError("Arduino yanıt vermedi.")
            return response

    def _auto_select_port(self) -> str | None:
        ports = self.list_ports()
        return ports[0] if ports else None


@dataclass
class FakeRecord:
    line: str
    response: str


class FakeTransport:
    def __init__(self) -> None:
        self.records: list[FakeRecord] = []
        self._connected = False
        self._enabled = False

    @property
    def is_connected(self) -> bool:
        return self._connected

    def list_ports(self) -> list[str]:
        return ["FAKE-ARDUINO"]

    def connect(self, port: str | None = None, baudrate: int = 115200, timeout: float = 1.0) -> None:
        self._connected = True

    def disconnect(self) -> None:
        self._enabled = False
        self._connected = False

    def send_line(self, line: str, timeout: float = 1.0) -> str:
        if not self._connected:
            raise TransportError("Sahte Arduino bağlı değil.")

        if line == "HELLO":
            response = "OK HELLO LEONARDO_HID_SAFE 1"
        elif line == "PING":
            response = "OK PONG"
        elif line == "ENABLE 1":
            self._enabled = True
            response = "OK ENABLED"
        elif line == "ENABLE 0":
            self._enabled = False
            response = "OK STOPPED DISABLED"
        elif line == "STOP":
            self._enabled = False
            response = "OK STOPPED APP_STOP"
        elif line == "RELEASE":
            response = "OK RELEASED"
        elif line.startswith("KP "):
            response = "OK KEY_PRESSED" if self._enabled else "ERR HID_NOT_ENABLED_OR_HEARTBEAT_TIMEOUT"
        else:
            response = "ERR UNKNOWN_COMMAND"

        self.records.append(FakeRecord(line=line, response=response))
        return response
