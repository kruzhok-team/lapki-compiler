import json

class JsonParser:
    @staticmethod
    async def parse(request):
        data = await request.json()
        
        try:
            return data
        except KeyError:
            return "Invalid request"
        