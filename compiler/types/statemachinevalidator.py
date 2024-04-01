# """Module implements validation Lapki IDE's scheme."""
# from typing import Dict

# try:
#     from .ide_types import (
#         IdeStateMachine,
#         Component,
#         Argument
#     )
#     from .platform_types import Platform
# except ImportError:
#     from compiler.types.ide_types import (
#         IdeStateMachine,
#         Component,
#         Argument
#     )
#     from compiler.types.platform_types import Platform


# class StateMachineValidatorException(Exception):
#     """Validation error."""

#     ...


# class StateMachineValidator:
#     """
#     Класс для валидации схемы формата Lapki IDE.

#     TODO:
#     - Проверка аргументов методов
#     - Проверка параметров компонентов
#     - Проверка типов компонентов
#     - Проверка методов (что вызывается метод, который прописан в платформе)
#     - Проверка указанной платформы на наличие
#     """

#     def __init__(self, data: IdeStateMachine, platform: Platform) -> None:
#         self.data = data
#         self.platform = platform

#     def _validateStates(self) -> bool:
#         ...

#     def _validateTransitions(self) -> bool:
#         for transition in self.data.transitions:
#             try:
#                 if (self.data.states[transition.source] and
#                         self.data.states[transition.target]):
#                     ...
#             except KeyError:
#                 raise StateMachineValidatorException(
#                     'Transition validate error!'
#                     f'Unknown source {transition.source}!'
#                 )

#         return True

#     def _validateComponents(self, components: Dict[str, Component]) -> bool:
#         """Функция проверяет соответствие компонентов указанной платформе."""
#         component_types = list(self.platform.components.keys())
#         for component_id in self.data.components:
#             component = self.data.components[component_id]
#             if component.type in component_types:
#                 parameters = list(
#                     self.platform.components[component.type].parameters.keys())
#                 for parameter_id in component.parameters:
#                     if parameter_id in parameters:
#                         continue
#                     raise StateMachineValidatorException(
#                         f'Component({component_id}):'
#                         f'unknown parameter {parameter_id}'
#                         f'in platform {self.platform.name}'
#                     )
#             raise StateMachineValidatorException(
#                 f'Component({component_id}):'
#                 f'unknown component {component.type}'
#                 f'in platform {self.platform.name}'
#             )

#         return True

#     def _validateArgs(
#             self,
#             component: Component,
#             method: str,
#             args: Dict[str, Argument | str]) -> bool:
#         ...

#     def validate(self) -> bool:
#         """Validate scheme."""
#         return self._validateComponents(self.data.components)
