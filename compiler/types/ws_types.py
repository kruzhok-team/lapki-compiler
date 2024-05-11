from typing import TypeAlias, Literal

Message: TypeAlias = Literal[
    'close',
    'berlogaImport',
    'arduino',
    'berlogaExport',
    'cgml',
    'add_platform',
    'remove_platform',
    'remove_platform_versions',
    'update_platform',
    'get_platform_json',
    'get_platform_images',
    'get_platform_sources',
    'auth'
]
