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
#include "sepaxml.h"
#include "soapclient.h"

std::string SoapBuilder::to_XML() {
    std::string out = R"(<?xml version="1.0" encoding="UTF-8"?>)";
    out+='\n';
    out +=m_header_template.el_tostring("env:Envelope");
    return out;
    //return m_body_template.tostring();//.canonicalize() //toString()
}
void SoapBuilder::set_create_cert_contents() {
    //m_application_request.el("FileReference")->SetText(m_params["file_reference"].c_str());
    m_body_template.el("pkif:SenderId")->SetText(m_params["customer_id"].c_str());
    m_body_template.el("pkif:CustomerId")->SetText(m_params["customer_id"].c_str());
    m_body_template.el("pkif:RequestId")->SetText(m_params["request_id"].c_str());
    m_body_template.el("pkif:Timestamp")->SetText(m_params["iso_time"].c_str());
    m_body_template.el("pkif:InterfaceVersion")->SetText("1");
    m_body_template.el("pkif:Environment")->SetText(m_params["environment"].c_str());

    if(m_params["environment"]=="test" && m_params["bank"] == "danske") {
        m_body_template.el("pkif:Environment")->SetText("customertest");
    }
    return;
}
void SoapBuilder::add_encrypted_request_to_soap(XmlDoc *encreq, std::string path = "") {
    
    if(path == "") {
        m_body_template.el("pkif:CreateCertificateIn")->SetText(encreq->tostring().c_str());
        //body_template.appendToTag("pkif:CreateCertificateIn", encreq.toXml(true))
    }
    else {
        m_body_template.el(path)->SetText(encreq->tostring().c_str());
        //body_template.appendToTag(path, encreq.toXml(true))
    }
    return;
}
void SoapBuilder::add_body_to_header(){
    m_body_template.el("env:Body")->DeleteAttribute("xmlns:wsu");
    //XmlDoc *d = new XmlDoc(m_body_template.el_tostring("env:Body"));
    m_header_template.el_insert_child("env:Envelope", m_body_template.el("env:Body"));
    //std::cout << "header template xml: " << m_header_template.tostring() << std::endl;
} 
tinyxml2::XMLError SoapBuilder::build_get_bank_certificate_request() {
    set_bank_certificate_contents();
    add_bank_certificate_body_to_soap();
    return tinyxml2::XML_SUCCESS;
}
void SoapBuilder::add_bank_certificate_body_to_soap() {
    tinyxml2::XMLElement * el = m_application_request.xml_doc().el("elem:GetBankCertificateRequest");
    m_body_template.el("pkif:GetBankCertificateIn")->InsertEndChild(el);
    return;
}
void SoapBuilder::set_bank_certificate_contents() {
    m_body_template.el("pkif:SenderId")->SetText(m_params["customer_id"].c_str());
    m_body_template.el("pkif:CustomerId")->SetText(m_params["customer_id"].c_str());
    m_body_template.el("pkif:RequestId")->SetText(m_params["request_id"].c_str());
    m_body_template.el("pkif:Timestamp")->SetText(m_params["iso_time"].c_str());
    m_body_template.el("pkif:InterfaceVersion")->SetText("1");
    return;
}
tinyxml2::XMLError SoapBuilder::build_renew_certificate_request() {
    set_create_cert_contents();

    XmlDoc *encrypted_request = Utilities::encrypt_application_request(m_application_request.xml_doc(), m_params);
    add_encrypted_request_to_soap(encrypted_request, "pkif:RenewCertificateIn");

    process_header();
    add_body_to_header();

    return tinyxml2::XML_SUCCESS;
}
void SoapBuilder::process_header() {

    m_header_template.el("wsu:Created")->SetText(m_params["iso_time"].c_str()); 
    m_header_template.el("wsu:Expires")->SetText(m_params["iso_expires"].c_str());

    //std::cout << "header template xml: " << m_header_template.tostring() << std::endl;
    m_header_template.el("wsse:BinarySecurityToken")->SetAttribute("wsu:Id", m_params["token_id"].c_str());
    m_header_template.el("wsse:Reference")->SetAttribute("URI", std::string("#"+m_params["token_id"]).c_str());

    m_header_template.el("wsu:Timestamp")->SetAttribute("wsu:Id", m_params["timestamp_id"].c_str()); 
   
    std::string ts_digest_in = m_header_template.el_tostring("wsu:Timestamp");
    std::string ts_digest_out = Utilities::calculate_digest(ts_digest_in, m_params["debug"] == "true");
    if(m_params["test_with_constant_values"]=="true"){
        if(m_params["bank"]=="aktia" && m_params["command"] == "download_file_list") {
            std::string d_in = R"(<wsu:Timestamp xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd" wsu:Id="timestamp-58b5fc852e42c6a8d">
    <wsu:Created>2023-04-13T05:38:05Z</wsu:Created>
    <wsu:Expires>2023-04-13T05:43:05Z</wsu:Expires>
    </wsu:Timestamp>)";
            if(d_in != ts_digest_in) {
                LOG(DEBUG) << "wsu:Timestamp ts_digest_in in check failed!!!"; 
            }
            if(ts_digest_out != "/RXo5RkrWlpoafjRfLMxvtHvrR4=") {
                LOG(DEBUG) << "wsu:Timestamp ts_digest_out check failed!!!"; 
            }
        }
    }
    
    m_header_template.el("dsig:Reference")->SetAttribute("URI", std::string("#"+m_params["timestamp_id"]).c_str());
    m_header_template.el("dsig:DigestValue")->SetText(ts_digest_out.c_str());
  
    std::string attr = "xmlns:env";
    std::string attr_v = "http://schemas.xmlsoap.org/soap/envelope/";
    if(m_params["command"] == "renew_certificate" && m_params["bank"] == "danske") {
        attr = "xmlns:pkif";
        attr_v = "http://danskebank.dk/PKI/PKIFactoryService";
    }
    std::string wsu_attr =  m_body_template.el("env:Body")->Attribute("xmlns:wsu");
    m_body_template.el("env:Body")->DeleteAttribute(attr.c_str());
    m_body_template.el("env:Body")->DeleteAttribute("wsu:Id");
    m_body_template.el("env:Body")->DeleteAttribute("xmlns:wsu");

    m_body_template.el("env:Body")->SetAttribute(attr.c_str(), attr_v.c_str());
    m_body_template.el("env:Body")->SetAttribute("xmlns:wsu", wsu_attr.c_str());
    m_body_template.el("env:Body")->SetAttribute("wsu:Id", m_params["body_id"].c_str());
    std::string body_digest_in = m_body_template.el_tostring("env:Body");
    std::string body_digest_out = Utilities::calculate_digest(body_digest_in, m_params["debug"] == "true");
    if(m_params["test_with_constant_values"]=="true"){
        if(m_params["bank"]=="aktia" && m_params["command"] == "download_file_list") {
            std::string d_in = R"(<env:Body xmlns:env="http://schemas.xmlsoap.org/soap/envelope/" xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd" wsu:Id="body-28e5958ab367a56f0">
    <cor:downloadFileListin xmlns:cor="http://bxd.fi/CorporateFileService">
      <bxd:RequestHeader xmlns:bxd="http://model.bxd.fi">
        <bxd:SenderId>21648386</bxd:SenderId>
        <bxd:RequestId>7bc83a4ce48</bxd:RequestId>
        <bxd:Timestamp>2023-04-13T05:38:05Z</bxd:Timestamp>
        <bxd:Language>EN</bxd:Language>
        <bxd:UserAgent>libgsepa-1.0</bxd:UserAgent>
        <bxd:ReceiverId>ITELFIHH</bxd:ReceiverId>
      </bxd:RequestHeader>
      <bxd:ApplicationRequest xmlns:bxd="http://model.bxd.fi">PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iVVRGLTgiPz4KPEFwcGxpY2F0aW9uUmVxdWVzdCB4bWxucz0iaHR0cDovL2J4ZC5maS94bWxkYXRhLyI+CiAgPEN1c3RvbWVySWQ+MjE2NDgzODY8L0N1c3RvbWVySWQ+CiAgPENvbW1hbmQ+RG93bmxvYWRGaWxlTGlzdDwvQ29tbWFuZD4KICA8VGltZXN0YW1wPjIwMjMtMDQtMTNUMDU6Mzg6MDVaPC9UaW1lc3RhbXA+CiAgPFN0YXR1cz5ORVc8L1N0YXR1cz4KICA8RW52aXJvbm1lbnQ+UFJPRFVDVElPTjwvRW52aXJvbm1lbnQ+CiAgPFNvZnR3YXJlSWQ+bGliZ3NlcGEtMS4wPC9Tb2Z0d2FyZUlkPgogIDxGaWxlVHlwZT5SQTwvRmlsZVR5cGU+CiAgCjxkc2lnOlNpZ25hdHVyZSB4bWxuczpkc2lnPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwLzA5L3htbGRzaWcjIj4KICAgIDxkc2lnOlNpZ25lZEluZm8geG1sbnM9Imh0dHA6Ly9ieGQuZmkveG1sZGF0YS8iIHhtbG5zOmRzaWc9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvMDkveG1sZHNpZyMiPgogICAgICA8ZHNpZzpDYW5vbmljYWxpemF0aW9uTWV0aG9kIEFsZ29yaXRobT0iaHR0cDovL3d3dy53My5vcmcvVFIvMjAwMS9SRUMteG1sLWMxNG4tMjAwMTAzMTUjV2l0aENvbW1lbnRzIj48L2RzaWc6Q2Fub25pY2FsaXphdGlvbk1ldGhvZD4KICAgICAgPGRzaWc6U2lnbmF0dXJlTWV0aG9kIEFsZ29yaXRobT0iaHR0cDovL3d3dy53My5vcmcvMjAwMC8wOS94bWxkc2lnI3JzYS1zaGExIj48L2RzaWc6U2lnbmF0dXJlTWV0aG9kPgogICAgICA8ZHNpZzpSZWZlcmVuY2UgVVJJPSIiPgogICAgICAgIDxkc2lnOlRyYW5zZm9ybXM+CiAgICAgICAgICA8ZHNpZzpUcmFuc2Zvcm0gQWxnb3JpdGhtPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwLzA5L3htbGRzaWcjZW52ZWxvcGVkLXNpZ25hdHVyZSI+PC9kc2lnOlRyYW5zZm9ybT4KICAgICAgICA8L2RzaWc6VHJhbnNmb3Jtcz4KICAgICAgICA8ZHNpZzpEaWdlc3RNZXRob2QgQWxnb3JpdGhtPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwLzA5L3htbGRzaWcjc2hhMSI+PC9kc2lnOkRpZ2VzdE1ldGhvZD4KICAgICAgICA8ZHNpZzpEaWdlc3RWYWx1ZT5iS1lTQ3BPckpBNmdCdkJvOCtiZGVIUzhDM0k9CjwvZHNpZzpEaWdlc3RWYWx1ZT4KICAgICAgPC9kc2lnOlJlZmVyZW5jZT4KICAgIDwvZHNpZzpTaWduZWRJbmZvPgogICAgPGRzaWc6U2lnbmF0dXJlVmFsdWU+VHQ5NXgzaXBhRTRMRmZtdVhzenFlMEdTRG1TM2FBaFBuaXlVZFUxY2poME1ZbStXYWpqelo2SGREVEUwCm5TNXpvLzQ1TlE3YWZpUzIzcHpQUVN6SVVUNGh3THE4ZmxINHJoazZHUXkwd29PNXpUV20xVEE5eXBmYQpNWnJSS09aT1hsVDN5MzkvNGd1WUQrbTBRRkFwN3NNMmRqbm5aUU1EOWZET0h3cVJGV1k4Y0pyVXRFL2wKU1Rrd1F4S1hEUVhOTWprRXRuR3duZ0JqczRLSE8zODBIVmNhUlVIdVVZWnB5NWgxMUtuWFBaNHlTcnNhCm1SUXZaNmdRVnBFRWM0cjU5QVZYWWJOM3dJWWVZOW9WSzZxWHFLUEZQOGE1bzI0bDdkdXQyZXdXdWtFQQpHQUdvZzZjV0ZNc0FOaTd3NWgrMGhZTzNqanByZ1FCbm4rOFZ0cHF3QVE9PQo8L2RzaWc6U2lnbmF0dXJlVmFsdWU+CiAgICA8ZHNpZzpLZXlJbmZvPgogICAgICA8ZHNpZzpYNTA5RGF0YT4KICAgICAgICA8ZHNpZzpYNTA5Q2VydGlmaWNhdGU+TUlJRXd6Q0NBcXVnQXdJQkFnSVBBWVc1UWZyK3l6bHVlazhhN1NtUk1BMEdDU3FHU0liM0RRRUJDd1VBTUQweEN6QUpCZ05WQkFZVEFrWkpNUkF3RGdZRFZRUUtEQWRUWVcxc2FXNXJNUnd3R2dZRFZRUUREQk5UWVcxc2FXNXJJRU4xYzNSdmJXVnlJRU5CTUI0WERUSXpNREV4TmpBMk1qWXlOMW9YRFRJMU1ERXhOVEEyTWpZeU4xb3dZakVMTUFrR0ExVUVCaE1DUmtreEpqQWtCZ05WQkFvTUhVRnBibVZwYzNSdmNHRnNkbVZzZFhRdFUyRmhjM1J2Y0dGdWEydHBNUmd3RmdZRFZRUUREQTlMUVZNdFVrRkxSVTVPVlZNZ1Qxa3hFVEFQQmdOVkJBUU1DREl4TmpRNE16ZzJNSUlCSWpBTkJna3Foa2lHOXcwQkFRRUZBQU9DQVE4QU1JSUJDZ0tDQVFFQXBzTEdQVjdDVjJReG5jM0diWG9URHQxWGlQdXZ2Q0ZhSktsT1hudElPN2N4alN0akpyeEZ2VHRrYkR1RmdnTVFQbDl1MmZ6ZmVUc056clRMMm9ocmNpYUZEdjNjV3Jld1ZNbDlNSWxWcDVEYnRqNGRnODNtMGE5SjJTdE05OSt2NGRoc3RJbHhxK0E2Uzk3SmwxeEZjdThCTnYvV09XTDkvNFkxQWpsT1JscW0zM282Q2dEcDVENS8yLzQ4MXNXNnA5cXkvVnQ5V2w2bTV5UWVRZ0pnTXF5VkFJSFg2Yzk1UnhIRlQ4K0JMcklzV3pHdzQ0dlVmakVGZ25PNWpFbEtRaHZwTENWVnA3TnkwenVGUmxQdHhyMWJMR3VNeElHSzhOeG0rNkNMMmNWc1NOT29VM2I5R1l3V0lQR011aDluQTYwckhLU2xVWHkwTFlmSnVMd0RSd0lEQVFBQm80R2FNSUdYTUI4R0ExVWRJd1FZTUJhQUZNcUFPRE9UaW1NRWtZMEZhVlpvUWpYbHgvKzhNQjBHQTFVZERnUVdCQlE0MmtYT1lGNmp2VTk0dTJXU1J3azhiblh5aFRBT0JnTlZIUThCQWY4RUJBTUNCUEF3UlFZRFZSMGZCRDR3UERBNm9EaWdOb1kwYUhSMGNEb3ZMMmgwZEhCamNtd3VkSEoxYzNRdWRHVnNhV0V1WTI5dEwzTmhiV3hwYm10amRYTjBiMjFsY21OaExtTnliREFOQmdrcWhraUc5dzBCQVFzRkFBT0NBZ0VBS0RwckV5dWhUc1JuYTZkc28wYXBOSXpGR3JrZFl4SGNxdWFLdXA4T2NpaHpxR2VTT28xV0JEbVhYNy9BNmNhSmlSMytvSUZnRzVHSjAxMmpEY0JFRTdUSGNicmJWTjRDYng4bElvUkVxWStwalFtSm9GeUYzWW9IYW5JUXNadnFvNG5mRTEyMHRNc1pJL2JYbUxTN3E4cTBrQVVsSm84RWFETU1Lem5LcW1IS3pLVnAzNER4WE5KRXJZcFpGd2p1VG91Y3VCTmt6bkM0NnBLQjFpTkNyZUEvQU82VWkyZUdWNEkzRWFqWjJZTVRON3lUcjNnN0N4anVrS1lqeXRkNmlrRS81Wm5ld2k0TEFkYUpRd2lRK3FKczdUR0dmSjRJam4wRk8wWjZDZDMwckk5a3EzcVVsd1lxSEF6MG1nVysvUU9DdVFlL0VNVER0VHBBL25sYjNpdHg0ZHBIYlEwMGlYQndDeVE2cEVCMmFqV3RrSGkvT3dQYVBFakE5SCtONkFTY0djOTJlNll1ZCtVTTNXZnpoRXduUXo1RGgyUkZRaDJsSnhxZnJPN3ZUMlRrZTFQMktmaWo2Ym4zOE55NGtESUVKb3ZvSnJsVEY0bUR3djZ6cWJOZ0J0eUZCcG1zck5Xem5xZnVTeUNzQU45L3l0VGxFYUgxVUswQkVRRUdnV0NMeXBSSnNwbUJXdit1Qld1dmtlVytGdEhBK3RVdjFwTDhRMzV2VFRvVTh0L0pHZ3RieFEwblhiS29sdGY0Z2ordm84ZHhxK3JpbVhKTUJEMkxkZnRQaFJvRFZIbkdFWi9nRm4wcnlITzRrY2UxNW5rN1I3ZG9OeVhpdENJQ2ZBKzZlTE5VTHBmRXlBQUc0Q2o2bjVFU2Z3QkpYYnE2MGFmUUJKblRGbHM9PC9kc2lnOlg1MDlDZXJ0aWZpY2F0ZT4KICAgICAgPC9kc2lnOlg1MDlEYXRhPgogICAgPC9kc2lnOktleUluZm8+CiAgPC9kc2lnOlNpZ25hdHVyZT48L0FwcGxpY2F0aW9uUmVxdWVzdD4K</bxd:ApplicationRequest>
    </cor:downloadFileListin>
  </env:Body>)";
            if(d_in != body_digest_in) {
                LOG(DEBUG) << "env:Body sig_in in check failed!!!"; 
            }
            if(body_digest_out != "Pr5S7dfokizgEhzZWYRwEIQ6ZEo=") {
                LOG(DEBUG) << "env:Body head_sig_out check failed!!!"; 
            }
        }
    }
    m_header_template.el("dsig:Reference", 1)->SetAttribute("URI", std::string("#"+m_params["body_id"]).c_str());
    m_header_template.el("dsig:DigestValue", 1)->SetText(body_digest_out.c_str());
    
    m_header_template.el("dsig:SignedInfo")->SetAttribute("xmlns:dsig", "http://www.w3.org/2000/09/xmldsig#");
    m_header_template.el("dsig:CanonicalizationMethod")->SetText("");
    m_header_template.el("dsig:SignatureMethod")->SetText("");
    m_header_template.el("dsig:Transform")->SetText("");
    m_header_template.el("dsig:Transform", 1)->SetText("");
    m_header_template.el("dsig:DigestMethod")->SetText("");
    m_header_template.el("dsig:DigestMethod", 1)->SetText("");

    std::string sig_in = m_header_template.el_tostring("dsig:SignedInfo"); 
    std::string head_sig_out = Utilities::calculate_signature(sig_in, m_params["signing_private_key"], false, m_params["debug"] == "true");

    if(m_params["test_with_constant_values"]=="true"){
        if(m_params["bank"]=="aktia" && m_params["command"] == "download_file_list") {
            std::string s_in = R"(<dsig:SignedInfo xmlns:dsig="http://www.w3.org/2000/09/xmldsig#">
          <dsig:CanonicalizationMethod Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#"></dsig:CanonicalizationMethod>
          <dsig:SignatureMethod Algorithm="http://www.w3.org/2000/09/xmldsig#rsa-sha1"></dsig:SignatureMethod>
          <dsig:Reference URI="#timestamp-58b5fc852e42c6a8d">
            <dsig:Transforms>
              <dsig:Transform Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#"></dsig:Transform>
            </dsig:Transforms>
            <dsig:DigestMethod Algorithm="http://www.w3.org/2000/09/xmldsig#sha1"></dsig:DigestMethod>
            <dsig:DigestValue>/RXo5RkrWlpoafjRfLMxvtHvrR4=</dsig:DigestValue>
          </dsig:Reference>
          <dsig:Reference URI="#body-28e5958ab367a56f0">
            <dsig:Transforms>
              <dsig:Transform Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#"></dsig:Transform>
            </dsig:Transforms>
            <dsig:DigestMethod Algorithm="http://www.w3.org/2000/09/xmldsig#sha1"></dsig:DigestMethod>
            <dsig:DigestValue>Pr5S7dfokizgEhzZWYRwEIQ6ZEo=</dsig:DigestValue>
          </dsig:Reference>
        </dsig:SignedInfo>)";
            if(s_in != sig_in) {
                LOG(DEBUG) << "dsig:SignedInfo sig_in in check failed!!!"; 
            }
            if(head_sig_out != "OW8DeN1zfFGqM32TeVRaibwAUFOXYt1fTa9rl/PpJhsTmL8NHcVcLeFFn+IoPZly8HoN69dnxdPxU42VSMcBV2UpwqBT/z4srh/c4i3S09LrnHj7BzcccUEGy9fsSg4rB0MzZRo7hbynucv+vMbmanso3cRWUQAc+jB1x4JswoYlJ5b9AYZ1EY2/08OONN8r2B0MD7kkA5g7+FErk1ZjomfExqbzbdYGrnNCBjtOw5GVq+fD2f50oIyEry+4ncO0fezlLEujS+upFBiHM/jE6D/riIIf/ACbrfhBZ1vkhvS3gemZbKyndwklYyBzxgT80bEaUfM3iuu+ps3CYirIgQ==") {
                LOG(DEBUG) << "dsig:SignedInfo head_sig_out check failed!!!"; 
            }
        }
    }
    m_header_template.el("dsig:SignatureValue")->SetText(head_sig_out.c_str());
    
    m_header_template.el("wsse:BinarySecurityToken")->SetText( Utilities::format_cert(m_params["own_signing_certificate"]).c_str());

    //println "process_header header: >>"+header_template.toXml(false)+"<<"
}
tinyxml2::XMLError SoapBuilder::init_build_soap() {
    tinyxml2::XMLError  err = m_application_request.build_application_request(m_params);
    if(err != tinyxml2::XML_SUCCESS) {
        return err;
    }


    m_header_template.Parse(Constants_SOAP_HEADER_XML.c_str());
    Utilities::load_soap_body_template(m_params["command"], m_params["bank"], m_body_template);

    return tinyxml2::XML_SUCCESS;
}

