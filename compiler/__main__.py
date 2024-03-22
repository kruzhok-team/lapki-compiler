"""Entry compiler's point."""
try:
    from .main import sync_main
except ImportError:
    from compiler.main import sync_main

if __name__ == '__main__':
    sync_main()
