import subprocess

class Compiler:
    #send source file to compiler
    #Доступные компиляторы сделать в виде Enum?
    @staticmethod
    async def compile(source_path : str, target_path : str, flags : list, compiler : str) -> str:
        output = subprocess.run([compiler, source_path, *flags, target_path], capture_output=True, text=True)
        
        return output
