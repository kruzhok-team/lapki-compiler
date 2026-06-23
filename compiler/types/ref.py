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
    art: str = ''

    @classmethod
    def from_dict(cls, refs: Dict[str, Any]) -> 'BlgMbRefs':
        return cls(art=refs.get('art', ''))


_REVISION_TO_DEFINE: Dict[str, str] = {
    'blg-mb-1-a12': 'BLG_MB_REVISION_A12',
    'blg-mb-1-b2':  'BLG_MB_REVISION_B2',
}

_COMPILERS_ACCEPTING_DEFINES: Set[str] = {
    'gcc', 'g++', 'arm-none-eabi-g++', 'arduino-cli'
}


def blg_mb_revision(refs: Dict[str, Any]) -> RefHandlerResult:
    """Map art: field from CyberBearLoader identify to a C #define flag."""
    board = BlgMbRefs.from_dict(refs)
    define = _REVISION_TO_DEFINE.get(board.art)
    if not define:
        return RefHandlerResult()
    extra_flags = {cmd: [f'-D{define}'] for cmd in _COMPILERS_ACCEPTING_DEFINES}
    return RefHandlerResult(extra_flags=extra_flags)


REF_HANDLERS: Dict[str, Callable[[Dict[str, Any]], RefHandlerResult]] = {
    'blg_mb_revision': blg_mb_revision,
}
