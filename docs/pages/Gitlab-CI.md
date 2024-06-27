<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- SPDX-FileCopyrightText: Copyright © VirtualFluids Project contributors, see AUTHORS.md in root folder -->

# Gitlab CI

VirtualFluids is using Gitlab CI to automate the build, test and deployment process. The Gitlab CI is configured in the file `.gitlab-ci.yml` in the root folder of the repository. The configuration is based on the [Gitlab CI/CD Documentation](https://docs.gitlab.com/ee/ci/).

Most Gitlab CI jobs are running whith each push to the repository. Additionally, some jobs are running on a schedule. The schedule is defined [here](https://git.rz.tu-bs.de/irmb/VirtualFluids/-/pipeline_schedules).

Almost all jobs are based on the Docker image from the Gitlab CI Docker Registry.

## Gitlab Runner

There a currently three machines, where the ci jobs are running. The machines are called `gitlab-runner01`, `gitlab-runner02` and `gitlab-runner03`. The machines are running on the Gitlab Runner software.
The computers are located in the old VR-Lab in the cellar of the TU Braunschweig.
The runner are registered in the Gitlab project and are using the tags `linux`, `win` and `gpu`. The tags are used in the `.gitlab-ci.yml` file to define on which runner the job should run.
All Gitlab runner are shared to all Maintainer in this sub-project: https://git.rz.tu-bs.de/irmb/shared/runner/-/settings/ci_cd.
With this they can use the runner in their own projects. They need to be enabled for every new project. (e.g. virtualfluids project: https://git.rz.tu-bs.de/irmb/VirtualFluids/-/settings/ci_cd).


### gitlab-runner01 and gitlab-runner02 (Linux)
- IP: 134.169.81.206/207
- Linux Version: Ubuntu 22.04
- Docker based
- gitlab runner is installed and updated via ubuntu package manager.



#### Update NVIDIA Driver:
0. Download a newer driver (optional). Only on demand, for example a new cuda version requires a new driver. Its needs to be checked if the new driver is compatible with the GPU.
    - get newest driver link from here: https://www.nvidia.com/de-de/drivers/
    - copy the download Link. e.g. https://us.download.nvidia.com/XFree86/Linux-x86_64/550.90.07/NVIDIA-Linux-x86_64-550.90.07.run
    - login to runner via ssh
    ```
    cd nvidia-driver
    wget https://us.download.nvidia.com/XFree86/Linux-x86_64/550.90.07/NVIDIA-Linux-x86_64-550.90.07.run
    ```
    - change permission
    ```
    sudo chmod +x <driver-file-name>
    ```

1. install driver ( e.g. 550.90.07.):
Pass the driver file name as argument to the script.
```
sudo ./install_nvidia_driver.sh NVIDIA-Linux-x86_64-550.90.07.run
```

Done!

2. check gpu access
```
runner@gitlab-runner02:~$ nvidia-smi
Fri Jun 14 08:44:18 2024       
+-----------------------------------------------------------------------------------------+
| NVIDIA-SMI 550.90.07              Driver Version: 550.90.07      CUDA Version: 12.4     |
|-----------------------------------------+------------------------+----------------------+
| GPU  Name                 Persistence-M | Bus-Id          Disp.A | Volatile Uncorr. ECC |
| Fan  Temp   Perf          Pwr:Usage/Cap |           Memory-Usage | GPU-Util  Compute M. |
|                                         |                        |               MIG M. |
|=========================================+========================+======================|
|   0  NVIDIA GeForce RTX 2070 ...    Off |   00000000:01:00.0 Off |                  N/A |
| 15%   60C    P0             N/A /  215W |       1MiB /   8192MiB |      0%      Default |
|                                         |                        |                  N/A |
+-----------------------------------------+------------------------+----------------------+
                                                                                         
+-----------------------------------------------------------------------------------------+
| Processes:                                                                              |
|  GPU   GI   CI        PID   Type   Process name                              GPU Memory |
|        ID   ID                                                               Usage      |
|=========================================================================================|
|  No running processes found                                                             |
+-----------------------------------------------------------------------------------------+
```

### Runner3 (Windows)
IP: 134.169.81.208
Windows 11
Tags: win, gpu
Shell based

Windows Gitlab Runner:
Manual: https://docs.gitlab.com/runner/install/windows.html

Updates:
1. OS
2. Visual Studio Installer
3. Download and install new cmake
4. cuda version

Installed Gitlab Runner here:
C:\GitLab-Runner