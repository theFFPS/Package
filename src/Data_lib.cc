#include <Data.hh>
#include <regex>
#include <IO.hh>
namespace TNB {
    Token::Token() { type = TOKEN_EOF; val = "\0"; }
    Token::Token(_t type, string val) { this->type = type; this->val = val; }
    Token::~Token() {}
    Lexer::~Lexer() {}
    AST::AST(_t type) { this->type = type; }
    AST::~AST() {}
    Parser::~Parser() {}
    AST Parser::ParseConstruction() {
        Eat(Token::TOKEN_START_CONTAINER);
        AST ret = AST(AST::AST_CONTAINER);
        while (token.type != Token::TOKEN_END_CONTAINER) {
            AST push = AST(AST::AST_TAG);
            Eat(Token::TOKEN_ID);
            push.tagName = prevToken.val;
            Eat(Token::TOKEN_COLON);
            if (token.type == Token::TOKEN_ID) { push.boolVal = ParseId().boolVal; push.valTyp = AST::AST_BOOLEAN; }
            else if (token.type == Token::TOKEN_NUMBER) { push.floatVal = ParseNumber().floatVal; push.valTyp = AST::AST_NUMBER; }
            else if (token.type == Token::TOKEN_STRING) { push.stringVal = ParseString().stringVal; push.valTyp = AST::AST_STRING; }
            else if (token.type == Token::TOKEN_START_CONTAINER) { push.containerVal = ParseConstruction().containerVal; push.valTyp = AST::AST_CONTAINER; }
            else if (token.type == Token::TOKEN_START_ARRAY) { push.arrayVal = ParseArray().arrayVal; push.valTyp = AST::AST_ARRAY; }
            if (token.type == Token::TOKEN_COMMA) Eat(Token::TOKEN_COMMA);
            ret.containerVal.push_back(push);
        }
        Eat(Token::TOKEN_END_CONTAINER);
        return ret;
    }
    AST Parser::ParseArray() {
        Eat(Token::TOKEN_START_ARRAY);
        AST ret = AST(AST::AST_ARRAY);
        while (token.type != Token::TOKEN_END_ARRAY) {
            AST push = AST(AST::AST_TAG);
            if (token.type == Token::TOKEN_ID) { push.boolVal = ParseId().boolVal; push.type = AST::AST_BOOLEAN; }
            else if (token.type == Token::TOKEN_NUMBER) { push.floatVal = ParseNumber().floatVal; push.type = AST::AST_NUMBER; }
            else if (token.type == Token::TOKEN_STRING) { push.stringVal = ParseString().stringVal; push.type = AST::AST_STRING; }
            else if (token.type == Token::TOKEN_START_CONTAINER) { push.containerVal = ParseConstruction().containerVal; push.type = AST::AST_CONTAINER; }
            else if (token.type == Token::TOKEN_START_ARRAY) { push.arrayVal = ParseArray().arrayVal; push.type = AST::AST_ARRAY; }
            if (token.type == Token::TOKEN_COMMA) Eat(Token::TOKEN_COMMA);
            ret.arrayVal.push_back(push);
        }
        Eat(Token::TOKEN_END_ARRAY);
        return ret;
    }
    AST Parser::ParseNumber() {
        AST ret = AST(AST::AST_NUMBER);
        Eat(Token::TOKEN_NUMBER);
        ret.floatVal = stof(prevToken.val);
        return ret;
    }
    AST Parser::ParseId() {
        AST ret = AST(AST::AST_TAG);
        if (token.val == "true" || token.val == "false") ret.type = AST::AST_BOOLEAN;
        if (token.val == "true") ret.boolVal = true;
        else if (token.val == "false") ret.boolVal = false;
        else ret.tagName = token.val;
        Eat(Token::TOKEN_ID);
        return ret;
    }
    AST Parser::ParseString() {
        AST ret = AST(AST::AST_STRING);
        ret.stringVal = token.val;
        Eat(Token::TOKEN_STRING);
        return ret;
    }
    AST Parser::ParseStmt() { return ParseConstruction(); }
    AST Parser::Parse() {
        AST ast = AST(AST::AST_COMPOUND);
        while ((token = lexer.NextToken()).type != Token::TOKEN_EOF) { ast.compoundSize++; ast.compound.push_back(ParseStmt()); }
        return ast;
    }
    void Parser::Eat(Token::_t type) { if (token.type != type) exit(-1); prevToken = token; token = lexer.NextToken(); }
    Parser::Parser(Lexer lexer) { this->lexer = lexer; }
    Lexer::Lexer(string content) { this->content = content; pos = 0; c = content[0]; length = content.size(); }
    void Lexer::Advance() { if (pos < (int)length) { pos++; c = content[pos]; } }
    void Lexer::SkipWhitespace() { while (c == ' ' || c == '\n' || c == '\t') Advance(); }
    Token Lexer::AdvanceWithToken(Token token) { Advance(); return token; }
    Token Lexer::GetString() { Token out = Token(Token::TOKEN_STRING, ""); Advance(); while (c != '"') { out.val += c; Advance(); } Advance(); return out; }
    Token Lexer::GetNumber() { Token out = Token(Token::TOKEN_NUMBER, ""); while (isdigit(c)) { out.val += c; Advance(); } return out; }
    Token Lexer::GetID() { Token out = Token(Token::TOKEN_ID, ""); while (isalnum(c) || c == '_') { out.val += c; Advance(); } return out; }
    Token Lexer::NextToken() { 
        while (pos < (int)length) {
            if (c == ' ' || c == '\n' || c == '\t') SkipWhitespace();
            if (c == '{') return AdvanceWithToken(Token(Token::TOKEN_START_CONTAINER, "{"));
            if (c == '}') return AdvanceWithToken(Token(Token::TOKEN_END_CONTAINER, "}"));
            if (c == ',') return AdvanceWithToken(Token(Token::TOKEN_COMMA, ","));
            if (c == ':') return AdvanceWithToken(Token(Token::TOKEN_COLON, ":"));
            if (c == '[') return AdvanceWithToken(Token(Token::TOKEN_START_ARRAY, "["));
            if (c == ']') return AdvanceWithToken(Token(Token::TOKEN_END_ARRAY, "]"));
            if (c == '"') return GetString();
            if (isdigit(c)) return GetNumber();
            if (isalnum(c) || c == '_') return GetID();
        }
        return Token(Token::TOKEN_EOF, "\0");
    }
    Element::~Element() {}
    TNBWriter::TNBWriter() {}
    TNBWriter::~TNBWriter() {}
    void TNBWriter::Write(string file) { WriteToFile(file, AsString()); }
    string TNBWriter::AsString() { return container.AsString(); }
    void TNBWriter::Push(string tag, string data) { container.Push(tag, (string)data); }
    void TNBWriter::Push(string tag, float data) { container.Push(tag, (float)data); }
    void TNBWriter::Push(string tag, bool data) { container.Push(tag, (bool)data); }
    void TNBWriter::Push(string tag, Array data) { container.Push(tag, (Array)data); }
    void TNBWriter::Push(string tag, Container data) { container.Push(tag, (Container)data); }
    void Container::Push(string tag, string data) { elements.push_back(Element(data, tag)); }
    void Container::Push(string tag, float data) { elements.push_back(Element(data, tag)); }
    void Container::Push(string tag, bool data) { elements.push_back(Element(data, tag)); }
    void Container::Push(string tag, Array data) { elements.push_back(Element(data, tag)); }
    void Container::Push(string tag, Container data) { elements.push_back(Element(data, tag)); } 
    Array::~Array() {}
    Array::Array() {}
    Container::Container() {}
    Container::~Container() {}
    string Element::GetTag() { return tag; }
    Element Container::Get(string tag) { for (Element element : elements) if (element.GetTag() == tag) return element; return Element(false, ""); }
    string Container::AsString() { string out = "{"; for (Element element : elements) out += " " + element.AsString() + ","; out.pop_back(); out += " }"; return out; }
    size_t Array::Size() { return elements.size(); }
    string Array::AsString() { string out = "["; for (ArrayElement element : elements) out += " " + element.AsString() + ","; out.pop_back(); out += " ]"; return out; }
    void Array::Pop() { elements.pop_back(); lastIndex--; }
    void Array::Push(ArrayElement element) { lastIndex++; element.SetIndex(lastIndex); elements.push_back(element); }
    ArrayElement Array::Get(int index) { if (index > lastIndex) exit(-1); for (ArrayElement element : elements) if (element.GetIndex() == index) return element; return ArrayElement("", -1000000); }
    ArrayElement::~ArrayElement() {}
    int ArrayElement::GetIndex() { return index; }
    void ArrayElement::SetIndex(int index) { this->index = index; }
    Element::Element(string val, string tag) { this->stringV = val; this->tag = tag; this->type = TYPE_STRING; }
    Element::Element(float val, string tag) { this->numberV = val; this->tag = tag; this->type = TYPE_NUMBER; }
    Element::Element(Array val, string tag) { this->qarrayV = val; this->tag = tag; this->type = TYPE_ARRAY; }
    Element::Element(bool val, string tag) { this->booleanV = val; this->tag = tag; this->type = TYPE_BOOLEAN; }
    Element::Element(Container val, string tag) { this->containerV = val; this->tag = tag; this->type = TYPE_CONTAINER; }
    string Element::AsString() {
        stringstream ss;
        if (type == TYPE_STRING) ss << tag << ": \"" << stringV << "\""; 
        else if (type == TYPE_BOOLEAN) { if (booleanV) ss << tag << ": true"; else ss << tag << ": false"; }
        else if (type == TYPE_NUMBER) ss << tag << ": " << numberV;
        else if (type == TYPE_ARRAY) ss << tag << ": " << qarrayV.AsString();
        else if (type == TYPE_CONTAINER) ss << tag << ": " << containerV.AsString();
        return ss.str();
    }
    string ArrayElement::GetString() {
        stringstream ss;
        if (type == TYPE_STRING) ss << stringV; 
        else if (type == TYPE_BOOLEAN) { if (booleanV) ss << "true"; else ss << "false"; }
        else if (type == TYPE_NUMBER) ss << numberV;
        else if (type == TYPE_ARRAY) ss << arrayV->AsString();
        else if (type == TYPE_CONTAINER) ss << containerV.AsString();
        return ss.str();
    }
    string Element::GetString() {
        stringstream ss;
        if (type == TYPE_STRING) ss << stringV; 
        else if (type == TYPE_BOOLEAN) { if (booleanV) ss << "true"; else ss << "false"; }
        else if (type == TYPE_NUMBER) ss << numberV;
        else if (type == TYPE_ARRAY) ss << qarrayV.AsString();
        else if (type == TYPE_CONTAINER) ss << containerV.AsString();
        return ss.str();
    }
    ArrayElement::ArrayElement(string val, int lastIndex) { this->stringV = val; this->index = lastIndex+1; this->type = TYPE_STRING; }
    ArrayElement::ArrayElement(float val, int lastIndex) { this->numberV = val; this->index = lastIndex+1; this->type = TYPE_NUMBER; }
    ArrayElement::ArrayElement(Array val, int lastIndex) {
        this->arrayV = &val;
        this->index = lastIndex+1;
        this->type = TYPE_ARRAY;
    }
    ArrayElement::ArrayElement(bool val, int lastIndex) { this->booleanV = val; this->index = lastIndex+1; this->type = TYPE_BOOLEAN; }
    ArrayElement::ArrayElement(Container val, int lastIndex) { this->containerV = val; this->index = lastIndex+1; this->type = TYPE_CONTAINER; }
    string ArrayElement::AsString() {
        stringstream ss;
        if (type == TYPE_STRING) ss << "\"" << stringV << "\""; 
        else if (type == TYPE_BOOLEAN) { if (booleanV) ss << "true"; else ss << "false"; }
        else if (type == TYPE_NUMBER) ss << numberV;
        else if (type == TYPE_ARRAY) ss << arrayV->AsString();
        else if (type == TYPE_CONTAINER) ss << containerV.AsString();
        return ss.str();
    }
    Array TNBParser::ExtractArrayFromAST(AST ast) {
        Array out;
        for (AST astToken : ast.arrayVal) {
            if (astToken.type == AST::AST_BOOLEAN) out.Push(ArrayElement((bool)astToken.boolVal, -1));
            if (astToken.type == AST::AST_STRING) out.Push(ArrayElement((string)astToken.stringVal, -1));
            if (astToken.type == AST::AST_NUMBER) out.Push(ArrayElement((float)astToken.floatVal, -1));
            if (astToken.type == AST::AST_CONTAINER) out.Push(ArrayElement((Container)ExtractContainerFromAST(astToken), -1));
            if (astToken.type == AST::AST_ARRAY) out.Push(ArrayElement((Array)ExtractArrayFromAST(astToken), -1));
        }
        return out;
    }
    Container TNBParser::ExtractContainerFromAST(AST ast) {
        Container out;
        for (AST astToken : ast.containerVal) {
            if (astToken.valTyp == AST::AST_BOOLEAN) out.Push(astToken.tagName, (bool)astToken.boolVal);
            if (astToken.valTyp == AST::AST_STRING) out.Push(astToken.tagName, (string)astToken.stringVal);
            if (astToken.valTyp == AST::AST_NUMBER) out.Push(astToken.tagName, (float)astToken.floatVal);
            if (astToken.valTyp == AST::AST_CONTAINER) out.Push(astToken.tagName, (Container)ExtractContainerFromAST(astToken));
            if (astToken.valTyp == AST::AST_ARRAY) out.Push(astToken.tagName, (Array)ExtractArrayFromAST(astToken));
        }
        return out;
    }
    Element TNBWriter::Get(string tag) { return container.Get(tag); }
    float ArrayElement::AsNumber() { if (type == TYPE_BOOLEAN) return booleanV; else if (type == TYPE_NUMBER) return numberV; exit(-1); }
    bool ArrayElement::AsBoolean() { if (type == TYPE_BOOLEAN) return booleanV; exit(-1); }
    Array ArrayElement::AsArray() { if (type == TYPE_ARRAY) return *arrayV; exit(-1); }
    Container ArrayElement::AsContainer() { if (type == TYPE_CONTAINER) return containerV; exit(-1); }
    float Element::AsNumber() { if (type == TYPE_BOOLEAN) return booleanV; else if (type == TYPE_NUMBER) return numberV; exit(-1); }
    bool Element::AsBoolean() { if (type == TYPE_BOOLEAN) return booleanV; exit(-1); }
    Array Element::AsArray() { if (type == TYPE_ARRAY) return qarrayV; exit(-1); }
    Container Element::AsContainer() { if (type == TYPE_CONTAINER) return containerV; exit(-1); }
    list<string> TNBParser::GetTags() { return container.GetTags(); }
    list<string> Container::GetTags() { list<string> out; for (Element element : elements) out.push_back(element.GetTag()); return out; }
    Element::__T Element::GetType() { return type; }
    ArrayElement::__T ArrayElement::GetType() { return type; }
    TNBParser *TNBParser::operator*() { return this; }
    TNBParser::TNBParser() {}
    TNBParser::TNBParser(string file) {
        string fData = ReadFile(file);
        bool declareVersion = true;
        bool declareSize = false;
        string versionSTR;
        string data;
        bool declareData = false;
        for (int i = 0; i < (int)fData.size(); i++) {
            if (fData[i] == '\\') {
                if (declareVersion) { declareVersion = false; declareSize = true; continue; }
                if (declareSize) { declareData = true; declareSize = false; continue; }
            }
            if (declareVersion) versionSTR += fData[i];
            if (declareData) data += fData[i];
        }
        version = versionSTR;
        size = (int)data.size();
        Lexer lexer = Lexer(data);
        Parser parser = Parser(lexer);
        AST ast = parser.Parse();
        for (AST astContainers : ast.compound) {
            for (AST astToken : astContainers.containerVal) {
                if (astToken.valTyp == AST::AST_BOOLEAN) container.Push(astToken.tagName, (bool)astToken.boolVal);
                if (astToken.valTyp == AST::AST_STRING) container.Push(astToken.tagName, (string)astToken.stringVal);
                if (astToken.valTyp == AST::AST_NUMBER) container.Push(astToken.tagName, (float)astToken.floatVal);
                if (astToken.valTyp == AST::AST_CONTAINER) container.Push(astToken.tagName, (Container)ExtractContainerFromAST(astToken));
                if (astToken.valTyp == AST::AST_ARRAY) container.Push(astToken.tagName, (Array)ExtractArrayFromAST(astToken));
            }
        }
    }
    string TNBParser::GetVersion() { return version; }
    int TNBParser::GetSize() { return size; }
    TNBParser::~TNBParser() {}
    Element TNBParser::Get(string tag) { return container.Get(tag); }
} 
