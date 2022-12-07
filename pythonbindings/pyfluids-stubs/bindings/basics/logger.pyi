from typing import Any, ClassVar

log: None

class Level:
    __members__: ClassVar[dict] = ...  # read-only
    INFO_HIGH: ClassVar[Level] = ...
    INFO_INTERMEDIATE: ClassVar[Level] = ...
    INFO_LOW: ClassVar[Level] = ...
    LOGGER_ERROR: ClassVar[Level] = ...
    WARNING: ClassVar[Level] = ...
    __entries: ClassVar[dict] = ...
    def __init__(self, arg0: int) -> None: ...
    def __eq__(self, arg0: object) -> bool: ...
    def __getstate__(self) -> int: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __int__(self) -> int: ...
    def __ne__(self, arg0: object) -> bool: ...
    def __setstate__(self, arg0: int) -> None: ...
    @property
    def name(self) -> str: ...

class Logger:
    def __init__(self, *args, **kwargs) -> None: ...
    @staticmethod
    def add_stdout() -> None: ...
    @staticmethod
    def enable_printed_rank_numbers() -> None: ...
    @staticmethod
    def set_debug_level(level: int) -> None: ...
    @staticmethod
    def time_stamp(time_stemp: TimeStamp) -> None: ...

class TimeStamp:
    __members__: ClassVar[dict] = ...  # read-only
    DISABLE: ClassVar[TimeStamp] = ...
    ENABLE: ClassVar[TimeStamp] = ...
    __entries: ClassVar[dict] = ...
    def __init__(self, arg0: int) -> None: ...
    def __eq__(self, arg0: object) -> bool: ...
    def __getstate__(self) -> int: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __int__(self) -> int: ...
    def __ne__(self, arg0: object) -> bool: ...
    def __setstate__(self, arg0: int) -> None: ...
    @property
    def name(self) -> str: ...
