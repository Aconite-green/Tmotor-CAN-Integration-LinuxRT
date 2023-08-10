# Realtime Motor Control Project

This project provides a guide on how to control six AK10-9 motors (Tmotor Company) in real-time using an embedded computer (Adventech). 

## Table of Contents

- [Realtime Motor Control Project](#realtime-motor-control-project)
  - [Table of Contents](#table-of-contents)
  - [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
    - [Installation](#installation)
  - [System Setup](#system-setup)
    - [Setting up the Network Interface](#setting-up-the-network-interface)
  - [Installing and Configuring the Real-time Kernel](#installing-and-configuring-the-real-time-kernel)
    - [Applying the Real-time Kernel in Code](#applying-the-real-time-kernel-in-code)
  - [Code Usage](#code-usage)
    - [Importing the Required Libraries](#importing-the-required-libraries)
    - [Sending Motor Control Signals](#sending-motor-control-signals)
  - [Interfacing with Tmotor](#interfacing-with-tmotor)
    - [Contacting Tmotor](#contacting-tmotor)
    - [Creating a Repository for Tmotor Control](#creating-a-repository-for-tmotor-control)
  - [License](#license)
  - [Acknowledgments](#acknowledgments)

## Getting Started

### Prerequisites

- [Ubuntu 20.04](https://releases.ubuntu.com/focal/)
- [Realtime kernel](https://docs.ros.org/en/foxy/Tutorials/Miscellaneous/Building-Realtime-rt_preempt-kernel-for-ROS-2.html) compatible with Ubuntu 20.04
- [Adventech embedded computer](https://www.advantech.com/en/products/f6935c48-a0b2-4c68-894a-40eb39c832b3/epc-c301/mod_0cdaa0da-61e8-4078-ac65-0d4cd00789f7) with CAN port
- Six [AK70-10 motors](https://store.tmotor.com/goods-1031-AK70-10.html) from Tmotor

### Installation

1. [Download](https://releases.ubuntu.com/focal/) and install Ubuntu 20.04 on the Adventech embedded computer. 
2. [Install](https://docs.ros.org/en/foxy/Tutorials/Miscellaneous/Building-Realtime-rt_preempt-kernel-for-ROS-2.html) a Realtime kernel that is compatible with Ubuntu 20.04.
3. Connect three AK70-10 motors from [Tmotor](https://store.tmotor.com/goods-1031-AK70-10.html) to each CAN port of the [Adventech](https://www.advantech.com/en/products/f6935c48-a0b2-4c68-894a-40eb39c832b3/epc-c301/mod_0cdaa0da-61e8-4078-ac65-0d4cd00789f7) embedded computer. The computer should be connected to a total of six motors. 
4. Download the provided source code onto the embedded computer.

## System Setup

### Setting up the Network Interface

In this project, you can set up the network interface by running the `CANControllerSetup.sh` script. This script automates the process of setting up the network interface for communication over the CAN interface.

```bash
./CANControllerSetup.sh
```

This script combines the functionality of the two scripts (`insmod_module_platform.sh` and `ipup_RDCCAN_06.sh`) discussed earlier, effectively making the setup process more streamlined.

The commands used in this script are explained below:

- **`modprobe`**: 

  The `modprobe` command is used in Linux to add a loadable kernel module (LKM) into the kernel or to remove an LKM from the kernel. This is used to load the necessary modules that the system needs to communicate over the CAN interface.

    ```
    modprobe can
    modprobe can-raw
    modprobe can-dev
    ```

- **`insmod`**: 

  The `insmod` command is used to insert a module into the Linux kernel. In our script, it installs the `can-ahc0512.ko` driver for the CAN controller.

    ```
    insmod can-ahc0512.ko
    ```

- **`ip link set`**: 

  The `ip link set` command is used to change the state of the device. In our case, it sets the type of `can0` to CAN and configures it with the specified bitrate, sample point, and restart time.

    ```
    ip link set can0 type can bitrate 1000000 sample-point 0.6 restart-ms 1
    ```

- **`ip -details -statistics link show`**: 

  The `ip` command is used to show detailed statistics for the `can0` interface. This helps in verifying if the interface has been set up correctly.

    ```
    ip -details -statistics link show can0
    ```

- **`ip link set up type can`**: 

  The `ip link set` command is also used to activate the `can0` device, making it ready to transmit and receive data.

    ```
    ip link set can0 up type can
    ```



## Installing and Configuring the Real-time Kernel

A real-time kernel is a kernel that guarantees to process certain events or data by a specific moment in time. It provides determinism and predictability, which is essential for real-time systems that need to perform tasks within a strict deadline, such as motor control in this project.

1. **Download the real-time kernel.** You can download it from [here](https://www.kernel.org/pub/linux/kernel/projects/rt/).

2. **Follow detailed instructions** on how to install the real-time kernel.

3. **Configure the kernel** to suit the project's needs. This can involve setting certain kernel parameters or enabling/disabling certain features.

4. **Verify that the kernel is configured correctly.** This can be done by checking the kernel version or the enabled features.

### Applying the Real-time Kernel in Code

We have applied the real-time kernel in our code to ensure that certain tasks are executed within a strict deadline. We have separated the process into two functions for improved readability, and all these functions are invoked in `configure_realtime()`. Here is the updated example:

```c
void set_process_priority()
{
    // Set process to max priority for reliable real-time execution
    int result = setpriority(PRIO_PROCESS, 0, MAX_PRIORITY);
    if (result == -1)
    {
        perror("setpriority failed");
        exit(-1);
    }
}

void configure_scheduler()
{
    // Set up a real-time, absolute deadline scheduler
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHEDULING_POLICY);
    int result = sched_setscheduler(0, SCHEDULING_POLICY, &param);
    if (result == -1)
    {
        perror("sched_setscheduler failed");
        exit(-1);
    }
}

void configure_realtime()
{
    set_process_priority();
    configure_scheduler();
}
```

## Code Usage

### Importing the Required Libraries

- Steps to import necessary libraries.
- Explanation of what each library does in the code.

### Sending Motor Control Signals

In this project, three motors are connected to each CAN port of the Adventech computer. *This distribution is important for maintaining real-time control capabilities.* When operating at a baud rate of 1 Mbps with a real-time requirement of 5 ms or less, the recommended maximum number of motors per CAN port is three. **This ensures the system can meet the real-time requirement and each motor can receive its command and respond within the specified timeframe.**

- Description of how the code uses the clock_t object (CPU tick) to send motor control signals every 10ms.
- Examples of how to modify this part of the code to customize the behavior of the motors.

## Interfacing with Tmotor

### Contacting Tmotor

- Steps on how to contact Tmotor for support or to provide them with feedback.
- Template or example messages to send to Tmotor.

### Creating a Repository for Tmotor Control

- Description of how to create a repository for Tmotor control.
- Steps to follow when uploading code or updates to the repository.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.

## Acknowledgments

- Many thanks to Tmotor company for their help and support.
