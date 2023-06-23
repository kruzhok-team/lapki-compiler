from SourceFile import SourceFile
import asyncio

class JsonParser:
    @staticmethod
    async def parse(request):
        data = await request.json()
        
        try:
            return data
        except KeyError:
            return "Invalid request"

    @staticmethod
    async def getFiles(json_data):
        files = []
        source = json_data["source"]
        
        for data in source:
            files.append(SourceFile(data["filename"], data["extension"], data["fileContent"]))
        
        return files