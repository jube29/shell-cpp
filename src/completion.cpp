#include "completion.h"

#include <cstring>
#include <optional>
#include <readline/history.h>
#include <readline/readline.h>
#include <string_view>
#include <unordered_map>

namespace {

class TrieNode {
public:
  // default constructor
  TrieNode() : parent_(nullptr), eow_(false) {}
  // single argument constructor, no argument type inference
  explicit TrieNode(TrieNode *parent) : parent_(parent), eow_(false) {}
  // parent getter
  std::optional<TrieNode *> parent() const { return parent_ ? std::optional<TrieNode *>(parent_) : std::nullopt; }
  // children getter
  std::unordered_map<char, TrieNode *> &children() { return children_; }
  // children getter
  const std::unordered_map<char, TrieNode *> &children() const { return children_; }
  // eow getter
  bool eow() const { return eow_; }
  // eow setter
  void set_eow(bool value) { eow_ = value; }

  bool has_children() const { return !children_.empty(); }

  std::optional<char> get_single_child_char() const {
    if (children_.size() != 1) {
      return std::nullopt;
    }
    return children_.begin()->first;
  }

private:
  TrieNode *parent_;
  std::unordered_map<char, TrieNode *> children_;
  bool eow_;
};

class Trie {
public:
  // default constructor
  Trie() : root_(new TrieNode()) {}
  // destructor
  ~Trie() { delete_subtree(root_); }
  // disable copy constructor
  Trie(const Trie &) = delete;
  // disable assignment operator
  Trie &operator=(const Trie &) = delete;
  // disable move constructor
  Trie(Trie &&) = delete;
  // disable move assignement
  Trie &operator=(Trie &&) = delete;

  void insert(const std::string_view &word) {
    TrieNode *curr = root_;
    for (auto c : word) {
      if (curr->children().find(c) == curr->children().end()) {
        curr->children()[c] = new TrieNode(curr);
      }
      curr = curr->children()[c];
    }
    curr->set_eow(true);
  }

  void get_all_completions(const std::string_view &prefix, std::vector<std::string> &results) {
    TrieNode *node = find_prefix(prefix);
    if (!node) {
      return;
    }
    std::string current(prefix);
    dfs_collect(node, current, results);
  }

private:
  TrieNode *find_prefix(const std::string_view &prefix) {
    TrieNode *curr = root_;
    for (auto c : prefix) {
      auto it = curr->children().find(c);
      if (it == curr->children().end()) {
        return nullptr;
      }
      curr = it->second;
    }
    return curr;
  }

  void dfs_collect(TrieNode *node, std::string &current, std::vector<std::string> &results) {
    if (node->eow()) {
      results.push_back(current);
    }
    for (const auto &[c, child] : node->children()) {
      current.push_back(c);
      dfs_collect(child, current, results);
      current.pop_back();
    }
  }

  void delete_subtree(TrieNode *node) {
    if (!node) {
      return;
    }
    for (auto &[c, child] : node->children()) {
      delete_subtree(child);
    }
    delete node;
  }

  TrieNode *root_;
};

Trie cmd_trie;

} // namespace

namespace completion {

void setup() { rl_attempted_completion_function = completer; }

void register_commands(const std::vector<std::string> &cmds) {
  for (const auto &cmd : cmds) {
    cmd_trie.insert(cmd);
  }
}

// Readline completion generator
static char *completion_generator(const char *text, int state) {
  static std::vector<std::string> matches;
  static size_t index;

  if (state == 0) {
    matches.clear();
    index = 0;
    cmd_trie.get_all_completions(text, matches);
  }

  if (index < matches.size()) {
    return strdup(matches[index++].c_str());
  }
  return nullptr;
}

// Readline attempted completion callback
char **completer(const char *text, int start, int end) {
  rl_attempted_completion_over = 1; // Disable filename completion
  return rl_completion_matches(text, completion_generator);
}

} // namespace completion

