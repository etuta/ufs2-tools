# 📂 ufs2-tools

A set of tools for inspecting and extracting data from a **raw UFS2 filesystem image**.

---

## Tools

### 🔍 `fs-find`

Recursively lists all files and directories in a UFS2 filesystem image.

**Usage:**

```
./fs-find <ufs2-image-file>
```

**Example:**

```
./fs-find disk.img
```

This will print a tree of all directory entries found in `disk.img`.

---

### 📝 `fs-cat`

Prints the contents of a file stored inside a UFS2 filesystem image.

**Usage:**

```
./fs-cat <ufs2-image-file> <path-to-file>
```

**Example:**

```
./fs-cat disk.img etc/hosts
```

This will output the contents of `etc/hosts` from the filesystem image.

---

## Features

✅ Memory-maps the filesystem image for fast access  
✅ Parses the UFS2 superblock and inodes  
✅ Supports direct and indirect block traversal (note: indirect block handling is partially implemented)  
✅ Minimal dependencies (standard C libraries and UFS headers)

---

## Building

Compile each tool separately with `gcc`:

```bash
gcc -o fs-find fs-find.c
gcc -o fs-cat fs-cat.c
```

Make sure your system has UFS headers (e.g., `/usr/include/ufs`).

---

## Limitations

⚠️ Indirect block traversal is partially implemented (some TODOs remain)  
⚠️ No write support—read-only inspection only  
⚠️ Assumes a valid UFS2 filesystem image (does not validate extensively)  
⚠️ No symbolic link handling  

---

## Example Output

### fs-find

```
etc
  hosts
  passwd
var
  log
    messages
```

### fs-cat

```
127.0.0.1   localhost
::1         localhost
```

---

## Author

Eliza Tuta

---
