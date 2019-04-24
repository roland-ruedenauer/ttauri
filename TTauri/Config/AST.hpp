
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <any>

namespace TTauri::Config {

struct ASTLocation {
    int firstLine;
    int lastLine;
    int firstColumn;
    int lastColumn;
};

struct ASTNode {
    ASTLocation location;
    ASTNode(ASTLocation location) : location(location) {}
};

struct ASTExpression : ASTNode {
    ASTExpression(ASTLocation location) : ASTNode(location) {}
};

struct ASTExpressions : ASTNode {
    std::vector<ASTExpression *> expressions;

    ASTExpressions(ASTLocation location, ASTExpression *firstExpression) : ASTNode(location), expressions({firstExpression}) {}

    ~ASTExpressions() {
        for (auto expression: expressions) {
            delete expression;
        }
    }
};

struct ASTIntegerLiteral : ASTExpression {
    int64_t value;

    ASTIntegerLiteral(ASTLocation location, int64_t value) : ASTExpression(location), value(value) {}
};

struct ASTFloatLiteral : ASTExpression {
    double value;

    ASTFloatLiteral(ASTLocation location, double value) : ASTExpression(location), value(value) {}
};

struct ASTBooleanLiteral : ASTExpression {
    bool value;

    ASTBooleanLiteral(ASTLocation location, bool value) : ASTExpression(location), value(value) {}
};

struct ASTStringLiteral : ASTExpression {
    std::string value;

    ASTStringLiteral(ASTLocation location, char *value) : ASTExpression(location), value(value) {
        free(value);
    }
};

struct ASTNullLiteral : ASTExpression {

    ASTNullLiteral(ASTLocation location) : ASTExpression(location) {}
};

struct ASTIdentifier : ASTExpression {
    std::string name;

    ASTIdentifier(ASTLocation location, char *name) : ASTExpression(location), name(name) {
        free(name);
    }
};

struct ASTMember : ASTExpression {
    ASTExpression *object;
    std::string name;

    ASTMember(ASTLocation location, ASTExpression *object, char *name) : ASTExpression(location), object(object), name(name) {
        free(name);
    }

    ~ASTMember() {
        delete object;
    }
};

struct ASTCall : ASTExpression {
    ASTExpression *object;
    std::vector<ASTExpression *> arguments;

    ASTCall(ASTLocation location, ASTExpression *object, char *name, ASTExpressions *arguments) :
        ASTExpression(location),
        object(new ASTMember(object->location, object, name)),
        arguments(arguments->expressions)
    {
        // We copied the pointers of the expression, so they must not be destructed when expressions is deleted.
        arguments->expressions.clear();
        delete arguments;
    }

    ASTCall(ASTLocation location, char *name, ASTExpressions *arguments) :
        ASTExpression(location),
        object(new ASTIdentifier(location, name)),
        arguments(arguments->expressions)
    {
        // We copied the pointers of the expression, so they must not be destructed when expressions is deleted.
        arguments->expressions.clear();
        delete arguments;
    }

    ASTCall(ASTLocation location, ASTExpression *object, char *name, ASTExpression *argument) :
        ASTExpression(location),
        object(new ASTMember(object->location, object, name)),
        arguments({argument})
    {
    }

    ASTCall(ASTLocation location, ASTExpression *object, char *name) :
        ASTExpression(location),
        object(new ASTMember(object->location, object, name)),
        arguments({})
    {
    }

    ~ASTCall() {
        delete object;
        for (auto argument: arguments) {
            delete argument;
        }
    }
};

struct ASTArray : ASTExpression {
    std::vector<ASTExpression *> expressions;

    ASTArray(ASTLocation location) : ASTExpression(location), expressions() { }

    ASTArray(ASTLocation location, ASTExpressions *expressions) : ASTExpression(location), expressions(expressions->expressions) {
        // We copied the pointers of the expression, so they must not be destructed when expressions is deleted.
        expressions->expressions.clear();
        delete expressions;
    }

    ~ASTArray() {
        for (auto expression: expressions) {
            delete expression;
        }
    }
};

}