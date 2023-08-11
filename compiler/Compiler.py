import asyncio
from aiopath import AsyncPath

try:
    from .wrapper import to_async
    from .config import LIBRARY_SOURCE_PATH, BUILD_DIRECTORY, LIBRARY_BINARY_PATH
except ImportError:
    from compiler.config import LIBRARY_SOURCE_PATH, BUILD_DIRECTORY, LIBRARY_BINARY_PATH


class CompilerResult:
    def __init__(self, _return_code: int, _output: str, _error: str):
        self.return_code = _return_code
        self.stdout = _output
        self.stderr = _error


class Compiler:
    c_default_libraries = ["qhsm"]
    
    supported_compilers = {"gcc": {
                                "extension": ["*\.c", "*\.cpp"],
                                "flags": ["-c", "-std=", "-Wall"]},
                           "g++": {
                                "extension": ["*.cpp", "*.c"],
                                "flags": ["-c", "-std=", "-Wall"]},
                           "arduino-cli": {"extension": ["ino"],
                                           "flags": ["-b", "avr:arduino:uno"]
                                           }}
    
    @staticmethod 
    def checkFlags(flags, compiler):
        # TODO
        pass
    
    @staticmethod
    async def getBuildFiles(libraries, compiler, directory):
        build_files = []
        for glob in Compiler.supported_compilers[compiler]["extension"]:
            async for file in AsyncPath(directory).glob(glob):
                build_files.append(file.name)
        
        match compiler:
            case "gcc" | "g++":
                for library in libraries:
                    build_files.append(''.join(["../", LIBRARY_BINARY_PATH, library, '.o']))
        
        return build_files
               
    @staticmethod
    async def compile(base_dir: str, build_files: list, flags: list, compiler: str) -> CompilerResult:
        match compiler:
            case "g++" | "gcc":
                await AsyncPath(base_dir + 'build/').mkdir(parents=True, exist_ok=True)
                flags.append("-o")
                flags.append("./build/a.out")
                process = await asyncio.create_subprocess_exec(compiler, *build_files, *flags, cwd=base_dir, text=False, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE)
            case "arduino-cli":
                process = await asyncio.create_subprocess_exec(compiler, "compile", "--export-binaries", *flags, *build_files, cwd=base_dir, text=False, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE)
        await process.wait()
        stdout, stderr = await process.communicate()

        return CompilerResult(process.returncode,
                              str(stdout.decode('utf-8')),
                              str(stderr.decode('utf-8')))

    @staticmethod
    async def includeLibraryFiles(libraries : list[str], target_directory : str, extension: str):
        paths_to_libs = [''.join([LIBRARY_SOURCE_PATH, library, extension]) for library in libraries]
        process = await asyncio.create_subprocess_exec("cp", *paths_to_libs, target_directory, cwd=BUILD_DIRECTORY)
