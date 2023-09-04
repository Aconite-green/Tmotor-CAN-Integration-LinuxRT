#ifndef CAN_UTILS_H
#define CAN_UTILS_H

#include <linux/can.h>
#include <math.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include "motor.h"
#include "error_handling.h" 


void pack_cmd(MotorLimits limits, struct can_frame *frame, int can_id, float p_des, float v_des, float kp, float kd, float t_ff);
void unpack_reply(MotorLimits limits, struct can_frame *frame);

void enterControlmode(struct can_frame *frame, int can_id);
void set_to_zero(struct can_frame *frame, int can_id);
void exitControlmode(struct can_frame *frame, int can_id);
int kbhit(void);
void stop_motor(struct can_frame *frame, int can_id);
void send_frame_and_receive_reply(int hsocket, struct can_frame *frame, Motor motor);

#endif