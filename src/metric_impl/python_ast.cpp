#include "metric_impl/python_ast.hpp"

#include <cctype>

namespace analyzer::metric::python_ast {

namespace {

struct Parser {
    const std::string &src;
    size_t pos = 0;

    static bool IsSpace(char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
    }

    void SkipSpaces() {
        while (pos < src.size() && IsSpace(src[pos]))
            ++pos;
    }

    int ParseInt() {
        int sign = 1;
        if (pos < src.size() && src[pos] == '-') {
            sign = -1;
            ++pos;
        }
        int value = 0;
        while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos]))) {
            value = value * 10 + (src[pos] - '0');
            ++pos;
        }
        return sign * value;
    }

    Position ParsePos() {
        Position p{0, 0};
        if (pos >= src.size() || src[pos] != '[')
            return p;

        ++pos;
        SkipSpaces();
        p.line = ParseInt();
        SkipSpaces();
        if (pos < src.size() && src[pos] == ',')
            ++pos;
        SkipSpaces();
        p.col = ParseInt();
        SkipSpaces();
        if (pos < src.size() && src[pos] == ']')
            ++pos;
        return p;
    }

    void SkipStringLiteral() {
        if (pos >= src.size() || src[pos] != '"')
            return;
        ++pos;
        while (pos < src.size()) {
            char c = src[pos];
            if (c == '\\') {
                pos += (pos + 1 < src.size()) ? 2 : 1;
            } else if (c == '"') {
                ++pos;
                break;
            } else {
                ++pos;
            }
        }
    }

    void SkipIdentifier() {
        while (pos < src.size()) {
            char c = src[pos];
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.') {
                ++pos;
            } else {
                break;
            }
        }
    }

    Node ParseNode() {
        SkipSpaces();
        if (pos >= src.size() || src[pos] != '(')
            return {};

        ++pos;
        size_t type_begin = pos;
        while (pos < src.size()) {
            char c = src[pos];
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.') {
                ++pos;
            } else {
                break;
            }
        }
        Node node;
        node.type = src.substr(type_begin, pos - type_begin);

        SkipSpaces();
        if (pos < src.size() && src[pos] == '[') {
            node.start = ParsePos();
            SkipSpaces();
            if (pos < src.size() && src[pos] == '-')
                ++pos;
            SkipSpaces();
            node.end = ParsePos();
        } else {
            node.start = {0, 0};
            node.end = {0, 0};
        }

        while (pos < src.size()) {
            SkipSpaces();
            if (pos >= src.size())
                break;
            char c = src[pos];
            if (c == ')') {
                ++pos;
                break;
            }
            if (c == '(') {
                node.children.push_back(ParseNode());
                continue;
            }
            if (c == '"') {
                SkipStringLiteral();
                continue;
            }
            if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                SkipIdentifier();
                SkipSpaces();
                if (pos < src.size() && src[pos] == ':')
                    ++pos;
                continue;
            }
            if (c == '[') {
                ParsePos();
                continue;
            }
            ++pos;
        }
        return node;
    }

    Node Parse() {
        SkipSpaces();
        if (pos >= src.size())
            return {};
        return ParseNode();
    }
};

void TraverseImpl(const Node &node, const std::function<void(const Node &)> &visitor) {
    visitor(node);
    for (const auto &child : node.children)
        TraverseImpl(child, visitor);
}

}  // namespace

Node Parse(const std::string &ast) {
    Parser parser{ast};
    return parser.Parse();
}

const Node *FindChild(const Node &node, std::string_view type) {
    for (const auto &child : node.children)
        if (child.type == type)
            return &child;
    return nullptr;
}

std::vector<const Node *> FindChildren(const Node &node, std::string_view type) {
    std::vector<const Node *> matches;
    for (const auto &child : node.children)
        if (child.type == type)
            matches.push_back(&child);
    return matches;
}

void Traverse(const Node &node, const std::function<void(const Node &)> &visitor) {
    TraverseImpl(node, visitor);
}

}  // namespace analyzer::metric::python_ast

