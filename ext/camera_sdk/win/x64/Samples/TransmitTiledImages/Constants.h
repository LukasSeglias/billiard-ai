// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

// Number of tiles by row / column in the display
#define MAX_TILES_ROW         ( 2 )
#define MAX_TILES_COLUMN      ( 2 )

// Default receiver size configuration for the buffer allocation
#define DEFAULT_RX_BUFFER_ALLOCATION_WIDTH ( 640 )
#define DEFAULT_RX_BUFFER_ALLOCATION_HEIGHT ( 480 )
#define DEFAULT_RX_BUFFER_ALLOCATION_SIZE ( DEFAULT_RX_BUFFER_ALLOCATION_WIDTH * DEFAULT_RX_BUFFER_ALLOCATION_HEIGHT * 4 )

// Pool size definition
#define RX_POOL_SIZE          ( 4 )
#define TX_POOL_SIZE          ( 4 )

// Display frame rate
#define DISPLAY_FPS           ( 30 )

// Custom messages
#define WM_LINKDISCONNECTED   ( WM_USER + 1 )
#define WM_REALLOCATIONFAIL   ( WM_USER + 2 )
#define WM_CONVERSIONFAIL     ( WM_USER + 3 )