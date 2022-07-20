#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace {

enum class OptionType : uint8_t {
    OT_bool,
    OT_int,
};

class OptionParser {
public:
    explicit OptionParser() {}
    void registerOption(const char *Name, const char *Desc, OptionType Type, void *Var);
    void parseString(const char *S);
    void printOptionDescriptions();

private:
    // Calculate at compile-time how many options are available.
#define APHOTIC_SHIELD_OPTION(...) +1
    static constexpr size_t MaxOptions = 0
#include "options.inc"
        ;
#undef APHOTIC_SHIELD_OPTION

    struct Option {
        const char *Name;
        const char *Desc;
        OptionType Type;
        void *Var;
    } Options[MaxOptions];

    size_t NumberOfOptions = 0;
    const char *Buffer = nullptr;
    uintptr_t Pos = 0;

    bool isSeparator(char C);
    bool isSeparatorOrNull(char C);
    void skipWhitespace();
    void parseOptions();
    bool parseOption();
    bool parseBool(const char *Value, bool *b);
    bool setOptionToValue(const char *Name, const char *Value);
};

void OptionParser::printOptionDescriptions() {
    printf("APHOTIC-SHIELD: Available options:\n");
    for (size_t I = 0; I < NumberOfOptions; ++I)
        printf("\t%s\n\t\t- %s\n", Options[I].Name, Options[I].Desc);
}

bool OptionParser::isSeparator(char C) {
    return C == ' ' || C == ',' || C == ':' || C == '\n' || C == '\t' || C == '\r';
}

bool OptionParser::isSeparatorOrNull(char C) {
    return !C || isSeparator(C);
}

void OptionParser::skipWhitespace() {
    while (isSeparator(Buffer[Pos]))
        ++Pos;
}

bool OptionParser::parseOption() {
    const uintptr_t NameStart = Pos;
    while (Buffer[Pos] != '=' && !isSeparatorOrNull(Buffer[Pos]))
        ++Pos;

    const char *Name = Buffer + NameStart;
    if (Buffer[Pos] != '=') {
        printf("APHOTIC-SHIELD: Expected '=' when parsing option '%s'.\n", Name);
        return false;
    }
    const uintptr_t ValueStart = ++Pos;
    const char *Value;
    if (Buffer[Pos] == '\'' || Buffer[Pos] == '"') {
        const char Quote = Buffer[Pos++];
        while (Buffer[Pos] != 0 && Buffer[Pos] != Quote)
            ++Pos;
        if (Buffer[Pos] == 0) {
            printf("APHOTIC-SHIELD: Unterminated string in option '%s'.\n", Name);
            return false;
        }
        Value = Buffer + ValueStart + 1;
        ++Pos;  // consume the closing quote
    } else {
        while (!isSeparatorOrNull(Buffer[Pos]))
            ++Pos;
        Value = Buffer + ValueStart;
    }

    return setOptionToValue(Name, Value);
}

void OptionParser::parseOptions() {
    while (true) {
        skipWhitespace();
        if (Buffer[Pos] == 0)
            break;
        if (!parseOption()) {
            printf("APHOTIC-SHIELD: Options parsing failed.\n");
            return;
        }
    }
}

void OptionParser::parseString(const char *S) {
    if (!S)
        return;
    Buffer = S;
    Pos = 0;
    parseOptions();
}

bool OptionParser::parseBool(const char *Value, bool *b) {
    if (strncmp(Value, "0", 1) == 0 || strncmp(Value, "no", 2) == 0 ||
        strncmp(Value, "false", 5) == 0) {
        *b = false;
        return true;
    }
    if (strncmp(Value, "1", 1) == 0 || strncmp(Value, "yes", 3) == 0 ||
        strncmp(Value, "true", 4) == 0) {
        *b = true;
        return true;
    }
    return false;
}

bool OptionParser::setOptionToValue(const char *Name, const char *Value) {
    for (size_t I = 0; I < NumberOfOptions; ++I) {
        const uintptr_t Len = strlen(Options[I].Name);
        if (strncmp(Name, Options[I].Name, Len) != 0 || Name[Len] != '=')
            continue;
        bool Ok = false;
        switch (Options[I].Type) {
        case OptionType::OT_bool:
            Ok = parseBool(Value, reinterpret_cast<bool *>(Options[I].Var));
            if (!Ok)
                printf("APHOTIC-SHIELD: Invalid boolean value '%s' for option '%s'.\n",
                       Value, Options[I].Name);
            break;
        case OptionType::OT_int:
            char *ValueEnd;
            *reinterpret_cast<int *>(Options[I].Var) =
                static_cast<int>(strtol(Value, &ValueEnd, 10));
            Ok = *ValueEnd == '"' || *ValueEnd == '\'' || isSeparatorOrNull(*ValueEnd);
            if (!Ok)
                printf("APHOTIC-SHIELD: Invalid integer value '%s' for option '%s'.\n",
                       Value, Options[I].Name);
            break;
        }
        return Ok;
    }

    printf("APHOTIC-SHIELD: Unknown option '%s'.\n", Name);
    return true;
}

void OptionParser::registerOption(const char *Name, const char *Desc, OptionType Type,
                                  void *Var) {
    assert(NumberOfOptions < MaxOptions &&
           "APHOTIC-SHIELD Error: Ran out of space for options.\n");
    Options[NumberOfOptions].Name = Name;
    Options[NumberOfOptions].Desc = Desc;
    Options[NumberOfOptions].Type = Type;
    Options[NumberOfOptions].Var = Var;
    ++NumberOfOptions;
}

aphotic_shield::options::Options *getOptionsInternal() {
    static aphotic_shield::options::Options AphoticShieldOptions;
    return &AphoticShieldOptions;
}

void registerAphoticShieldOptions(OptionParser *parser,
                                  aphotic_shield::options::Options *o) {
#define APHOTIC_SHIELD_OPTION(Type, Name, DefaultValue, Description)                     \
    parser->registerOption(#Name, Description, OptionType::OT_##Type, &o->Name);
#include "options.inc"
#undef APHOTIC_SHIELD_OPTION
}

}  // namespace

namespace aphotic_shield {
namespace options {

void initOptions(const char *OptionsStr) {
    Options *o = getOptionsInternal();
    o->setDefaults();

    OptionParser Parser;
    registerAphoticShieldOptions(&Parser, o);

    // Override from the provided options string.
    Parser.parseString(OptionsStr);

    if (o->help)
        Parser.printOptionDescriptions();

    if (!o->Enabled)
        return;

    if (o->MaxSimultaneousAllocations <= 0) {
        printf("APHOTIC-SHIELD ERROR: MaxSimultaneousAllocations must be > 0 when "
               "APHOTIC-SHIELD is enabled.\n");
        o->Enabled = false;
    }
}

Options &getOptions() {
    return *getOptionsInternal();
}

}  // namespace options
}  // namespace aphotic_shield
