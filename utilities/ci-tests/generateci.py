# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © VirtualFluids Project contributors, see AUTHORS.md in root folder

from jinja2 import Template
from pathlib import Path

def build_regression_job_string(regression_tests: list[(str,str)], regression_ci_template: Path) -> str:
    template = Template(regression_ci_template.read_text())
    return template.render(test_jobs=regression_tests)

def trim_parent_path(name: str, test_file_dir: str) -> str:
    return name.replace(str(test_file_dir)+"/", "")

def generate_ci_file_for_tests(templates_dir: Path, regression_ci_template: str,
                               regression_ci_file: str, test_file_dir: Path):
    regression_ci_template_path = templates_dir / regression_ci_template
    generated_dir = Path("generated")
    regression_ci_file = generated_dir / regression_ci_file

    generated_dir.mkdir(parents=True, exist_ok=True)
    regression_tests_files = [(item.stem, trim_parent_path(str(item.parent), test_file_dir)) for item in test_file_dir.rglob("*_test.sh")]
    print(regression_tests_files)
    regression_tests_ci_file = build_regression_job_string(regression_tests_files, regression_ci_template_path)
    regression_ci_file.write_text(regression_tests_ci_file)