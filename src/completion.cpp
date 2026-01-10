#include "completion.h"

#include <array>
#include <cstring>
#include <optional>
#include <readline/history.h>
#include <readline/readline.h>
#include <string_view>

namespace {

class TrieNode {
public:
  // default constructor
  TrieNode() : parent_(nullptr), eow_(false) { children_.fill(nullptr); }
  // single argument constructor, no argument type inference
  explicit TrieNode(TrieNode *parent) : parent_(parent), eow_(false) { children_.fill(nullptr); }
  // parent getter
  std::optional<TrieNode *> parent() const { return parent_ ? std::optional<TrieNode *>(parent_) : std::nullopt; }
  // children getter
  std::array<TrieNode *, 26> &children() { return children_; }
  // children getter
  const std::array<TrieNode *, 26> &children() const { return children_; }
  // eow getter
  bool eow() const { return eow_; }
  // eow setter
  void set_eow(bool value) { eow_ = value; }

  bool has_children() const {
    for (const TrieNode *child : this->children()) {
      if (child) {
        return true;
      }
    }
    return false;
  }

  std::optional<size_t> get_single_child_idx() const {
    std::optional<size_t> result = std::nullopt;
    for (auto i{0uz}; i != this->children().size(); i++) {
      if (!this->children()[i])
        continue;
      // 0 is truthy in this context
      if (result)
        return std::nullopt;
      result = i;
    }
    return result;
  }

private:
  TrieNode *parent_;
  std::array<TrieNode *, 26> children_;
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
      int idx = c - 'a';
      if (!curr->children()[idx]) {
        curr->children()[idx] = new TrieNode(curr);
      }
      curr = curr->children()[idx];
    }
    curr->set_eow(true);
  }

  bool search(const std::string_view &word) const {
    TrieNode *curr = root_;
    for (auto c : word) {
      int idx = c - 'a';
      if (!curr->children()[idx]) {
        return false;
      }
      curr = curr->children()[idx];
    }
    return curr->eow();
  }

  bool isPrefix(const std::string_view &word) const {
    TrieNode *curr = root_;
    for (auto c : word) {
      int idx = c - 'a';
      if (!curr->children()[idx]) {
        return false;
      }
      curr = curr->children()[idx];
    }
    return true;
  }

  void remove(const std::string_view &word) {
    if (!search(word)) {
      return;
    }

    TrieNode *curr = root_;
    for (auto c : word) {
      int idx = c - 'a';
      curr = curr->children()[idx];
    }

    // word 'removed' form trie
    curr->set_eow(false);

    // can't delete if prefix
    if (curr->has_children()) {
      return;
    }

    // start from latest word node
    for (auto it = word.rbegin(); it != word.rend(); ++it) {
      // if actually a word or has other children: can't delete
      if (curr->eow() || curr->has_children()) {
        break;
      }

      // move to parent node and delete corresponding child
      auto parent_opt = curr->parent();
      if (parent_opt) {
        auto parent = *parent_opt;
        int idx = *it - 'a';
        parent->children()[idx] = nullptr;
        delete curr;
        curr = parent;
      }
    }
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
      int idx = c - 'a';
      if (!curr->children()[idx]) {
        return nullptr;
      }
      curr = curr->children()[idx];
    }
    return curr;
  }

  void dfs_collect(TrieNode *node, std::string &current, std::vector<std::string> &results) {
    if (node->eow()) {
      results.push_back(current);
    }
    for (size_t i = 0; i < 26; ++i) {
      if (node->children()[i]) {
        current.push_back('a' + i);
        dfs_collect(node->children()[i], current, results);
        current.pop_back();
      }
    }
  }

  void delete_subtree(TrieNode *node) {
    if (!node) {
      return;
    }
    for (TrieNode *child : node->children()) {
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

