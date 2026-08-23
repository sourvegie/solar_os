from pathlib import Path
import unittest


REPOSITORY = Path(__file__).resolve().parents[1]
SSH_SOURCE = (REPOSITORY / "src/apps/solar_os_ssh.c").read_text(encoding="utf-8")


class SshSessionPolicyTest(unittest.TestCase):
    def test_resumable_ssh_owns_a_private_terminal(self):
        app_definition = SSH_SOURCE.split(
            "const solar_os_app_t solar_os_ssh_app =", 1
        )[1]
        self.assertIn(".flags = SOLAR_OS_APP_FLAG_RESUMABLE,", app_definition)
        self.assertNotIn("SOLAR_OS_APP_FLAG_SHELL_INLINE", app_definition)

    def test_only_port_shell_close_preserves_the_shared_terminal(self):
        close_policy = SSH_SOURCE.split(
            "static void ssh_request_close", 1
        )[1].split("static bool ssh_is_printable", 1)[0]
        self.assertIn("ssh_is_private_display_session(ctx)", close_policy)
        self.assertIn("solar_os_context_set_status_message(ctx, status)", close_policy)
        self.assertIn("solar_os_context_request_terminal_preserve(ctx)", close_policy)


if __name__ == "__main__":
    unittest.main()
