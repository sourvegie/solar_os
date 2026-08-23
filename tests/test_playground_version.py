import os
from pathlib import Path
import subprocess
import tempfile
import unittest


REPOSITORY = Path(__file__).resolve().parents[1]
PLAYGROUND = (REPOSITORY / "src/services/solar_os_playground.c").read_text(
    encoding="utf-8"
)


def extract_function(name: str) -> str:
    start = PLAYGROUND.index(f"static bool {name}(")
    opening = PLAYGROUND.index("{", start)
    depth = 0
    for index in range(opening, len(PLAYGROUND)):
        if PLAYGROUND[index] == "{":
            depth += 1
        elif PLAYGROUND[index] == "}":
            depth -= 1
            if depth == 0:
                return PLAYGROUND[start : index + 1]
    raise AssertionError(f"unterminated function: {name}")


class PlaygroundVersionTest(unittest.TestCase):
    def test_numeric_base_version_accepts_custom_suffixes(self):
        source = "\n".join(
            [
                "#include <assert.h>",
                "#include <ctype.h>",
                "#include <errno.h>",
                "#include <limits.h>",
                "#include <stdbool.h>",
                "#include <stddef.h>",
                "#include <stdlib.h>",
                extract_function("playground_parse_semver"),
                extract_function("playground_version_at_least"),
                "int main(void)",
                "{",
                '    assert(playground_version_at_least("0.8.2-custom", "0.8.2"));',
                '    assert(playground_version_at_least("0.8.2+custom", "0.8.2"));',
                '    assert(playground_version_at_least("4.8.9-stardust.1", "4.8.8"));',
                '    assert(playground_version_at_least("4.8.9-rc.1+fork", "4.8.9"));',
                '    assert(!playground_version_at_least("0.8.1-custom", "0.8.2"));',
                '    assert(!playground_version_at_least("0.8-custom", "0.8.2"));',
                '    assert(!playground_version_at_least("0.8.2custom", "0.8.2"));',
                '    assert(!playground_version_at_least("0.8.2-", "0.8.2"));',
                "    return 0;",
                "}",
            ]
        )

        with tempfile.TemporaryDirectory() as directory:
            directory_path = Path(directory)
            source_path = directory_path / "playground_version_test.c"
            binary_path = directory_path / "playground_version_test"
            source_path.write_text(source, encoding="utf-8")
            subprocess.run(
                [
                    os.environ.get("CC", "cc"),
                    "-std=c11",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    str(source_path),
                    "-o",
                    str(binary_path),
                ],
                check=True,
            )
            subprocess.run([str(binary_path)], check=True)


if __name__ == "__main__":
    unittest.main()
