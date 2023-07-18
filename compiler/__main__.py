try:
    from .main import sync_main
except ImportError:
    from compiler.main import sync_main

sync_main()
