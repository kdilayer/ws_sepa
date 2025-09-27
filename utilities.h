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
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <iostream>
#include <string>
#include <stdlib.h>
#include <sstream>
#include <cassert>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <map>
#include "tinyxml2.h"
#include "constants.h"
#include "logger.h"

#include <ctime>
#include <iostream>

#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <string>
#include <limits>
#include <stdexcept>
#include <cctype>
#include "xmldoc.h"

class Utilities {
    public:
        static std::string base64_decode_to_utf8(const std::string& base64_input) {
            BIO* bio = BIO_new_mem_buf(base64_input.data(), base64_input.size());
            BIO* b64 = BIO_new(BIO_f_base64());
            bio = BIO_push(b64, bio);
            BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

            std::vector<char> buffer(base64_input.size());
            int decoded_length = BIO_read(bio, buffer.data(), buffer.size());
            BIO_free_all(bio);

            if (decoded_length > 0) {
                return std::string(buffer.data(), decoded_length);
            }
            return "";
        }
        static std::string formatCommand(std::string cmd) {
            std::string out;
            bool nextToUpper=true;
            int i=0;
            while( i < cmd.length() ) {
                char c = cmd.at(i);
                switch(c) {
                    case '_':
                        nextToUpper=true;
                    break;
                    default:
                        if(nextToUpper) {
                            out+=toupper(c);
                            nextToUpper=false;
                        }
                        else {
                            out+=c;
                        }
                    break;
                }
                ++i;
            }
            return out;
        }
        static void replaceAll(std::string &s, const std::string &from, const std::string &to){
            size_t pos = 0;
            while ((pos = s.find(from, pos)) != std::string::npos)
            {
                s.replace(pos, from.size(), to);
                pos += to.size();
            }
        }
        static std::string calculate_signature(std::string in, std::string prv_key_pem, bool linebreaks = true, bool debug=false) {
            BIO *bufio = BIO_new_mem_buf((void*)prv_key_pem.c_str(), prv_key_pem.length());
            RSA *rsa = NULL;
            PEM_read_bio_RSAPrivateKey(bufio, &rsa, 0, NULL);

            EVP_MD_CTX* sign_ctx = EVP_MD_CTX_create();
            EVP_PKEY* priKey  = EVP_PKEY_new();
            EVP_PKEY_assign_RSA(priKey, rsa);

            size_t sig_len=0;
            if (EVP_DigestSignInit(sign_ctx, NULL, EVP_sha1(), NULL, priKey)<=0) {
                LOG(DEBUG) << "EVP_DigestSignInit failed";
            }
            if (EVP_DigestSignUpdate(sign_ctx, in.c_str(), in.length()) <= 0) {
                LOG(DEBUG) << "EVP_DigestSignUpdate failed";
            }
            if (EVP_DigestSignFinal(sign_ctx, NULL, &sig_len) <=0) {
                LOG(DEBUG) << "EVP_DigestSignFinal failed";
            }
            unsigned char *sig = (unsigned char*)malloc(sig_len);
            if (EVP_DigestSignFinal(sign_ctx, sig, &sig_len) <= 0) {
                LOG(DEBUG) << "EVP_DigestSignFinal failed";
            }
            EVP_MD_CTX_free(sign_ctx);
            EVP_PKEY_free(priKey);
            
            std::string signature_out = encode(std::string((char*)sig, sig_len));
            if(linebreaks) {
                signature_out = Utilities::insertLineBreaks(signature_out, 60);
            }
            free(sig);

            
            if(bufio) {
                BIO_free (bufio); 
            }
            if(debug) std::cout << "CALC SIGNATURE IN: >>" << in << "<<" << std::endl;
    		if(debug) std::cout << "CALC SIGNATURE OUT: >>" << signature_out << "<<" << std::endl;
            return signature_out;
        }
        
