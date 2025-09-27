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
#include "webservicesapi.h"
#include <ctime>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <map>
#include <cstdio>
#include <sstream>
#include <vector>
#include "sepaxml.h"
#include "soapclient.h"
#include "wsfiledescriptor.h"
#include "util.h"

WebServicesAPI::WebServicesAPI(const ConfigProfile& config)
    : configProfile(config) {}

std::string internal_read_subject_info(const std::string& programFullName, const std::string& oldcertfile) {
    std::string scriptCom = programFullName + " x509 -noout -subject -in " + oldcertfile;
    std::array<char, 256> buffer;
    std::string out;
    FILE* pipe = popen(scriptCom.c_str(), "r");
    if (!pipe) {
        // log error if needed
        return "";
    }
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        out += buffer.data();
    }
    int exitValue = pclose(pipe);
    if (exitValue != 0) {
        // log error if needed
        return "";
    }

    // Remove "subject=" from the output
    std::string wosubject = out;
    string_replaceall(wosubject, "subject=", "");
    string_replaceall(wosubject, "\n", "");
    string_replaceall(wosubject, "C = ", "/C=");
    string_replaceall(wosubject, "O = ", "/O=");
    string_replaceall(wosubject, "CN = ", "/CN=");
    string_replaceall(wosubject, "SN = ", "/SN=");
    string_replaceall(wosubject, ", ", "");

    
    return wosubject;
}
std::map<std::string, std::string> internal_openssl_generate_csr(
    const std::string& bank,
    const std::string& uniqueId,
    const std::string& tmpDataFilesDir,
    const std::string& prefix,
    const std::string& oldPrivateKeyPem,
    const std::string& oldCertificatePem,
    const std::string& sn_subjectInfo
    ) {
    namespace fs = std::filesystem;
    std::map<std::string, std::string> result;
    std::string identifier = uniqueId;
    std::string programFullName = "/usr/bin/openssl";
    std::string oldCertFile = tmpDataFilesDir + "/" + identifier + "_" + prefix + "_cert.pem";
    std::string oldPrivateKeyFile = tmpDataFilesDir + "/" + identifier + "_" + prefix + "_key.key";
    std::string newCsrFile = tmpDataFilesDir + "/" + identifier + "_" + prefix + "_csr_new.csr";
    std::string newPrivateKeyFile = tmpDataFilesDir + "/" + identifier + "_" + prefix + "_key_new.key";

    fs::path old_cert_path(oldCertFile);
    fs::path old_private_key_path(oldPrivateKeyFile);
    fs::path csr_file_tmp_path(newCsrFile);

    mkdir(tmpDataFilesDir.c_str(), 0755);
    // If all files exist, skip re-generation
    if (fs::exists(old_cert_path) && fs::exists(old_private_key_path) && fs::exists(csr_file_tmp_path)) {
        std::ifstream csr_in(newCsrFile);
        std::stringstream buffer;
        buffer << csr_in.rdbuf();
        result["sign_csr"] = buffer.str();
        return result;
    }

    // Write old certificate PEM to file
    std::ofstream cert_out(oldCertFile);
    cert_out << oldCertificatePem;
    cert_out.close();

    //use old subjectinfo if exists
    std::string subjectInfo = internal_read_subject_info(programFullName, oldCertFile);
    if(subjectInfo == "") {
        subjectInfo = sn_subjectInfo;
    }
    // Optionally log subjectInfo

    // Write old private key PEM to file
    std::ofstream key_out(oldPrivateKeyFile);
    key_out << oldPrivateKeyPem;
    key_out.close();

    // Check if old files exist
    /*
    if (!fs::exists(old_cert_path) || !fs::exists(old_private_key_path)) {
        result["success"] = "false";
        return result;
    } 
    */

    std::string keylength = (bank == "nordea") ? "1024" : "2048";
    std::vector<std::string> args = {
        programFullName, "req", "-subj", subjectInfo, "-newkey", "rsa:" + keylength, "-nodes",
        "-keyout", newPrivateKeyFile, "-out", newCsrFile
    };

    // Build command string
    std::ostringstream cmd;
    for (const auto& arg : args) {
        if (arg.find(' ') != std::string::npos || arg.find('/') != std::string::npos) {
            cmd << '"' << arg << '"' << ' ';
        } else {
            cmd << arg << ' ';
        }
    }
    

    std::string scriptCom = cmd.str();
    scriptCom+=" 2>/dev/null";

    // Execute command
    LOG(DEBUG)<< "Executing command: " << scriptCom;
    int ret = std::system(scriptCom.c_str());
    // Optionally log ret

    fs::path csr_file(newCsrFile);
    fs::path prvkey_file(newPrivateKeyFile);
    if (!fs::exists(csr_file) || !fs::exists(prvkey_file)) {
        // Clean up
        fs::remove(old_private_key_path);
        fs::remove(old_cert_path);
        result["success"] = "false";
        return result;
    }

    std::ifstream csr_in(newCsrFile);
    std::stringstream csr_buffer;
    csr_buffer << csr_in.rdbuf();
    std::string csr_content = csr_buffer.str();

    std::ifstream prvkey_in(newPrivateKeyFile);
    std::stringstream prvkey_buffer;
    prvkey_buffer << prvkey_in.rdbuf();
    std::string prvkey_content = prvkey_buffer.str();

    // Clean up
    fs::remove(old_private_key_path);
    fs::remove(old_cert_path);
    fs::remove(csr_file);
    fs::remove(prvkey_file);

    rmdir(tmpDataFilesDir.c_str());
    result["csr_content"] = csr_content;
    result["prvkey_content"] = prvkey_content;
    result["success"] = "true";
    return result;
}
bool WebServicesAPI::configIsValid(bool needcertsandkeys, bool needcertcn, bool neednewcertsandkeys) const {
    if(configProfile.getBank().empty()) {
        LOG(ERROR) << "Bank information is missing, specify bank='aktia', 'nordea', 'op'";
        return false;
    }
    if(configProfile.getCustomerId().empty()) {
        LOG(ERROR) << "Customer ID is missing, specify customer_id='your_customer_id' (from you webservice contract)";
        return false;
    }
    if(configProfile.getOutputFolder().empty()) {
        LOG(ERROR) << "Output folder is missing, specify output_folder='/example/output' (the working forlder for ws_sepa program)";
        return false;
    }
    if(configProfile.getBankIban().empty()) {
        LOG(ERROR) << "Bank IBAN is missing, specify bank_iban='your bank account code e.g FI9649631110000606'";
        return false;
    }
    if(needcertsandkeys) {
        if(configProfile.getSigningPrivateKey().empty()) {
            LOG(ERROR) << "Signing private key is missing, specify signing_private_key='your_signing_private_key'";
            return false;
        }
        if(configProfile.getOwnSigningCertificate().empty()) {
            LOG(ERROR) << "Own signing certificate is missing, specify own_signing_certificate='your_own_signing_certificate'";
            return false;
        }
        if(configProfile.getBankEncryptionCertificate().empty()) {
            LOG(ERROR) << "Bank encryption certificate is missing, specify bank_encryption_certificate='your_bank_encryption_certificate'";
            return false;
        }
    }
    if(needcertcn) {
        if(configProfile.getCsrCn().empty()) {
            LOG(ERROR) << "CSR Common Name is missing, specify csr_cn='your_csr_cn'";
            return false;
        }
    }

    return true;
}
bool WebServicesAPI::PKI_generateCSR(const std::string cn) {
    if(!configIsValid(false, true)) {
        LOG(ERROR) << "Missing input fields";
        return false;
    }    
    /*
    if cn is "" we get the cn from the old certificate
    */
    // TODO: Implement CSR generation logic
    LOG(DEBUG) << "Generating CSR for CN: " << cn;
    std::map<std::string, std::string> signing_csr =  internal_openssl_generate_csr(configProfile.getBank(),
                                  configProfile.getCustomerId(),
                                  configProfile.getOutputFolder()+"/tmp_cert",
                                  configProfile.getBank() + "_" + configProfile.getBankIban() + "_sign",
                                  configProfile.getSigningPrivateKey(),
                                  configProfile.getOwnSigningCertificate(),
                                  configProfile.getCsrCn());
    if(signing_csr["success"] == "true") {
        configProfile.setNewSigningCsr(signing_csr["csr_content"]);
        configProfile.setNewSigningPrivateKey(signing_csr["prvkey_content"]);
        return true;
    }
    return false;
}
bool WebServicesAPI::PKI_getNewCertificate() {
    /*we dont need old keys for this, as long as we have csr*/
    if(!configIsValid(false)) {
        LOG(ERROR) << "Missing input fields";
        return false;
    }    
    if(configProfile.getNewSigningCsr().empty() || configProfile.getNewSigningPrivateKey().empty()) {
        LOG(ERROR) << "New CSR or private key is missing, generate CSR first";
        return false;
    }
    if(configProfile.getBankEncryptionCertificate().empty()) {
        LOG(ERROR) << "Bank encryption certificate is missing, specify bank_encryption_certificate='your_bank_encryption_certificate'";
        return false;
    }
    /*save the new key an csr just in case
    std::string ncsr = configProfile.getNewSigningCsr();
    std::string nkey = configProfile.getNewSigningPrivateKey();
    mkdir(std::string(configProfile.getOutputFolder() + "/tmp_keys").c_str(), 0755);
    WriteFileContent(configProfile.getOutputFolder() + "/tmp_keys/new_signing.csr", ncsr, true);
    WriteFileContent(configProfile.getOutputFolder() + "/tmp_keys/new_signing.key", nkey, true);
    */
    LOG(DEBUG) << "Getting new certificate...";

    //
    std::map<std::string, std::string> params;
    configProfile.toParams(params);
    params["signing_csr"] = configProfile.getNewSigningCsr();

    //params["debug"]="true";
    params["command"]="get_certificate";

    AktiaSoapBuilder sb(params);
    sb.build_soap();
    std::string xml_out = sb.to_XML();
    if(params["debug"]=="true") {
        LOG(DEBUG) << "XML: >>" << xml_out << "<<";
    }
    SoapClient sc(params);
    bool ok = sc.send_request(params["command"],  xml_out, sb.m_params["debug"] == "true");
    if(!ok){
        if(sb.m_params["debug"]=="true") {
            LOG(ERROR) << "failed to send, bailing...";
            return -1;
        }
    }
    if(sc.has_response() ) { 
        std::string content = sc.raw_response();
        /* debug */
        std::string fname = configProfile.getOutputFolder() + "/sepa_response_get_certificate.txt";
		if(WriteFileContent(fname, content, true)) {
        	LOG(DEBUG) << "RESP written to file: " << fname << "";
		}
        
        ApplicationResponse response(params, sc.raw_response());
        std::string xml_response= response.to_XML();
        /* debug */
        fname = configProfile.getOutputFolder() + "/sepa_response_get_certificate_apprequest.xml";
        if(WriteFileContent(fname, xml_response, true)) {
        	LOG(DEBUG) << "RESP written to file: " << fname << "";
            LOG(DEBUG) << "RESP: " << xml_response << "";
		}
        if(response.is_valid()) {
            std::string new_cert = response.getOwnEncryptionCertificate(xml_response);
            configProfile.setNewOwnSigningCertificate(new_cert);
            return configProfile.storeNewSigningCertAndKey();
        }
        else {
            LOG(ERROR) << "Invalid response " << response.getErrorMessage();
        }
    }
    else {
        LOG(ERROR) << "Did not understand response, bailing...";
        return -1;
    }
    return true;
}

