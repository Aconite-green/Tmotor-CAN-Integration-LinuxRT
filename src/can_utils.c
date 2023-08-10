// can_utils.c 파일
#include "can_utils.h"

int float_to_uint(float x, float x_min, float x_max, unsigned int bits)
{
    float span = x_max - x_min;
    if (x < x_min)
        x = x_min;
    else if (x > x_max)
        x = x_max;
    return (int)((x - x_min) * ((float)((1 << bits) / span)));
}

float uint_to_float(int x_int, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    return ((float)x_int) * span / ((float)((1 << bits) - 1)) + offset;
}

// Sends routine code
void pack_cmd(MotorLimits limits, struct can_frame *frame, int can_id, float p_des, float v_des, float kp, float kd, float t_ff)
{
    p_des = fminf(fmaxf(limits.P_MIN, p_des), limits.P_MAX);
    v_des = fminf(fmaxf(limits.V_MIN, v_des), limits.V_MAX);
    kp = fminf(fmaxf(limits.Kp_MIN, kp), limits.Kp_MAX);
    kd = fminf(fmaxf(limits.Kd_MIN, kd), limits.Kd_MAX);
    t_ff = fminf(fmaxf(limits.T_MIN, t_ff), limits.T_MAX);

    /// convert floats to unsigned ints ///
    int p_int = float_to_uint(p_des, limits.P_MIN, limits.P_MAX, 16);
    int v_int = float_to_uint(v_des, limits.V_MIN, limits.V_MAX, 12);
    int kp_int = float_to_uint(kp, limits.Kp_MIN, limits.Kp_MAX, 12);
    int kd_int = float_to_uint(kd, limits.Kd_MIN, limits.Kd_MAX, 12);
    int t_int = float_to_uint(t_ff, limits.T_MIN, limits.T_MAX, 12);

    // Set CAN frame id and data length code
    frame->can_id = can_id & CAN_SFF_MASK; // Replace YOUR_CAN_ID with the appropriate id
    frame->can_dlc = 8;                    // Data Length Code is set to maximum allowed length

    /// pack ints into the can buffer ///
    frame->data[0] = p_int >> 8;                           // Position 8 higher
    frame->data[1] = p_int & 0xFF;                         // Position 8 lower
    frame->data[2] = v_int >> 4;                           // Speed 8 higher
    frame->data[3] = ((v_int & 0xF) << 4) | (kp_int >> 8); // Speed 4 bit lower KP 4bit higher
    frame->data[4] = kp_int & 0xFF;                        // KP 8 bit lower
    frame->data[5] = kd_int >> 4;                          // Kd 8 bit higher
    frame->data[6] = ((kd_int & 0xF) << 4) | (t_int >> 8); // KP 4 bit lower torque 4 bit higher
    frame->data[7] = t_int & 0xff;                         // torque 4 bit lower
}

// Receive routine code
void unpack_reply(MotorLimits limits, struct can_frame *frame)
{
    /// unpack ints from can buffer ///
    int id = frame->data[0];                                    // 驱动 ID 号
    int p_int = (frame->data[1] << 8) | frame->data[2];         // Motor position data
    int v_int = (frame->data[3] << 4) | (frame->data[4] >> 4);  // Motor speed data
    int i_int = ((frame->data[4] & 0xF) << 8) | frame->data[5]; // Motor torque data
    /// convert ints to floats ///
    float p = uint_to_float(p_int, limits.P_MIN, limits.P_MAX, 16);
    float v = uint_to_float(v_int, limits.V_MIN, limits.V_MAX, 12);
    float i = uint_to_float(i_int, -limits.T_MAX, limits.T_MAX, 12);

    float position = p; // Read the corresponding data according to the ID code
    float speed = v;
    float torque = i;

    printf("unpack data : Id : %d, position : %f, speed : %f, torpue : %f\n", id, position, speed, torque);
}

void enterControlmode(struct can_frame *frame, int can_id)
{
    // Set CAN frame id and data length code
    frame->can_id = can_id & CAN_SFF_MASK; // Replace YOUR_CAN_ID with the appropriate id

    frame->can_dlc = 8; // Data Length Code is set to maximum allowed length
    /// pack ints into the can buffer ///
    frame->data[0] = 0xFF; // Position 8 higher
    frame->data[1] = 0xFF; // Position 8 lower
    frame->data[2] = 0xFF; // Speed 8 higher
    frame->data[3] = 0xFF; // Speed 4 bit lower KP 4bit higher
    frame->data[4] = 0xFF; // KP 8 bit lower
    frame->data[5] = 0xFF; // Kd 8 bit higher
    frame->data[6] = 0xFF; // KP 4 bit lower torque 4 bit higher
    frame->data[7] = 0xFC; // torque 4 bit lower
}

