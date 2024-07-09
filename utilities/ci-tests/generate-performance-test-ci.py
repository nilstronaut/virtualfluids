# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © VirtualFluids Project contributors, see AUTHORS.md in root folder

from pathlib import Path
import generateci

performance_ci_template = "performance-tests-ci.yml.j2"
performance_ci_file = "performance-tests-ci.yml"
test_file_dir = Path("tests/performance-tests")

if __name__ == "__main__":
    generateci.generate_ci_file_for_tests(
        Path(__file__).parent, performance_ci_template, performance_ci_file, test_file_dir)