bool WebServicesAPI::PKI_renewCertificate() {
    if(!configIsValid(false, true)) {
        LOG(ERROR) << "Missing input fields";
        return false;
    }    
    // TODO: Implement CSR generation logic
    LOG(DEBUG) << "Generating CSR based on old cert";
    std::map<std::string, std::string> signing_csr =  internal_openssl_generate_csr(configProfile.getBank(),
                                  configProfile.getCustomerId(),
                                  configProfile.getOutputFolder()+"/tmp_cert",
                                  configProfile.getBank() + "_" + configProfile.getBankIban() + "_sign",
                                  configProfile.getSigningPrivateKey(),
                                  configProfile.getOwnSigningCertificate(),
                                  configProfile.getCsrCn());
    if(signing_csr["success"] == "true") {
        configProfile.setNewSigningCsr(signing_csr["csr_content"]);
        configProfile.setNewSigningPrivateKey(signing_csr["prvkey_content"]);
        return PKI_getNewCertificate();
    }
    return false;
}

bool WebServicesAPI::PKI_certificateValid(std::string input) const {
    if(PKI_certificateValidTimstamp(input) < time(nullptr) ) {
        LOG(ERROR) << "Certificate has expired!";
        return false;
    }   
    return true;
}
bool _ASN1_TIME_to_tm(const ASN1_TIME *pTime, struct tm *pTm)
{
    bool result = false;
    time_t sinceEpoch = 0;
    int days = 0, seconds = 0;
    if (!pTime)
        return false;

    ASN1_TIME *epochTime = ASN1_TIME_new();
    if (!epochTime)
        return false;
    do {
        if (!ASN1_TIME_set(epochTime, time_t(0)))
            break;
        if (!ASN1_TIME_diff(&days, &seconds, epochTime, pTime))
            break;
        // No of seconds in a day = 86400
        sinceEpoch = time_t(86400LL * days + seconds);
        gmtime_r(&sinceEpoch, pTm);
        
        result = true;
    } while (0);

    ASN1_TIME_free(epochTime);
    return result;
}
time_t WebServicesAPI::PKI_certificateValidTimstamp(std::string input) const {
    struct tm tmNat = {0};
    if(input == "") {
        LOG(ERROR)<< "Signing certificate required!";
        return 0;
    }
    const char *cert_buffer = input.c_str();
    BIO *cbio = BIO_new_mem_buf((void*)cert_buffer, -1);
    X509 *x509 = PEM_read_bio_X509(cbio, NULL, 0, NULL);
    
    if(x509){
        ASN1_TIME *nat = X509_getm_notAfter(x509);
        _ASN1_TIME_to_tm(nat, &tmNat);

        BIO_free (cbio); 
        X509_free(x509);
        /*
        if(true) {
            char buf[64];
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmNat);
            LOG(DEBUG) << "Certificate valid until: " << buf;
        }
        */
        return mktime(&tmNat);
    }
    BIO_free (cbio); 
    return mktime(&tmNat);; // Dummy: valid for 60 days
}


bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    if(file.good()){
        //seek to end of file
        file.seekg(0, std::ios::end);
        std::streampos size = file.tellg();
        file.close();
        if(size < 1024 ) {
            return false;
        }
        return true;
    }
    file.close();
    return false;
}

bool WebServicesAPI::FS_downloadFiles(std::string fileType, std::string fetchPolicy, int &fetchCount){
    bool anyok = false;
    
    std::vector<WSFileDescriptor> filesToDownload = FS_downloadFileList(fileType, fetchPolicy); //download_file_list, fileType=OP, status:/previously one of "NEW"/"ALL"
    if(filesToDownload.size() == 0) {
        return true;  //nothing to download all ok
    }
    for(int i=0; i<filesToDownload.size(); i++) {
        std::string fname = configProfile.getOutputFolder() + "/incoming/"+filesToDownload[i].getFileType()+"_" + filesToDownload[i].getFileReference()+".txt";
        if(fileExists(fname)){
            LOG(INFO) << "File has already been downloaded: " << fname << ", skipping download.";
            anyok = true;
            continue;
        }
        if(!FS_downloadFile(filesToDownload[i])){
            LOG(ERROR) << "Failed to download file: " << filesToDownload[i].getFileName();
        }
        else {
            //LOG(INFO) << "File downloaded successfully: " << filesToDownload[i].getFileName();
            anyok = true;
        }
        //Sleep for 1 seconds to avoid request id collision on server side
        std::this_thread::sleep_for(std::chrono::seconds(1));


    }
    return anyok;
}
std::vector<std::string> WebServicesAPI::getFilesToUpload() {
    std::vector<std::string> files;
    std::string dir_path = configProfile.getOutputFolder() + "/outgoing";
    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path().filename());
        }
    }
    return files;
}
std::string WebServicesAPI::getUploadableType(std::string fileToUpload) {
    for (const auto& fileType : configProfile.getUploadFileTypes()) {
        std::string type = fileType.first;
        if (string_startswith(fileToUpload, type+"_")) {
            return type;
        }
    }
    return "";
}
std::string removeFirst3Letters(const std::string& str) {
    if (str.length() <= 3) {
        return "";
    }
    return str.substr(3);
}
std::string getStringStartBetween(const std::string& str, const std::string& start, const std::string& end, std::string& out) {
    size_t startPos = str.find(start);
    if (startPos != std::string::npos) {
        startPos += start.length();
        size_t endPos = str.find(end, startPos);
        if (endPos != std::string::npos) {
            out = str.substr(startPos, endPos - startPos);
            return out;
        }
    }
    return "";
}
std::vector<std::string> WebServicesAPI::FS_uploadFiles(int &uploadCount) {
    bool anyok = false;
    std::string dir_path = configProfile.getOutputFolder() + "/outgoing";
    std::vector<std::string> filesToUpload = getFilesToUpload();
    std::vector<std::string> uploadedFiles;
    for(int i=0; i<filesToUpload.size(); i++) { 
        std::string filename = filesToUpload[i];
        std::string fileType = getUploadableType(filename);
        if(fileType != ""   ) {
            LOG(INFO) << "File: " << filename << " found in outgoing directory, sending...";
            std::string content = ReadFileContent(dir_path + "/" + filename);
            //string_replaceall(content, "\r\n", "");
            if(content != "") {
                std::string errorMessage = "";
                std::string uploaded_dir = "";
                if(FS_uploadFile(fileType, filename, content, errorMessage)){
                    uploadCount++;
                    
                    //this file has been processed, move it to uploaded folder
                    uploaded_dir = configProfile.getOutputFolder() + "/uploaded";
                    
                }
                else {
                    //we failed to upload the file, we now need to create an error file.

                    uploaded_dir = configProfile.getOutputFolder() + "/not_uploaded";

                    std::string msgId="UNKNOWN";
                    getStringStartBetween(content, "<MsgId>", "</MsgId>", msgId);
                    std::string error_feedback_content = formattedString(R"(
<Document>
	<CstmrPmtStsRpt>
		<OrgnlGrpInfAndSts>
			<OrgnlMsgId>%s</OrgnlMsgId>
			<GrpSts>RJCT %s</GrpSts>
		</OrgnlGrpInfAndSts>
	</CstmrPmtStsRpt>
</Document>)", msgId.c_str(), errorMessage.c_str()  );

                    std::string fdir = configProfile.getOutputFolder() + "/incoming";
                    std::filesystem::create_directories(fdir);
                    std::string name = removeFirst3Letters(filename);
                    name = fdir + "/XP_" + name;
                    if(WriteFileContent(name, error_feedback_content, true)) {
                        LOG(INFO) << "Wrote error feedback file: " << name;
                    }
                }
                //we are done with this file, move it where it belonds
                std::filesystem::create_directories(uploaded_dir);
                //add timestamp to filename                    
                std::time_t t = std::time(nullptr);
                char timebuf[32];
                std::strftime(timebuf, sizeof(timebuf), "%Y%m%d%H%M%S", std::localtime(&t));
                std::string newfilename = filename + "_" + timebuf;
                //move the file to uploaded folder
                std::filesystem::rename(dir_path + "/" + filename, uploaded_dir + "/" + newfilename);

                uploadedFiles.push_back(filename);

                //Sleep for 1 seconds to avoid request id collision on server side
                std::this_thread::sleep_for(std::chrono::seconds(1));

            }
            else {
                LOG(ERROR) << "File content is empty: " << filename;
            }
        }
    }
    //remove uploaded files
    for(int i=0; i<uploadedFiles.size(); i++) {
        std::string filename = uploadedFiles[i];
        std::string filepath = dir_path + "/" + filename;
        // Remove the file from filesToUpload array by name
        filesToUpload.erase(
            std::remove(
                    filesToUpload.begin(), 
                    filesToUpload.end(), 
                    filename), 
               filesToUpload.end());
        
    }
    
    return filesToUpload;
}

