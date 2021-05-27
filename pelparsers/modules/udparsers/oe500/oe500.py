import json
from collections import OrderedDict
from pel.utils import hexdump

def parse_signature_list(version: int, stream: memoryview) -> OrderedDict:
    """
    Parser for the signature list.
    """

    out = OrderedDict()

    out["Warning"] = "User data parser TBD"

    return out


def parse_register_dump(version: int, stream: memoryview) -> OrderedDict:
    """
    Parser for the register dump.
    """

    out = OrderedDict()

    out["Warning"] = "User data parser TBD"

    return out


def parse_guard_list(version: int, stream: memoryview) -> OrderedDict:
    """
    Parser for the guard list.
    """

    out = OrderedDict()

    out["Warning"] = "User data parser TBD"

    return out


def parse_default(version: int, stream: memoryview) -> OrderedDict:
    """
    Default parser for user data sections that are not currently supported.
    """

    out = OrderedDict()

    out["Warning"] = "Unsupported user data type"

    return out


def parseUDToJson(subtype: int, version: int, data: memoryview) -> str:
    """
    Default function required by all component PEL user data parsers.
    """

    # Determine which parser to use.
    parsers = {
        1: parse_signature_list,
        2: parse_register_dump,
        3: parse_guard_list,
    }
    subtype_func = parsers.get(subtype, parse_default)

    # Get the parsed output of this data.
    out = subtype_func(version, data)

    # Include hex dump for debug.
    out["Hex Dump"] = hexdump(data)

    # Convert to JSON format and dump to a string.
    return json.dumps(out)

