#include "../include/path_manager.h"




int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

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


    pack_cmd_and_save_to_csv(argv[1], argv[2], container);

    return 0;
}

void pack_cmd_and_save_to_csv(char *input_file, char *output_file, MotorContainer container) {
    // Load the original commands from the input CSV file.
    Buffer buffer = load_motor_commands_from_csv(input_file);

    // Open the output CSV file.
    FILE *file = fopen(output_file, "w");
    if (!file) {
        printf("Could not open output file.\n");
        exit(1);
    }

    // Iterate over each motor command in the buffer.
    for (int i = 0; i < buffer.size; i++) {
        // Get the current motor command.
        MotorCommand command = buffer.commands[i];

        // Find the limits for this motor.
        MotorLimits* limits = get_motor_limits_by_id(&container,command.motor_id);
        if (!limits) {
            printf("Unknown motor id: %d\n", command.motor_id);
            exit(1);
        }

        // Pack the command into a CAN frame.
        struct can_frame frame;
        pack_cmd(*limits, &frame, command.motor_id, command.p_des, command.v_des, command.kp, command.kd, command.t_ff);

        // Write the CAN frame to the output CSV file.
        fprintf(file, "%d,%d", frame.can_id, frame.can_dlc);
        for (int j = 0; j < frame.can_dlc; j++) {
            fprintf(file, ",%d", frame.data[j]);
        }
        fprintf(file, "\n");

        printf("Processed command for motor id: %d\n", command.motor_id);
        
        // Free the memory for limits as it is no longer needed.
        free(limits);
    }

    // Close the output CSV file.
    fclose(file);
}

Buffer load_motor_commands_from_csv(char *input_file) {
    FILE *file = fopen(input_file, "r");
    if (!file) {
        printf("Could not open input file.\n");
        Buffer empty_buffer = {NULL, 0, 0};
        return empty_buffer;
    }

    Buffer buffer;
    buffer.size = 0;
    buffer.capacity = 100; 
    buffer.commands = malloc(sizeof(MotorCommand) * buffer.capacity);
    if (!buffer.commands) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (buffer.size == buffer.capacity) {
            buffer.capacity *= 2;
            MotorCommand* temp = realloc(buffer.commands, sizeof(MotorCommand) * buffer.capacity);
            if (!temp) {
                printf("Memory allocation failed.\n");
                free(buffer.commands);
                exit(1);
            }
            buffer.commands = temp;
        }

        MotorCommand command;
        int fields = sscanf(line, "%d,%f,%f,%f,%f,%f", &command.motor_id, &command.p_des, &command.v_des, 
                            &command.kp, &command.kd, &command.t_ff);
        if (fields != 6) {
            printf("Line format error.\n");
            continue;
        }

        buffer.commands[buffer.size++] = command;
    }

    fclose(file);
    return buffer;
}