bool WebServicesAPI::FS_uploadFile(const std::string& fileType, const std::string& fileName, const std::string& fileContent, std::string &errorMessage) {
    std::map<std::string, std::string> params;
    configProfile.toParams(params);
    params["command"]="upload_file";
    params["file_type"]=fileType;
    params["content"]=fileContent;
 
    AktiaSoapBuilder sb(params);
    sb.build_soap();
    std::string xml_out = sb.to_XML();
    if(params["debug"]=="true") {
        LOG(DEBUG) << "XML: >>" << xml_out << "<<";
    }
    SoapClient sc(params);
    bool ok = sc.send_request(params["command"],  xml_out, sb.m_params["debug"] == "true");
    if(!ok){
        if(sb.m_params["debug"]=="true") {
            LOG(ERROR) << "failed to send, bailing...";
            return false;
        }
    }
    if(sc.has_response() ) { 
        std::string content = sc.raw_response();
        /*debug
        std::string fname = configProfile.getOutputFolder() + "/sepa_response_downloadfile.xml";
		if(WriteFileContent(fname, content, true)) {
        	LOG(DEBUG) << "RESP written to file: " << fname << "";
		}
        */
        ApplicationResponse response(params, sc.raw_response());
        std::string xml_response = response.to_XML();
        
        /* debug
        fname = configProfile.getOutputFolder() + "/sepa_response_downloadfile_apprequest.xml";
        if(WriteFileContent(fname, xml_response, true)) {
        	LOG(DEBUG) << "RESP written to file: " << fname << "";
            LOG(DEBUG) << "RESP: " << xml_response << "";
		}
        */
        if(response.is_valid()) {
            return true;
        }
        errorMessage = "Code: " + response.getErrorCode()+" Message: "+response.getErrorMessage();
    }
    return false; // Return true if upload is successful
}
bool WebServicesAPI::FS_downloadFile(WSFileDescriptor &fileDescriptor) {
    //LOG(INFO) << "Downloading file with descriptor: " << fileDescriptor.getFileName();

    std::map<std::string, std::string> params;
    configProfile.toParams(params);
    //params["debug"]="true";
    params["command"]="download_file";
    params["file_type"]=fileDescriptor.getFileType();
    params["file_reference"]=fileDescriptor.getFileReference();

    AktiaSoapBuilder sb(params);
    sb.build_soap();
    std::string xml_out = sb.to_XML();
    if(params["debug"]=="true") {
        LOG(DEBUG) << "XML: >>" << xml_out << "<<";
    }
    SoapClient sc(params);
    bool ok = sc.send_request(params["command"],  xml_out, sb.m_params["debug"] == "true");
    if(!ok){
        if(sb.m_params["debug"]=="true") {
            LOG(ERROR) << "failed to send, bailing...";
            return false;
        }
    }
    if(sc.has_response() ) { 
        /*debug
        std::string content = sc.raw_response();
        std::string fname = configProfile.getOutputFolder() + "/sepa_response_downloadfile.xml";
		if(WriteFileContent(fname, content, true)) {
        	LOG(DEBUG) << "RESP written to file: " << fname << "";
		}
        */
        ApplicationResponse response(params, sc.raw_response());
        std::string xml_response = response.to_XML();
        
        /* debug
        fname = configProfile.getOutputFolder() + "/sepa_response_downloadfile_apprequest.xml";
        if(WriteFileContent(fname, xml_response, true)) {
        	LOG(DEBUG) << "RESP written to file: " << fname << "";
            LOG(DEBUG) << "RESP: " << xml_response << "";
		}
        */
        if(response.is_valid()) {
            //ApplicationResponse / Content
            XmlDoc doc(xml_response);
            tinyxml2::XMLElement* root = doc.el("Content");
            if(root) {
                std::string content = root->GetText();
                if(content != "") {
                    //content is base64 encoded data, decode it
                    std::string decoded_content = content;
                    if(fileDescriptor.getFileType() != "XT") {
                        decoded_content = iso_8859_1_to_utf8(Utilities::base64_decode_to_utf8(content));
                    }
                    else {
                        decoded_content = Utilities::base64_decode_to_utf8(content);
                    }

                    //LOG(DEBUG) << "Decoded Content: " << decoded_content;
                    if(decoded_content != "") {
                        std::string fname = configProfile.getOutputFolder() + "/incoming/"+fileDescriptor.getFileType()+"_" + fileDescriptor.getFileReference()+".txt";
                        if(WriteFileContent(fname, decoded_content, true)){
                            LOG(INFO) << "Downloaded and wrote file: " << fname;
                            return true;
                        }
                        else {
                            LOG(ERROR) << "Could not write decoded content to file: " << fname;
                        }
                    }
                    else {
                        LOG(ERROR) << "Invalid response, decoded content is empty";
                    }
                }
                else {
                    LOG(ERROR) << "Invalid response, content is empty";
                }
            }
            else {
                LOG(ERROR) << "Invalid response, content not found";
            }
        }
    }
    return false;
}
const std::vector<WSFileDescriptor> WebServicesAPI::FS_downloadFileList(const std::string& fileType, const std::string fetchPolicy){
    // TODO: Implement file list download logic
    //std::cout << "Downloading file list for type " << fileType << " with policy " << fetchPolicy << std::endl;
    std::vector<WSFileDescriptor> fileList;
    std::map<std::string, std::string> params;
    configProfile.toParams(params);
    //params["debug"]="true";
    params["command"]="download_file_list";
    params["file_type"]=fileType;
    if(fetchPolicy != "") {
        params["status"] = fetchPolicy;
    }

    AktiaSoapBuilder sb(params);
    sb.build_soap();
    std::string xml_out = sb.to_XML();
    if(params["debug"]=="true") {
        LOG(DEBUG) << "XML: >>" << xml_out << "<<";
    }
    SoapClient sc(params);
    bool ok = sc.send_request(params["command"],  xml_out, sb.m_params["debug"] == "true");
    if(!ok){
        if(sb.m_params["debug"]=="true") {
            LOG(ERROR) << "failed to send, bailing...";
            return fileList;
        }
    }
    if(sc.has_response() ) { 
        /*debug
        std::string content = sc.raw_response();
        std::string fname = configProfile.getOutputFolder() + "/sepa_response_downloadfiles.xml";
		if(WriteFileContent(fname, content, true)) {
        	LOG(DEBUG) << "RESP written to file: " << fname << "";
		}
		*/

        ApplicationResponse response(params, sc.raw_response());
        std::string xml_response = response.to_XML();
        /*debug
        fname = configProfile.getOutputFolder() + "/sepa_response_downloadfiles_apprequest.xml";
        if(WriteFileContent(fname, xml_response, true)) {
        	LOG(DEBUG) << "RESP written to file: " << fname << "";
            LOG(DEBUG) << "RESP: " << xml_response << "";
		}
        */
        if(response.is_valid()) {
            XmlDoc doc(xml_response);
            
            tinyxml2::XMLElement* root = doc.el("FileDescriptors");
            if (root) {
                for (tinyxml2::XMLElement* fileElement = root->FirstChildElement("FileDescriptor"); 
                fileElement; fileElement = fileElement->NextSiblingElement("FileDescriptor")) {
                    
                    std::string fileReference = fileElement->FirstChildElement("FileReference")->GetText();
                    std::string fileType = fileElement->FirstChildElement("FileType")->GetText();
                    std::string fileTimestamp = fileElement->FirstChildElement("FileTimestamp")->GetText();
                    std::string status = fileElement->FirstChildElement("Status")->GetText();

                    std::string serviceId = "";
                    if(fileElement->FirstChildElement("ServiceId")) {
                        serviceId = fileElement->FirstChildElement("ServiceId")->GetText();
                    }
                    std::string fileName = "";
                    if(fileElement->FirstChildElement("UserFilename")) {
                        fileName = fileElement->FirstChildElement("UserFilename")->GetText();
                    }
                    std::string lastDownloadTimestamp="";
                    if(fileElement->FirstChildElement("LastDownloadTimestamp")) {
                        lastDownloadTimestamp = fileElement->FirstChildElement("LastDownloadTimestamp")->GetText();
                    }
                    WSFileDescriptor fileDescriptor(fileReference, fileType, fileTimestamp, status, serviceId, fileName, lastDownloadTimestamp);
                    if (fileReference != "") {
                        fileList.push_back(fileDescriptor);
                    }
                    

                }
            }
            return fileList;
        }
        else {
            LOG(ERROR) << "Invalid response";
        }
    }
    else {
        LOG(ERROR) << "Did not understand response, bailing...";
        
    }
    
    return fileList;
}
