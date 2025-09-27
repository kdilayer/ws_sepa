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
#include <fstream>

#include "utilities.h"
#include "httplib.h"
#include "util.h"

class SoapClient {
	std::map<std::string, std::string> m_params;
    std::string m_response_string;
	std::tuple<std::string, std::string> location() {
		//wsdlsoap:address, att: location
		bool is_pki=false;
		if(m_params["command"] == "get_bank_certificate"  ||
			m_params["command"] == "create_certificate"  ||
			m_params["command"] == "renew_certificate" || 
			m_params["command"] == "get_certificate") {
				is_pki=true;
		}
		std::string loc;
        std::string path;
		if(m_params["bank"]=="aktia") {
			if(is_pki) {
				loc = "https://ws.samlink.fi"; 
                path = "/wsdl/CertificateService.xml";
			}
			else {
				loc = "https://ws.samlink.fi"; 
                path = "/services/CorporateFileService";
			}
		}
		else if(m_params["bank"]=="op") {
			if(is_pki) {
				loc = "https://wsk.op.fi"; 
                path = "/services/OPCertificateService";
			}
			else {
				loc = "https://wsk.op.fi/services/CorporateFileService"; 
                path = "/services/CorporateFileService";
			}
		}
		else if(m_params["bank"]=="danske") {
			if(is_pki) {
				loc = "https://businessws.danskebank.com";
                 path = "/ra/pkiservice.asmx";
			}
			else {
				loc = "https://businessws.sampopankki.fi";
                path = "/edifileservice/edifileservice.asmx";
			}
		}
		return std::make_tuple(loc, path);
	}
public: 
	SoapClient(std::map<std::string, std::string> params): m_params(params){

    }
	bool send_request(std::string command, std::string xml, bool debug = false) {
		std::string cmd = Utilities::formatCommand(command);
        
        std::string _host;
        std::string _path;
        tie(_host, _path) = location();
        //LOG(DEBUG) << "Connecting to: " << _host << _path;

		httplib::Client cli(_host);
        cli.set_default_headers({
        { "SOAPAction", cmd }
        });
        httplib::Result resp = cli.Post(_path, xml, "text/xml; charset=utf-8");
		if(!resp || resp.error() != httplib::Error::Success) {
			LOG(ERROR) << "HTTP request failed: " << httplib::to_string(resp.error());
			return false;
		}
		auto respval = resp.value();

        m_response_string=respval.body;
		//m_response_string = iso_8859_1_to_utf8(m_response_string);

        if(resp.value().status == 200) {
            return true;
        }
		return false;
	}
	bool has_response() {
		if(m_response_string == "") {
            return false;
        };
        return true;
	}
	std::string raw_response() {
		return m_response_string;
	}
};