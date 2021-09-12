#pragma once

#include "AST.h"

namespace remniw {

class FrontEnd {
private:
    TypeContext& TheTypeContext;

public:
    FrontEnd(TypeContext& TheTypeContext): TheTypeContext(TheTypeContext) {}

    // Parse an input stream and return an AST.
    std::unique_ptr<ProgramAST> parse(std::istream& Stream);
};

}  // namespace remniw
