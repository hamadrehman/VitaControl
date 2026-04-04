#include <psp2kern/ctrl.h>

#include "switch_pro_controller.h"

SwitchProController::SwitchProController(uint32_t mac0, uint32_t mac1, int port): Controller(mac0, mac1, port)
{
    // Prepare a write request to switch to standard mode
    static uint8_t report[12] = {};
    report[0]  = 0x01;
    report[1]  = 0x01;
    report[10] = 0x03;
    report[11] = 0x30;

    // Send the write request
    requestReport(HID_REQUEST_WRITE, report, sizeof(report));
}

void SwitchProController::processReport(uint8_t *buffer, size_t length)
{
    if (buffer[0] == 0x30)
        processStandardReport(buffer, length);
    else if (buffer[0] == 0x3F)
        processSimpleReport(buffer, length);
}

void SwitchProController::processStandardReport(uint8_t *buffer, size_t length)
{
    // Interpret the data as a standard input report
    SwitchProReport0x30 *report = (SwitchProReport0x30*)buffer;

    // Clear the old control data
    controlData.buttons = 0;

    // Map the face buttons
    if (report->b) controlData.buttons |= SCE_CTRL_CROSS;
    if (report->a) controlData.buttons |= SCE_CTRL_CIRCLE;
    if (report->x) controlData.buttons |= SCE_CTRL_TRIANGLE;
    if (report->y) controlData.buttons |= SCE_CTRL_SQUARE;

    // Map the D-pad
    if (report->up)    controlData.buttons |= SCE_CTRL_UP;
    if (report->right) controlData.buttons |= SCE_CTRL_RIGHT;
    if (report->down)  controlData.buttons |= SCE_CTRL_DOWN;
    if (report->left)  controlData.buttons |= SCE_CTRL_LEFT;

    // Map the triggers
    if (report->l)      controlData.buttons |= SCE_CTRL_L1;
    if (report->r)      controlData.buttons |= SCE_CTRL_R1;
    if (report->zl)     controlData.buttons |= SCE_CTRL_LTRIGGER;
    if (report->zr)     controlData.buttons |= SCE_CTRL_RTRIGGER;
    if (report->stickL) controlData.buttons |= SCE_CTRL_L3;
    if (report->stickR) controlData.buttons |= SCE_CTRL_R3;

    // Map the menu buttons
    if (report->plus)  controlData.buttons |= SCE_CTRL_START;
    if (report->minus) controlData.buttons |= SCE_CTRL_SELECT;
    if (report->home)  controlData.buttons |= SCE_CTRL_PSBUTTON;

    // Map the extra buttons
    if (report->capture) controlData.buttons |= SCE_CTRL_EXT1;

    // Map the sticks
    controlData.leftX  = report->leftX  >> 4;
    controlData.leftY  = report->leftY  >> 4;
    controlData.rightX = report->rightX >> 4;
    controlData.rightY = report->rightY >> 4;

    // Reverse up and down
    controlData.leftY  = 255 - controlData.leftY;
    controlData.rightY = 255 - controlData.rightY;

    // Map the motion controls
    motionState.accelerX  = report->accelerX;
    motionState.accelerY  = report->accelerY;
    motionState.accelerZ  = report->accelerZ;
    motionState.velocityX = report->velocityX;
    motionState.velocityY = report->velocityY;
    motionState.velocityZ = report->velocityZ;

    // TODO: implement battery level
}

void SwitchProController::processSimpleReport(uint8_t *buffer, size_t length)
{
    // Interpret the data as a simple input report (used by 8BitDo and other clones)
    SwitchProReport0x3F *report = (SwitchProReport0x3F*)buffer;

    // Clear the old control data
    controlData.buttons = 0;

    // Map the face buttons
    if (report->b) controlData.buttons |= SCE_CTRL_CROSS;
    if (report->a) controlData.buttons |= SCE_CTRL_CIRCLE;
    if (report->x) controlData.buttons |= SCE_CTRL_TRIANGLE;
    if (report->y) controlData.buttons |= SCE_CTRL_SQUARE;

    // Map the D-pad from hat switch value
    switch (report->dpad)
    {
        case 0: controlData.buttons |= SCE_CTRL_UP; break;
        case 1: controlData.buttons |= SCE_CTRL_UP | SCE_CTRL_RIGHT; break;
        case 2: controlData.buttons |= SCE_CTRL_RIGHT; break;
        case 3: controlData.buttons |= SCE_CTRL_DOWN | SCE_CTRL_RIGHT; break;
        case 4: controlData.buttons |= SCE_CTRL_DOWN; break;
        case 5: controlData.buttons |= SCE_CTRL_DOWN | SCE_CTRL_LEFT; break;
        case 6: controlData.buttons |= SCE_CTRL_LEFT; break;
        case 7: controlData.buttons |= SCE_CTRL_UP | SCE_CTRL_LEFT; break;
        default: break; // 8 = centered / released
    }

    // Map the triggers
    if (report->l)      controlData.buttons |= SCE_CTRL_L1;
    if (report->r)      controlData.buttons |= SCE_CTRL_R1;
    if (report->zl)     controlData.buttons |= SCE_CTRL_LTRIGGER;
    if (report->zr)     controlData.buttons |= SCE_CTRL_RTRIGGER;
    if (report->stickL) controlData.buttons |= SCE_CTRL_L3;
    if (report->stickR) controlData.buttons |= SCE_CTRL_R3;

    // Map the menu buttons
    if (report->plus)  controlData.buttons |= SCE_CTRL_START;
    if (report->minus) controlData.buttons |= SCE_CTRL_SELECT;
    if (report->home)  controlData.buttons |= SCE_CTRL_PSBUTTON;

    // Map the extra buttons
    if (report->capture) controlData.buttons |= SCE_CTRL_EXT1;

    // Map the sticks (already 8-bit in simple mode)
    controlData.leftX  = report->leftX;
    controlData.leftY  = report->leftY;
    controlData.rightX = report->rightX;
    controlData.rightY = report->rightY;

    // No motion data available in simple mode
}
