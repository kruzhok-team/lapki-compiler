import subprocess
from aioshutil import make_archive
from pathlib import Path
class CompilerResult:
    def __init__(self, _return_code : int, _output : str, _error : str, _path : str):
        self.return_code = _return_code
        self.stdout = _output
        self.stderr = _error
        self.binary_path = _path
        
class Compiler:
    supported_compilers = {"gcc" : "c", 
                           "g++" : "cpp", 
                           "arduino-cli": "ino"}

    
    @staticmethod
    async def compile(source_directory_path : str, filename : str, flags : list, compiler : str) -> CompilerResult: 
        source_file_path = ''.join([source_directory_path, filename, '.', Compiler.supported_compilers[compiler]])
        build_directory = source_directory_path + "build/"
        Path(build_directory).mkdir(parents=True, exist_ok=True)
        match compiler:
            case "gcc" | "g++":
                output = subprocess.run([compiler, source_file_path, *flags, build_directory + filename], capture_output=True, text=True)
            case "arduino-cli":
                output = subprocess.run([compiler, "compile", *flags, source_file_path], capture_output=True, text=True)
        if output.returncode == 0:
            path = source_directory_path + filename + ".zip"
            await make_archive(source_directory_path + filename, "zip", source_directory_path)
        else:
            path = ""

        return CompilerResult(output.returncode, output.stdout, output.stderr, path)
