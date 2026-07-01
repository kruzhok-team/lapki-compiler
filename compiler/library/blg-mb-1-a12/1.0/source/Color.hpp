#pragma once

namespace detail {

    struct Color {

        // Duration of red, green, blue shining and darkness (to reduce brightness)
        uint16_t colorsValue[4];   // red, green, blue, black
    };

    // Цвета, не предназначенные для использования пользователем
    Color ReservedColor1 = { 1, 1, 1, 1 };
    Color ReservedColor2 = { 1, 1, 1, 1 };
}

// На ревизии b2 RGB-светодиод глаз имеет обратный порядок каналов (BGR).
// Макрос переставляет R и B при компиляции под b2.
#ifdef BLG_MB_REVISION_B2
#define MAKE_COLOR(r, g, b, black) {{ b, g, r, black }}
#else
#define MAKE_COLOR(r, g, b, black) {{ r, g, b, black }}
#endif

detail::Color ColorRed          = MAKE_COLOR(14,  0,  0,  0);
detail::Color ColorReddish      = MAKE_COLOR( 1,  0,  0, 50);
detail::Color ColorOrange       = MAKE_COLOR(21,  1,  0,  0);
detail::Color ColorGreen        = MAKE_COLOR( 0, 10,  0,  0);
detail::Color ColorLime         = MAKE_COLOR( 6,  4,  0,  0);
detail::Color ColorBlue         = MAKE_COLOR( 0,  0, 20,  0);
detail::Color ColorCyan         = MAKE_COLOR( 0, 12, 10,  0);
detail::Color ColorPink         = MAKE_COLOR(10,  0,  2,  0);
detail::Color ColorPurple       = MAKE_COLOR( 6,  0, 10,  0);
detail::Color ColorYellow       = MAKE_COLOR(10,  2,  0,  0);
detail::Color ColorWhite        = MAKE_COLOR(15,  4,  1,  0);
detail::Color ColorBlack        = MAKE_COLOR( 0,  0,  0, 99);
detail::Color ColorPerfectWhite = MAKE_COLOR(10, 10, 10,  0);
