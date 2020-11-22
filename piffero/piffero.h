#pragma once
#include "lib/rapidjson/reader.h"
#include "lib/rapidjson/writer.h"
#include "lib/rapidjson/filereadstream.h"
#include "lib/rapidjson/filewritestream.h"
#include "lib/rapidjson/memorybuffer.h"
#include "lib/rapidjson/memorystream.h"
#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>  
#include <vector>
#include <cctype>
#include <regex>

using namespace rapidjson;
using namespace std;
namespace piffero {

    enum Last { startobject = 0, endobject = 1, startarray = 2, endarray = 3, key = 4, value = 5 };

    class JSONPath {
    public:
        int rangeStart;
        string value;
        bool isLast;
        JSONPath* next_;


        JSONPath(string  value_) {
            isLast = true;
            value = value;
            rangeStart = -1;
        }

        JSONPath(string  value_, int range) {
            isLast = true;
            value = value_;
            rangeStart = range;
        }

        JSONPath(string value_, JSONPath* next) {
            isLast = false;
            next_ = next;
            rangeStart = -1;
        }

        JSONPath(string value_, int range, JSONPath* next) {
            isLast = false;
            next_ = next;
            rangeStart = range;
        }
    };


    class JSONPathParser {
    public:
        // facilmente ottimizzabile (non oggi) 
        static JSONPath jsonPathParse(string path) {
            vector<string> split = JSONPathParser::split(path, ".");
            JSONPath* current;
            for (int i = split.size() - 1; i <= 0; i--) {
                string current_value = split[i];

                size_t open = current_value.find('[');
                size_t close = current_value.find(']');

                if (open != string::npos && close != string::npos) {

                    vector<string> splitted = JSONPathParser::split(current_value, "[");
                    current_value = splitted[0];

                    string::size_type sz;
                    string s_index = splitted[1].erase(splitted[1].size() - 1);
                    int index = stoi(s_index, &sz);
                    if (i == split.size() - 1) {
                        JSONPath path(current_value, index);
                        current = &path;
                    }
                    else {
                        JSONPath path(current_value, index, current);
                        current = &path;
                    }
                }
                else if (i == split.size() - 1) {
                    JSONPath path(current_value);
                    current = &path;
                }
                else {
                    JSONPath path(current_value, current);
                    current = &path;
                }
            }
            return *current;
        }


        static vector<string> split(const string& str, const string& delim)
        {
            vector<string> tokens;
            size_t prev = 0, pos = 0;
            do
            {
                pos = str.find(delim, prev);
                if (pos == string::npos) pos = str.length();
                string token = str.substr(prev, pos - prev);
                if (!token.empty()) tokens.push_back(token);
                prev = pos + delim.length();
            } while (pos < str.length() && prev < str.length());
            return tokens;
        }
    };

    struct PifferoStatus {
        bool verified;
        bool recording;
        bool isArray;
        bool isPrimitiveTypeArray;
        bool isRootArray;
        bool isMatching;
        bool end;
        int depthCounter;
        int currentindex;
        string lastKey;
        Last last;
        JSONPath& jsonPath;

        PifferoStatus(JSONPath& path) : jsonPath(path) {
            verified = false;
            recording = false;
            isArray = false;
            if (jsonPath.value.compare("$") == 0) {
                recording = true;
            } if (jsonPath.rangeStart >= 0) {
                isArray = true;
            }
            isPrimitiveTypeArray = false;
            isRootArray = false;
            isMatching = false;
            end = false;
            depthCounter = 0;
            currentindex = -1;
        }
    };
    template <typename InputStream, typename Handler>
    class JSONParser {

        static void parsePath(InputStream& is, Handler& handler, string Path) {
            JSONPath path = JsonParse

        }

    };




    class RecorsiveParser {
    public:
        bool hasNext = false;
        bool isFirst = false;
        RecorsiveParser& next;
        stringstream* ss;
        Last last;
        string parser;
        RecorsiveParser(stringstream* out) : next(*this) {
            ss = out;
            hasNext = false;
            isFirst = false;
            parser = "ultimo";
        }

        RecorsiveParser(RecorsiveParser& nextFilter) : next(nextFilter) {
            hasNext = true;
            isFirst = true;
        }



        bool Null() {
            addCommaIfNeeded();
            if (hasNext) {
                return next.Null();
            }
            *ss << "null";
            last = value;
            return true;
        }
        bool Bool(bool b) {
            if (hasNext) {
                return next.Bool(b);
            }
            addCommaIfNeeded();
            if (b) {
                *ss << "true";
            }
            else {
                *ss << false;
            }
            last = value;
            return true;
        }
        bool Int(int i) {
            if (hasNext) {
                return next.Int(i);
            }
            addCommaIfNeeded();
            *ss << i;
            last = value;
            return true;
        }

        bool Uint(unsigned u) {
            if (hasNext) {
                return next.Uint(u);
            }
            addCommaIfNeeded();
            *ss << u;
            last = value;
            return true;
        }
        bool Int64(int64_t i) {
            if (hasNext) {
                return next.Int64(i);
            }
            addCommaIfNeeded();
            *ss << i;
            last = value;
            return true;
        }
        bool Uint64(uint64_t u) {
            if (hasNext) {
                return next.Uint64(u);
            }
            addCommaIfNeeded();
            *ss << u;
            //ParseNext();
            last = value;
            return true;
        }
        bool Double(double d) {
            if (hasNext) {
                return next.Double(d);
            }
            addCommaIfNeeded();
            *ss << d;
            last = value;
            //ParseNext();
            return true;
        }
        bool RawNumber(const char* str, SizeType length, bool copy) {
            if (hasNext) {
                return next.RawNumber(str, length, copy);
            }
            addCommaIfNeeded();
            *ss << str;
            last = value;
            return true;
        }

        bool String(const char* str, SizeType length, bool copy) {
            if (hasNext) {
                return next.String(str, length, copy);
            }
            addCommaIfNeeded();
            *ss << "\"";
            *ss << str;
            *ss << "\"";
            last = value;
            // ParseNext();
            return true;
        }
        bool StartObject() {
            if (hasNext) {
                return next.StartObject();
            }
            addCommaIfNeeded();
            *ss << "{";
            last = startobject;
            return true;
        }
        bool Key(const char* str, SizeType length, bool copy) {
            if (hasNext) {
                return next.Key(str, length, copy);
            }
            addCommaIfNeeded();
            *ss << "\"" << str << "\"" << ":";
            last = key;
            return true;
        }
        bool EndObject(SizeType memberCount) {
            if (hasNext) {
                return next.EndObject(memberCount);
            }
            *ss << "}";
            last = endobject;
            return true;
        }
        bool StartArray() {
            if (hasNext) {
                return next.StartArray();
            }
            addCommaIfNeeded();
            *ss << "[";
            last = startarray;
            return true;
        }
        bool EndArray(SizeType elementCount) {
            if (hasNext) {
                return next.EndArray(elementCount);
            }
            *ss << "]";
            last = endarray;
            return true;
        }


        void addCommaIfNeeded() {
            if (last == endarray || last == endobject || last == value) {
                *ss << ",";
            }
        }
    };
}