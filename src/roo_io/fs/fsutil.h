#pragma once

#include <memory>

#include "roo_io/data/multipass_input_stream_reader.h"
#include "roo_io/data/output_stream_writer.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

/// Recursively deletes the file or directory at `path`.
///
/// Returns `kNotFound` when `path` does not exist. Directory removal stops at
/// the first failing descendant operation and leaves the remaining subtree
/// untouched.
Status DeleteRecursively(roo_io::Mount& fs, const char* path);

/// Recursively creates the directory at `path`.
///
/// Existing intermediate directories are accepted. Returns
/// `kDirectoryExists` if `path` already names a directory, `kNotDirectory` if
/// an existing file blocks the path, or another filesystem error from the
/// first failing step.
Status MkDirRecursively(roo_io::Mount& fs, const char* path);

/// Recursively creates the parent directory of `path` without creating `path`.
///
/// This is useful before creating a file at `path`. If `path` itself already
/// exists, whether as a file or a directory, the result is `kDirectoryExists`.
/// Otherwise the first failing parent-directory creation error is returned.
Status MkParentDirRecursively(roo_io::Mount& fs, const char* path);

/// Opens `path` for buffered typed reading.
///
/// The returned reader wraps `fs.fopen(path)` directly and reports mount or
/// open failures through the reader status.
MultipassInputStreamReader OpenDataFile(roo_io::Mount& fs, const char* path);

/// Opens `path` for buffered typed writing using `update_policy`.
///
/// The returned writer wraps `fs.fopenForWrite(path, update_policy)` directly
/// and reports mount, permission, or open failures through the writer status.
OutputStreamWriter OpenDataFileForWrite(roo_io::Mount& fs, const char* path,
                                        roo_io::FileUpdatePolicy update_policy);

}  // namespace roo_io
