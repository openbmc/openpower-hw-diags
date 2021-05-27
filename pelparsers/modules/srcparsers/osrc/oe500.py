import json
from collections import OrderedDict

def parseSRCToJson(refcode: str,
                   word2: str, word3: str, word4: str, word5: str,
                   word6: str, word7: str, word8: str, word9: str) -> str:
    """
    SRC Parser for openpower-hw-diags analyzer component.
    """

    out = OrderedDict()

    out["SRC Details"] = "TBD"

    return json.dumps(out)

