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

class ApplicationRequest {
    XmlDoc m_application_request;
	std::map<std::string, std::string> m_params;
public:
    XmlDoc &xml_doc () {
        return m_application_request;
    }
    
   tinyxml2::XMLError build_application_request(std::map<std::string, std::string> parms, bool process=true) {
		m_params = parms;
		tinyxml2::XMLError  err = Utilities::load_body_template(m_params["command"], m_params["bank"], m_application_request);
        if(err != tinyxml2::XML_SUCCESS) {
            return err;
        }
		//println "application_request: " + application_request
		if(process) 
			process_xml();

        //std::cout << "DEBUG xml: " << m_application_request.tostring() << std::endl;
		return err;
	}
    
	void process_xml() {

		set_common_nodes();
		set_nodes_contents();
		process_signature();
		return;
	}
	void set_common_nodes() {
		if((m_params["command"] == "get_bank_certificate" ||
			m_params["command"] == "create_certificate" ||
			m_params["command"] == "renew_certificate" ||
			m_params["command"] == "get_certificate")) {
			return;
		}
        m_application_request.el("Environment")->SetText(Utilities::string_toupper(m_params["environment"]).c_str() );
		m_application_request.el("CustomerId")->SetText(m_params["customer_id"].c_str() );
		m_application_request.el("Timestamp")->SetText(m_params["iso_time"].c_str() );
		m_application_request.el("SoftwareId")->SetText(m_params["software_id"].c_str() );
		m_application_request.el("Command")->SetText(Utilities::formatCommand(m_params["command"]).c_str()  );
        
	}
    
