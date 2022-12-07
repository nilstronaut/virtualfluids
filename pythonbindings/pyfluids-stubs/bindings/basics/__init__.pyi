from typing import ClassVar

from typing import overload

class ConfigurationFile:
    def __init__(self) -> None: ...
    def contains(self, key: str) -> bool: ...
    @overload
    def get_bool_value(self, key: str) -> bool: ...
    @overload
    def get_bool_value(self, key: str, default_value: bool) -> bool: ...
    @overload
    def get_double_value(self, key: str) -> float: ...
    @overload
    def get_double_value(self, key: str, default_value: float) -> float: ...
    @overload
    def get_float_value(self, key: str) -> float: ...
    @overload
    def get_float_value(self, key: str, default_value: float) -> float: ...
    @overload
    def get_int_value(self, key: str) -> int: ...
    @overload
    def get_int_value(self, key: str, default_value: int) -> int: ...
    @overload
    def get_string_value(self, key: str) -> str: ...
    @overload
    def get_string_value(self, key: str, default_value: str) -> str: ...
    @overload
    def get_uint_value(self, key: str) -> int: ...
    @overload
    def get_uint_value(self, key: str, default_value: int) -> int: ...
    def load(self, file: str) -> bool: ...

class LbmOrGks:
    __members__: ClassVar[dict] = ...  # read-only
    GKS: ClassVar[LbmOrGks] = ...
    LBM: ClassVar[LbmOrGks] = ...
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
