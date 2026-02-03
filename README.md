# Tail for Windows (MSVC Port)

This is a Windows/MSVC port of GNU `tail` with a tail-lite feature set.

## Download (Windows EXE)

- Static build (x64): **REPLACE_WITH_EXE_URL**

## License

This project is licensed under the GNU General Public License v3.0 or later.
See `COPYING` for details.

## Build (MSVC)

Open the solution in Visual Studio and build `tail` (x64).

## Notes

- `-f` uses polling with `-s` (default 1.0s).
- By default, follow mode reopens files each interval to reduce sharing conflicts.
- Use `-K, --keep-open` for faster follow when the writer allows shared reads.
- Wildcards like `C:\temp\*.log` are expanded by the program on Windows.

## Tail-Lite Feature Set

Supported options:
- `-n, --lines=NUM`
- `-c, --bytes=NUM`
- `-f, --follow`
- `-s, --sleep-interval=NUM`
- `-K, --keep-open`
- `-q, --quiet, --silent`
- `-v, --verbose`
- `-z, --zero-terminated`
- `--help`
- `--version`

Not supported (intentionally omitted for Windows-lite):
- `-F`
- `--pid`
- `--retry`
- `--follow=name`
- inotify-based follow

## Examples

```powershell
# Show last 20 lines
.\x64\Debug\tail.exe -n 20 C:\temp\test.log

# Follow with default polling (1s)
.\x64\Debug\tail.exe -f C:\temp\test.log

# Follow faster: poll 5x per second
.\x64\Debug\tail.exe -f -s 0.2 C:\temp\test.log

# Follow faster and keep the file open (only if writer allows sharing)
.\x64\Debug\tail.exe -f -s 0.1 -K C:\temp\test.log

# Follow multiple files with wildcard expansion (Windows)
.\x64\Debug\tail.exe -f C:\temp\*.log

# Bytes mode: last 4 KB
.\x64\Debug\tail.exe -c 4096 C:\temp\test.log
```
