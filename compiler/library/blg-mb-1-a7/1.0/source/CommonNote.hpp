#pragma once

// Эта структура должна быть эквивалентной струтктуре Sound
// В противном случае нужно поменять код компонента SpeakerNote
struct Note {

    uint16_t* sound{};
    uint32_t size{};

    uint16_t prescaler{1};
};