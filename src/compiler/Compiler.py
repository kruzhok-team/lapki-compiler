import subprocess
from pathlib import Path
from RequestError import RequestError
import asyncio
from config import LIBRARY_SOURCE_PATH, BASE_DIRECTORY, LIBRARY_BINARY_PATH
class CompilerResult:
    def __init__(self, _return_code : int, _output : str, _error : str):
        self.return_code = _return_code
        self.stdout = _output
        self.stderr = _error
        
class Compiler:
    c_default_libraries = ["qhsm"]
    
    supported_compilers = {"gcc" : {
                                "extension" : ["*\.c", "*\.cpp"],
                                "flags" : ["-c", "-std=", "-Wall"]}, 
                           "g++" : {
                                "extension" : ["*.c", "*.cpp"],
                                "flags" : ["-c", "-std=", "-Wall"]}, 
                           "arduino-cli": {"extension": "ino",
                                           "flags": ["-b", "avr:arduino:uno"]
                                           }}
    #TODO
    @staticmethod 
    def checkFlags(flags, compiler):
        pass
    
    @staticmethod
    async def getBuildFiles(libraries, compiler, directory):
        build_files = []
        for glob in Compiler.supported_compilers[compiler]["extension"]:
            for file in Path(directory).glob(glob):
                build_files.append(file.name)
        
        for library in libraries:
            build_files.append(''.join(["../",LIBRARY_BINARY_PATH, library, '.o']))
        
        return build_files
        
        
    @staticmethod
    async def compile(base_dir : str, build_files : list, flags : list, compiler : str) -> CompilerResult:
        match compiler:
            case "g++" | "gcc":
                Path(base_dir + 'build/').mkdir(parents=True, exist_ok=True)
                flags.append("-o")
                flags.append("./build/a.out")
                result = subprocess.run([compiler, *build_files, *flags], cwd=base_dir, capture_output=True, text=True) 
            case "arduino-cli":
                print([compiler, "compile", *flags, *build_files])
                result = subprocess.run([compiler, "compile", *flags, *build_files], cwd=base_dir, capture_output=True, text=True) 
                print(result.stderr)
        return CompilerResult(result.returncode, result.stdout, result.stderr)

    @staticmethod
    async def includeHFiles(libraries : list[str], target_directory : str):
        libraries.append(*Compiler.c_default_libraries)
        paths_to_libs = [''.join([LIBRARY_SOURCE_PATH, library, '.h']) for library in libraries]
        process = await asyncio.create_subprocess_exec("cp", *paths_to_libs, target_directory, cwd=BASE_DIRECTORY)
        stdout, stderr  = await process.communicate()
        retcode = process.returncode
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
