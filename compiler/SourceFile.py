class SourceFile:
    def __init__(self, _filename: str, _format: str, _content: str) -> None:
        self.name = _filename
        self.extension = _format
        self.content = _content

    def __repr__(self) -> str:
        return f'File: {self.name}.{self.extension}, {self.content}'

    def __str__(self) -> str:
        return f'File: {self.name}.{self.extension}, {self.content}'

    def __eq__(self, __value: 'SourceFile') -> bool:
        return self.name == __value.name \
            and self.extension == __value.extension \
            and self.content == __value.content
