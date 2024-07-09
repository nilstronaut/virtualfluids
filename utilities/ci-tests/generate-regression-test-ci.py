# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © VirtualFluids Project contributors, see AUTHORS.md in root folder

from pathlib import Path
import generateci

regression_ci_template = "regression-tests-ci.yml.j2"
regression_ci_file = "regression-tests-ci.yml"
test_file_dir = Path("tests/regression-tests")

if __name__ == "__main__":
    generateci.generate_ci_file_for_tests(
        Path(__file__).parent, regression_ci_template, regression_ci_file, test_file_dir)
