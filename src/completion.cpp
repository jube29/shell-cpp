#include "completion.h"

#include <array>
#include <optional>
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

  std::string complete(const std::string &word) {
    std::string suffix{};
    // set curr node to last word char
    TrieNode *curr = root_;
    for (auto c : word) {
      if (curr->children()[c - 'a']) {
        curr = curr->children()[c - 'a'];
      } else {
        return suffix;
      }
    }

    // try to complete only when node is a single child and LCP isn't met
    while (auto opt = curr->get_single_child_idx()) {
      auto single_child_idx = *opt;
      curr = curr->children()[single_child_idx];
      suffix += static_cast<char>(single_child_idx + 'a');
    }

    return suffix;
  }

private:
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

void register_commands(std::vector<std::string> &cmds) {
  for (const auto &cmd : cmds) {
    cmd_trie.insert(cmd);
  }
}

} // namespace completion

