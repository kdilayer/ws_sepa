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


std::string uchar_tohex(unsigned char * what, size_t len);

class ApplicationResponse: public XmlDoc {
	std::map<std::string, std::string> m_params;
    std::string m_error_message;
    std::string m_error_code;
	std::string decrypt_embedded_key(std::string encrypted_application_response) {

		LOG(DEBUG) << "enc_ar: " << encrypted_application_response;
		XmlDoc d(encrypted_application_response);
		std::string cv;
        if(d.el("xenc:CipherValue")) {
            cv = d.el("xenc:CipherValue")->GetText();
        }
        else {
            LOG(DEBUG) << "encrypted application response does not cotain ChipherValue";
            return "";
        }
		
		
		Utilities::replaceAll(cv, "\r", "");
		Utilities::replaceAll(cv, "\n", "");

		std::cout << "to_decoding ["<<std::to_string(cv.length()) << "]: >>" << cv << "<<" << std::endl;
		
		std::string cv_out = Utilities::decode(cv);
		/*
		unsigned char *cv_out = NULL;
		int cv_out_len= 0;

		Utilities::decode_b(cv, &cv_out, &cv_out_len);
		*/
		//std::cout << "decoded: " << uchar_tohex((unsigned char*)cv_out.c_str(), cv_out.length()) << std::endl;

		std::string prv_key_pem = m_params["encryption_private_key"];
		if(m_params["testing"] == "true") {
			prv_key_pem = m_params["signing_private_key"];
		}
		std::string decrypted_key = Utilities::decrypt_key(prv_key_pem, (unsigned char*)cv_out.c_str(), cv_out.length());


		/*
		byte[] enc_key = Utilities.decode(encrypted_application_response.getTagText("xenc:CipherValue"))
		PKCS8EncodedKeySpec keySpec = new PKCS8EncodedKeySpec( Utilities.decode(Utilities.format_prv_key(params['encryption_private_key'])) );
		KeyFactory kf = KeyFactory.getInstance("RSA");
	    	PrivateKey privKey = kf.generatePrivate(keySpec);

		Cipher cipher = Cipher.getInstance("RSA");  
		cipher.init(Cipher.DECRYPT_MODE, privKey);  
		byte[] out = cipher.doFinal(enc_key);  
		return out
		*/
		return "";
	}
    std::string decrypt_application_response(std::string encrypted_application_response) {
        std::string key = decrypt_embedded_key(encrypted_application_response);
        return "todo";
		/*
		XmlDoc encrypted_application_response = new XmlDoc(new String(Utilities.decode(response.getTagText("ApplicationResponse")), "UTF-8"))

		def key = decrypt_embedded_key(encrypted_application_response)
		//println "encrypted application response: " + encrypted_application_response

		byte []encrypted_data_d = Utilities.decode(encrypted_application_response.getTagText("xenc:CipherValue", 1))
		byte []iv = new byte[8];
		byte []encrypted_data = new byte[encrypted_data_d.size()-8]
		System.arraycopy(encrypted_data_d, 0, iv, 0, 8);
		System.arraycopy(encrypted_data_d, 8, encrypted_data, 0, encrypted_data_d.size()-8);


		SecretKey secretKey = new SecretKeySpec(key, "DESede"); //"**DESede/PKCS#5**");
		IvParameterSpec ivSpec = new IvParameterSpec(iv);
		Cipher cipher = Cipher.getInstance("DESede/CBC/PKCS5Padding");
		cipher.init(Cipher.DECRYPT_MODE, secretKey, ivSpec);
		byte[] decout = cipher.doFinal(encrypted_data);
		String decrypted = new String(decout, "UTF-8")
		//println "decrypted_data: " + decrypted
		return new XmlDoc(decrypted)
		*/        
    }
	std::string read_application_response() {
		std::string encoded_string;
        bool is_encrypted = false;
        if(m_params["bank"] == "aktia" || m_params["bank"] == "op") {
            if(this->el("mod:ApplicationResponse")) {
                encoded_string = this->el("mod:ApplicationResponse")->GetText();
            }
            else {
                encoded_string = this->el("cer:ApplicationResponse")->GetText();
            }
        }
        else {
            //other banks - nordea etc...
            encoded_string = this->el("ApplicationResponse")->GetText();
            is_encrypted = true;
        }
		Utilities::replaceAll(encoded_string, "\r", "");
		Utilities::replaceAll(encoded_string, "\n", "");

		std::string decoded_response = Utilities::decode(encoded_string);
        //LOG(DEBUG) << "Decoded response: >>" << decoded_response << "<<";

        if(is_encrypted)
            return decrypt_application_response(decoded_response);

		return decoded_response;
	}
public:
    ApplicationResponse(std::map<std::string, std::string> &params, std::string xml_response): m_params(params) {
		this->Parse(xml_response.c_str());
	}
    bool is_valid() {

        if(m_params["bank"] == "aktia") {
            bool is_pki=false;
            if(m_params["command"] == "get_certificate"  ||
                m_params["command"] == "get_bank_certificate"  ||
                m_params["command"] == "create_certificate"  ||
                m_params["command"] == "renew_certificate") {
                    is_pki=true;
            }
            if(is_pki) {
                m_error_message = this->el("cer:ResponseText")->GetText();
                m_error_code =  this->el("cer:ResponseCode")->GetText(); 
            }
            else {
                m_error_message = this->el("mod:ResponseText")->GetText();
                m_error_code =  this->el("mod:ResponseCode")->GetText(); 
            }
        }
        else if(m_params["bank"] == "danske") {
            bool is_pki=false;
            if(m_params["command"] == "get_certificate"  ||
                m_params["command"] == "get_bank_certificate"  ||
                m_params["command"] == "create_certificate"  ||
                m_params["command"] == "renew_certificate") {
                    is_pki=true;
            }
            if(is_pki) {
                if(this->el("pkif:PKIFactoryServiceFault")) {
                    m_error_message = this->el("pkif:ReturnText")->GetText();
                    m_error_code =  this->el("pkif:ReturnCode")->GetText(); 

                }
                else if(m_params["command"] == "get_bank_certificate"){ 
                    m_error_message = this->el("pkie:ReturnText")->GetText();
                    m_error_code =  this->el("pkie:ReturnCode")->GetText(); 
                }
                else if(m_params["command"] == "create_certificate" || m_params["command"] == "renew_certificate"){
                    m_error_message = this->el("tns:ReturnText")->GetText();
                    m_error_code =  this->el("tns:ReturnCode")->GetText(); 
                }
            }
            else {
                m_error_message = this->el("ResponseText")->GetText();
                m_error_code =  this->el("ResponseCode")->GetText(); 
            }
        }
        else if(m_params["bank"] == "op") {
            bool is_pki=false;
            if(m_params["command"] == "get_certificate"  ||
                m_params["command"] == "get_bank_certificate"  ||
                m_params["command"] == "create_certificate" ) {
                    is_pki=true;
            }
            if(is_pki) {
                m_error_message = this->el("cer:ReturnText")->GetText();
                m_error_code =  this->el("cer:ReturnCode")->GetText(); 
            }
            else {
                m_error_message = this->el("mod:ReturnText")->GetText();
                m_error_code =  this->el("mod:ReturnCode")->GetText(); 
            }
        }
        else {
            LOG(ERROR) << "Unsupported bank: " << m_params["bank"];
        }
        if(atoi(m_error_code.c_str()) == 0) {
            return true;
        }
        LOG(ERROR) << "Response error code: " << m_error_code << ", reason: " << m_error_message;
        return false;
    }
	std::string to_XML(){

		std::string ret = read_application_response();
        return ret;
	}
    std::string getErrorMessage() { 
        return m_error_message;
    }
    std::string getErrorCode() {
        return m_error_code;
    }
    //https://www.op.fi/documents/10208/1130741/The+Web+Services+Channel+User+Guide/3c74cfaf-3121-41c2-bdb4-d7eac4f44a6c
    std::string getOwnEncryptionCertificate(std::string decoded_xml) {

        LOG(ERROR) << "getOwnEncryptionCertificate XML: " << decoded_xml;
        if(m_params["bank"] != "aktia") {
            LOG(ERROR) << "Unsupported bank: " << m_params["bank"];
            return "";
        }
        /*
        <CertApplicationResponse>
            <Certificates>
                <Certificate>
                    <Name>CN=1000...</Name>
                    <CertificateFormat>X509v3</CertificateFormat>
                    <Certificate>MIID... (base64 encoded)</Certificate>
                </Certificate>
            </Certificates>
        </CertApplicationResponse>
        */
        XmlDoc d(decoded_xml);
        tinyxml2::XMLElement* root = d.el("Certificates");
        if(root) {
            tinyxml2::XMLElement* cert = root->FirstChildElement("Certificate");
            if(cert) {
                tinyxml2::XMLElement* cert2 = cert->FirstChildElement("Certificate");
                if(cert2) {
                    std::string cert_text = cert2->GetText();
                    //encoded in base 64
                    cert_text = Utilities::base64_decode_to_utf8(cert_text);
                    return cert_text;
                }
            }
        }
        return "";
    }
};
