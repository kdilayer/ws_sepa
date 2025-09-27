/*
 * Copyright (c) 2025 @https://github.com/kdilayer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <map>
#include "tinyxml2.h"
#include "utilities.h"

#include <vector>

class XmlDoc: public tinyxml2::XMLDocument {
    bool print_compact = true;
    public:
        XmlDoc()/*: tinyxml2::XMLDocument(true, tinyxml2::COLLAPSE_WHITESPACE)*/ {
        }
        XmlDoc(std::string content)/*: tinyxml2::XMLDocument(true, tinyxml2::COLLAPSE_WHITESPACE)*/ {
            this->Parse(content.c_str());
        }
        void el_insert_child(std::string const &name, tinyxml2::XMLElement *child) {
            auto* _el = el(name);
            _el->InsertEndChild(child->DeepClone(this));

        }
        std::string el_tostring(tinyxml2::XMLElement *_el) {
            if(_el) {
                tinyxml2::XMLPrinter printer(NULL, print_compact);
                _el->Accept( &printer );
                std::string s = std::string(printer.CStr());

                if (!s.empty() && s[s.length()-1] == '\n') {
                    s.erase(s.length()-1);
                }
                return s;
            }
            return "";

        }
        std::string el_tostring(std::string const &name){
            auto* _el = el(name);
            if(_el) {
                tinyxml2::XMLPrinter printer(NULL, print_compact);
                _el->Accept( &printer );
                std::string s = std::string(printer.CStr());

                if (!s.empty() && s[s.length()-1] == '\n') {
                    s.erase(s.length()-1);
                }
                return s;
            }
            return "";
        }
        std::string tostring() {
            tinyxml2::XMLPrinter printer(NULL, print_compact);
            this->Print( &printer );
            return std::string(printer.CStr());
        }
        tinyxml2::XMLElement *el_copy(std::string const &name){

            auto* _el = el(name);
            if(_el) {
                 tinyxml2::XMLNode *_n = _el->DeepClone(NULL);
                 if(_n) {
                    return _n->ToElement();
                 }
                /*
                tinyxml2::XMLElement* new_el =  this->NewElement(name.c_str());
                _copy(new_el, _el);
                return new_el;
                */
            }
            return NULL;
        }
        void el_delete(std::string const &name) {
            auto* ch = el(name);
            if(ch) {
                ch->Parent()->DeleteChild(ch);
            }
        }
        tinyxml2::XMLElement *el(std::string name, int nth_el=0) {
            tinyxml2::XMLNode * xElem = this->FirstChild();
            int ctr=0;
            while(xElem)
            {
                if (xElem->Value() && !std::string(xElem->Value()).compare(name)) {
                    if(nth_el == ctr) {
                        return xElem->ToElement();
                    }
                    ctr++;
                }
                /*
                *   We move through the XML tree following these rules (basically in-order tree walk):
                *   
                *   (1) if there is one or more child element(s) visit the first one
                *       else
                *   (2)     if there is one or more next sibling element(s) visit the first one
                *               else
                *   (3)             move to the parent until there is one or more next sibling elements
                *   (4)             if we reach the end break the loop
                */
                if (xElem->FirstChildElement()) //(1)
                    xElem = xElem->FirstChildElement();
                else if (xElem->NextSiblingElement())  //(2)
                    xElem = xElem->NextSiblingElement();
                else
                {
                    while(xElem->Parent() && !xElem->Parent()->NextSiblingElement()) //(3)
                        xElem = xElem->Parent();
                    if(xElem->Parent() && xElem->Parent()->NextSiblingElement())
                        xElem = xElem->Parent()->NextSiblingElement();
                    else //(4)
                        break;
                }//else
            }//while
            return NULL;
        }
        
        void _copy(tinyxml2::XMLNode *p_dest_parent, const tinyxml2::XMLNode *p_src){
            // Protect from evil
            if (p_dest_parent == NULL || p_src == NULL) {
                return;
            }

            // Get the document context where new memory will be allocated from
            tinyxml2::XMLDocument *p_doc = p_dest_parent->GetDocument();

            // Make the copy
            tinyxml2::XMLNode *p_copy = p_src->ShallowClone(p_doc);
            if (p_copy == NULL) {
                // Error handling required (e.g. throw)
                return;
            }
            // Add this child
            p_dest_parent->InsertEndChild(p_copy);

            // Add the grandkids
            for (const tinyxml2::XMLNode *p_node = p_src->FirstChild(); p_node != NULL; p_node = p_node->NextSibling()) {
                _copy(p_copy, p_node);
            }
        }
};