from __future__ import annotations

import argparse
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from macro_control.app import MacroApp
from macro_control.controller import MacroController
from macro_control.transport import FakeTransport, SerialTransport


def main() -> int:
    parser = argparse.ArgumentParser(description="Arduino Leonardo HID Makro Kontrol Paneli")
    parser.add_argument("--fake", action="store_true", help="Donanım olmadan sahte Arduino ile çalıştır")
    args = parser.parse_args()

    transport = FakeTransport() if args.fake else SerialTransport()
    controller = MacroController(transport)
    app = MacroApp(controller)
    app.run()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