	void set_nodes_contents() {
        
		if(m_params["command"] == "create_certificate") set_create_certificate_nodes();
		//TODO: else if(m_params["command"] == "get_certificate") set_get_certificate_nodes();
		else if(m_params["command"] == "download_file_list") set_download_file_list_nodes();
		else if(m_params["command"] == "download_file") set_download_file_nodes();	
		else if(m_params["command"] == "upload_file") set_upload_file_nodes();
		else if(m_params["command"] == "get_bank_certificate") set_get_bank_certificate_nodes();
		else if(m_params["command"] == "renew_certificate" || m_params["command"] == "get_certificate" ) {
			if(m_params["bank"] == "aktia") {
				set_create_certificate_nodes();
				return;
			}
			set_renew_certificate_nodes();
		}
		else if(m_params["debug"] != "") {
            std::cout << "DEBUG: !!! **** missing/invalid sepa command ***** !!!!" << std::endl;
        }
        
	}
	void set_renew_certificate_nodes() {
        
		m_application_request.el("tns:CustomerId")->SetText(m_params["customer_id"].c_str());
		m_application_request.el("tns:Timestamp")->SetText(m_params["iso_time"].c_str());
		m_application_request.el("tns:RequestId")->SetText("");
		m_application_request.el("tns:KeyGeneratorType")->SetText("software");
		m_application_request.el("tns:EncryptionCertPKCS10")->SetText(Utilities::format_cert_request(m_params["encryption_csr"]).c_str() );
		m_application_request.el("tns:SigningCertPKCS10")->SetText(Utilities::format_cert_request(m_params["signing_csr"]).c_str() );
		m_application_request.el("tns:Environment")->SetText(m_params["environment"].c_str());
		if(m_params["environment"]=="test" && m_params["bank"] == "danske") {
			m_application_request.el("tns:Environment")->SetText("customertest");
		}
        
	}
	void set_create_certificate_nodes() {
		if(m_params["bank"] == "aktia") {

			m_application_request.el("CustomerId")->SetText(m_params["customer_id"].c_str());
			m_application_request.el("Timestamp")->SetText(m_params["iso_time"].c_str());
			m_application_request.el("Environment")->SetText(Utilities::string_toupper(m_params["environment"]).c_str() );
			m_application_request.el("SoftwareId")->SetText(m_params["software_id"].c_str());
			m_application_request.el("Command")->SetText("GetCertificate");

			if(m_params["command"] == "renew_certificate") {
				m_application_request.el("RequestId")->SetText(m_params["request_id"].c_str());
				m_application_request.el("Service")->SetText("ISSUER");

				m_application_request.el_delete("TransferKey");
			}
			else {
				m_application_request.el("TransferKey")->SetText(m_params["pin"].c_str());
			}
			m_application_request.el( "Content")->SetText(Utilities::format_cert_request(m_params["signing_csr"]).c_str() );
			//println "set_create_certificate_nodes [command: "+m_params["command"]+"]: " + application_request
		}
		else {
			m_application_request.el("tns:CustomerId")->SetText(m_params["customer_id"].c_str());
			m_application_request.el("tns:Timestamp")->SetText(m_params["iso_time"].c_str());
			m_application_request.el("tns:RequestId")->SetText(m_params["request_id"].c_str());
			m_application_request.el("tns:KeyGeneratorType")->SetText("software");
			m_application_request.el("tns:EncryptionCertPKCS10")->SetText(Utilities::format_cert_request(m_params["encryption_csr"]).c_str() );
			m_application_request.el("tns:SigningCertPKCS10")->SetText(Utilities::format_cert_request(m_params["signing_csr"]).c_str() );
			m_application_request.el("tns:Environment")->SetText(m_params["environment"].c_str());
			if(m_params["environment"]=="test" && m_params["bank"] == "danske") {
				m_application_request.el("tns:Environment")->SetText("customertest");
			}

			m_application_request.el("tns:PIN")->SetText(m_params["pin"].c_str());
		}
        
	}
	void set_get_bank_certificate_nodes() {
		m_application_request.el("elem:BankRootCertificateSerialNo")->SetText("1111110002");
		m_application_request.el("elem:Timestamp")->SetText(m_params["iso_time"].c_str());	
	}
	void process_signature() {

		if(m_params["command"] == "get_certificate" || 
            m_params["command"] == "get_bank_certificate" ||
            m_params["command"] == "create_certificate")
			return;

		if(m_params["command"] == "renew_certificate") {
			if(m_params["bank"] == "aktia") {
			}
			process_signature_renew_certificate();
			return;
		}
		tinyxml2::XMLElement* new_el = m_application_request.el_copy("dsig:Signature");
		m_application_request.el_delete("dsig:Signature");
        std::string digest_in =m_application_request.el_tostring("ApplicationRequest");
        
		std::string digest = Utilities::calculate_digest(digest_in, m_params["debug"] == "true");
        if(m_params["test_with_constant_values"]=="true"){
            if(m_params["bank"]=="aktia" && m_params["command"] == "download_file_list") {
                std::string d_in = R"(<ApplicationRequest xmlns="http://bxd.fi/xmldata/">
  <CustomerId>21648386</CustomerId>
  <Command>DownloadFileList</Command>
  <Timestamp>2023-04-13T05:38:05Z</Timestamp>
  <Status>NEW</Status>
  <Environment>PRODUCTION</Environment>
  <SoftwareId>libgsepa-1.0</SoftwareId>
  <FileType>RA</FileType>
  
</ApplicationRequest>)";
                if(d_in != digest_in) {
                    LOG(DEBUG) << "ApplicationRequest digest_in check failed!!!"; 
                }
                if(digest != "bKYSCpOrJA6gBvBo8+bdeHS8C3I=") {
                    LOG(DEBUG) << "ApplicationRequest digest check failed!!!"; 
                }
            }
        }

        new_el->SetAttribute("xmlns:dsig", "http://www.w3.org/2000/09/xmldsig#");
		
		auto ar = m_application_request.el("ApplicationRequest");
		ar->InsertEndChild(new_el);

		m_application_request.el("dsig:DigestValue")->SetText( (digest+"\n").c_str() );
		m_application_request.el("dsig:SignedInfo")->SetAttribute("xmlns", "http://bxd.fi/xmldata/");
		m_application_request.el("dsig:SignedInfo")->SetAttribute("xmlns:dsig", "http://www.w3.org/2000/09/xmldsig#");

