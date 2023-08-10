#ifndef PATH_MANAGER_H
#define PATH_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <linux/can.h>
#include "motor.h"  // Assuming this contains MotorLimits definition
#include "can_utils.h"
#include "error_handling.h"

typedef struct {
    MotorCommand* commands;
    int size;
    int capacity;
} Buffer;

Buffer load_motor_commands_from_csv(char *input_file);
void pack_cmd_and_save_to_csv(char *input_file, char *output_file, MotorContainer container);

#endif /* PATH_MANAGER_H */
