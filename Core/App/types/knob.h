#ifndef KNOB_TASK_H
#define KNOB_TASK_H

#include "tim.h"
#include "main.h"

typedef enum {
  Knob_Short = 0,
  Knob_Long,
  Knob_R,
  Knob_L,
} Knob_type;

typedef struct {
  Knob_type type;
} Knob_message;

extern Knob_message mess;

void StartEncoder(void *argument);
#endif