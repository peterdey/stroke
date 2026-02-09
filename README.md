# Stroke

Stroke is a focused command-line utility for inspecting and editing the
individual components of a file's access time (`atime`), modification time
(`mtime`) and change time (`ctime`). It complements `touch(1)` by letting you
see every timestamp in one shot, copy clocks from a reference file, and set
precise values via familiar ISO-8601 or relative expressions ("now -2 hours",
"2024-01-01T12:00Z", "@1700000000", ...).

Run `stroke` with no setters to inspect files. Add one or more setters to
switch into mutation mode:

```
stroke [OPTIONS] FILE...
  -m, --mtime=SPEC      set modification time
  -a, --atime=SPEC      set access time
  -c, --ctime=SPEC      set change time (root/CAP_SYS_TIME required)
      --copy=REF        copy all clocks from REF (setters override)
      --dry-run         validate changes without touching the files
  -l, --symlinks        operate on symlinks rather than targets
  -p, --preserve-ctime  keep ctime stable while editing mtime/atime
  -q, --quiet           suppress the per-file report
  -v, --verbose         extra diagnostics
```

All setters accept the GNU `parse-datetime` grammar: relative offsets
(`now +3days`), natural-language shorthands, explicit epochs (`@123`),
structured timestamps, and numeric UTC offsets
(`2026-01-02 23:01:22 +0100`). Named zones (`Europe/Berlin`, `PST`, etc.)
fall back to the host's local timezone on older libcs, so prefer explicit
offsets or UTC literals when the zone matters.

## Features at a glance

- Read-only by default: `stroke file ...` prints all timestamps and exits.
- Clean setters: `-m/-a/-c` are always assignments; no more overloaded grammar.
- `--copy=REF` mirrors all clocks from another path, and you can override
  individual components (e.g. `--copy ref -m 'now'`).
- `--dry-run` performs every validation (permissions, parse errors, ctimes)
  and prints the post-change report without touching disk.
- Works on regular files or symbolic links (`-l/--symlinks`).
- Keeps the classic "preserve ctime while touching mtime/atime" behaviour when
  `--preserve-ctime` is supplied and you have the necessary privilege.

## Usage examples

Inspect timestamps:

```bash
stroke my.log /var/www/html/index.html
```

Copy all timestamps from one file to another:

```bash
stroke --copy=reference.img target.iso
```

Apply a timestamp literal to multiple files:

```bash
stroke --mtime '2024-01-01 12:00' target1 target2
```

Use relative expressions and overrides:

```bash
stroke --copy backup.tar --mtime 'now -2h' --dry-run *.tar
```

For every option and supported timestamp format, run `man stroke`.

### About ctime modifications

POSIX filesystems do not expose a direct API for editing `ctime`. When you ask
stroke to adjust change time (either explicitly or via `--preserve-ctime`) it:

1. Temporarily changes the system clock to the requested timestamp.
2. Performs an operation such as `chmod` so the kernel records the new `ctime`.
3. Restores the original clock value.

Because of this implementation detail:

- You must run stroke with privileges that allow `settimeofday` (typically root
  or CAP_SYS_TIME) whenever `--ctime` or `--preserve-ctime` is involved.
- Other processes on the host briefly observe the skewed system clock; avoid
  ctime adjustments on multi‑tenant systems where this would be disruptive.
- On platforms that forbid changing the clock, stroke will report an error and
  leave `ctime` untouched even if `mtime`/`atime` changes succeed.

## Installation

### Prebuilt packages

Grab the latest `.deb` package from the
[Releases](https://github.com/peterdey/stroke/releases) page and install 
with `dpkg`:

```bash
sudo dpkg -i stroke_0.1.4-1_amd64.deb
```

You can also download the accompanying source package (`.dsc`,
`.debian.tar.xz`, `.orig.tar.gz`) from the same release.

### From source

The traditional Autotools flow is still supported:

```bash
./configure
make
sudo make install
```

## Contributing & support

Issues and patches are welcome,

## License

Copyright (C) 2009 Free Software Foundation, Inc.

Copyright (C) 2009 Sören Wellhöfer <soeren.wellhoefer@gmx.net>

Copyright (C) 2026 Peter Dey

Stroke is released under the GNU General Public License, version 3,
The full text is available in [`LICENSE`](LICENSE).
