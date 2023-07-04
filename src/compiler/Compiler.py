import subprocess
from pathlib import Path
from RequestError import RequestError
import asyncio
from config import LIBRARY_SOURCE_PATH, BASE_DIRECTORY
class CompilerResult:
    def __init__(self, _return_code : int, _output : str, _error : str, _path : str):
        self.return_code = _return_code
        self.stdout = _output
        self.stderr = _error
        self.binary_path = _path
        
class Compiler:
    c_default_libraries = ["qhsm.h"]
    
    cpp_main_function = "int main(){\
                        \t\t} %s"
    
    supported_compilers = {"gcc" : {
                                "extension" : "c",
                                "flags" : ["-std=", "-Wall"]}, 
                           "g++" : "cpp", 
                           "arduino-cli": "ino"}

    @staticmethod 
    def checkFlags(flags, compiler):
        pass
    
    @staticmethod
    def getBuildFiles(libraries, compiler):
        pass
    
    @staticmethod
    async def compile(base_dir : str, flags : list, compiler : str) -> CompilerResult:
        match compiler:
            case "g++" | "gcc":
                Path(base_dir + 'build/').mkdir(parents=True, exist_ok=True)
                flags.append("-o")
                flags.append("./build/a.out")
        result = subprocess.run([compiler, *flags], cwd=base_dir, capture_output=True, text=True) 
        return CompilerResult(result.returncode, result.stdout, result.stderr, base_dir + "build/")

    @staticmethod
    async def includeHFiles(libraries : list[str], target_directory : str):
        libraries.append("qhsm.h")
        paths_to_libs = [''.join([LIBRARY_SOURCE_PATH, library]) for library in libraries]
        await asyncio.create_subprocess_exec("cp", *paths_to_libs, target_directory, cwd=BASE_DIRECTORY)
        # data, _ = await process.communicate()
    # @staticmethod
    # async def compile(source_directory_path : str, filename : str, flags : list, compiler : str) -> CompilerResult: 
    #     source_file_path = ''.join([source_directory_path, filename, '.', Compiler.supported_compilers[compiler]])
    #     build_directory = source_directory_path + "build/"
    #     Path(build_directory).mkdir(parents=True, exist_ok=True)
    #     match compiler:
    #         case "gcc" | "g++":
    #             output = subprocess.run([compiler, source_file_path, *flags, build_directory + filename], capture_output=True, text=True)
    #         case "arduino-cli":
    #             output = subprocess.run([compiler, "compile", *flags, source_file_path], capture_output=True, text=True)
    #     if output.returncode == 0:
    #         path = source_directory_path + filename + ".zip"
    #         await make_archive(source_directory_path + filename, "zip", source_directory_path)
    #     else:
    #         path = ""

    #     return CompilerResult(output.returncode, output.stdout, output.stderr, path)