void SoapBuilder::set_generic_request_contents() {
    m_body_template.el("bxd:SenderId")->SetText(m_params["customer_id"].c_str());
    m_body_template.el("bxd:RequestId")->SetText(m_params["request_id"].c_str());
    m_body_template.el("bxd:Timestamp")->SetText(m_params["iso_time"].c_str());
    m_body_template.el("bxd:Language")->SetText(m_params["language"].c_str());
    m_body_template.el("bxd:UserAgent")->SetText(m_params["software_id"].c_str());
    m_body_template.el("bxd:ReceiverId")->SetText("DABAFIHH");
}
void SoapBuilder::add_encrypted_generic_request_to_soap(XmlDoc *encrypted_request) {
    std::string appreq = Utilities::insertLineBreaks(Utilities::encode(encrypted_request->tostring()), 60);
    m_body_template.el("bxd:ApplicationRequest")->SetText(appreq.c_str());
}
tinyxml2::XMLError DanskeSoapBuilder::build_soap(){
    this->init_build_soap();
    std::string command = m_params["command"];

    if(command =="create_certificate"){
        return build_certificate_request();
    }
    else if(command =="renew_certificate"){
        return build_renew_certificate_request();
    }
    else if(command =="get_bank_certificate"){
        return build_get_bank_certificate_request();
    }
    else if(command =="upload_file" || 
            command == "download_file" || 
            command == "get_user_info" || 
            command == "download_file_list"){
        return build_generic_request();
    }
    return tinyxml2::XML_ERROR_EMPTY_DOCUMENT;
}
tinyxml2::XMLError DanskeSoapBuilder::build_certificate_request() {
    set_create_cert_contents();

    XmlDoc *encrypted_request = Utilities::encrypt_application_request(m_application_request.xml_doc(), m_params);
    add_encrypted_request_to_soap(encrypted_request);
    return tinyxml2::XML_SUCCESS;
}
tinyxml2::XMLError DanskeSoapBuilder::build_generic_request() {
    set_generic_request_contents();

    XmlDoc *encrypted_request = Utilities::encrypt_application_request(m_application_request.xml_doc(), m_params);
    this->add_encrypted_generic_request_to_soap(encrypted_request);

    process_header();
    add_body_to_header();
    return tinyxml2::XML_SUCCESS;
}
tinyxml2::XMLError AktiaSoapBuilder::build_soap(){
    this->init_build_soap();
    std::string command = m_params["command"];

    if(command =="get_certificate" || 
        command =="renew_certificate"){
        return build_certificate_request();
    }
    else if(command =="get_bank_certificate"){
        return tinyxml2::XML_ERROR_EMPTY_DOCUMENT;//return DanskeSoapBuilder.build_get_bank_certificate_request(application_request, template, params)
    }
    else if(command =="upload_file" || 
            command == "download_file" || 
            command == "get_user_info" || 
            command == "download_file_list"){
        return build_generic_request();
    }
    return tinyxml2::XML_ERROR_EMPTY_DOCUMENT;
}
tinyxml2::XMLError AktiaSoapBuilder::build_certificate_request() {
    set_create_cert_contents();

    //remove signature
    if(m_params["command"] == "get_certificate")  {
        m_application_request.xml_doc().el_delete("dsig:Signature");
    }

    std::string appreq = m_application_request.xml_doc().tostring();
    LOG(DEBUG) << "APPREQ OUT: >>" << appreq << "<<";
    std::string appreq_ecoded = Utilities::encode(appreq);
    
    //m_body_template.el("opc:ApplicationRequest")->SetText(appreq_ecoded.c_str());
    m_body_template.el("opc:ApplicationRequest")->SetText(Utilities::insertLineBreaks(appreq_ecoded,60).c_str());

    //LOG(DEBUG) << m_body_template.tostring();
    m_body_template.el("env:Body")->DeleteAttribute("wsu:Id");
    m_body_template.el("env:Body")->DeleteAttribute("xmlns:wsu");

    //std::string wsu_attr = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd";
    //m_body_template.el("env:Body")->SetAttribute("xmlns:wsu", wsu_attr.c_str());
    //m_body_template.el("env:Body")->SetAttribute("wsu:Id", m_params["body_id"].c_str());

    m_header_template.el("env:Envelope")->DeleteAttribute("xmlns:wsse");
    m_header_template.el("env:Envelope")->DeleteAttribute("xmlns:wsu");
    m_header_template.el("env:Envelope")->DeleteAttribute("xmlns:dsig");
    
    m_header_template.el("env:Envelope")->SetAttribute("xmlns:opc", "http://mlp.op.fi/OPCertificateService");

    m_header_template.el_insert_child("env:Envelope", m_body_template.el("env:Body"));
    m_header_template.el_delete("wsse:Security");
    LOG(DEBUG) << "ENVELOPE OUT: >>" << m_header_template.tostring() << "<<";
    #if 0
        if(m_params["bank"] == "aktia") {
            m_body_template.el("opc:ApplicationRequest")->SetText(Utilities::insertLineBreaks(appreq,60).c_str());
        }
        else 
        m_body_template.el("bxd:ApplicationRequest")->SetText(Utilities::insertLineBreaks(appreq,60).c_str());
    #endif
    return tinyxml2::XML_SUCCESS;
}

