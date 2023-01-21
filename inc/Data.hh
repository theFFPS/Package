#pragma once
#ifndef __DATA__HH__
#define __DATA__HH__
#include <list>
#include <iostream>
#include <sstream>
using namespace std;
namespace TNB {
    class Element;
    class AST {
        public:
            enum _t {
                AST_COMPOUND,
                AST_TAG,
                AST_CONTAINER,
                AST_ARRAY,
                AST_STRING,
                AST_NUMBER,
                AST_BOOLEAN,
            };
            _t valTyp;
            _t type;
            string stringVal;
            bool boolVal = false;
            float floatVal = 0;
            list<AST> arrayVal;
            list<AST> containerVal;
            list<AST> compound;
            size_t compoundSize = 0;
            string tagName;
            AST(_t type);
            ~AST();
    };
    class Array;
    class Token {
        public:
            enum _t {
                TOKEN_EOF,
                TOKEN_START_CONTAINER,
                TOKEN_END_CONTAINER,
                TOKEN_START_ARRAY,
                TOKEN_END_ARRAY,
                TOKEN_STRING,
                TOKEN_NUMBER,
                TOKEN_ID,
                TOKEN_COLON,
                TOKEN_COMMA,
            };
            _t type;
            string val;
            Token();
            ~Token();
            Token(_t type, string val);
    };
    class Lexer {
        private:
            char c;
            int pos;
            size_t length;
            string content;
        public:
            Lexer(string content);
            ~Lexer();
            void Advance();
            Token NextToken();
            void SkipWhitespace();
            Token AdvanceWithToken(Token token);
            Token GetString();
            Token GetNumber();
            Token GetID();
    };
    class Parser {
        private:
            Lexer lexer = Lexer("");
            Token token = Token(Token::TOKEN_EOF, "\"");
            Token prevToken = Token(Token::TOKEN_EOF, "\"");
        public:
            Parser(Lexer lexer);
            ~Parser();
            AST Parse();
            AST ParseStmt();
            AST ParseConstruction();
            AST ParseArray();
            AST ParseNumber();
            AST ParseId();
            AST ParseString();
            void Eat(Token::_t type);
    };
    class Container {
        private:
            list<Element> elements = {};
        public:
            Container();
            ~Container();
            Element Get(string tag);
            string AsString();
            void Push(string tag, string data);
            void Push(string tag, float data);
            void Push(string tag, bool data);
            void Push(string tag, Array data);
            void Push(string tag, Container data);
            list<string> GetTags();
    };
    class ArrayElement {
        private:
            enum __T {
                TYPE_STRING,
                TYPE_NUMBER,
                TYPE_BOOLEAN,
                TYPE_ARRAY,
                TYPE_CONTAINER
            };
            __T           type;
            string        stringV;
            float         numberV;
            bool          booleanV;
            Array         *arrayV;
            int           index;
            Container     containerV;
        public:
            ~ArrayElement();
            ArrayElement(string val, int lastIndex);
            ArrayElement(float val, int lastIndex);
            ArrayElement(bool val, int lastIndex);
            ArrayElement(Array val, int lastIndex);
            ArrayElement(Container val, int lastIndex);
            string AsString();
            float AsNumber();
            bool AsBoolean();
            Array AsArray();
            Container AsContainer();
            string GetString();
            int GetIndex();
            __T GetType();
            void SetIndex(int index);
    };
    class Array {
        private:
            list<ArrayElement> elements = {};
            int lastIndex = -1;
        public:
            void         Push(ArrayElement element);
            ArrayElement Get(int index);
            size_t       Size();
            void         Pop();
            ~Array();
            Array();
            string       AsString();
    };
    class Element {
        private:
            enum __T {
                TYPE_STRING,
                TYPE_NUMBER,
                TYPE_BOOLEAN,
                TYPE_ARRAY,
                TYPE_CONTAINER
            };
            __T           type;
            string        stringV;
            float         numberV;
            bool          booleanV;
            Array         qarrayV;
            string        tag;
            Container     containerV;
        public:
            string GetTag();
            Element(string val, string tag);
            Element(float val, string tag);
            Element(Array val, string tag);
            Element(bool val, string tag);
            Element(Container val, string tag);
            ~Element();
            string AsString();
            float AsNumber();
            bool AsBoolean();
            Array AsArray();
            Container AsContainer();
            string GetString();
            __T GetType();
    };
    class TNBParser {
        private:
            Container container;
            string version = "";
            int size = 0;
        public:
            TNBParser(string file);
            TNBParser();
            ~TNBParser();
            Element Get(string tag);
            int GetSize();
            Array ExtractArrayFromAST(AST ast);
            Container ExtractContainerFromAST(AST ast);
            string GetVersion();
            list<string> GetTags();
            TNBParser *operator*();
    };
    class TNBWriter {
        private:
            Container container;
        public:
            string AsString();
            TNBWriter();
            ~TNBWriter();
            void Push(string tag, string data);
            void Push(string tag, float data);
            void Push(string tag, bool data);
            void Push(string tag, Array data);
            void Push(string tag, Container data);
            void Write(string file);
            Element Get(string tag);
    };
}
#endif
