from SourceFile import SourceFile
import asyncio

class CJsonParser:
    @staticmethod
    async def parseStateMachine(request):
        data = await request.json()
        
        try:
            return data
        except KeyError:
            return "Invalid request"

    @staticmethod
    async def getFiles(json_data):
        files = []
        
        for data in json_data:
            files.append(SourceFile(data["filename"], data["extension"], data["fileContent"]))
        
        return files
    
    