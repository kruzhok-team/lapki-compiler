"""Модуль содержит класс для разрешения зависимостей платформ."""
from typing import Any, List, Literal

from compiler.platform_manager import (
    PlatformManager,
    get_img_path,
    get_source_path
)
from compiler.types.platform_types import Component, Platform
from aiopath import AsyncPath


class DependencyError(Exception):
    """Ошибки при разрешении зависимостей платформы."""

    ...


class DependenciesResolver:
    """Класс для разрешения зависимостей и наследования платформы."""

    def __init__(self):
        self.platform_manager = PlatformManager()

    def get_resolved_component(self,
                               platform: Platform,
                               component_id: str
                               ) -> Component | None:
        """Получить компонент с учетом наследования."""
        component: Component | None = platform.components.get(component_id)

        if component is not None:
            return component

        platforms_meta = self.platform_manager.platforms_info.get(platform.id)

        if platforms_meta is None:
            return None

        for parent_platform in platforms_meta.dependencies:
            component = parent_platform.components.get(component_id)
            if component is not None:
                return component

        return None

    async def get_resolved_file(
        self,
        platform: Platform,
        filename: str,
        in_source: bool = True
    ) -> str | None:
        """Найти абсолютный путь до файла с учетом наследования."""
        dependencies = [
            platform,
            *self.platform_manager.versions_info[platform.id].dependencies
        ]
        for dep in dependencies:
            path = await AsyncPath(get_source_path(
                dep.id, dep.version) if in_source else get_img_path(
                    platform.id, platform.version
            )
            ).absolute()
            async for f in path.rglob(filename):
                return str(await f.absolute())

        return None

    def __resolve_lists(
            self,
            dependencies: List[Platform],
            field: Literal[
                'default_include_files',
                'default_build_files',
                'default_setup_functions']) -> List[Any]:
        """Собрать списки из родительских платформ в единый список."""
        result: List[Any] = []
        for dep in dependencies:
            attr_list: List[Any] = getattr(dep, field)
            for attr in attr_list:
                if attr not in result:
                    result.append(attr)

        return result

    async def get_resolved_default_includes(
        self,
        platform: Platform
    ) -> List[str]:
        """
        Собрать все уникальные инклюды в default_include_files\
            из родительских платформ в\
            единый список (включая свои инклюды).

        Инклюды идут от самых глубоких платформ к самым верхним.
        """
        dependencies = [
            platform, *
            self.platform_manager.versions_info[platform.id].dependencies
        ][::-1]

        return self.__resolve_lists(
            dependencies, 'default_include_files')

    async def get_resolved_default_build_files(
        self,
        platform: Platform
    ) -> List[str]:
        """
        Собрать все уникальные файлы проекта в default_build_files\
            из родительских платформ в\
            единый список (включая свои файлы проекта).

        Файлы проекта идут от самых глубоких платформ к самым верхним.
        """
        dependencies = [
            platform, *
            self.platform_manager.versions_info[platform.id].dependencies
        ][::-1]

        return self.__resolve_lists(
            dependencies, 'default_build_files')

    async def get_resolved_default_setup_functions(
        self,
        platform: Platform
    ) -> List[str]:
        """
        Собрать все уникальные вызовы в setup в default_setup_functions\
            из родительских платформ в\
            единый список (включая свои вызовы в setup).

        Вызовы идут от самых глубоких платформ к самым верхним.
        """
        dependencies = [
            platform, *
            self.platform_manager.versions_info[platform.id].dependencies
        ][::-1]

        return self.__resolve_lists(
            dependencies, 'default_setup_functions')

    def resolve_main_function(self, platform: Platform) -> bool:
        """Найти, используется ли main-функция\
            в какой-либо унаследованной платформе."""
        return any([
            platform.main_function,
            *[
                dep.main_function
                for dep in (self.platform_manager.
                            versions_info[platform.id].dependencies)
            ]
        ])

    def resolve_main_file_extension(self, platform: Platform) -> bool:
        """Найти разрешение генерируемого main-файла с учетом наследования."""
        ...

    async def resolve_dependencies(
        self, platform: Platform, _loaded_deps: list[Platform] | None = None
    ) -> list[Platform]:
        """
        Разрешить зависимости платформы.

        Рекурсивно идем по зависимостям платформ,
        передавая список загруженных платформ.
        _loaded_deps используется для передачи
        загруженных платформ по рекурсии.
        """
        loaded_deps: List[Platform] = (
            [*_loaded_deps] if _loaded_deps
            is not None else []
        )
        new_deps: List[Platform] = []
        for dep in platform.inherits:
            if dep.platform_id == platform.id:
                raise DependencyError(
                    f'Platfom ({platform.id}): dependency resolution error. '
                    'Inheriting from itself is forbidden.')
            dep_platform = await self.platform_manager.get_platform(
                dep.platform_id,
                dep.version
            )
            if dep_platform not in loaded_deps:
                new_deps.append(dep_platform)
                loaded_deps.append(dep_platform)
                resolved = await self.resolve_dependencies(
                    dep_platform,
                    loaded_deps
                )
                if platform.id in [resolved_platform.id for
                                   resolved_platform in resolved]:
                    raise DependencyError(
                        f'Platfom ({platform.id}): '
                        'dependency resolution error. '
                        'Circular inheritance.')
                new_deps.extend(resolved)
                loaded_deps.extend(resolved)
        return new_deps
