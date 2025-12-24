#ifndef REDIRECTION_GUARD_H
#define REDIRECTION_GUARD_H

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <optional>
#include <string>
#include <unistd.h>
#include <vector>

// Redirection information for a command
struct Redirection {
  std::optional<std::string> input_file;  // <
  std::optional<std::string> output_file; // > or 1>
  bool append_output = false;             // true for >>, false for >
  std::optional<std::string> error_file;  // 2>
  bool append_error = false;              // true for 2>>, false for 2>
};

// RAII class to manage file descriptor redirections
// Automatically saves original FDs, redirects them, and restores on destruction
class RedirectionGuard {
public:
  explicit RedirectionGuard(const Redirection &redir) {
    // Handle stdout redirection (> or >>)
    if (redir.output_file.has_value()) {
      setup_redirection(STDOUT_FILENO, *redir.output_file, redir.append_output);
    }

    // Handle stderr redirection (2> or 2>>)
    if (redir.error_file.has_value()) {
      setup_redirection(STDERR_FILENO, *redir.error_file, redir.append_error);
    }

    // TODO: Handle stdin redirection (<) when needed
  }

  ~RedirectionGuard() {
    restore();
  }

  // Disable copy (would cause double-close of file descriptors)
  RedirectionGuard(const RedirectionGuard &) = delete;
  RedirectionGuard &operator=(const RedirectionGuard &) = delete;

private:
  void setup_redirection(int fd, const std::string &filename, bool append) {
    // Save original FD
    int saved = dup(fd);
    if (saved == -1) {
      std::cerr << "Warning: failed to save fd " << fd << ": " << strerror(errno) << std::endl;
      return;
    }

    // Open target file
    int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
    int file = open(filename.c_str(), flags, 0644);
    if (file == -1) {
      std::cerr << "Failed to open " << filename << ": " << strerror(errno) << std::endl;
      close(saved);
      return;
    }

    // Redirect using dup2
    if (dup2(file, fd) == -1) {
      std::cerr << "Failed to redirect fd " << fd << ": " << strerror(errno) << std::endl;
      close(file);
      close(saved);
      return;
    }

    // Close the file fd (the dup2 created a copy at fd)
    close(file);

    // Store saved FD for restoration
    saved_fds_.push_back({fd, saved});
  }

  void restore() {
    // Restore all saved FDs in reverse order
    for (auto it = saved_fds_.rbegin(); it != saved_fds_.rend(); ++it) {
      if (dup2(it->saved, it->original) == -1) {
        std::cerr << "Warning: failed to restore fd " << it->original << std::endl;
      }
      close(it->saved);
    }
    saved_fds_.clear();
  }

  struct SavedFD {
    int original;  // The original FD number (STDOUT_FILENO or STDERR_FILENO)
    int saved;     // The dup'd FD to restore from
  };

  std::vector<SavedFD> saved_fds_;
};

#endif