        m_application_request.el("dsig:CanonicalizationMethod")->SetText("");
        m_application_request.el("dsig:SignatureMethod")->SetText("");
        m_application_request.el("dsig:Transform")->SetText("");
        m_application_request.el("dsig:DigestMethod")->SetText("");

        //std::cout << "DEBUG xml: " << m_application_request.tostring() << std::endl;
        std::string sig_in = m_application_request.el_tostring("dsig:SignedInfo"); 
        
        std::string sig_out = Utilities::calculate_signature(sig_in, m_params["signing_private_key"], true, m_params["debug"]=="true" );
        if(m_params["test_with_constant_values"]=="true"){
            if(m_params["bank"]=="aktia" && m_params["command"] == "download_file_list") {
                std::string sin =R"(<dsig:SignedInfo xmlns="http://bxd.fi/xmldata/" xmlns:dsig="http://www.w3.org/2000/09/xmldsig#">
      <dsig:CanonicalizationMethod Algorithm="http://www.w3.org/TR/2001/REC-xml-c14n-20010315#WithComments"></dsig:CanonicalizationMethod>
      <dsig:SignatureMethod Algorithm="http://www.w3.org/2000/09/xmldsig#rsa-sha1"></dsig:SignatureMethod>
      <dsig:Reference URI="">
        <dsig:Transforms>
          <dsig:Transform Algorithm="http://www.w3.org/2000/09/xmldsig#enveloped-signature"></dsig:Transform>
        </dsig:Transforms>
        <dsig:DigestMethod Algorithm="http://www.w3.org/2000/09/xmldsig#sha1"></dsig:DigestMethod>
        <dsig:DigestValue>bKYSCpOrJA6gBvBo8+bdeHS8C3I=
</dsig:DigestValue>
      </dsig:Reference>
    </dsig:SignedInfo>)";
                std::string sout = R"(Tt95x3ipaE4LFfmuXszqe0GSDmS3aAhPniyUdU1cjh0MYm+WajjzZ6HdDTE0
nS5zo/45NQ7afiS23pzPQSzIUT4hwLq8flH4rhk6GQy0woO5zTWm1TA9ypfa
MZrRKOZOXlT3y39/4guYD+m0QFAp7sM2djnnZQMD9fDOHwqRFWY8cJrUtE/l
STkwQxKXDQXNMjkEtnGwngBjs4KHO380HVcaRUHuUYZpy5h11KnXPZ4ySrsa
mRQvZ6gQVpEEc4r59AVXYbN3wIYeY9oVK6qXqKPFP8a5o24l7dut2ewWukEA
GAGog6cWFMsANi7w5h+0hYO3jjprgQBnn+8VtpqwAQ==
)";
                if(sig_in != sin) {
                    LOG(DEBUG) << "dsig:SignedInfo signature_in check failed!!!"; 
                }
                if(sig_out != sout) {
                    LOG(DEBUG) << "dsig:SignedInfo signature_out check failed!!!"; 
                }
            }
        }
		m_application_request.el("dsig:SignatureValue")->SetText(sig_out.c_str());
		m_application_request.el("dsig:X509Certificate")->SetText(Utilities::format_cert(m_params["own_signing_certificate"]).c_str() );
        
	}
	void process_signature_renew_certificate() {
        
		if(m_params["command"] == "get_certificate" || m_params["command"] == "get_bank_certificate" || m_params["command"] == "create_certificate")
			return;

		std::string signAttrPrefix = "dsig:";
		std::string signAttrPostfix = ":dsig";
		if(m_params["bank"] == "danske" && m_params["command"] == "renew_certificate") {
			signAttrPrefix="";
			signAttrPostfix="";
		}
		std::string sigtemp = m_application_request.el_tostring(signAttrPrefix+"Signature");
		m_application_request.el_delete(signAttrPrefix+"Signature");
		
        std::string digest = Utilities::calculate_digest(m_application_request.tostring(), m_params["debug"] == "true");

		if(m_params["command"] == "renew_certificate") {
			if(m_params["bank"] == "danske") {
				tinyxml2::XMLElement* newElement = m_application_request.NewElement(std::string(signAttrPrefix+"Signature").c_str());
				newElement->SetAttribute(std::string("xmlns"+signAttrPostfix).c_str(), "http://www.w3.org/2000/09/xmldsig#");
				newElement->SetText(sigtemp.c_str());
				auto ar = m_application_request.el("tns:RenewCertificateRequest");
				ar->InsertEndChild(newElement);
			}
			else {
				tinyxml2::XMLElement* newElement = m_application_request.NewElement(std::string(signAttrPrefix+"Signature").c_str());
				newElement->SetAttribute(std::string("xmlns"+signAttrPostfix).c_str(), "http://www.w3.org/2000/09/xmldsig#");
				newElement->SetText(sigtemp.c_str());
				auto ar = m_application_request.el("CertApplicationRequest");
				ar->InsertEndChild(newElement);

				auto el = m_application_request.el(std::string(signAttrPrefix+ "SignedInfo").c_str());
				el->SetAttribute("xmlns", "http://op.fi/mlp/xmldata/");
			}
		}
		else {
			tinyxml2::XMLElement* newElement = m_application_request.NewElement(std::string(signAttrPrefix+"Signature").c_str());
			newElement->SetAttribute(std::string("xmlns"+signAttrPostfix).c_str(), "http://www.w3.org/2000/09/xmldsig#");
			newElement->SetText(sigtemp.c_str());
			auto ar = m_application_request.el("CertApplicationRequest");
			ar->InsertEndChild(newElement);

			auto el = m_application_request.el(std::string(signAttrPrefix+ "SignedInfo"));
			el->SetAttribute("xmlns", "http://bxd.fi/xmldata/");
		}
		auto el2 = m_application_request.el(std::string(signAttrPrefix+ "SignedInfo"));
		el2->SetAttribute(std::string("xmlns"+signAttrPostfix).c_str(), "http://www.w3.org/2000/09/xmldsig#");

		m_application_request.el(std::string(signAttrPrefix+"CanonicalizationMethod"))->SetText("");
		m_application_request.el(std::string(signAttrPrefix+"SignatureMethod"))->SetText("");
		m_application_request.el(std::string(signAttrPrefix+"Transform"))->SetText("");
		m_application_request.el(std::string(signAttrPrefix+"DigestMethod"))->SetText("");

		if(m_params["bank"] != "aktia") {
			//m_application_request.OpenElement("${signAttrPrefix}Transform", 1)
			m_application_request.el(std::string(signAttrPrefix+"Transform"))->SetText("");
		}
		m_application_request.el(std::string(signAttrPrefix+"SignatureValue"))->SetText(std::string(digest + "\n").c_str());
		m_application_request.el(std::string(signAttrPrefix+"X509Certificate"))->SetText(Utilities::format_cert(m_params["own_signing_certificate"]).c_str());
		m_application_request.el(std::string(signAttrPrefix+"DigestValue"))->SetText(Utilities::calculate_signature(m_application_request.el_tostring(std::string(signAttrPrefix+"SignedInfo") ), m_params["signing_private_key"]).c_str() );


		//println "process_signature_renew_certificate() APPLICATIONREQUEST: >>"  + application_request.toXml()
        
	}
	void set_download_file_list_nodes() {
        
		m_application_request.el("Status")->SetText(m_params["status"].c_str());
		if(m_params["file_type"] != "")
			m_application_request.el("FileType")->SetText(m_params["file_type"].c_str());
		else
			m_application_request.el("FileType");
        
	}
	void set_download_file_nodes() {
		m_application_request.el("Status")->SetText(m_params["status"].c_str() );
		m_application_request.el("FileType")->SetText(m_params["file_type"].c_str());
		m_application_request.el("FileReference")->SetText(m_params["file_reference"].c_str());
        
	}
	void set_upload_file_nodes() {
		m_application_request.el("Content")->SetText(Utilities::encode(m_params["content"]).c_str());
		m_application_request.el("FileType")->SetText(m_params["file_type"].c_str());
	}
    
};