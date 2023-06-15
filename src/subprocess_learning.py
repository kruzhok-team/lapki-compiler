import subprocess

flags = ["-o"]
source = "cpp_example.cpp"
target = "cpp_example"

result = subprocess.run(["gcc", source, *flags, target], capture_output=True, text=True)

print(result.stdout)
print("-------------------------")
print(result.returncode)
print("-------------------------")
print(result.stderr)

subprocess.run([f"./{target}"])