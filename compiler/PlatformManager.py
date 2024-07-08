"""Module for managing platforms."""
import asyncio
import json
import os
import uuid
import traceback
from copy import deepcopy
from typing import (
    AsyncGenerator,
    Callable,
    Dict,
    Literal,
    List,
    Any,
    Optional,
    Set
)
from pprint import pprint

from aiofile import async_open
from aiopath import AsyncPath
from compiler.config import get_config
from compiler.types.inner_types import File
from compiler.types.platform_types import PlatformMeta, Platform

PlatformId = str
PlatformVersion = str


async def _write_source(path: str, source_files: List[File]) -> None:
    for source in source_files:
        filename = os.path.join(path, f'{source.filename}.{source.extension}')
        await AsyncPath(filename).parent.mkdir(parents=True, exist_ok=True)
        mode = 'wb' if isinstance(source.fileContent, bytes) else 'w'
        async with async_open(filename, mode) as f:
            await f.write(source.fileContent)


async def _delete_platform(platform_id: str) -> None:
    """Delete all sources of all versions. Doesn't delete anything in dict."""
    async def path_without_version(
            platform_id: str,
            func: Callable[[str, str], str]) -> AsyncPath:
        # Так как удаляем платформу полностью, нам не нужна версия
        # Поэтому оставляем пустое значение, и получаем путь/platform_id//
        # И убираем последний слэш
        return await AsyncPath(func(platform_id, '')[:-1]).parent.absolute()

    paths = [get_path_to_platform, get_source_path]
    for path_func in paths:
        path_to_platform = await path_without_version(
            platform_id,
            path_func
        )
        await asyncio.create_subprocess_exec('rm', '-r', path_to_platform)


def _get_img_path(id: str, version: str) -> str:
    """
    Get path to images dir.

    LIBRARY_PATH/id/version/img/
    """
    return os.path.join(_gen_platform_path(get_config().library_path,
                                           id,
                                           version),
                        'img/')


def get_source_path(id: str, version: str) -> str:
    """
    Get path to source dir.

    LIBRARY_PATH/id/version/source/
    """
    return os.path.join(_gen_platform_path(
        get_config().library_path,
        id,
        version), 'source/')


def get_path_to_platform(id: str, version: str) -> str:
    """
    Get path to platform JSON scheme.

    PLATFORM_DIRECTORY/id/version/id-version.json
    """
    return os.path.join(_gen_platform_path(
        get_config().platform_directory, id, version),
        f'{id}-{version}.json')


def _gen_platform_path(base_path: str, id: str, version: str) -> str:
    return os.path.join(base_path, id, version)


async def _read_platform_files(
        source_dir: str,
        mode: Literal['rb', 'r']) -> AsyncGenerator[File, None]:
    async for source in AsyncPath(source_dir).rglob('*'):
        if not await source.is_file():
            continue
        async with async_open(source, mode) as f:
            content = await f.read()
            # Убираем точку в начале .suff.suff
            extension = ''.join(source.suffixes).replace('.', '', 1)
            filename = str(source.relative_to(source_dir)).split('.')[0]
            yield File(filename=filename,
                       extension=extension,
                       fileContent=content)


class PlatformException(Exception):
    """Error during process platforms."""

    ...


