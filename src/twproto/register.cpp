#include <base/system.h>
#include "stdlib.h"

int main(int argc, const char **argv) // ignore_convention
{
    dbg_logger_stdout();
    char aPath[256];
    char aPathEscaped[512];
    str_copy(aPath, argv[0], sizeof(aPath));
    char *pBackSlash = (char *)str_find_rev(aPath, "\\");
    if (pBackSlash)
        pBackSlash[0] = 0;

    int x = 0;
    for (int i = 0; i < str_length(aPath); i++)
    {
        aPathEscaped[x++] = aPath[i];
        if (aPath[i] == '\\')
            aPathEscaped[x++] = aPath[i];
    }


    dbg_msg("", "Generating registry information");

    IOHANDLE File = io_open("tw.reg", IOFLAG_WRITE);

    io_write(File, "Windows Registry Editor Version 5.00\n", str_length("Windows Registry Editor Version 5.00\n"));

    io_write(File, "[HKEY_CLASSES_ROOT\\tw]\n", str_length("[HKEY_CLASSES_ROOT\\tw]\n"));

    io_write(File, "\"URL Protocol\"=\"\"\n", str_length("\"URL Protocol\"=\"\"\n"));

    io_write(File, "@=\"URL:Teeworlds Protocol\"\n", str_length("@=\"URL:Teeworlds Protocol\"\n"));

    io_write(File, "[HKEY_CLASSES_ROOT\\tw\\DefaultIcon]\n", str_length("[HKEY_CLASSES_ROOT\\tw\\DefaultIcon]\n"));

    io_write(File, "@=\"", str_length("@=\""));
    io_write(File, aPathEscaped, str_length(aPathEscaped));
    io_write(File, "n-client.exe", str_length("n-client.exe"));
    io_write(File, "\"\n", str_length("\"\n"));

    io_write(File, "[HKEY_CLASSES_ROOT\\tw\\shell]\n", str_length("[HKEY_CLASSES_ROOT\\tw\\shell]\n"));

    io_write(File, "@=\"open\"\n", str_length("@=\"open\"\n"));

    io_write(File, "[HKEY_CLASSES_ROOT\\tw\\shell\\open]\n", str_length("[HKEY_CLASSES_ROOT\\tw\\shell\\open]\n"));

    io_write(File, "[HKEY_CLASSES_ROOT\\tw\\shell\\open\\command]\n", str_length("[HKEY_CLASSES_ROOT\\tw\\shell\\open\\command]\n"));

    io_write(File, "@=\"", str_length("@=\""));
    io_write(File, aPathEscaped, str_length(aPathEscaped));
    io_write(File, "tw_proto_start.bat %1\"\n", str_length("tw_proto_start.bat %1\"\n"));

    io_close(File);

    dbg_msg("", "Registering protocol");
    int i = system("tw.reg");
    if (i)
        dbg_msg("", "Errorcode: %i");
    dbg_msg("", "Cleaning up");
    fs_remove("tw.reg");

    return 0;
}
