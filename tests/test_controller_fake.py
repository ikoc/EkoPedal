import unittest

from desktop.macro_control.controller import MacroController
from desktop.macro_control.transport import FakeTransport, TransportError


class ControllerFakeTransportTest(unittest.TestCase):
    def test_connect_enable_press_stop(self):
        transport = FakeTransport()
        controller = MacroController(transport)

        self.assertIn("OK HELLO", controller.connect("FAKE-ARDUINO"))
        self.assertEqual(controller.enable_hid(), "OK ENABLED")
        self.assertEqual(controller.press_key("1", 50), "OK KEY_PRESSED")
        self.assertIn("OK STOPPED", controller.stop())

        lines = [record.line for record in transport.records]
        self.assertIn("HELLO", lines)
        self.assertIn("ENABLE 1", lines)
        self.assertIn("KP 1 50", lines)
        self.assertIn("STOP", lines)

    def test_key_requires_hid_enabled(self):
        transport = FakeTransport()
        controller = MacroController(transport)
        controller.connect("FAKE-ARDUINO")

        with self.assertRaises(TransportError):
            controller.press_key("1", 50)

    def test_unexpected_enable_response_does_not_mark_hid_active(self):
        class EmergencyTransport(FakeTransport):
            def send_line(self, line: str, timeout: float = 1.0) -> str:
                if line == "ENABLE 1":
                    return "OK STOPPED EMERGENCY_STOP"
                return super().send_line(line, timeout)

        controller = MacroController(EmergencyTransport())
        controller.connect("FAKE-ARDUINO")
        with self.assertRaises(TransportError):
            controller.enable_hid()
        self.assertFalse(controller.hid_enabled)

    def test_rejects_unknown_firmware_on_connect(self):
        class OtherFirmware(FakeTransport):
            def send_line(self, line: str, timeout: float = 1.0) -> str:
                if line == "HELLO":
                    return "OK HELLO OTHER 1"
                return super().send_line(line, timeout)

        transport = OtherFirmware()
        controller = MacroController(transport)
        with self.assertRaises(TransportError):
            controller.connect("FAKE-ARDUINO")
        self.assertFalse(transport.is_connected)


if __name__ == "__main__":
    unittest.main()