void set_to_zero(struct can_frame *frame, int can_id)
{
    /// pack ints into the can buffer ///
    frame->data[0] = 0xFF; // Position 8 higher
    frame->data[1] = 0xFF; // Position 8 lower
    frame->data[2] = 0xFF; // Speed 8 higpaher
    frame->data[3] = 0xFF; // Speed 4 bit lower KP 4bit higher
    frame->data[4] = 0xFF; // KP 8 bit lower
    frame->data[5] = 0xFF; // Kd 8 bit higher
    frame->data[6] = 0xFF; // KP 4 bit lower torque 4 bit higher
    frame->data[7] = 0xFE; // torque 4 bit lower

    // Set CAN frame id and data length code
    frame->can_id = can_id & CAN_SFF_MASK; // Replace YOUR_CAN_ID with the appropriate id
    frame->can_dlc = 8;                    // Data Length Code is set to maximum allowed length
}

void exitControlmode(struct can_frame *frame, int can_id)
{
    // Set CAN frame id and data length code
    frame->can_id = can_id & CAN_SFF_MASK; // Replace YOUR_CAN_ID with the appropriate id
    frame->can_dlc = 8;                    // Data Length Code is set to maximum allowed length
    /// pack ints into the can buffer ///
    frame->data[0] = 0xFF; // Position 8 higher
    frame->data[1] = 0xFF; // Position 8 lower
    frame->data[2] = 0xFF; // Speed 8 higher
    frame->data[3] = 0xFF; // Speed 4 bit lower KP 4bit higher
    frame->data[4] = 0xFF; // KP 8 bit lower
    frame->data[5] = 0xFF; // Kd 8 bit higher
    frame->data[6] = 0xFF; // KP 4 bit lower torque 4 bit higher
    frame->data[7] = 0xFD; // torque 4 bit lower
}

void stop_motor(struct can_frame *frame, int can_id)
{
    
    /// convert floats to unsigned ints ///
    int p_int = 0;
    int v_int = 0;
    int kp_int =0;
    int kd_int =0;
    int t_int = 0;
    
    // Set CAN frame id and data length code
    frame->can_id = can_id & CAN_SFF_MASK; // Replace YOUR_CAN_ID with the appropriate id
    frame->can_dlc = 8;                    // Data Length Code is set to maximum allowed length

    /// pack ints into the can buffer ///
    frame->data[0] = p_int >> 8;                           // Position 8 higher
    frame->data[1] = p_int & 0xFF;                         // Position 8 lower
    frame->data[2] = v_int >> 4;                           // Speed 8 higher
    frame->data[3] = ((v_int & 0xF) << 4) | (kp_int >> 8); // Speed 4 bit lower KP 4bit higher
    frame->data[4] = kp_int & 0xFF;                        // KP 8 bit lower
    frame->data[5] = kd_int >> 4;                          // Kd 8 bit higher
    frame->data[6] = ((kd_int & 0xF) << 4) | (t_int >> 8); // KP 4 bit lower torque 4 bit higher
    frame->data[7] = t_int & 0xff;                         // torque 4 bit lower
}

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

void print_frame(const char *action, struct can_frame *frame)
{
    printf("%s ", action);
    if (frame->can_id & CAN_EFF_FLAG)
        printf("EFF %8X ", frame->can_id & CAN_EFF_MASK);
    else
        printf("SFF %3X ", frame->can_id & CAN_SFF_MASK);
    printf("[%u] ", frame->can_dlc);
    if (frame->can_id & CAN_RTR_FLAG)
        printf("remote request ");
    else
    {
        for (int i = 0; i < frame->can_dlc; i++)
            printf("%02X ", frame->data[i]);
    }
    printf("\n");
}

#include <sys/select.h>

void send_frame_and_receive_reply(int hsocket, struct can_frame *frame, Motor motor)
{
    int result;
    struct timeval tv;
    fd_set read_fds;

    result = send(hsocket, frame, sizeof(struct can_frame), 0);
    if (result <= 0)
    {
        check_socket_error(result, "send");
    }
    else
    {
        print_frame("send", frame);
    }

    // Set timeout value
    tv.tv_sec = 1; // 1 second
    tv.tv_usec = 0;

    // Initialize file descriptor set
    FD_ZERO(&read_fds);
    FD_SET(hsocket, &read_fds);

    // Wait until timeout or data received
    result = select(hsocket + 1, &read_fds, NULL, NULL, &tv);

    // If data is available, read it
    if (result > 0)
    {
        result = recv(hsocket, frame, sizeof(struct can_frame), MSG_DONTROUTE | MSG_CONFIRM);
        if (result <= 0)
        {

            check_socket_error(result, "recv");
        }
        unpack_reply(motor.limits, frame);
    }
    else if (result == 0)
    {
        // Timeout reached

        printf("Timeout reached while waiting for reply. Continuing to next sequence.\n");
    }
    else
    {
        // Error on select
        printf("in func3\n");
        perror("select");
    }
}
