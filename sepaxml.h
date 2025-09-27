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
#include "xmldoc.h"

#include "applicationrequest.h"
#include "applicationresponse.h"

class SoapBuilder {

public:
    void set_create_cert_contents();
    void add_encrypted_request_to_soap(XmlDoc *encreq, std::string path);
    void add_body_to_header();
    void process_header();
    void set_bank_certificate_contents();
    void add_bank_certificate_body_to_soap();
    void set_generic_request_contents(); 
    void add_encrypted_generic_request_to_soap(XmlDoc *encrypted_request);

    ApplicationRequest m_application_request;
    std::map<std::string, std::string> m_params;
    XmlDoc m_header_template;
    XmlDoc m_body_template;

    SoapBuilder(std::map<std::string, std::string> &params): m_params(params) {
        m_params["language"] = "EN";
        
        m_params["software_id"]= "libgsepa-1.0";
        m_params["iso_time"]= Utilities::currentIsoDateTime();
		m_params["request_id"]= Utilities::getRandomHexString(10);
		m_params["iso_expires"]= Utilities::currentIsoDateTime(5); // 5 min after creation

		m_params["timestamp_id"]= "timestamp-"+ Utilities::randomUUID();
		m_params["body_id"]= "body-"+Utilities::randomUUID();
		m_params["token_id"]= "token-"+Utilities::randomUUID();

        if(m_params["status"] == "") {
            m_params["status"] = "NEW";
        }
        if(m_params["test_with_constant_values"]=="true"){ // for testing only

            m_params["iso_time"]= "2023-04-13T05:38:05Z";
            m_params["iso_expires"]= "2023-04-13T05:43:05Z";
            m_params["request_id"]= "7bc83a4ce48";
            m_params["timestamp_id"]= "timestamp-58b5fc852e42c6a8d";
            m_params["body_id"]= "body-28e5958ab367a56f0";
            m_params["token_id"]= "token-8859fcc577d15af3e";

			m_params["xxx_encrypt_key"]="TEST1234567890";
			m_params["xxx_encrypt_iv"]="HJHFHJHF";
		}

    }
    tinyxml2::XMLError init_build_soap();
    tinyxml2::XMLError build_renew_certificate_request();
    tinyxml2::XMLError build_get_bank_certificate_request();
    
    std::string to_XML();
};

class DanskeSoapBuilder: public SoapBuilder {
public:
    DanskeSoapBuilder(std::map<std::string, std::string> &params): SoapBuilder(params){

    }
    tinyxml2::XMLError build_certificate_request();
    tinyxml2::XMLError build_soap();
    tinyxml2::XMLError build_generic_request();
};
class AktiaSoapBuilder: public SoapBuilder {
	//https://www.samlink.fi/wp-content/uploads/2018/10/WSPalvelukuvausAktia.pdf
    tinyxml2::XMLError build_certificate_request();
    
    tinyxml2::XMLError build_generic_request();
    tinyxml2::XMLError set_create_cert_contents();

public:    
    AktiaSoapBuilder(std::map<std::string, std::string> &params): SoapBuilder(params){

    }
    tinyxml2::XMLError build_soap();
};
class OpSoapBuilder: public SoapBuilder {

};
class NordeaSoapBuilder: public SoapBuilder {

};

class SoapResponse {
public: 
	std::map<std::string, std::string> m_params;
	ApplicationResponse m_application_response;
 	SoapResponse(std::map<std::string, std::string> params, std::string xml_response): 
		m_params(params),
		m_application_response(params, xml_response) {
	}
};
class DanskeResponse: public SoapResponse {
	public:
		DanskeResponse(std::map<std::string, std::string> params, std::string xml_response): SoapResponse(params, xml_response){

		}
		std::string to_XML() {
			return m_application_response.to_XML();
		}
		bool validate_response() {
			if(m_params["command"] == "get_certificate" ||  // ['get_certificate', 'get_bank_certificate', 'create_certificate', "renew_certificate"].contains(params['command']) ){
				m_params["command"] == "get_bank_certificate" ||
				m_params["command"] == "create_certificate" ||
				m_params["command"] == "renew_certificate") {
				int responseCode = 12;

				//general error
				if(m_application_response.el("pkif:PKIFactoryServiceFault") != NULL) {
					//if(params['debug']) println "ERROR Response (code: " + response.getTagText("pkif:ReturnCode") + "): " +response.getTagText("pkif:ReturnText")
					//this.errorMessage=response.getTagText("pkif:ReturnText")
					return false;
				}

				if(m_params["command"] == "get_bank_certificate"){ 
					//if(params['debug']) println "Response (code: " + response.getTagText("pkie:ReturnCode") + "): " +response.getTagText("pkie:ReturnText")
					//this.errorMessage=response.getTagText("pkie:ReturnText")
					responseCode = atoi(m_application_response.el("pkie:ReturnCode")->GetText());
				}
				else if(m_params["command"] == "create_certificate" || m_params["command"] == "renew_certificate"){
					//this.errorMessage=response.getTagText("tns:ReturnText")
					//if(params['debug']) println "Response (code: " + response.getTagText("tns:ReturnCode") + "): " +response.getTagText("tns:ReturnText")
					responseCode = atoi(m_application_response.el("tns:ReturnCode")->GetText());
				}
				else{
					//if(params['debug']) println("XXX: unknown response:" + response.toXml() )
				}
				
				if(responseCode != 0){
					return false;
				}
				return true;
			}
			else { 
				//if(params['debug']) println "DanskeResponse (code: " + response.getTagText("ResponseCode") + "): " +response.getTagText("ResponseText")
				//this.errorMessage=response.getTagText("ResponseText")
				int rcode = atoi(m_application_response.el("ResponseCode")->GetText());
				if(rcode != 0){
					return false;
				}
				return true;
			}
			return true;
		}
};
