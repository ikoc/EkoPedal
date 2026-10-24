from __future__ import annotations

import queue
import threading
import tkinter as tk
from tkinter import messagebox, ttk

from . import protocol
from .controller import MacroController
from .transport import TransportError


class MacroApp:
    def __init__(self, controller: MacroController) -> None:
        self.controller = controller
        self.controller.on_status = self._post_status
        self.status_queue: queue.Queue[str] = queue.Queue()

        self.root = tk.Tk()
        self.root.title("Arduino Leonardo HID Makro Kontrol")
        self.root.geometry("620x420")
        self.root.minsize(560, 380)

        self.port_var = tk.StringVar()
        self.status_var = tk.StringVar(value="Bağlantı yok.")
        self.key_var = tk.StringVar(value="1")
        self.hold_var = tk.StringVar(value="50")
        self.hid_var = tk.StringVar(value="HID pasif")
        self._pending_key_after: str | None = None
        self._countdown_remaining = 0

        self._build_ui()
        self._refresh_ports()
        self._pump_status()
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)

    def run(self) -> None:
        self.root.mainloop()

    def _build_ui(self) -> None:
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)

        main = ttk.Frame(self.root, padding=18)
        main.grid(row=0, column=0, sticky="nsew")
        main.columnconfigure(0, weight=1)
        main.columnconfigure(1, weight=1)

        title = ttk.Label(main, text="Makro Kontrol Paneli", font=("Segoe UI", 18, "bold"))
        title.grid(row=0, column=0, columnspan=2, sticky="w", pady=(0, 12))

        connection = ttk.LabelFrame(main, text="Bağlantı", padding=12)
        connection.grid(row=1, column=0, columnspan=2, sticky="ew", pady=(0, 12))
        connection.columnconfigure(1, weight=1)

        ttk.Label(connection, text="COM port").grid(row=0, column=0, sticky="w")
        self.port_combo = ttk.Combobox(connection, textvariable=self.port_var, state="normal")
        self.port_combo.grid(row=0, column=1, sticky="ew", padx=8)
        ttk.Button(connection, text="Yenile", command=self._refresh_ports).grid(row=0, column=2, padx=(0, 8))
        ttk.Button(connection, text="Bağlan", command=self._connect_async).grid(row=0, column=3)
        ttk.Button(connection, text="Kes", command=self._disconnect_async).grid(row=0, column=4, padx=(8, 0))

        safety = ttk.LabelFrame(main, text="Güvenlik", padding=12)
        safety.grid(row=2, column=0, columnspan=2, sticky="ew", pady=(0, 12))
        safety.columnconfigure(1, weight=1)

        self.hid_label = ttk.Label(safety, textvariable=self.hid_var, font=("Segoe UI", 11, "bold"))
        self.hid_label.grid(row=0, column=0, sticky="w")
        ttk.Button(safety, text="HID Etkinleştir", command=self._enable_async).grid(row=0, column=1, sticky="e")
        ttk.Button(safety, text="HID Kapat", command=self._disable_async).grid(row=0, column=2, padx=(8, 0))
        stop_button = ttk.Button(safety, text="DURDUR", command=self._stop_async)
        stop_button.grid(row=0, column=3, padx=(8, 0))

        test = ttk.LabelFrame(main, text="Güvenli Tek Tuş Testi", padding=12)
        test.grid(row=3, column=0, columnspan=2, sticky="ew", pady=(0, 12))
        test.columnconfigure(1, weight=1)

        ttk.Label(test, text="Tuş").grid(row=0, column=0, sticky="w")
        ttk.Entry(test, textvariable=self.key_var, width=16).grid(row=0, column=1, sticky="w", padx=8)
        ttk.Label(test, text="Basılı süre (ms)").grid(row=0, column=2, sticky="w")
        ttk.Entry(test, textvariable=self.hold_var, width=10).grid(row=0, column=3, sticky="w", padx=8)
        self.send_button = ttk.Button(test, text="Tek Tuş Gönder", command=self._press_key_async)
        self.send_button.grid(row=0, column=4, sticky="e")

        help_text = (
            "İlk denemeyi Not Defteri'nde yapın. Arduino açılışta otomatik tuş basmaz; "
            "önce bağlantı ve HID etkinleştirme gerekir."
        )
        ttk.Label(main, text=help_text, wraplength=560).grid(row=4, column=0, columnspan=2, sticky="ew", pady=(4, 12))

        status = ttk.LabelFrame(main, text="Durum", padding=12)
        status.grid(row=5, column=0, columnspan=2, sticky="nsew")
        status.columnconfigure(0, weight=1)
        ttk.Label(status, textvariable=self.status_var, wraplength=560).grid(row=0, column=0, sticky="ew")
        main.rowconfigure(5, weight=1)

    def _run_worker(self, action) -> None:
        def worker() -> None:
            try:
                result = action()
                if result:
                    self._post_status(result)
            except Exception as exc:
                self._post_status(str(exc))
                self.root.after(0, lambda: messagebox.showerror("Hata", str(exc)))
            finally:
                self.root.after(0, self._sync_state)

        threading.Thread(target=worker, daemon=True).start()

    def _connect_async(self) -> None:
        port = self.port_var.get().strip() or None
        self._run_worker(lambda: self.controller.connect(port))

    def _disconnect_async(self) -> None:
        self._cancel_pending_key()
        self._run_worker(lambda: self.controller.disconnect())

    def _enable_async(self) -> None:
        self._run_worker(lambda: self.controller.enable_hid())

    def _disable_async(self) -> None:
        self._cancel_pending_key()
        self._run_worker(lambda: self.controller.disable_hid())

    def _stop_async(self) -> None:
        self._cancel_pending_key()
        self._run_worker(lambda: self.controller.stop())

    def _press_key_async(self) -> None:
        if not self.controller.hid_enabled:
            self._post_status("Önce HID'i etkinleştirin.")
            return
        key = self.key_var.get()
        hold = self.hold_var.get()
        try:
            protocol.key_press(key, hold)
        except (ValueError, protocol.ProtocolError) as exc:
            self._post_status(str(exc))
            return
        self.send_button.state(["disabled"])
        self._countdown_remaining = 5
        self._countdown_tick(key, hold)

    def _countdown_tick(self, key: str, hold: str) -> None:
        if self._countdown_remaining == 0:
            self._pending_key_after = None
            self.send_button.state(["!disabled"])
            self._run_worker(lambda: self.controller.press_key(key, hold))
            return
        self._post_status(f"{self._countdown_remaining} saniye içinde Not Defteri'ne geçin. İptal: DURDUR.")
        self._countdown_remaining -= 1
        self._pending_key_after = self.root.after(1000, self._countdown_tick, key, hold)

    def _cancel_pending_key(self) -> None:
        if self._pending_key_after is not None:
            self.root.after_cancel(self._pending_key_after)
            self._pending_key_after = None
            self.send_button.state(["!disabled"])
            self._post_status("Bekleyen tek tuş testi iptal edildi.")

    def _refresh_ports(self) -> None:
        try:
            ports = self.controller.list_ports()
        except TransportError as exc:
            ports = []
            self._post_status(str(exc))
        self.port_combo["values"] = ports
        if ports and not self.port_var.get():
            self.port_var.set(ports[0])

    def _post_status(self, message: str) -> None:
        self.status_queue.put(message)

    def _pump_status(self) -> None:
        try:
            while True:
                self.status_var.set(self.status_queue.get_nowait())
        except queue.Empty:
            pass
        self.root.after(150, self._pump_status)

    def _sync_state(self) -> None:
        self.hid_var.set("HID aktif" if self.controller.hid_enabled else "HID pasif")

    def _on_close(self) -> None:
        self._cancel_pending_key()
        try:
            self.controller.disconnect()
        except Exception:
            pass
        self.root.destroy()
