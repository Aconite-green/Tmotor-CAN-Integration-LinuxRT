#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <time.h>

#include <linux/can/raw.h>

// User Defined
#include "socket_config.h"
#include "realtime_config.h"
#include "motor.h"
#include "can_utils.h"

#define PI 3.142592
#define CAN1_INDEX 1
#define CAN2_INDEX 0
#define DEG_TO_RAD PI / 180

void find_motor_id(int hsocket)
{
    struct can_frame frame;

    for (int motor_id = 1; motor_id <= 127; motor_id++)
    {
        // 모터 ID를 설정해보세요.
        // 여기에서 필요한 방식으로 ID를 설정하면 됩니다.
        enterControlmode(&frame, motor_id);
        send(hsocket, &frame, sizeof(struct can_frame), 0);

        struct timeval tv;
        fd_set read_fds;

        // Set timeout value
        tv.tv_sec = 1; // 1 second
        tv.tv_usec = 0;

        // Initialize file descriptor set
        FD_ZERO(&read_fds);
        FD_SET(hsocket, &read_fds);

        // Wait until timeout or data received
        int result = select(hsocket + 1, &read_fds, NULL, NULL, &tv);

        // If data is available, read it
        if (result > 0)
        {
            result = recv(hsocket, &frame, sizeof(struct can_frame), MSG_DONTROUTE | MSG_CONFIRM);
            if (result <= 0)
            {
                check_socket_error(result, "recv");
            }
            else
            {
                // 여기서 신호를 확인하고 현재 모터의 ID 값을 알 수 있습니다.
                printf("Motor with ID %d is found.\n", motor_id);
                sleep(1);
            }
        }
        else if (result == 0)
        {
            printf("Timeout reached for motor ID %d. Continuing to next ID.\n", motor_id);
        }
        else
        {
            perror("select");
        }
    }
}