        static std::string calculate_digest(std::string input, bool debug=false) {
            // SHA1 digest
            unsigned char hash_output[SHA_DIGEST_LENGTH];
            SHA_CTX sha_1;
            SHA1_Init(&sha_1);
            SHA1_Update(&sha_1, input.c_str(), input.length());
            SHA1_Final(hash_output, &sha_1);

            std::string out_str = encode(std::string(( char *)hash_output, SHA_DIGEST_LENGTH));
            if(debug == true) {
                std::cout << "CALC DIGEST IN: >>" << input << "<<" << std::endl; 
                std::cout << "CALC DIGEST OUT: >>" << out_str << "<<" << std::endl; 
            }
            return out_str;
        }
        static std::string insertLineBreaks(std::string in, int each){
            std::string out="";
            int i = 0;
            for (i = 0; i < in.length(); i++) {
                if (i > 0 && (i % each == 0)) {
                    out+='\n';
                }

		        out+=in.at(i);
		    }
            if(i % each != 0)
        	    out+='\n';
            return out;            
        }
        #if 0 
        static std::string encode(std::string input) {
            return encode((unsigned char *)input.c_str(), input.length());
        }
        #endif
        #if 0
        static int decode_b(std::string input, unsigned char **output, int *output_len) { //
            *output_len = 3*input.length()/4;
            *output = (unsigned char *)(calloc(*output_len+1, 1));
            EVP_ENCODE_CTX *ctx = EVP_ENCODE_CTX_new();
            EVP_DecodeInit(ctx);
            EVP_DecodeUpdate(ctx, *output, output_len, (unsigned char*)input.c_str(), input.length());
            int left=0;
            int ret = EVP_DecodeFinal(ctx, *output, &left);
            EVP_ENCODE_CTX_free(ctx);
            return ret;
        }
        #endif

        static constexpr char b64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        static constexpr char reverse_table[128] = {
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
            64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
            64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
        };
        static std::string encode(const ::std::string &bindata){
            using ::std::string;
            using ::std::numeric_limits;

            if (bindata.size() > (numeric_limits<string::size_type>::max() / 4u) * 3u) {
                throw ::std::length_error("Converting too large a string to base64.");
            }

            const ::std::size_t binlen = bindata.size();
            // Use = signs so the end is properly padded.
            string retval((((binlen + 2) / 3) * 4), '=');
            ::std::size_t outpos = 0;
            int bits_collected = 0;
            unsigned int accumulator = 0;
            const string::const_iterator binend = bindata.end();

            for (string::const_iterator i = bindata.begin(); i != binend; ++i) {
                accumulator = (accumulator << 8) | (*i & 0xffu);
                bits_collected += 8;
                while (bits_collected >= 6) {
                    bits_collected -= 6;
                    retval[outpos++] = b64_table[(accumulator >> bits_collected) & 0x3fu];
                }
            }
            if (bits_collected > 0) { // Any trailing bits that are missing.
                assert(bits_collected < 6);
                accumulator <<= 6 - bits_collected;
                retval[outpos++] = b64_table[accumulator & 0x3fu];
            }
            assert(outpos >= (retval.size() - 2));
            assert(outpos <= retval.size());
            return retval;
        }

        static std::string decode(const ::std::string &ascdata) {
            using ::std::string;
            string retval;
            const string::const_iterator last = ascdata.end();
            int bits_collected = 0;
            unsigned int accumulator = 0;

            for (string::const_iterator i = ascdata.begin(); i != last; ++i) {
                const int c = *i;
                if (::std::isspace(c) || c == '=') {
                    // Skip whitespace and padding. Be liberal in what you accept.
                    continue;
                }
                if ((c > 127) || (c < 0) || (reverse_table[c] > 63)) {
                    throw ::std::invalid_argument("This contains characters not legal in a base64 encoded string.");
                }
                accumulator = (accumulator << 6) | reverse_table[c];
                bits_collected += 6;
                if (bits_collected >= 8) {
                    bits_collected -= 8;
                    retval += static_cast<char>((accumulator >> bits_collected) & 0xffu);
                }
            }
            return retval;
        }
        #if 0
        static std::string decode(std::string input) { //const char *input, int length
            int output_len = 3*input.length()/4;
            unsigned char *output = (unsigned char *)(calloc(output_len+1, 1));
            EVP_ENCODE_CTX *ctx = EVP_ENCODE_CTX_new();
            EVP_DecodeInit(ctx);
            EVP_DecodeUpdate(ctx, output, &output_len, (unsigned char*)input.c_str(), input.length());
            int left=0;
            int ret = EVP_DecodeFinal(ctx, output, &left);
            EVP_ENCODE_CTX_free(ctx);
            std::string out_str((char*)output, output_len );
            free(output);
            return out_str;
        }
        static std::string encode(unsigned char *text, int len) {
            EVP_ENCODE_CTX *ectx = EVP_ENCODE_CTX_new();
            int size = len*2;
            size = size > 64 ? size : 64;
            unsigned char* out = (unsigned char*)malloc( size );
            int outlen = 0;
            int tlen = 0;

            EVP_EncodeInit(ectx);
            EVP_EncodeUpdate(ectx,
                    out,
                    &outlen,
                    (const unsigned char*)text,
                    len
                    );
            tlen += outlen;
            EVP_EncodeFinal( ectx, out+tlen, &outlen );
            tlen += outlen;

            std::string str((char*)out, tlen );
            free( out );
            return str;
        }
        #endif
        
