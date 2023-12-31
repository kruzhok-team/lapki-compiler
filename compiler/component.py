from dataclasses import dataclass

@dataclass
class Component:
    name: str
    type: str
    parameters: dict
    # signals: dict
    # variables: dict[str, None]
    # methods: dict[str, dict]

    def __repr__(self) -> str:
        return f"Component(name: {self.name}, type: {self.type}, params: {self.parameters})"

    def __str__(self) -> str:
        return f"Component(name: {self.name}, type: {self.type}, params: {self.parameters})"
