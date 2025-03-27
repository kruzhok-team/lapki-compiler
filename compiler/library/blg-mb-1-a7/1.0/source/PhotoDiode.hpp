#pragma once

namespace detail {

    namespace PtohotoDiode {

        bool isInit = false;
    }
}

class PhotoDiode {

public:

    uint16_t value;

    PhotoDiode() {

        if (!detail::PtohotoDiode::isInit) {

            mrx::hal::photoDiode::init();

            mrx::hal::photoDiode::start();

            detail::PtohotoDiode::isInit = true;
        }
    }

    // call this function in loop()
    void scan() {

        value = mrx::hal::photoDiode::getSense();
    }
};