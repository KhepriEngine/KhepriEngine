#pragma once

#include "stream.hpp"

#include <gsl/gsl-lite.hpp>

#include <cstdio>
#include <filesystem>
#include <memory>

namespace khepri::io {

/// Modes for dealing with files
enum class open_mode
{
    read,       /// Opens an existing file for reading.
    read_write, /// Creates a new file for reading and writing.
};

/// A file-based stream
class File : public Stream
{
    using path   = std::filesystem::path;
    using handle = FILE*;

public:
    /// Opens a file for reading or reading and writing.
    /// \throws khepri::io::error if the file cannot be opened.
    File(const path& path, open_mode mode);
    ~File() override;

    File(const File&) = delete;
    File(File&&)      = delete;
    File& operator=(const File&) = delete;
    File& operator=(File&&) = delete;

    /// \see stream::readable
    [[nodiscard]] bool readable() const noexcept override
    {
        return true;
    }

    /// \see stream::writable
    [[nodiscard]] bool writable() const noexcept override
    {
        return m_mode == open_mode::read_write;
    }

    /// \see stream::seekable
    [[nodiscard]] bool seekable() const noexcept override
    {
        return true;
    }

    /// \see stream::read
    size_t read(void* buffer, size_t count) override;

    /// \see stream::write
    size_t write(const void* buffer, size_t count) override;

    /// \see stream::seek
    long long seek(long long offset, io::seek_origin origin) override;

private:
    gsl::owner<handle> m_handle;
    open_mode          m_mode;
};

} // namespace khepri::io
