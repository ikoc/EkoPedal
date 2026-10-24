import unittest

from desktop.macro_control import protocol


class ProtocolTest(unittest.TestCase):
    def test_basic_commands(self):
        self.assertEqual(protocol.hello().line, "HELLO")
        self.assertEqual(protocol.ping().line, "PING")
        self.assertEqual(protocol.enable_hid(True).line, "ENABLE 1")
        self.assertEqual(protocol.enable_hid(False).line, "ENABLE 0")
        self.assertEqual(protocol.stop().line, "STOP")

    def test_key_press_single_character(self):
        self.assertEqual(protocol.key_press("1", 50).line, "KP 1 50")

    def test_key_press_special_key(self):
        self.assertEqual(protocol.key_press("enter", 25).line, "KP ENTER 25")

    def test_hold_ms_is_clamped(self):
        self.assertEqual(protocol.key_press("1", 1).line, "KP 1 10")
        self.assertEqual(protocol.key_press("1", 9999).line, "KP 1 1000")

    def test_rejects_bad_key(self):
        with self.assertRaises(protocol.ProtocolError):
            protocol.key_press("CTRL+ALT+DEL", 50)


if __name__ == "__main__":
    unittest.main()
