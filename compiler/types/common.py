"""Module contains common geometry classes."""
from dataclasses import dataclass
from typing import TypeAlias


@dataclass
class Point:
    """Point data class."""

    x: float
    y: float


@dataclass
class Rectangle:
    """Rectangle data class."""

    x: float
    y: float
    width: float
    height: float


Key: TypeAlias = str
