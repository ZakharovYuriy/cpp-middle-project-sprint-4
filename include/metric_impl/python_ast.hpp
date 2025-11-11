#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace analyzer::metric::python_ast {

struct Position {
    int line{0};
    int col{0};
};

struct Node {
    std::string type;
    Position start;
    Position end;
    std::vector<Node> children;
};

Node Parse(const std::string &ast);

const Node *FindChild(const Node &node, std::string_view type);
std::vector<const Node *> FindChildren(const Node &node, std::string_view type);

void Traverse(const Node &node, const std::function<void(const Node &)> &visitor);

}  // namespace analyzer::metric::python_ast

