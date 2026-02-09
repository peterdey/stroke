# Stroke

Stroke is a command‑line utility for editing the individual components
of a file's access time (`atime`), modification time (`mtime`) and 
change time (`ctime`).

It complements the classic `touch` utility by letting you:

- Copy timestamps from one file to any number of targets.
- Set exact timestamps with second‑level precision.
- Tweak only selected components (for example “add two hours to ctime”).

The full behaviour of every option is documented in `man stroke`.

## Features at a glance

- Works on both symbolic links and regular files (`-l/--symlinks`).
- Can adjust change time (with the necessary privileges - i.e. root) or 
  preserve it while modifying other times.
- Supports batch files and `stdin` so that complex modifier expressions can be
  scripted and reused.

## Usage examples

Inspect timestamps:

```bash
stroke -i /path/to/file
```

Copy all timestamps from one file to another:

```bash
stroke -r reference.img target.iso
```

Apply a timestamp literal to multiple files:

```bash
stroke -s 202401011200 target1 target2
```

Use modifier expressions for fine‑grained tweaks:

```bash
stroke mY=aY=2024,mM=aM=+1 my.log
```

For every option and the complete grammar, run `man stroke`.

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