int main()
{

    // Real-time configuration
    configure_realtime();

    // Define socket interfaces
    const char *interfaces[] = {"can0", "can1"};
    int socket_count = sizeof(interfaces) / sizeof(interfaces[0]);
    int sockets[socket_count];

    // Create sockets
    for (int i = 0; i < socket_count; i++)
    {
        sockets[i] = create_socket(interfaces[i]);
        if (sockets[i] == -1)
        {
            printf("Failed to create socket %s\n", interfaces[i]);
            return 1;
        }
    }

    // Set socket options
    struct timeval timeout = {.tv_sec = 0, .tv_usec = 1000000}; // 1000ms
    struct can_filter rfilter[4] = {
        {.can_id = 0x123, .can_mask = CAN_SFF_MASK},
        {.can_id = 0x200, .can_mask = 0x700},
        {.can_id = 0x80123456, .can_mask = 0x1FFFF000},
        {.can_id = 0x80333333, .can_mask = CAN_EFF_MASK}};
    int loopback = 0;
    int recv_own_msgs = 0;
    int enable_block = 1; // Or 0, depending on your needs

    configure_sockets(sockets, socket_count, enable_block, timeout, rfilter, loopback, recv_own_msgs);
    struct can_frame frame;

    // Initialize motors
    Motor Tmotors[] = {
        {"AK70_10", 1, {0, 0, 0}},
        {"AK70_10", 2, {0, 0, 0}},
        {"AK70_10", 3, {0, 0, 0}},
        {"AK70_10", 4, {0, 0, 0}},
        {"AK70_10", 5, {0, 0, 0}},
        {"AK70_10", 6, {0, 0, 0}}};

    MotorContainer container;
    container.num_motors = sizeof(Tmotors) / sizeof(Motor);
    memcpy(container.motors, Tmotors, sizeof(Tmotors));

    for (int i = 0; i < container.num_motors; i++)
    {
        MotorLimits *limits = get_motor_limits(container.motors[i].type);
        if (limits != NULL)
        {
            container.motors[i].limits = *limits;
            free(limits); // Don't forget to free the dynamically allocated memory
        }
    }

    // FIne motor ID
    //find_motor_id(sockets[0]);

    getchar();
    // Control motors based on their sockets
    for (int i = 0; i < container.num_motors; i++)
    {
        int socket_index;

        // Motor 1,2,3 uses can0 (socket index 0)
        // Motor 4,5,6 uses can1 (socket index 1)
        if (container.motors[i].id > 3)
        {
            socket_index = CAN1_INDEX;
        }
        else
        {
            socket_index = CAN2_INDEX;
        }

        // Set motor to 0
        set_to_zero(&frame, container.motors[i].id);
        send_frame_and_receive_reply(sockets[socket_index], &frame, container.motors[i]);
        printf("Motor %d set to zero.\n", container.motors[i].id);
        sleep(1);
        // Set motor to Control mode
        enterControlmode(&frame, container.motors[i].id);
        send_frame_and_receive_reply(sockets[socket_index], &frame, container.motors[i]);
        printf("Motor %d set to Control mode.\n\n", container.motors[i].id);
        sleep(1);
    }
    printf("dont press anything all baljag");
    getchar();

    // Stop motors and exit control mode
    for (int i = 0; i < container.num_motors; i++)
    {
        int socket_index;

        if (container.motors[i].id > 3)
        {
            socket_index = CAN1_INDEX;
        }
        else
        {
            socket_index = CAN2_INDEX;
        }

        // stop motors
        pack_cmd(container.motors[i].limits, &frame, container.motors[i].id, 0, 0, 0, 0, 0);
        send_frame_and_receive_reply(sockets[socket_index], &frame, container.motors[i]);
        printf("Motor %d stopped.\n", container.motors[i].id);
        sleep(1);
        exitControlmode(&frame, container.motors[i].id);
        send_frame_and_receive_reply(sockets[socket_index], &frame, container.motors[i]);
        printf("Motor %d exit control mode.\n\n", container.motors[i].id);
        sleep(1);
    }

    getchar();
    // Specify motor values
    float vel = 0;
    float kp = 10;
    float kd = 1;
    float torque = 0;

    double pos[container.num_motors];
    pos[0] = 0;
    pos[1] = 0;
    pos[2] = 0;
    pos[3] = 0;
    pos[4] = 0;
    pos[5] = 0;
    clock_t external = clock();
    int period = 10;  // ms
    int exitFlag = 0; // flag to control exit from outer while loop

    while (1)
    {
        while (1)
        {
            clock_t internal = clock();
            double elapsed_time = (double)(internal - external) / (double)CLOCKS_PER_SEC * 1000;
            if (elapsed_time > period)
            {
                for (int i = 0; i < container.num_motors; i++)
                {
                    int socket_index;

                    if (container.motors[i].id <= 3)
                    {
                        socket_index = CAN1_INDEX;
                    }
                    else
                    {
                        socket_index = CAN2_INDEX;
                    }

                    // Pack motor commands and send frame
                    pack_cmd(container.motors[i].limits, &frame, container.motors[i].id, pos[i] * DEG_TO_RAD, vel, kp, kd, torque);
                    send_frame_and_receive_reply(sockets[socket_index], &frame, container.motors[i]);
                    printf("Motor %d set to position %f.\n", container.motors[i].id, pos[i]);

                    // Check if pos reached 10
                    if (pos[i] >= 10)
                    {
                        exitFlag = 1;
                        break;
                    }
                }

                if (exitFlag)
                    break;

                // Change position values for next cycle, for example:
                pos[0] += 0.01;
                pos[1] += 0.01;
                pos[2] += 0.01;
                pos[3] += 0.01;
                pos[4] += 0.01;
                pos[5] += 0.01;

                external = clock();
                break;
            }
        }

        if (exitFlag)
            break;

        // Check for user input to quit program
        if (kbhit())
        {
            char c = getchar();
            if (c == 'q' || c == 'Q')
            {
                printf("Exit control mode.\n");
                break;
            }
        }
    }

    // Stop motors and exit control mode
    for (int i = 0; i < container.num_motors; i++)
    {
        int socket_index;

        if (container.motors[i].id <= 3)
        {
            socket_index = CAN1_INDEX;
        }
        else
        {
            socket_index = CAN2_INDEX;
        }

        stop_motor(&frame, container.motors[i].id);
        send_frame_and_receive_reply(sockets[socket_index], &frame, container.motors[i]);
        printf("Motor %d stopped.\n", container.motors[i].id);

        exitControlmode(&frame, container.motors[i].id);
        send_frame_and_receive_reply(sockets[socket_index], &frame, container.motors[i]);
        printf("Motor %d exit control mode.\n", container.motors[i].id);
    }

    // Close sockets
    for (int i = 0; i < socket_count; i++)
    {
        close(sockets[i]);
    }

    return 0;
}
