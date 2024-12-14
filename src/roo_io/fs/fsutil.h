#pragma once

#include <memory>

#include "roo_io/data/multipass_input_stream_reader.h"
#include "roo_io/data/output_stream_writer.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

// Recursively deletes the specified file or directory, if it exists.
// Does nothing, and returns kNotFound, if it does not exist.
Status DeleteRecursively(roo_io::Mount& fs, const char* path);

// Recursively creates the specified directory. If the directory already exists,
// returns 'kDirectoryExists'. If the path, or any of its subcomponents, names
// an existing file, returns 'kNotDirectory'.
Status MkDirRecursively(roo_io::Mount& fs, const char* path);

// Recursively creates a parent directory of the specified named file or
// directory, without creating the target file or directory itself. If the
// parent directory already exists, returns 'kDirectoryExists'. If the parent
// directory or any of its subcomponents is an existing file, returns
// 'kNotDirectory'.
Status MkParentDirRecursively(roo_io::Mount& fs, const char* path);

// Opens the specified file for reading, and returns a reader that allows to
// efficiently read typed data from that file.
MultipassInputStreamReader OpenDataFile(roo_io::Mount& fs, const char* path);

// Opens the specified file for writing, and returns a writer that allows to
// efficiently write typed data to that file.
OutputStreamWriter OpenDataFileForWrite(roo_io::Mount& fs, const char* path,
                                        roo_io::FileUpdatePolicy update_policy);

}  // namespace roo_io
