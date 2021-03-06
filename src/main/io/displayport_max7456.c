/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#ifdef USE_MAX7456

#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/display.h"
#include "drivers/max7456.h"
#include "drivers/vcd.h"

#include "io/displayport_max7456.h"
#include "io/osd.h"
#include "io/osd_slave.h"

displayPort_t max7456DisplayPort;

PG_REGISTER_WITH_RESET_FN(displayPortProfile_t, displayPortProfileMax7456, PG_DISPLAY_PORT_MAX7456_CONFIG, 0);

void pgResetFn_displayPortProfileMax7456(displayPortProfile_t *displayPortProfile)
{
    displayPortProfile->colAdjust = 0;
    displayPortProfile->rowAdjust = 0;

    // Set defaults as per MAX7456 datasheet
    displayPortProfile->invert = false;
    displayPortProfile->blackBrightness = 0;
    displayPortProfile->whiteBrightness = 2;
}

static int grab(displayPort_t *displayPort)
{
    // FIXME this should probably not have a dependency on the OSD or OSD slave code
    UNUSED(displayPort);
#ifdef OSD
    osdResetAlarms();
    resumeRefreshAt = 0;
#endif

    return 0;
}

static int release(displayPort_t *displayPort)
{
    UNUSED(displayPort);

    return 0;
}

static int clearScreen(displayPort_t *displayPort)
{
    UNUSED(displayPort);

    max7456Invert(displayPortProfileMax7456()->invert);
    max7456Brightness(displayPortProfileMax7456()->blackBrightness, displayPortProfileMax7456()->whiteBrightness);

    max7456ClearScreen();

    return 0;
}

static int drawScreen(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    max7456DrawScreen();

    return 0;
}

static int screenSize(const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return maxScreenSize;
}

static int writeString(displayPort_t *displayPort, uint8_t x, uint8_t y, const char *s)
{
    UNUSED(displayPort);
    max7456Write(x, y, s);

    return 0;
}

static int writeChar(displayPort_t *displayPort, uint8_t x, uint8_t y, uint8_t c)
{
    UNUSED(displayPort);
    max7456WriteChar(x, y, c);

    return 0;
}

static bool isTransferInProgress(const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return max7456DmaInProgress();
}

static void resync(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    max7456RefreshAll();
    displayPort->rows = max7456GetRowsCount() + displayPortProfileMax7456()->rowAdjust;
    displayPort->cols = 30 + displayPortProfileMax7456()->colAdjust;
}

static int heartbeat(displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return 0;
}

static uint32_t txBytesFree(const displayPort_t *displayPort)
{
    UNUSED(displayPort);
    return UINT32_MAX;
}

static const displayPortVTable_t max7456VTable = {
    .grab = grab,
    .release = release,
    .clearScreen = clearScreen,
    .drawScreen = drawScreen,
    .screenSize = screenSize,
    .writeString = writeString,
    .writeChar = writeChar,
    .isTransferInProgress = isTransferInProgress,
    .heartbeat = heartbeat,
    .resync = resync,
    .txBytesFree = txBytesFree,
};

displayPort_t *max7456DisplayPortInit(const vcdProfile_t *vcdProfile)
{
    displayInit(&max7456DisplayPort, &max7456VTable);
    max7456Init(vcdProfile);
    resync(&max7456DisplayPort);
    return &max7456DisplayPort;
}
#endif // USE_MAX7456
