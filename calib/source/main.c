#include <switch.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define SQUARE_SIZE 220
#define SQUARE_GAP 120
#define SQUARE_Y ((SCREEN_HEIGHT - SQUARE_SIZE) / 2)
#define LEFT_SQUARE_X ((SCREEN_WIDTH / 2) - SQUARE_GAP / 2 - SQUARE_SIZE)
#define RIGHT_SQUARE_X ((SCREEN_WIDTH / 2) + SQUARE_GAP / 2)

typedef struct {
    u8 background;
    u8 left;
    u8 right;
} CalibMode;

static const CalibMode kModes[] = {
    { 0, 8, 24 },
    { 128, 118, 138 },
    { 255, 231, 247 },
};
#define NUM_MODES (int)(sizeof(kModes) / sizeof(kModes[0]))

static void FillRect(u32* framebuf, u32 stride, int x, int y, int w, int h, u8 level) {
    u32 packed = RGBA8_MAXALPHA(level, level, level);
    for (int row = y; row < y + h; row++) {
        u32* line = (u32*)((u8*)framebuf + row * stride);
        for (int col = x; col < x + w; col++)
            line[col] = packed;
    }
}

int main(int argc, char* argv[]) {
    PadState pad;
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);

    Framebuffer fb;
    framebufferCreate(&fb, nwindowGetDefault(), SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_FORMAT_RGBA_8888, 2);
    framebufferMakeLinear(&fb);

    int modeIndex = 0;

    while (appletMainLoop()) {
        padUpdate(&pad);

        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus)
            break;

        if (kDown & HidNpadButton_L)
            modeIndex = (modeIndex + NUM_MODES - 1) % NUM_MODES;
        if (kDown & HidNpadButton_R)
            modeIndex = (modeIndex + 1) % NUM_MODES;

        const CalibMode* mode = &kModes[modeIndex];

        u32 stride;
        u32* framebuf = (u32*)framebufferBegin(&fb, &stride);

        FillRect(framebuf, stride, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, mode->background);
        FillRect(framebuf, stride, LEFT_SQUARE_X, SQUARE_Y, SQUARE_SIZE, SQUARE_SIZE, mode->left);
        FillRect(framebuf, stride, RIGHT_SQUARE_X, SQUARE_Y, SQUARE_SIZE, SQUARE_SIZE, mode->right);

        framebufferEnd(&fb);
    }

    framebufferClose(&fb);
    return 0;
}
