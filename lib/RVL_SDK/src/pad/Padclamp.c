#include <revolution/pad.h>

#include <math.h>

// Clamp region values (CF folds these into immediates; no data object is emitted)
enum {
    CLAMP_MIN_TRIGGER = 30,
    CLAMP_MAX_TRIGGER = 180,
    CLAMP_MIN_STICK = 15,
    CLAMP_MAX_STICK = 72,
    CLAMP_XY_STICK = 40,
    CLAMP_MIN_SUBSTICK = 15,
    CLAMP_MAX_SUBSTICK = 59,
    CLAMP_XY_SUBSTICK = 31,
    CLAMP_RAD_STICK = 56,
    CLAMP_RAD_SUBSTICK = 44,
};

// prototypes
static void ClampStick(s8* px, s8* py, s8 max, s8 xy, s8 min);
static void ClampCircle(s8* px, s8* py, s8 radius, s8 min);
static void ClampTrigger(u8* trigger, u8 min, u8 max);

static void ClampStick(s8* px, s8* py, s8 max, s8 xy, s8 min) {
    int x = *px;
    int y = *py;
    int signX;
    int signY;
    int d;

    if (0 <= x) {
        signX = 1;
    } else {
        signX = -1;
        x = -x;
    }

    if (0 <= y) {
        signY = 1;
    } else {
        signY = -1;
        y = -y;
    }

    if (x <= min) {
        x = 0;
    } else {
        x -= min;
    }
    if (y <= min) {
        y = 0;
    } else {
        y -= min;
    }

    if (x == 0 && y == 0) {
        *px = *py = 0;
        return;
    }

    if (xy * y <= xy * x) {
        d = xy * x + (max - xy) * y;
        if (xy * max < d) {
            x = (s8)(xy * max * x / d);
            y = (s8)(xy * max * y / d);
        }
    } else {
        d = xy * y + (max - xy) * x;
        if (xy * max < d) {
            x = (s8)(xy * max * x / d);
            y = (s8)(xy * max * y / d);
        }
    }

    *px = (s8)(signX * x);
    *py = (s8)(signY * y);
}

static void ClampCircle(s8* px, s8* py, s8 radius, s8 min) {
    int x = *px;
    int y = *py;
    int squared;
    int length;

    if (-min < x && x < min) {
        x = 0;
    } else if (0 < x) {
        x -= min;
    } else {
        x += min;
    }

    if (-min < y && y < min) {
        y = 0;
    } else if (0 < y) {
        y -= min;
    } else {
        y += min;
    }

    squared = x * x + y * y;
    if (radius * radius < squared) {
        length = sqrtf(squared);
        x = (x * radius) / length;
        y = (y * radius) / length;
    }

    *px = x;
    *py = y;
}

static void ClampTrigger(u8* trigger, u8 min, u8 max) {
    if (*trigger <= min) {
        *trigger = 0;
    } else {
        if (max < *trigger) {
            *trigger = max;
        }
        *trigger -= min;
    }
}

#if 0 // not linked in City Folk
void PADClamp(PADStatus * status) {
    int i;

    for (i = 0; i < 4; i++, status++) {
        if (status->err == PAD_ERR_NONE) {
            ClampStick(&status->stickX, &status->stickY, CLAMP_MAX_STICK, CLAMP_XY_STICK, CLAMP_MIN_STICK);
            ClampStick(&status->substickX, &status->substickY, CLAMP_MAX_SUBSTICK, CLAMP_XY_SUBSTICK, CLAMP_MIN_SUBSTICK);
            ClampTrigger(&status->triggerLeft, CLAMP_MIN_TRIGGER, CLAMP_MAX_TRIGGER);
            ClampTrigger(&status->triggerRight, CLAMP_MIN_TRIGGER, CLAMP_MAX_TRIGGER);
        }
    }
}
#endif

void PADClampCircle(PADStatus* status) {
    int i;
    for (i = 0; i < 4; ++i, status++) {
        if (status->err == PAD_ERR_NONE) {
            ClampCircle(&status->stickX, &status->stickY, CLAMP_RAD_STICK, CLAMP_MIN_STICK);
            ClampCircle(&status->substickX, &status->substickY, CLAMP_RAD_SUBSTICK, CLAMP_MIN_SUBSTICK);
            ClampTrigger(&status->triggerLeft, CLAMP_MIN_TRIGGER, CLAMP_MAX_TRIGGER);
            ClampTrigger(&status->triggerRight, CLAMP_MIN_TRIGGER, CLAMP_MAX_TRIGGER);
        }
    }
}
