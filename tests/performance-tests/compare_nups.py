# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © VirtualFluids Project contributors, see AUTHORS.md in root folder

import sys
import argparse
import yaml

default_tolerance_percent = 5

def get_nups_from_yaml(file_path: str) -> float:
    with open(file_path, 'r') as fp:
        dict = yaml.safe_load(fp)
    return dict["simulation"]["nups"]

def compare_nups(file_path_result: str, file_path_reference: str, tolerance: float) -> bool:
    """Compare nups from two simulation meta data files
    file_path_result    -- file path to result .yaml
    file_path_reference -- file path to reference .yaml
    tolerance           -- tolerance in %
    returns true if job passed
    """
    
    result_mnups = get_nups_from_yaml(file_path_result) * 1e-6
    reference_mnups = get_nups_from_yaml(file_path_reference) * 1e-6
    relative_performance = (result_mnups - reference_mnups) / reference_mnups * 100
    print("\nComparing MNUPS:\n %5.2f - result \n %5.2f - reference\nRelative change in performance: %.2f %%"
          %(result_mnups, reference_mnups, relative_performance))
    if (relative_performance * -1) < tolerance:
        print("[ PASSED ] The simulation was fast enough (with a tolerance of %.2f%%)." %tolerance)
        return True
    else:
        print("[ FAILED ] The simulation was more than %.2f%% slower than the reference simulation." %tolerance)
        return False

def main():

    parser = argparse.ArgumentParser(
                        prog='compare_nups',
                        description='Compare nups from two simulation meta data files')
    parser.add_argument('path_to_result', help="file path to result metadata .yaml ", type=str)
    parser.add_argument('path_to_reference', help="file path to reference metadata .yaml", type=str)
    parser.add_argument('-t', '--tolerance', help="tolerance in percent", default=default_tolerance_percent, type=float, required=False)

    args = vars(parser.parse_args())
    result_yaml = args['path_to_result']
    reference_yaml = args['path_to_reference']
    tolerance_percent = args['tolerance']

    has_passed = compare_nups(result_yaml, reference_yaml, tolerance_percent)
    sys.exit(not has_passed) # exit code 0 -> test passed, exit code 1 -> test failed

if __name__ == "__main__":
   main()