        static std::string format_cert(std::string cert) {
            std::string start_tag = "-----BEGIN CERTIFICATE-----";
            std::string end_tag = "-----END CERTIFICATE-----";
            int start = cert.find(start_tag)+start_tag.length();
            int end = cert.find(end_tag);
            if(start == -1 || end == -1) {
                return "";
            }
            std::string content = cert.substr(start, cert.length() - start_tag.length() - end_tag.length());
            replaceAll(content, "\r", "");
            replaceAll(content, "\n", "");
            return content;
        }
        static std::string format_cert_request(std::string cert) {
            std::string start_tag = "-----BEGIN CERTIFICATE REQUEST-----";
            std::string end_tag = "-----END CERTIFICATE REQUEST-----";
            int start = cert.find(start_tag)+start_tag.length();
            int end = cert.find(end_tag);
            if(start == -1 || end == -1) {
                return "";
            }
            std::string content = cert.substr(start, cert.length() - start_tag.length() - end_tag.length());
            replaceAll(content, "\r", "");
            replaceAll(content, "\n", "");
            return content;
        }
        
        static std::string string_toupper(std::string value) {
            transform(value.begin(), value.end(), value.begin(), ::toupper);
            return value;
        }
        static std::string randomUUID() { //return a 128-bit value as hex string
            unsigned char bytes[16];
            if (RAND_bytes(bytes, sizeof(bytes)) != 1) {
                throw std::runtime_error("RAND_bytes failed");
            }
            char buf[33];
            for (int i = 0; i < 16; ++i) {
                sprintf(buf + i * 2, "%02x", bytes[i]);
            }
            buf[32] = '\0';
            return std::string(buf);
        }
        static std::string getRandomHexString(int size) {
            std::string out;
            unsigned char *buf = (unsigned char *)malloc(size);
            RAND_bytes((unsigned char*)buf, size);
            for(int i=0; i < size; i++) {
                char tmp[3];
                sprintf(tmp, "%02x", buf[i]);
                out+=tmp;
            }
            free(buf);
            return out;
        }
        static std::string currentIsoDateTime(int min_infuture=0) {
            time_t now;
            time(&now); //number of seconds since 1 Jan 1970 in UTC (not timezone specific)
            if(min_infuture > 0){
                long ONE_MINUTE_IN_SECS=60;//seconds
                now = now + (min_infuture * ONE_MINUTE_IN_SECS);
                
            }
            char buf[sizeof "2011-10-08T07:07:09Z"];
            strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
            // this will work too, if your compiler doesn't support %F or %T:
            //strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
            return std::string(buf);
        }
        static tinyxml2::XMLError load_soap_body_template(std::string command, std::string bank, tinyxml2::XMLDocument &document) {
            if(command == "download_file_list"){
                return document.Parse(Constants_SOAP_DOWNLOAD_FILE_LIST_XML.c_str());
            }
            else if (command == "get_user_info") {
                return document.Parse(Constants_SOAP_GET_USER_INFO_XML.c_str());
            }
            else if (command == "upload_file") {
                return document.Parse(Constants_SOAP_UPLOAD_FILE_XML.c_str());
            }
            else if (command == "download_file") {
                return document.Parse(Constants_SOAP_DOWNLOAD_FILE_XML.c_str());
            }
            else if (command == "get_certificate") {
                if(bank == "aktia")
                    return document.Parse(Constants_AKTIA_SOAP_CREATE_CERTIFICATE_XML.c_str());
                return document.Parse(Constants_SOAP_GET_CERTIFICATE_XML.c_str());
            }
            else if (command == "renew_certificate") {
                if(bank == "aktia")
                    return document.Parse(Constants_AKTIA_SOAP_CREATE_CERTIFICATE_XML.c_str());
                return document.Parse(Constants_SOAP_RENEW_CERTIFICATE_XML.c_str());
            }
            else if (command == "get_bank_certificate") {
                return document.Parse(Constants_SOAP_DANSKE_GET_BANK_CERTIFICATE_XML.c_str());
            }
            else if (command == "create_certificate") {
                if(bank == "aktia")
                    return document.Parse(Constants_AKTIA_SOAP_CREATE_CERTIFICATE_XML.c_str());
                else
                    return document.Parse(Constants_SOAP_CREATE_CERTIFICATE_XML.c_str());
            }
            return tinyxml2::XML_ERROR_EMPTY_DOCUMENT;
    	}
        static tinyxml2::XMLError load_body_template(std::string command, std::string bank, tinyxml2::XMLDocument &document) {
            if(command == "download_file_list") {
                return document.Parse(Constants_APPREQ_DOWNLOAD_FILE_LIST_XML.c_str());
            }
            else if(command == "get_user_info"){
                return document.Parse(Constants_APPREQ_GET_USER_INFO_XML.c_str());
            }
            else if(command == "upload_file"){
                if(bank == "aktia")
                    return document.Parse(Constants_AKTIA_APPREQ_UPLOAD_FILE_XML.c_str()); 
                return document.Parse(Constants_APPREQ_UPLOAD_FILE_XML.c_str());

            }
            else if(command == "download_file"){
                 return document.Parse(Constants_APPREQ_DOWNLOAD_FILE_XML.c_str()); 
            }
            else if(command == "get_certificate"){
                if(bank == "aktia")
                    return document.Parse(Constants_AKTIA_APPREQ_CREATE_CERTIFICATE_XML.c_str());
                 return document.Parse(Constants_APPREQ_GET_CERTIFICATE_XML.c_str()); 
            }
            else if(command == "get_bank_certificate"){
                return document.Parse(Constants_APPREQ_DANSKE_GET_BANK_CERTIFICATE_XML.c_str());
            }
            else if(command == "renew_certificate"){
                if(bank == "aktia")
                        return document.Parse(Constants_AKTIA_APPREQ_CREATE_CERTIFICATE_XML.c_str());
                return document.Parse(Constants_APPREQ_DANSKE_RENEW_CERTIFICATE_XML.c_str());
            }
            else if(command == "create_certificate"){
                if(bank == "aktia")
                    return document.Parse(Constants_AKTIA_APPREQ_CREATE_CERTIFICATE_XML.c_str());
                else
                    return document.Parse(Constants_APPREQ_CREATE_CERTIFICATE_XML.c_str());
            }
           
		    return tinyxml2::XML_ERROR_EMPTY_DOCUMENT;
    	}
        static XmlDoc *encrypt_application_request(XmlDoc &application_request, std::map<std::string, std::string> params) {
            const char *cert_buffer = params["bank_encryption_certificate"].c_str();
            if(params["testing"] == "true") {
                cert_buffer = params["own_signing_certificate"].c_str();
            }

            BIO *cbio = BIO_new_mem_buf((void*)cert_buffer, -1);
            X509 *cert = PEM_read_bio_X509(cbio, NULL, 0, NULL);
            EVP_PKEY * encryption_public_key = X509_get_pubkey (cert);

            std::string signAttrPrefix = "dsig:";
            if(params["command"] == "renew_certificate") {
                signAttrPrefix="";
            }
            std::string digest = application_request.el_tostring(signAttrPrefix+"DigestValue");
            application_request.el(signAttrPrefix+"SignedInfo")->SetText("");
            application_request.el(signAttrPrefix+"SignedInfo")->DeleteAttribute("xmlns");
            application_request.el(signAttrPrefix+"SignedInfo")->DeleteAttribute("xmlns:dsig");
/*
            ApplicationRequest *temp = new ApplicationRequest();
            temp->build_application_request(params, false);
            tinyxml2::XMLElement *clean = temp->xml_doc().el(signAttrPrefix+"SignedInfo");
*/
            //tinyxml2::XMLElement *clean = NULL; //TODO FIXME ABOVE; where can I get a clean copy?
            //application_request.el_delete(signAttrPrefix+"SignedInfo");
            //application_request.el(signAttrPrefix+"SignedInfo")->InsertFirstChild(clean);
            application_request.el(signAttrPrefix+"DigestValue")->SetText(digest.c_str());

            //if(params['debug']) println "TO BE ENCRYPTED: >>" + application_request.toXml(false) + "<<"
            //if(params['debug']) println "TO BE ENCRYPTED: >>" + application_request.canonicalize(false) + "<<"

            std::string key="";
            std::string ret = encrypt_ar(application_request.tostring(), params, key);
            //if(ret.empty() ) {
                //if(params['debug'])  println "ERROR in encrypt_ar" 
            //    return;
            //}

            std::string encrypted_application_request = ret;
            
            bool sig_injectlinebreaks = true;
            if(params["command"] == "renew_certificate") {
                //sig_injectlinebreaks=false
            }
            std::string encrypted_key = encrypt_key(key, encryption_public_key, params, sig_injectlinebreaks);


            //if(params['debug']) println "ENCRYPTED KEY: >>" + encrypted_key + "<<"

            auto *enc_ar = build_encrypted_ar(Utilities::format_cert(params["bank_encryption_certificate"]), encrypted_key, encrypted_application_request);
            BIO_free (cbio); 
            return enc_ar;
            
            #if 0
            X509Certificate encryption_certificate = X509Certificate.getInstance(params['bank_encryption_certificate'].getBytes())
            PublicKey encryption_public_key = (PublicKey)encryption_certificate.getPublicKey()
            
            //XXX HACK: HAVE TO MODIFY SOME ATTS HERE
            String signAttrPrefix = "dsig:"

            if(params['command'] == "renew_certificate") {
                signAttrPrefix=""
            }
            String digest = application_request.application_request.getTagText("${signAttrPrefix}DigestValue")
            application_request.application_request.setTagText("${signAttrPrefix}SignedInfo", "")
            application_request.application_request.removeTagAttribute("${signAttrPrefix}SignedInfo", "xmlns")
            application_request.application_request.removeTagAttribute("${signAttrPrefix}SignedInfo", "xmlns:dsig")
            //println "NOW: >>"+application_request.toXml(false)+"<<"
            def temp = new ApplicationRequest()
            temp.initialize(params, false)

            def clean = temp.application_request.getTagText("${signAttrPrefix}SignedInfo")

            application_request.application_request.setTagText("${signAttrPrefix}SignedInfo", clean)
            application_request.application_request.expandTag("${signAttrPrefix}DigestValue")
            application_request.application_request.setTagText("${signAttrPrefix}DigestValue", digest)

            //if(params['debug']) println "TO BE ENCRYPTED: >>" + application_request.toXml(false) + "<<"
            if(params['debug']) println "TO BE ENCRYPTED: >>" + application_request.canonicalize(false) + "<<"
            def ret = encrypt_ar(application_request, params)
            if(!ret) {
                if(params['debug'])  println "ERROR in encrypt_ar" 
                return
            }
            def encrypted_application_request = ret[0]
            def key = ret[1]
            boolean sig_injectlinebreaks = true
            if(params['command'] == "renew_certificate") {
                //sig_injectlinebreaks=false
            }
            def encrypted_key = encrypt_key(key, encryption_public_key, sig_injectlinebreaks)
            if(params['debug']) println "ENCRYPTED KEY: >>" + encrypted_key + "<<"
            //System.exit(0)
            return build_encrypted_ar(Utilities.format_cert(params['bank_encryption_certificate']), encrypted_key, encrypted_application_request)
            #endif
        }
        static XmlDoc *build_encrypted_ar(std::string cert, std::string encrypted_key, std::string encrypted_data) {
            XmlDoc *ar = new XmlDoc(Constants_APPREQ_ENCRYPTED_REQUEST_XML);

            std::cout << "xenc:CipherValue [Encrypted Key]: " << encrypted_key << std::endl;
            ar->el("dsig:X509Certificate")->SetText(cert.c_str());
            ar->el("xenc:CipherValue")->SetText(encrypted_key.c_str());
            ar->el("xenc:CipherValue", 1)->SetText(encrypted_data.c_str());
            #if 0
            ar.setTagText("dsig:X509Certificate", cert)
            ar.setTagText("xenc:CipherValue", encrypted_key)
            ar.setTagText("xenc:CipherValue", encrypted_data, 1)
            return ar
            #endif
            return ar;
        }
        static std::string decrypt_key(std::string prv_key_pem, unsigned char *in, int in_len) {
            BIO *bufio = BIO_new_mem_buf((void*)prv_key_pem.c_str(), prv_key_pem.length());
            RSA *rsa_key = NULL;
            PEM_read_bio_RSAPrivateKey(bufio, &rsa_key, 0, NULL);
            
            EVP_PKEY *pkey = EVP_PKEY_new(); //PEM_read_bio_PrivateKey(bufio, NULL, NULL, NULL);
            EVP_PKEY_assign_RSA(pkey, rsa_key);
            EVP_PKEY_CTX* dec_ctx = EVP_PKEY_CTX_new(pkey, NULL);

            EVP_PKEY_decrypt_init(dec_ctx);

            if (EVP_PKEY_CTX_set_rsa_padding(dec_ctx, RSA_PKCS1_PADDING) <= 0) {
                return "";
            }
            size_t out_len = 0;
            int ret = EVP_PKEY_decrypt(dec_ctx, NULL, &out_len, in, in_len);
            unsigned char *out=(unsigned char*)malloc(out_len);
            memset(out, 0, out_len);
            
            ret = EVP_PKEY_decrypt(dec_ctx, out, &out_len, in, in_len);
            if(ret <= 0 ) {
                out_len = 0;
            }
            EVP_PKEY_free(pkey);
            EVP_PKEY_CTX_free(dec_ctx);
            BIO_free (bufio); 
            std::string out_str = std::string((char*)out, out_len);
            free(out);
            return out_str;
        }
        