tinyxml2::XMLError AktiaSoapBuilder::set_create_cert_contents() {
    m_body_template.el("opc:SenderId")->SetText(m_params["customer_id"].c_str());
    m_body_template.el("opc:RequestId")->SetText(m_params["request_id"].c_str());
    m_body_template.el("opc:Timestamp")->SetText(m_params["iso_time"].c_str());
    return tinyxml2::XML_SUCCESS;
}
tinyxml2::XMLError AktiaSoapBuilder::build_generic_request() {
    set_generic_request_contents();
    m_body_template.el("bxd:ReceiverId")->SetText("ITELFIHH");
    std::string appreq = m_application_request.xml_doc().tostring();
    if(m_params["debug"] == "true") {
        LOG(DEBUG) << "APPREQ OUT: >>" << appreq << "<<";
    }
    std::string appreq_ecoded = Utilities::encode(appreq);
    if(false && m_params["debug"] == "true") {
        LOG(DEBUG) << "APPREQ ENCODED OUT: >>" << appreq_ecoded << "<<";
    }
    std::string areq_in = R"(<?xml version="1.0" encoding="UTF-8"?>
<ApplicationRequest xmlns="http://bxd.fi/xmldata/">
  <CustomerId>21648386</CustomerId>
  <Command>DownloadFileList</Command>
  <Timestamp>2023-04-13T05:38:05Z</Timestamp>
  <Status>NEW</Status>
  <Environment>PRODUCTION</Environment>
  <SoftwareId>libgsepa-1.0</SoftwareId>
  <FileType>RA</FileType>
  <dsig:Signature xmlns:dsig="http://www.w3.org/2000/09/xmldsig#">
    <dsig:SignedInfo xmlns="http://bxd.fi/xmldata/" xmlns:dsig="http://www.w3.org/2000/09/xmldsig#">
      <dsig:CanonicalizationMethod Algorithm="http://www.w3.org/TR/2001/REC-xml-c14n-20010315#WithComments"></dsig:CanonicalizationMethod>
      <dsig:SignatureMethod Algorithm="http://www.w3.org/2000/09/xmldsig#rsa-sha1"></dsig:SignatureMethod>
      <dsig:Reference URI="">
        <dsig:Transforms>
          <dsig:Transform Algorithm="http://www.w3.org/2000/09/xmldsig#enveloped-signature"></dsig:Transform>
        </dsig:Transforms>
        <dsig:DigestMethod Algorithm="http://www.w3.org/2000/09/xmldsig#sha1"></dsig:DigestMethod>
        <dsig:DigestValue>0JFq7r7r5XcwTJn9L2sTyVOAKhE=
</dsig:DigestValue>
      </dsig:Reference>
    </dsig:SignedInfo>
    <dsig:SignatureValue>Qaf1ReICIagsONkp5WGbd8K7zpp+kNJRrG217sm2BqVoknW2DjL+nTGfjOBN
voszKgPeTEEaDO0yQj3HqKzN684QVNJhNPv1sSU6cvP9UeHe/bYaT9GNCvc2
sPtc8Xbzfm11TvpU7lSuNN6l8ivAIW33rFF+jJgbhuccApS8nlne5NcSh4vT
OxPMp/z0H/eYUNLUmn37gAfaffba0B9aKBrWbeylL1MkU6trR7HOU8nao43m
O5oEM2FUOYd8mUwaEed/x3oZajvEBbtIzgmegi+F9XzSSuS9cuxJJDsTNPId
V1gmkWjMnVcq/dYYzvxHp96+00euxnpYakkb/OhzGA==
</dsig:SignatureValue>
    <dsig:KeyInfo>
      <dsig:X509Data>
        <dsig:X509Certificate>MIIEwzCCAqugAwIBAgIPAYW5Qfr+yzluek8a7SmRMA0GCSqGSIb3DQEBCwUAMD0xCzAJBgNVBAYTAkZJMRAwDgYDVQQKDAdTYW1saW5rMRwwGgYDVQQDDBNTYW1saW5rIEN1c3RvbWVyIENBMB4XDTIzMDExNjA2MjYyN1oXDTI1MDExNTA2MjYyN1owYjELMAkGA1UEBhMCRkkxJjAkBgNVBAoMHUFpbmVpc3RvcGFsdmVsdXQtU2Fhc3RvcGFua2tpMRgwFgYDVQQDDA9LQVMtUkFLRU5OVVMgT1kxETAPBgNVBAQMCDIxNjQ4Mzg2MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEApsLGPV7CV2Qxnc3GbXoTDt1XiPuvvCFaJKlOXntIO7cxjStjJrxFvTtkbDuFggMQPl9u2fzfeTsNzrTL2ohrciaFDv3cWrewVMl9MIlVp5Dbtj4dg83m0a9J2StM99+v4dhstIlxq+A6S97Jl1xFcu8BNv/WOWL9/4Y1AjlORlqm33o6CgDp5D5/2/481sW6p9qy/Vt9Wl6m5yQeQgJgMqyVAIHX6c95RxHFT8+BLrIsWzGw44vUfjEFgnO5jElKQhvpLCVVp7Ny0zuFRlPtxr1bLGuMxIGK8Nxm+6CL2cVsSNOoU3b9GYwWIPGMuh9nA60rHKSlUXy0LYfJuLwDRwIDAQABo4GaMIGXMB8GA1UdIwQYMBaAFMqAODOTimMEkY0FaVZoQjXlx/+8MB0GA1UdDgQWBBQ42kXOYF6jvU94u2WSRwk8bnXyhTAOBgNVHQ8BAf8EBAMCBPAwRQYDVR0fBD4wPDA6oDigNoY0aHR0cDovL2h0dHBjcmwudHJ1c3QudGVsaWEuY29tL3NhbWxpbmtjdXN0b21lcmNhLmNybDANBgkqhkiG9w0BAQsFAAOCAgEAKDprEyuhTsRna6dso0apNIzFGrkdYxHcquaKup8OcihzqGeSOo1WBDmXX7/A6caJiR3+oIFgG5GJ012jDcBEE7THcbrbVN4Cbx8lIoREqY+pjQmJoFyF3YoHanIQsZvqo4nfE120tMsZI/bXmLS7q8q0kAUlJo8EaDMMKznKqmHKzKVp34DxXNJErYpZFwjuToucuBNkznC46pKB1iNCreA/AO6Ui2eGV4I3EajZ2YMTN7yTr3g7CxjukKYjytd6ikE/5Znewi4LAdaJQwiQ+qJs7TGGfJ4Ijn0FO0Z6Cd30rI9kq3qUlwYqHAz0mgW+/QOCuQe/EMTDtTpA/nlb3itx4dpHbQ00iXBwCyQ6pEB2ajWtkHi/OwPaPEjA9H+N6AScGc92e6Yud+UM3WfzhEwnQz5Dh2RFQh2lJxqfrO7vT2Tke1P2Kfij6bn38Ny4kDIEJovoJrlTF4mDwv6zqbNgBtyFBpmsrNWznqfuSyCsAN9/ytTlEaH1UK0BEQEGgWCLypRJspmBWv+uBWuvkeW+FtHA+tUv1pL8Q35vTToU8t/JGgtbxQ0nXbKoltf4gj+vo8dxq+rimXJMBD2LdftPhRoDVHnGEZ/gFn0ryHO4kce15nk7R7doNyXitCICfA+6eLNULpfEyAAG4Cj6n5ESfwBJXbq60afQBJnTFls=</dsig:X509Certificate>
      </dsig:X509Data>
    </dsig:KeyInfo>
  </dsig:Signature></ApplicationRequest>
)";
    std::string areq_enc = R"(PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iVVRGLTgiPz4KPEFwcGxpY2F0aW9uUmVxdWVzdCB4bWxucz0iaHR0cDovL2J4ZC5maS94bWxkYXRhLyI+CiAgPEN1c3RvbWVySWQ+MjE2NDgzODY8L0N1c3RvbWVySWQ+CiAgPENvbW1hbmQ+RG93bmxvYWRGaWxlTGlzdDwvQ29tbWFuZD4KICA8VGltZXN0YW1wPjIwMjMtMDQtMTNUMDU6Mzg6MDVaPC9UaW1lc3RhbXA+CiAgPFN0YXR1cz5ORVc8L1N0YXR1cz4KICA8RW52aXJvbm1lbnQ+UFJPRFVDVElPTjwvRW52aXJvbm1lbnQ+CiAgPFNvZnR3YXJlSWQ+bGliZ3NlcGEtMS4wPC9Tb2Z0d2FyZUlkPgogIDxGaWxlVHlwZT5SQTwvRmlsZVR5cGU+CiAgPGRzaWc6U2lnbmF0dXJlIHhtbG5zOmRzaWc9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvMDkveG1sZHNpZyMiPgogICAgPGRzaWc6U2lnbmVkSW5mbyB4bWxucz0iaHR0cDovL2J4ZC5maS94bWxkYXRhLyIgeG1sbnM6ZHNpZz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC8wOS94bWxkc2lnIyI+CiAgICAgIDxkc2lnOkNhbm9uaWNhbGl6YXRpb25NZXRob2QgQWxnb3JpdGhtPSJodHRwOi8vd3d3LnczLm9yZy9UUi8yMDAxL1JFQy14bWwtYzE0bi0yMDAxMDMxNSNXaXRoQ29tbWVudHMiPjwvZHNpZzpDYW5vbmljYWxpemF0aW9uTWV0aG9kPgogICAgICA8ZHNpZzpTaWduYXR1cmVNZXRob2QgQWxnb3JpdGhtPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwLzA5L3htbGRzaWcjcnNhLXNoYTEiPjwvZHNpZzpTaWduYXR1cmVNZXRob2Q+CiAgICAgIDxkc2lnOlJlZmVyZW5jZSBVUkk9IiI+CiAgICAgICAgPGRzaWc6VHJhbnNmb3Jtcz4KICAgICAgICAgIDxkc2lnOlRyYW5zZm9ybSBBbGdvcml0aG09Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvMDkveG1sZHNpZyNlbnZlbG9wZWQtc2lnbmF0dXJlIj48L2RzaWc6VHJhbnNmb3JtPgogICAgICAgIDwvZHNpZzpUcmFuc2Zvcm1zPgogICAgICAgIDxkc2lnOkRpZ2VzdE1ldGhvZCBBbGdvcml0aG09Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvMDkveG1sZHNpZyNzaGExIj48L2RzaWc6RGlnZXN0TWV0aG9kPgogICAgICAgIDxkc2lnOkRpZ2VzdFZhbHVlPjBKRnE3cjdyNVhjd1RKbjlMMnNUeVZPQUtoRT0KPC9kc2lnOkRpZ2VzdFZhbHVlPgogICAgICA8L2RzaWc6UmVmZXJlbmNlPgogICAgPC9kc2lnOlNpZ25lZEluZm8+CiAgICA8ZHNpZzpTaWduYXR1cmVWYWx1ZT5RYWYxUmVJQ0lhZ3NPTmtwNVdHYmQ4Szd6cHAra05KUnJHMjE3c20yQnFWb2tuVzJEakwrblRHZmpPQk4Kdm9zektnUGVURUVhRE8weVFqM0hxS3pONjg0UVZOSmhOUHYxc1NVNmN2UDlVZUhlL2JZYVQ5R05DdmMyCnNQdGM4WGJ6Zm0xMVR2cFU3bFN1Tk42bDhpdkFJVzMzckZGK2pKZ2JodWNjQXBTOG5sbmU1TmNTaDR2VApPeFBNcC96MEgvZVlVTkxVbW4zN2dBZmFmZmJhMEI5YUtCcldiZXlsTDFNa1U2dHJSN0hPVThuYW80M20KTzVvRU0yRlVPWWQ4bVV3YUVlZC94M29aYWp2RUJidEl6Z21lZ2krRjlYelNTdVM5Y3V4SkpEc1ROUElkClYxZ21rV2pNblZjcS9kWVl6dnhIcDk2KzAwZXV4bnBZYWtrYi9PaHpHQT09CjwvZHNpZzpTaWduYXR1cmVWYWx1ZT4KICAgIDxkc2lnOktleUluZm8+CiAgICAgIDxkc2lnOlg1MDlEYXRhPgogICAgICAgIDxkc2lnOlg1MDlDZXJ0aWZpY2F0ZT5NSUlFd3pDQ0FxdWdBd0lCQWdJUEFZVzVRZnIreXpsdWVrOGE3U21STUEwR0NTcUdTSWIzRFFFQkN3VUFNRDB4Q3pBSkJnTlZCQVlUQWtaSk1SQXdEZ1lEVlFRS0RBZFRZVzFzYVc1ck1Sd3dHZ1lEVlFRRERCTlRZVzFzYVc1cklFTjFjM1J2YldWeUlFTkJNQjRYRFRJek1ERXhOakEyTWpZeU4xb1hEVEkxTURFeE5UQTJNall5TjFvd1lqRUxNQWtHQTFVRUJoTUNSa2t4SmpBa0JnTlZCQW9NSFVGcGJtVnBjM1J2Y0dGc2RtVnNkWFF0VTJGaGMzUnZjR0Z1YTJ0cE1SZ3dGZ1lEVlFRRERBOUxRVk10VWtGTFJVNU9WVk1nVDFreEVUQVBCZ05WQkFRTUNESXhOalE0TXpnMk1JSUJJakFOQmdrcWhraUc5dzBCQVFFRkFBT0NBUThBTUlJQkNnS0NBUUVBcHNMR1BWN0NWMlF4bmMzR2JYb1REdDFYaVB1dnZDRmFKS2xPWG50SU83Y3hqU3RqSnJ4RnZUdGtiRHVGZ2dNUVBsOXUyZnpmZVRzTnpyVEwyb2hyY2lhRkR2M2NXcmV3Vk1sOU1JbFZwNURidGo0ZGc4M20wYTlKMlN0TTk5K3Y0ZGhzdElseHErQTZTOTdKbDF4RmN1OEJOdi9XT1dMOS80WTFBamxPUmxxbTMzbzZDZ0RwNUQ1LzIvNDgxc1c2cDlxeS9WdDlXbDZtNXlRZVFnSmdNcXlWQUlIWDZjOTVSeEhGVDgrQkxySXNXekd3NDR2VWZqRUZnbk81akVsS1FodnBMQ1ZWcDdOeTB6dUZSbFB0eHIxYkxHdU14SUdLOE54bSs2Q0wyY1ZzU05Pb1UzYjlHWXdXSVBHTXVoOW5BNjBySEtTbFVYeTBMWWZKdUx3RFJ3SURBUUFCbzRHYU1JR1hNQjhHQTFVZEl3UVlNQmFBRk1xQU9ET1RpbU1Fa1kwRmFWWm9RalhseC8rOE1CMEdBMVVkRGdRV0JCUTQya1hPWUY2anZVOTR1MldTUndrOGJuWHloVEFPQmdOVkhROEJBZjhFQkFNQ0JQQXdSUVlEVlIwZkJENHdQREE2b0RpZ05vWTBhSFIwY0RvdkwyaDBkSEJqY213dWRISjFjM1F1ZEdWc2FXRXVZMjl0TDNOaGJXeHBibXRqZFhOMGIyMWxjbU5oTG1OeWJEQU5CZ2txaGtpRzl3MEJBUXNGQUFPQ0FnRUFLRHByRXl1aFRzUm5hNmRzbzBhcE5JekZHcmtkWXhIY3F1YUt1cDhPY2loenFHZVNPbzFXQkRtWFg3L0E2Y2FKaVIzK29JRmdHNUdKMDEyakRjQkVFN1RIY2JyYlZONENieDhsSW9SRXFZK3BqUW1Kb0Z5RjNZb0hhbklRc1p2cW80bmZFMTIwdE1zWkkvYlhtTFM3cThxMGtBVWxKbzhFYURNTUt6bktxbUhLektWcDM0RHhYTkpFcllwWkZ3anVUb3VjdUJOa3puQzQ2cEtCMWlOQ3JlQS9BTzZVaTJlR1Y0STNFYWpaMllNVE43eVRyM2c3Q3hqdWtLWWp5dGQ2aWtFLzVabmV3aTRMQWRhSlF3aVErcUpzN1RHR2ZKNElqbjBGTzBaNkNkMzBySTlrcTNxVWx3WXFIQXowbWdXKy9RT0N1UWUvRU1URHRUcEEvbmxiM2l0eDRkcEhiUTAwaVhCd0N5UTZwRUIyYWpXdGtIaS9Pd1BhUEVqQTlIK042QVNjR2M5MmU2WXVkK1VNM1dmemhFd25RejVEaDJSRlFoMmxKeHFmck83dlQyVGtlMVAyS2ZpajZibjM4Tnk0a0RJRUpvdm9KcmxURjRtRHd2NnpxYk5nQnR5RkJwbXNyTld6bnFmdVN5Q3NBTjkveXRUbEVhSDFVSzBCRVFFR2dXQ0x5cFJKc3BtQld2K3VCV3V2a2VXK0Z0SEErdFV2MXBMOFEzNXZUVG9VOHQvSkdndGJ4UTBuWGJLb2x0ZjRnait2bzhkeHErcmltWEpNQkQyTGRmdFBoUm9EVkhuR0VaL2dGbjByeUhPNGtjZTE1bms3Ujdkb055WGl0Q0lDZkErNmVMTlVMcGZFeUFBRzRDajZuNUVTZndCSlhicTYwYWZRQkpuVEZscz08L2RzaWc6WDUwOUNlcnRpZmljYXRlPgogICAgICA8L2RzaWc6WDUwOURhdGE+CiAgICA8L2RzaWc6S2V5SW5mbz4KICA8L2RzaWc6U2lnbmF0dXJlPjwvQXBwbGljYXRpb25SZXF1ZXN0Pgo=)";

    std::string appreq_enc_in = Utilities::encode(areq_in);
    if(appreq_enc_in != areq_enc) {
        LOG(DEBUG) << "Application request encoding failed";
    }
    m_body_template.el("bxd:ApplicationRequest")->SetText(appreq_ecoded.c_str());
    process_header();
    add_body_to_header();
    return tinyxml2::XML_SUCCESS;
}
