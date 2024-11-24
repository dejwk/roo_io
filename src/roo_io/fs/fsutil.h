#pragma once

#include <memory>

#include "roo_io/data/multipass_input_stream_reader.h"
#include "roo_io/data/output_stream_writer.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

Status DeleteRecursively(roo_io::Mount& fs, const char* path);

Status MkDirRecursively(roo_io::Mount& fs, const char* path);

MultipassInputStreamReader OpenDataFile(roo_io::Mount& fs, const char* path);

OutputStreamWriter OpenDataFileForWrite(roo_io::Mount& fs, const char* path,
                                        roo_io::FileUpdatePolicy update_policy);

}  // namespace roo_io
