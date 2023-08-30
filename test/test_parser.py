import pytest
from compiler.CJsonParser import CJsonParser, Labels
from compiler.SourceFile import SourceFile
import json
pytest_plugins = ('pytest_asyncio',)


@pytest.mark.asyncio
async def test_getfiles():
    with open('examples/ExampleRequestSource.json', 'r') as f:
        data = json.load(f)
    
    files = await CJsonParser.getFiles(data["source"])
    assert files == [SourceFile("cpp_example", "cpp", "*code here*"), SourceFile("cpp_example", "hpp", "*code here*")]
    
# @pytest.mark.asyncio
# async def test_createUser():
#     userData = {"User": {
#                     "functions": {
#                         "printMyValue": {
#                             "returnType": "void",
#                             "args": {
#                                 "value": {
#                                     "type": "char[]"
#                                 }
#                             },
#                             "code": "Serial.println(value);"
#                         }
#                     },

#                     "variables": {
#                         "myVariable": {
#                             "type": "int",
#                             "value": "1000"
#                         }
#                      },

#                     "signals": ["mySignal"]
#                  }
#                 },
#     assertData = [
#                     Labels.USER_VAR_H.value + "\nstatic int myVariable;",
#                     Labels.USER_FUNC_H.value + "\nstatic void printMyValue(char[] value);",
#                     Labels.USER_VAR_C.value + "\nint User::myVariable = 1000;",
#                     Labels.USER_FUNC_C.value + "\nstatic void User::printMyValue(char[] value){\n\tSerial.println(value);\n}",
#     ]
    
#     assertSisgnals = ["mySignal"]
    
#     notes, signals = CJsonParser.createUserCode(userData)

#     assert notes == assertData
#     assert signals == assertSisgnals


@pytest.mark.parametrize("function, expected", 
                         [pytest.param(
                            {
                                    "printMyValue": {
                                        "returnType": "void",
                                        "args": {
                                            "value": {
                                                "type": "char[]"
                                            }
                                        },
                                        "code": "Serial.println(value);"
                                    }
                            }, 
                            ("\nstatic void printMyValue(char value[]);",
                             "\nvoid User::printMyValue(char value[]){\nSerial.println(value);\n}")),
                          pytest.param(
                            {
                                    "printMyValue": {
                                        "returnType": "void",
                                        "args": {
                                            "value": {
                                                "type": "char[]"
                                            }
                                        },
                                        "code": "Serial.println(value);"
                                    },
                                    "myFunction": {
                                        "returnType": "void",
                                        "args": {
                                            "value": {
                                                "type": "char[]"
                                            }
                                        },
                                        "code": "Serial.print(\"bla\");"
                                    },
                            }, 
                            ("\nstatic void printMyValue(char value[]);\n\nstatic void myFunction(char value[]);",
                            "\nvoid User::printMyValue(char value[]){\nSerial.println(value);\n}\n\nvoid User::myFunction(char value[]){\nSerial.print(\"bla\");\n}")),
                          pytest.param({}, ("", ""))]
                         )
def test_getUserFunctions(function, expected):
    result = CJsonParser.getUserFunctions(function)
    assert result == expected


@pytest.mark.parametrize("variables, expected", [
    pytest.param({
                    "myVariable": {
                        "type": "int",
                        "value": "1000"
                    }
                 }, ("\nstatic int myVariable;", "\nint User::myVariable = 1000;"))    
])
def test_getUserVariable(variables, expected):
    result = CJsonParser.getUserVariables(variables)
    assert result == expected