        static std::string encrypt_key(std::string in, EVP_PKEY *public_key, std::map<std::string, std::string> params, bool injectlinebreaks = true) {
            std::string out_str = "";
            EVP_PKEY_CTX* enc_ctx = EVP_PKEY_CTX_new(public_key, NULL);
            if (EVP_PKEY_encrypt_init(enc_ctx) <= 0) {
                return "";
            }
            // Any algorithm specific control operations can be performec now before
            if (EVP_PKEY_CTX_set_rsa_padding(enc_ctx, RSA_PKCS1_PADDING) <= 0) {
                return "";
            }
            size_t outlen;
            unsigned char* out;
            if (EVP_PKEY_encrypt(enc_ctx, NULL, &outlen, (unsigned char*)in.c_str(), in.length() ) <= 0) {
                return "";
            }
            out = new unsigned char[outlen];
            memset(out, 0, outlen);
            if (EVP_PKEY_encrypt(enc_ctx, out, &outlen, (unsigned char*)in.c_str(), in.length()) <= 0) {
                return "";
            }
            //std::cout << "encrypted key: " << uchar_tohex(out, outlen) << std::endl;
            std::string encoded_str = encode(std::string((char*)out, outlen));


            std::cout << "from_encoding ["<<std::to_string(encoded_str.length()) <<"]: >>" << encoded_str << "<<" << std::endl;
            if(injectlinebreaks) {
                out_str = Utilities::insertLineBreaks(encoded_str, 60);
            }
            else {
                out_str = encoded_str;
            }
            EVP_PKEY_CTX_free(enc_ctx);
            delete out;
            #if 0
            Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
            cipher.init(Cipher.ENCRYPT_MODE, public_key);
            byte[] encrypted_key = cipher.doFinal(key.getBytes("UTF-8"));
            
            if(injectlinebreaks) {
                out = Utilities.insertLineBreaks(Utilities.encode(encrypted_key), 60)
            }
            else {
                out = Utilities.encode(encrypted_key)
            }
            #endif

            return out_str;
        }
        static int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext) {
            EVP_CIPHER_CTX *ctx;

            int len;

            int plaintext_len;

            /* Create and initialise the context */
            if(!(ctx = EVP_CIPHER_CTX_new())) {
                return -1;
            }

            /*
            * Initialise the decryption operation. IMPORTANT - ensure you use a key
            * and IV size appropriate for your cipher
            
            */
            if(1 != EVP_DecryptInit_ex(ctx, EVP_des_ede_cbc(), NULL, key, iv)){
                return -1;
            }

            /*
            * Provide the message to be decrypted, and obtain the plaintext output.
            * EVP_DecryptUpdate can be called multiple times if necessary.
            */
            if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)){
                return -1;
            }
            plaintext_len = len;

            /*
            * Finalise the decryption. Further plaintext bytes may be written at
            * this stage.
            */
            if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)){
                return -1;
            }
            plaintext_len += len;

            /* Clean up */
            EVP_CIPHER_CTX_free(ctx);

            return plaintext_len;
        }
        static int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext){
            EVP_CIPHER_CTX *ctx = NULL;
            int len = 0;
            int ciphertext_len = 0;

            /* Create and initialise the context */
            if(!(ctx = EVP_CIPHER_CTX_new())) {
                return -1;
            }

            /*
            * Initialise the encryption operation. IMPORTANT - ensure you use a key
            * and IV size appropriate for your cipher
            */
            if(1 != EVP_EncryptInit_ex(ctx, EVP_des_ede_cbc(), NULL, key, iv)) {
                return -1;
            }

            /*
            * Provide the message to be encrypted, and obtain the encrypted output.
            * EVP_EncryptUpdate can be called multiple times if necessary
            */
            if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
                return -1;
            }
            ciphertext_len = len;

            /*
            * Finalise the encryption. Further ciphertext bytes may be written at
            * this stage.
            */
            if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
                return -1;
            }
            ciphertext_len += len;

            /* Clean up */
            EVP_CIPHER_CTX_free(ctx);

            return ciphertext_len;
        }
        static std::string encrypt_ar(std::string plaintext, std::map<std::string, std::string> params, std::string &key_out) {
            std::string key = Utilities::getRandomHexString(24);
            std::string iv = Utilities::getRandomHexString(8);

            if(params["xxx_encrypt_key"] != ""){
                key = params["xxx_encrypt_key"];
                iv = params["xxx_encrypt_iv"];
            }
            key_out=key;
            unsigned char ciphertext[((plaintext.length() + 16) / 16) * 16 * 2];
            int ciphertext_len = sizeof(ciphertext);
            memset(ciphertext, 0 ,sizeof(ciphertext));
           
            ciphertext_len = encrypt((unsigned char*)plaintext.c_str(), plaintext.length(), (unsigned char*)key.c_str(), (unsigned char*)iv.c_str(), ciphertext);

            unsigned char encout[((plaintext.length() + 16) / 16) * 16 * 2];
            memset(encout, 0 ,sizeof(encout));
            memcpy(encout, iv.c_str() ,iv.length());
            memcpy(encout+iv.length(), ciphertext ,ciphertext_len);

            bool injectlinebreaks = false;
            if(params["command"] == "renew_certificate") {
                injectlinebreaks = true;
            }

            std::string out = "";
            if(injectlinebreaks) {
                out = Utilities::insertLineBreaks(encode(std::string((char *)encout, iv.length() + ciphertext_len)), 60);
            }
            else {
                out = Utilities::encode(std::string((char *)encout, iv.length() + ciphertext_len));
            }
            #if 0
            String rpwd = Utilities.getRandomHexString(24)
            String ivr = Utilities.getRandomHexString(8)
            if(params['xxx_encrypt_key'] != null){
                if(params['debug']) println " ==> XXX SET ENC IV: " + params['xxx_encrypt_key']
                rpwd = params['xxx_encrypt_key']
                ivr = params['xxx_encrypt_iv']
            }
            byte[] key = rpwd.getBytes("UTF-8")
            byte[] iv = ivr.getBytes("UTF-8")
            
            SecretKey secretKey = new SecretKeySpec(key, "DESede"); //"**DESede/PKCS#5**");
            IvParameterSpec ivSpec = new IvParameterSpec(iv);
            Cipher cipher = Cipher.getInstance("DESede/CBC/PKCS5Padding");
            cipher.init(Cipher.ENCRYPT_MODE, secretKey, ivSpec);
            String tbe
            tbe = application_request.canonicalize(false)
            byte[] encar = cipher.doFinal(tbe.getBytes("UTF-8") );
            byte[] encout = new byte[iv.length + encar.length];
            System.arraycopy(iv, 0, encout, 0, iv.length);
            System.arraycopy(encar, 0, encout, iv.length, encar.length);

            boolean injectlinebreaks = false
            if(params['command'] == "renew_certificate") {
                injectlinebreaks = true
            }

            String out = ""
            if(injectlinebreaks) {
                out = Utilities.insertLineBreaks(Utilities.encode(encout), 60)
            }
            else {
                out = Utilities.encode(encout)
            }
            return [out, rpwd]
            #endif

            return out;
        }   	    
};