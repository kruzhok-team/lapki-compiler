#pragma once

#include "Picture.hpp"

// Список картинок
const Picture heart = { Pattern35{
    0, 0, 0, 0, 0,
    0, 100, 0, 100, 0,
    100, 100, 100, 100, 100,
    100, 100, 100, 100, 100,
    0, 100, 100, 100, 0,
    0, 0, 100, 0, 0,
    0, 0, 0, 0, 0,
} };

const Picture note = { Pattern35{
    0, 0, 0, 0, 0,
    0, 0, 100, 0, 0,
    0, 0, 100, 100, 0,
    0, 0, 100, 0, 100,
    100, 100, 100, 0, 0,
    100, 100, 100, 0, 0,
    0, 0, 0, 0, 0,
} };

const Picture smile = { Pattern35{
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 100, 0, 100, 0,
    0, 0, 0, 0, 0,
    100, 0, 0, 0, 100,
    0, 100, 100, 100, 0,
    0, 0, 0, 0, 0,
} };

const Picture sadness = { Pattern35{
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 100, 0, 100, 0,
    0, 0, 0, 0, 0,
    0, 100, 100, 100, 0,
    100, 0, 0, 0, 100,    
    0, 0, 0, 0, 0,
} };

const Picture cross = { Pattern35{
    0, 0, 0, 0, 0,
    100, 0, 0, 0, 100,
    0, 100, 0, 100, 0,
    0, 0, 100, 0, 0,
    0, 100, 0, 100, 0,
    100, 0, 0, 0, 100,
    0, 0, 0, 0, 0,
} };

const Picture square = { Pattern35{
    100, 100, 100, 100, 100,
    100, 0, 0, 0, 100,
    100, 0, 0, 0, 100,
    100, 0, 0, 0, 100,
    100, 0, 0, 0, 100,
    100, 0, 0, 0, 100,
    100, 100, 100, 100, 100,
} };

const Picture rhombus = { Pattern35{
    0, 0, 0, 0, 0,
    0, 0, 100, 0, 0,
    0, 100, 0, 100, 0,
    100, 0, 0, 0, 100,
    0, 100, 0, 100, 0,
    0, 0, 100, 0, 0,
    0, 0, 0, 0, 0,
} };