import importlib
import json

def parseSRCToJson(refcode: str,
                   word2: str, word3: str, word4: str, word5: str,
                   word6: str, word7: str, word8: str, word9: str) -> str:
    """
    SRC parser for BMC generated PELs.

    This returns a string containing formatted JSON data. The data is simply
    appended to the end of the "Primary SRC" section of the PEL and will not
    impact any other fields in that section.

    IMPORTANT:
    This function is simply a wrapper for component SRC parsers. To define a
    parser for a component, create a module in the same directory as this module
    with a file name in the format of `oxx00.py`, where `xx` is the component ID
    in lower case (example: oe500.py). Then add this same function definition to
    the new module.
    """

    # Need to search for the SRC parser module for this component. The component
    # ID can be pulled from the third byte of the reference code.
    module_name = 'srcparsers.osrc.o' + refcode[4:6].lower() + '00'

    try:
        # Grab the module of it exist. If not, it will throw an exception.
        module = importlib.import_module(module_name)

    except ModuleNotFoundError:
        # The module does not exist. No need to parse the SRC.
        # TODO: Yes, it is intentional to use 'json.dumps()' here for now.
        #       Simply returning an empty string will result in a peltool
        #       exception for "Bad JSON from parser". The json dump instead
        #       returns the string '""', which appends '"SRC Details": ""' to
        #       the Primary SRC
        out = json.dumps("")

    else:
        # The module was found. Call the component parser in that module.
        out = module.parseSRCToJson(refcode,
                                    word2, word3, word4, word5,
                                    word6, word7, word8, word9)

    return out

