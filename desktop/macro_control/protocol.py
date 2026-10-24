from __future__ import annotations

from dataclasses import dataclass


ALLOWED_SPECIAL_KEYS = {
    "SPACE",
    "ENTER",
    "ESC",
    "TAB",
    "BACKSPACE",
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
}

MIN_HOLD_MS = 10
DEFAULT_HOLD_MS = 50
MAX_HOLD_MS = 1000


class ProtocolError(ValueError):
    """Raised when a command cannot be encoded safely."""


@dataclass(frozen=True)
class Command:
    line: str

    def encode(self) -> bytes:
        return f"{self.line}\n".encode("ascii")


def hello() -> Command:
    return Command("HELLO")


def ping() -> Command:
    return Command("PING")


def enable_hid(enabled: bool) -> Command:
    return Command(f"ENABLE {1 if enabled else 0}")


def stop() -> Command:
    return Command("STOP")


def release_all() -> Command:
    return Command("RELEASE")


def normalize_key_token(token: str) -> str:
    cleaned = token.strip()
    if not cleaned:
        raise ProtocolError("Tuş alanı boş olamaz.")

    upper = cleaned.upper()
    if upper in ALLOWED_SPECIAL_KEYS:
        return upper

    if len(cleaned) == 1 and 32 <= ord(cleaned) <= 126:
        if cleaned == " ":
            return "SPACE"
        return cleaned

    raise ProtocolError(
        "Geçersiz tuş. Tek ASCII karakter veya SPACE, ENTER, ESC, TAB, F1-F12 gibi özel tuş kullanın."
    )


def clamp_hold_ms(value: int | str | None) -> int:
    if value is None or value == "":
        return DEFAULT_HOLD_MS
    try:
        parsed = int(value)
    except (TypeError, ValueError) as exc:
        raise ProtocolError("Basılı tutma süresi sayı olmalı.") from exc

    if parsed < MIN_HOLD_MS:
        return MIN_HOLD_MS
    if parsed > MAX_HOLD_MS:
        return MAX_HOLD_MS
    return parsed


def key_press(token: str, hold_ms: int | str | None = DEFAULT_HOLD_MS) -> Command:
    safe_token = normalize_key_token(token)
    safe_hold = clamp_hold_ms(hold_ms)
    return Command(f"KP {safe_token} {safe_hold}")