class PlatformManager:
    """
    Класс-синглтон, отвечающий за загрузку платформ.

    TODO: А также их удаление из памяти, если их не используют
    какое-то время.
    TODO: Фикс словаря __platforms: в данный момент в качестве ключа
    используется идентификатор платформы, но надо:
    <id платформы>-<версия платформы>
    """

    _instance: Optional['PlatformManager'] = None
    _initialized: bool = False

    def __init__(self) -> None:
        if not self._initialized:
            self.__platforms: Dict[str, Platform] = {}
            self.__versions_info: Dict[PlatformId, PlatformMeta] = {}
            self._initialized = True

    def __new__(cls, *args, **kwargs) -> 'PlatformManager':
        """
        Класс-синглтон, отвечающий за загрузку платформ.

        Всегда возращает один инстанс.
        """
        if cls._instance is None:
            cls._instance = super().__new__(cls, *args, **kwargs)
        return cls._instance

    @property
    def versions_info(self):
        """Information about platform's versions."""
        return deepcopy(self.__versions_info)

    def gen_platform_id(self) -> str:
        """Generate platform id."""
        return uuid.uuid4().hex

    def set_platforms_info(
            self,
            new_value: Dict[PlatformId, PlatformMeta]) -> None:
        """Set versions_info."""
        self.__versions_info = new_value

    async def add_platform(self,
                           platform: Platform,
                           source_files: List[File],
                           images: List[File] | None = None
                           ) -> Dict[PlatformId, PlatformMeta]:
        """
        Add platform's\
            info to versions_info.

        Raise PlatformException if platform is already exist.
        """
        if self.platform_exist(platform.id):
            raise PlatformException(
                f'Platform with id {platform.id} is already exist.')
        await self._save_platform(platform, source_files, images)
        new_versions_info = deepcopy(self.__versions_info)
        new_versions_info[platform.id] = PlatformMeta(
            versions=set((platform.version,)),
            name=platform.name,
            author=platform.author)

        return new_versions_info

    async def _save_platform(self,
                             platform: Platform,
                             source_files: List[File],
                             images: List[File] | None = None) -> None:
        """
        Save platform to folder.

        Create:
        - platform source folder
        - platform image folder
        - platform raw json scheme
        """
        config = get_config()
        platform_path = _gen_platform_path(
            config.platform_directory, platform.id, platform.version)
        platform_library_path = _gen_platform_path(
            config.library_path, platform.id, platform.version)
        source_path = get_source_path(platform.id, platform.version)
        img_path = _get_img_path(platform.id, platform.version)
        json_platform = platform.model_dump_json(indent=4)
        await AsyncPath(platform_path).mkdir(parents=True, exist_ok=False)
        await AsyncPath(platform_library_path).mkdir(parents=True,
                                                     exist_ok=False)
        await AsyncPath(source_path).mkdir(exist_ok=False)
        await AsyncPath(img_path).mkdir(exist_ok=False)
        await _write_source(platform_path, [
            File(filename=f'{platform.id}-{platform.version}',
                 extension='json',
                 fileContent=json_platform)])
        await _write_source(source_path, source_files)

        if images is not None:
            await _write_source(img_path, images)

    async def load_platform(self,
                            path_to_platform: str | AsyncPath) -> Platform:
        """Load platform from file and add it to dict."""
        try:
            async with async_open(path_to_platform, 'r') as f:
                unprocessed_platform_data: str = await f.read()
                platform = Platform(
                    **json.loads(unprocessed_platform_data))
                if self.__platforms.get(platform.id, None) is None:
                    self.__platforms[platform.id] = platform
                else:
                    raise PlatformException(
                        f'Platform with id {platform.id} is already exists.')
                return platform
        except Exception as e:
            raise PlatformException(
                f'Во время обработки файла "{path_to_platform}"'
                f'произошла ошибка! {e}')

    async def init_platforms(self, path_to_schemas_dir: str) -> None:
        """Find platforms in directory and add it to Dict."""
        print(f'Поиск схем в папке "{path_to_schemas_dir}"...')
        async for path in AsyncPath(path_to_schemas_dir).rglob('*json'):
            try:
                async with async_open(path, 'r') as f:
                    unprocessed_platform_data: str = await f.read()
                    platform = Platform(
                        **json.loads(unprocessed_platform_data))
                    id = platform.id
                    if self.platform_exist(id):
                        versions = (
                            self.__versions_info[id].versions)
                        versions.add(
                            platform.version)
                    else:
                        self.__versions_info[id] = PlatformMeta(
                            versions=set((platform.version,)),
                            name=platform.name,
                            author=platform.author)
            except Exception:
                print(
                    f'Во время обработки файла "{await path.absolute()}"'
                    f'произошла ошибка!' + '\n' + traceback.format_exc())

        print('Были найдены платформы:')
        pprint(dict(self.__versions_info), indent=3)

    async def get_platform(self, platform_id: str, version: str) -> Platform:
        """Get platform object by id."""
        platform: Platform | None = self.__platforms.get(platform_id)

        if platform is not None:
            return platform

        if (version not in
                self.__versions_info[platform_id].versions):
            raise PlatformException(
                f'Unsupported platform {platform_id}, version {version}')

        return await self.load_platform(
            get_path_to_platform(platform_id, version))

    async def get_raw_platform_scheme(self,
                                      platform_id: str,
                                      version: str) -> str:
        """
        Get raw platform JSON scheme.

        If platform wasn't loaded, will load it.

        Raise PlatformException if platform or version doesn't exist.
        """
        platform: Platform | None = self.__platforms.get(platform_id)

        if platform is not None:
            return platform.model_dump_json(indent=4)

        if not self.has_version(platform_id, version):
            raise PlatformException(f'Unsupported platform {platform_id}')

        path_to_platform = get_path_to_platform(platform_id, version)
        async with async_open(path_to_platform, 'r') as f:
            return await f.read()

    def platform_exist(self, platform_id: str) -> bool:
        """Check that platform exist."""
        return (platform_id in self.__versions_info.keys() or
                platform_id in self.__platforms.keys())

    def has_version(self, platform_id: str, version: str) -> bool:
        """Check, that platform has received version."""
        platform = self.__versions_info[platform_id]
        return (
            version in
            platform.versions
        )

    async def update_platform(
        self,
        platform: Platform,
        source_files: List[File],
        images: List[File]
    ) -> Dict[PlatformId, PlatformMeta]:
        """
        Update platform.

        Raise PlatformException if platform doesn't exist or
        platform's version is already exist.
        """
        if not self.platform_exist(platform.id):
            raise PlatformException(
                f'Platform with id {platform.id} doesnt exist.')
        if self.has_version(platform.id, platform.version):
            raise PlatformException(
                f'Platform with id {platform} '
                f'already has version {platform.version}'
            )
        await self._save_platform(platform, source_files, images)

        new_versions_info = deepcopy(
            self.__versions_info)
        new_versions_info[platform.id].versions.add(platform.version)
        new_versions_info[platform.id].author = platform.author
        new_versions_info[platform.id].name = platform.name

        return new_versions_info

    async def get_platform_sources(
            self,
            platform_id: str,
            version: str) -> AsyncGenerator[File, Any]:
        """Get platform source-code files by id and version."""
        source_dir = get_source_path(platform_id, version)
        return _read_platform_files(source_dir, 'r')

    async def get_platform_images(
            self,
            platform_id: str,
            version: str) -> AsyncGenerator[File, Any]:
        """Get platform images by id and version."""
        source_dir = _get_img_path(platform_id, version)
        return _read_platform_files(source_dir, 'rb')

    async def delete_platform_by_versions(
        self,
        platform_id: str,
        versions: Set[str]
    ) -> Dict[PlatformId, PlatformMeta]:
        """
        Delete platform versions.

        If the platform does not have any versions\
            after this operation, it will be deleted

        Raise PlatformException, if platform or version doesn't exist
        """
        new_versions_info = deepcopy(self.__versions_info)
        if not self.platform_exist(platform_id):
            raise PlatformException(
                f'Platform with id {platform_id} doesnt exist.')

        for version in versions:
            if not self.has_version(platform_id, version):
                raise PlatformException(
                    f'Platform with id {platform_id} and '
                    f'version {version} doesnt exist.')
        versions_info = new_versions_info[platform_id].versions
        for version in versions:
            json_scheme_folder = await AsyncPath(
                get_path_to_platform(platform_id, version)).parent.absolute()
            await asyncio.create_subprocess_exec('rm',
                                                 '-r',
                                                 json_scheme_folder
                                                 )
            library_folder = await AsyncPath(
                get_source_path(platform_id, version)).parent.absolute()
            await asyncio.create_subprocess_exec('rm', '-r', library_folder)
            versions_info.remove(version)

        if len(versions_info) == 0:
            await _delete_platform(platform_id)
            del new_versions_info[platform_id]
            if self.__platforms.get(platform_id, None) is not None:
                del self.__platforms[platform_id]

        return new_versions_info

    async def delete_platform(
            self,
            platform_id: str
    ) -> Dict[PlatformId, PlatformMeta]:
        """
        Delete full platform by id.

        Raise PlatformException, if platform doesn't exist.
        """
        if not self.platform_exist(platform_id):
            raise PlatformException(f'Platform {platform_id} doesnt exist.')
        await _delete_platform(platform_id)
        new_versions_info = deepcopy(self.__versions_info)
        del new_versions_info[platform_id]
        if self.__platforms.get(platform_id, None) is not None:
            del self.__platforms[platform_id]
        return new_versions_info
