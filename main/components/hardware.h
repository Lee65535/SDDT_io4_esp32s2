#pragma once

// position in data->Switch, ref to segatools
enum switch_zero {
    TEST = 1 << 9,
    SERVICE = 1 << 6,
    LA = 1 << 0,
    LB = 1 << 5,
    LC = 1 << 4,
    RA = 1 << 1,
    RC = 1 << 15,
    RM = 1 << 13,
    RS = 1 << 14
};
enum switch_one {
    RB = 1 << 0,
    LM = 1 << 14,
    LS = 1 << 15
};

void hardware_init();

bool hardware_update(io4_output_t *output);

void input_callback(io4_input_t *input);
