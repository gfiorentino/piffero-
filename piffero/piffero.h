#include "../lib/rapidjson/reader.h"
#include "../lib/rapidjson/writer.h"
#include "../lib/rapidjson/filereadstream.h"
#include "../lib/rapidjson/filewritestream.h"
#include "../lib/rapidjson/memorybuffer.h"
#include "../lib/rapidjson/memorystream.h"
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

        JSONPath(string  value_) {
            isLast = true;
            value = value_;
            rangeStart = -1;
        }

        JSONPath(string  value_, int range) {
            isLast = true;
            value = value_;
            rangeStart = range;
        }
    };


    class JSONPathParser {
    public:
        // facilmente ottimizzabile (non oggi o forse si ) 
        static vector<JSONPath> jsonPathParse(string jsonpath) 
        {
            vector<string> split = JSONPathParser::split(jsonpath, ".");
            vector<JSONPath> paths;
            for (int i = 0; i < split.size(); i++) {
                string current_value = split[i];
                size_t open = current_value.find('[');
                size_t close = current_value.find(']');
                if (open != string::npos && close != string::npos) {
                    vector<string> splitted = JSONPathParser::split(current_value, "[");
                    current_value = splitted[0];
                    string::size_type sz;
                    string s_index = splitted[1].erase(splitted[1].size() - 1);
                    int index = stoi(s_index, &sz);
                    JSONPath path(current_value, index);
                    paths.push_back(path);
                }
                else {
                    JSONPath path(current_value);
                    paths.push_back(path);
                } 
            }
            return paths;
        };
   
    
  

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

        PifferoStatus(JSONPath& path): jsonPath(path) {
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

    // non è più ricorsivo ma sti cazzi 
    class RecorsiveParser {
    public:
        bool hasNext_ = false; 
        stringstream* ss;
        Last last;
        bool isRecording = true;
        JSONPath& jsonpath;

    
        RecorsiveParser(stringstream& out, JSONPath& path, bool hasNext):jsonpath(path) {
            ss = &out;
            hasNext_ = hasNext;
        }
        

        bool Null() {
            addCommaIfNeeded();
            if (hasNext_) {
                return true;
            }
            *ss << "null";
            last = value;
            return true;
        }
        bool Bool(bool b) {
            if (hasNext_) {
                return true;
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
            if (hasNext_) {
                return true;
            }
            addCommaIfNeeded();
            *ss << i;
            last = value;
            return true;
        }

        bool Uint(unsigned u) {
            if (hasNext_) {
                return true;;
            }
            addCommaIfNeeded();
            *ss << u;
            last = value;
            return true;
        }
        bool Int64(int64_t i) {
            if (hasNext_) {
                return true;
            }
            addCommaIfNeeded();
            *ss << i;
            last = value;
            return true;
        }
        bool Uint64(uint64_t u) {
            if (hasNext_) {
                return true;
            }
            addCommaIfNeeded();
            *ss << u;
            //ParseNext();
            last = value;
            return true;
        }
        bool Double(double d) {
            if (hasNext_) {
                return true;
            }
            addCommaIfNeeded();
            *ss << d;
            last = value;
            //ParseNext();
            return true;
        }
        bool RawNumber(const char* str, SizeType length, bool copy) {
            if (hasNext_) {
                return true;
            }
            addCommaIfNeeded();
            *ss << str;
            last = value;
            return true;
        }

        bool String(const char* str, SizeType length, bool copy) {
            if (hasNext_) {
                return true;
            }
            addCommaIfNeeded();
            *ss << "\"" << str << "\"";
            last = value;
     
            return true;
        }
        bool StartObject() {
            if (hasNext_) {
                return true;
            }
            addCommaIfNeeded();
            *ss << "{";
            last = startobject;
            return true;
        }
        bool Key(const char* str, SizeType length, bool copy) {
            if (hasNext_) {
                return true;
            }
            addCommaIfNeeded();
            *ss << "\"" << str << "\"" << ":";
            last = key;
            return true;
        }
        bool EndObject(SizeType memberCount) {
            if (hasNext_) {
                return true;
            }
            *ss << "}";
            last = endobject;
            return true;
        }
        bool StartArray() {
            if (hasNext_) {
                return true;
            }
            addCommaIfNeeded();
            *ss << "[";
            last = startarray;
            return true;
        }
        bool EndArray(SizeType elementCount) {
            if (hasNext_) {
                return true;
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


    class MainParser {
    public:
        vector<RecorsiveParser>& parserVector;
        bool isLast = false;
        int parserIndex = 0;

        MainParser(vector<RecorsiveParser>& parserVector_): parserVector(parserVector_) {
        }
        void shiftParser() {
            if (parserVector[parserIndex].isRecording && !isLast) {
                parserIndex++;
                isLast = parserIndex == parserVector.size() - 1;
            }
        }

        bool Null() {
            bool result = parserVector[parserIndex].Null();
            shiftParser();
            return true;
        }
        bool Bool(bool b) {
            bool result = parserVector[parserIndex].Bool(b);
            shiftParser();
            return true;
        }
        bool Int(int i) {
            bool result = parserVector[parserIndex].Int(i);
            shiftParser();
            return true;
        }

        bool Uint(unsigned u) {
            bool result = parserVector[parserIndex].Uint(u);
            shiftParser();
            return true;
        }
        bool Int64(int64_t i) {
            bool result = parserVector[parserIndex].Int64(i);
            shiftParser();
            return true;
        }
        bool Uint64(uint64_t u) {
            bool result = parserVector[parserIndex].Uint64(u);
            shiftParser();
            return true;
        }
        bool Double(double d) {
            bool result = parserVector[parserIndex].Double(d);
            shiftParser();
            return true;
        }
        bool RawNumber(const char* str, SizeType length, bool copy) {
            bool result = parserVector[parserIndex].RawNumber(str, length, copy);
            shiftParser();
            return true;
        }

        bool String(const char* str, SizeType length, bool copy) {
            bool result = parserVector[parserIndex].String(str, length, copy);
            shiftParser();
            return true;
        }
        bool StartObject() {
            bool result = parserVector[parserIndex].StartObject();
            shiftParser();
            return true;
        }
        bool Key(const char* str, SizeType length, bool copy) {
            bool result = parserVector[parserIndex].Key(str, length, copy);
            shiftParser();
            return true;
        }
        bool EndObject(SizeType memberCount) {
            bool result = parserVector[parserIndex].EndObject(memberCount);
            shiftParser();
            return true;
        }
        bool StartArray() {
            bool result = parserVector[parserIndex].StartArray();
            shiftParser();
            return true;
        }
        bool EndArray(SizeType elementCount) {
            bool result = parserVector[parserIndex].EndArray(elementCount);
            shiftParser();
            return true;
        }

    };

    template <typename InputStream, typename OutputStream>
    class JSONParser {
    public:
        JSONParser() {
        }
        void parsePath(InputStream& is, stringstream& os, string path) {
            vector<JSONPath> jsonpath = JSONPathParser::jsonPathParse(path);
            vector<RecorsiveParser> parserVector;
         
            for (int i = 0; i < jsonpath.size() - 1; i ++ ) {
                 RecorsiveParser parser(os, jsonpath[i], true);
                 parserVector.push_back(parser);
            
            }

            RecorsiveParser parser(os, jsonpath[jsonpath.size() - 1], false);
            parserVector.push_back(parser);

            MainParser master(parserVector);
            Reader reader;
            reader.Parse(is, master);
        }

    };

}