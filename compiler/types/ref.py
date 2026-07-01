"""Ref handler types and registry for platform-specific board data processing."""
from dataclasses import dataclass, field
from typing import Any, Callable, Dict, List, Set

from compiler.fullgraphmlparser.stateclasses import ParserNote


@dataclass
class RefHandlerResult:
    """Result returned by a ref handler function."""
    extra_flags: Dict[str, List[str]] = field(default_factory=dict)
    code_insertions: List[ParserNote] = field(default_factory=list)


@dataclass
class BlgMbRefs:
    """Typed refs for the КиберМишка (blg-mb) board."""
    hardware_ref: str = ''

    @classmethod
    def from_dict(cls, refs: Dict[str, Any]) -> 'BlgMbRefs':
        return cls(hardware_ref=refs.get('hardware_ref', ''))


# Hardware ref IDs from CyberBearLoader `identify` → C preprocessor define.
# Source: refs.md (Hardware section).
_HARDWARE_REF_TO_DEFINE: Dict[str, str] = {
    '08c1d6bb4bf0433f': 'BLG_MB_REVISION_A12',  # blg-mb-1-a12 rev2
    '963dbd892b0c33e0': 'BLG_MB_REVISION_A12',  # blg-mb-1-a12 rev3
    'aed0abf069a5effe': 'BLG_MB_REVISION_A12',  # blg-mb-1-a12 rev4
    'dabf4fe83914a2f3': 'BLG_MB_REVISION_A12',  # blg-mb-1-a12 rev5
    '9c0f9b027b6df847': 'BLG_MB_REVISION_B2',   # blg-mb-1-b2 rev1
}

_COMPILERS_ACCEPTING_DEFINES: Set[str] = {
    'gcc', 'g++', 'arm-none-eabi-g++', 'arduino-cli'
}


def blg_mb_revision(refs: Dict[str, Any]) -> RefHandlerResult:
    """Map Hardware ref ID from CyberBearLoader identify to a C #define flag."""
    board = BlgMbRefs.from_dict(refs)
    define = _HARDWARE_REF_TO_DEFINE.get(board.hardware_ref)
    print(f"{board} | {define}")
    # exit()
    if not define:
        return RefHandlerResult()
    extra_flags = {cmd: [f'-D{define}'] for cmd in _COMPILERS_ACCEPTING_DEFINES}
    return RefHandlerResult(extra_flags=extra_flags)


REF_HANDLERS: Dict[str, Callable[[Dict[str, Any]], RefHandlerResult]] = {
    'blg_mb_revision': blg_mb_revision,
}
