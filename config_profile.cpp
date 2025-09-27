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
#include "config_profile.h"
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

 ConfigProfile::ConfigProfile(std::string configFile_){
    configFile=configFile_;

}
bool ConfigProfile::load() {
    std::string configFileContent;
    if(configFile.empty() || (configFileContent = ReadFileContent(configFile)).empty()) {
        LOG(ERROR) << "Config file not specified or is empty: " + configFile;
        return false;
    }
    return parseConfig(configFileContent);
}

bool ConfigProfile::parseJson(const rapidjson::Value& profile) {
    if(profile.IsObject() && profile.HasMember("name") && profile["name"].IsString()) {
        name = profile["name"].GetString();
    } else {
        LOG(DEBUG) << "Profile name not found in profile ";
    }
    if (profile.IsObject() && profile.HasMember("conn_name") && profile["conn_name"].IsString()) {
        conn_name = profile["conn_name"].GetString();
    } else {
        LOG(DEBUG) << "Conn Name not found in profile ";
    }
    if(profile.IsObject() && profile.HasMember("bank_iban") && profile["bank_iban"].IsString()) {
        bank_iban = profile["bank_iban"].GetString();
    } else {
        LOG(DEBUG) << "Bank IBAN not found in profile ";
    }   
    if(profile.IsObject() && profile.HasMember("customer_id") && profile["customer_id"].IsString()) {
        customer_id = profile["customer_id"].GetString();
    } else {
        LOG(DEBUG) << "Customer ID not found in profile ";
    }
    if(profile.IsObject() && profile.HasMember("bank") && profile["bank"].IsString()) {
        bank = profile["bank"].GetString();
    } else {
        LOG(DEBUG) << "Bank not found in profile ";
    }
    if(profile.IsObject() && profile.HasMember("environment") && profile["environment"].IsString()) {
        environment = profile["environment"].GetString();
    } else {
        LOG(DEBUG) << "Environment not found in profile ";
    }
    if(profile.IsObject() && profile.HasMember("signing_private_key") && profile["signing_private_key"].IsString()) {
        signing_private_key = profile["signing_private_key"].GetString();
    } else {
        LOG(DEBUG) << "Signing Private Key not found in profile ";
    }
    if(profile.IsObject() && profile.HasMember("encryption_private_key") && profile["encryption_private_key"].IsString()) {
        encryption_private_key = profile["encryption_private_key"].GetString();
    } else {
        LOG(DEBUG) << "Encryption Private Key not found in profile ";
    }
    if(profile.IsObject() && profile.HasMember("own_signing_certificate") && profile["own_signing_certificate"].IsString()) {
        own_signing_certificate = profile["own_signing_certificate"].GetString();
    } else {
        LOG(DEBUG) << "Own Signing Certificate not found in profile ";
    }
    if(profile.IsObject() && profile.HasMember("bank_encryption_certificate") && profile["bank_encryption_certificate"].IsString()) {
        bank_encryption_certificate = profile["bank_encryption_certificate"].GetString();
    } else {
        LOG(DEBUG) << "Bank Encryption Certificate not found in profile ";
    }
    if(profile.IsObject() && profile.HasMember("csr_cn") && profile["csr_cn"].IsString()) {
        csr_cn = profile["csr_cn"].GetString();
    } else {
        LOG(DEBUG) << "CSR CN not found in profile ";
    }
    if(profile.IsObject() && profile.HasMember("pincode") && profile["pincode"].IsString()) {
        pincode = profile["pincode"].GetString();
    } else {
        LOG(DEBUG) << "Pincode not found in profile ";
    }
    if(profile.IsObject() && profile.HasMember("output_folder") && profile["output_folder"].IsString()) {
        output_folder = profile["output_folder"].GetString();
    } else {
        LOG(DEBUG) << "Output Folder not found in profile ";
    }
    if(profile.IsObject() && profile.HasMember("download_file_types") && profile["download_file_types"].IsArray()) {
        for (rapidjson::Value::ConstValueIterator itr = profile["download_file_types"].Begin(); itr != profile["download_file_types"].End(); ++itr) {
            std::string type = "";
            std::string status = "";
            if (itr->HasMember("type") && (*itr)["type"].IsString()) {
                type = (*itr)["type"].GetString();
            }
            if (itr->HasMember("status") && (*itr)["status"].IsString()) {
                status = (*itr)["status"].GetString();
            }
            if(type.empty()) {
                LOG(WARNING) << "Invalid download file type entry in profile - missing type";
                continue;
            }
            download_file_types.push_back({type, status});
        }
    } else {
        LOG(DEBUG) << "Download File Types not found in profile ";
    }
    if(profile.IsObject() && profile.HasMember("upload_file_types") && profile["upload_file_types"].IsArray()) {
        for (rapidjson::Value::ConstValueIterator itr = profile["upload_file_types"].Begin(); itr != profile["upload_file_types"].End(); ++itr) {
            std::string type = "";
            std::string status = "";
            if (itr->HasMember("type") && (*itr)["type"].IsString()) {
                type = (*itr)["type"].GetString();
            }
            if (itr->HasMember("status") && (*itr)["status"].IsString()) {
                status = (*itr)["status"].GetString();
            }
            if(type.empty()) {
                LOG(WARNING) << "Invalid upload file type entry in profile - missing type";
                continue;
            }
            upload_file_types.push_back({type, status});
        }
    } else {
        LOG(DEBUG) << "Upload File Types not found in profile ";
    }
    return true;
}
bool ConfigProfile::parseConfig(std::string configFileContent){
    rapidjson::Document layout;
    rapidjson::ParseResult iok = layout.Parse(configFileContent.c_str());
    if(iok) {
        LOG(INFO) << "Config file " << configFile << " loaded successfully";
    } else {
        LOG(ERROR) << "Failed to parse config file " << configFile << ": "
                   << rapidjson::GetParseError_En(layout.GetParseError())
                   << " (offset " << layout.GetErrorOffset() << ")";
        // Optionally, print the line number:
        size_t offset = layout.GetErrorOffset();
        size_t line = 1;
        for (size_t i = 0; i < offset && i < configFileContent.size(); ++i) {
            if (configFileContent[i] == '\n') ++line;
        }
        LOG(ERROR) << "Parse error occurred at line: " << line;
        return false;
    }
    if (!layout.IsObject() || !layout.HasMember("profile")) {
        LOG(ERROR) << "Invalid config file format: 'profiles' array not found";
        return false;
    }
    const rapidjson::Value& profile = layout["profile"];
    return parseJson(profile);
}
void ConfigProfile::toParams(std::map<std::string, std::string> &params) {
    params["conn_name"] = conn_name;
    params["bank_iban"] = bank_iban;
    params["customer_id"] = customer_id;
    params["bank"] = bank;
    params["environment"] = environment;
    params["signing_private_key"] = signing_private_key;
    params["encryption_private_key"] = encryption_private_key;
    params["own_signing_certificate"] = own_signing_certificate;
    params["bank_encryption_certificate"] = bank_encryption_certificate;
    params["pin"] = pincode;
    params["new_signing_csr"] = new_signing_csr;
    params["new_signing_private_key"] = new_signing_private_key;
}

bool ConfigProfile::save() {
    // Save the new configuration
    LOG(INFO) << "Saving new configuration";

    //first read the old file
    std::string configFileContent;
    if(configFile.empty() || (configFileContent = ReadFileContent(configFile)).empty()) {
        LOG(ERROR) << "Config file not specified or is empty: " + configFile;
        return false;
    }

    //make a copy of the config file - just in case
    std::string backupConfigFile = configFile + std::string(".")+ std::to_string((long)time(nullptr)) + ".bak";
    if(!WriteFileContent(backupConfigFile, configFileContent)) {
        LOG(ERROR) << "Failed to create backup config file: " + backupConfigFile;
    }

     //first read the old file
    rapidjson::Document layout;
    rapidjson::ParseResult iok = layout.Parse(configFileContent.c_str());
    if(!iok) {
        LOG(INFO) << "Config file " << configFile << " could not be parsed";
        return false;
    }
  
    if (!layout.IsObject() || !layout.HasMember("profile")) {
        LOG(ERROR) << "Invalid config file format: 'profiles' array not found";
        return false;
    }
    const rapidjson::Value& profile = layout["profile"];
    if(profile.IsObject() && !profile.HasMember("name") ) {
        layout["profile"].AddMember("name", rapidjson::Value().SetString("", layout.GetAllocator()), layout.GetAllocator());
    }
    if(profile.IsObject() && !profile.HasMember("conn_name")) {
        layout["profile"].AddMember("conn_name", rapidjson::Value().SetString("", layout.GetAllocator()), layout.GetAllocator());
    }
    if(profile.IsObject() && !profile.HasMember("bank_iban") ) {
        layout["profile"].AddMember("bank_iban", rapidjson::Value().SetString("", layout.GetAllocator()), layout.GetAllocator());
    }
    if(profile.IsObject() && !profile.HasMember("customer_id") ) {
        layout["profile"].AddMember("customer_id", rapidjson::Value().SetString("", layout.GetAllocator()), layout.GetAllocator());
    }
    if(profile.IsObject() && !profile.HasMember("bank")) {
        layout["profile"].AddMember("bank", rapidjson::Value().SetString("", layout.GetAllocator()), layout.GetAllocator());
    }
    if(profile.IsObject() && !profile.HasMember("environment")) {
        layout["profile"].AddMember("environment", rapidjson::Value().SetString("", layout.GetAllocator()), layout.GetAllocator());
    }
    if(profile.IsObject() && !profile.HasMember("signing_private_key") ) {
        layout["profile"].AddMember("signing_private_key", rapidjson::Value().SetString("", layout.GetAllocator()), layout.GetAllocator());
    }
    if(profile.IsObject() && !profile.HasMember("encryption_private_key")) {
        layout["profile"].AddMember("encryption_private_key", rapidjson::Value().SetString("", layout.GetAllocator()), layout.GetAllocator());
    }
    if(profile.IsObject() && !profile.HasMember("own_signing_certificate")) {
        layout["profile"].AddMember("own_signing_certificate", rapidjson::Value().SetString("", layout.GetAllocator()), layout.GetAllocator());
    }
    if(profile.IsObject() && !profile.HasMember("bank_encryption_certificate") ) {
        layout["profile"].AddMember("bank_encryption_certificate", rapidjson::Value().SetString("", layout.GetAllocator()), layout.GetAllocator());
    }
    if(profile.IsObject() && !profile.HasMember("signing_csr") ) {
        layout["profile"].AddMember("signing_csr", rapidjson::Value().SetString("", layout.GetAllocator()), layout.GetAllocator());
    }
    if(profile.IsObject() && !profile.HasMember("old_own_signing_certificate")) {
        layout["profile"].AddMember("old_own_signing_certificate", rapidjson::Value().SetString("", layout.GetAllocator()), layout.GetAllocator());
    }
    if(profile.IsObject() && !profile.HasMember("old_signing_private_key")) {
        layout["profile"].AddMember("old_signing_private_key", rapidjson::Value().SetString("", layout.GetAllocator()), layout.GetAllocator());
    }
    if(profile.IsObject() && !profile.HasMember("old_signing_csr")) {
        layout["profile"].AddMember("old_signing_csr", rapidjson::Value().SetString("", layout.GetAllocator()), layout.GetAllocator());
    }
    
    layout["profile"]["name"].SetString(getName().c_str(), layout.GetAllocator());
    layout["profile"]["conn_name"].SetString(getConnName().c_str(), layout.GetAllocator());
    layout["profile"]["bank_iban"].SetString(getBankIban().c_str(), layout.GetAllocator());
    layout["profile"]["customer_id"].SetString(getCustomerId().c_str(), layout.GetAllocator());
    layout["profile"]["bank"].SetString(getBank().c_str(), layout.GetAllocator());
    layout["profile"]["environment"].SetString(getEnvironment().c_str(), layout.GetAllocator());
    layout["profile"]["signing_private_key"].SetString(getSigningPrivateKey().c_str(), layout.GetAllocator());
    layout["profile"]["encryption_private_key"].SetString(getEncryptionPrivateKey().c_str(), layout.GetAllocator());
    layout["profile"]["own_signing_certificate"].SetString(getOwnSigningCertificate().c_str(), layout.GetAllocator());
    
    layout["profile"]["bank_encryption_certificate"].SetString(getBankEncryptionCertificate().c_str(), layout.GetAllocator());
    layout["profile"]["signing_csr"].SetString(getSigningCsr().c_str(), layout.GetAllocator());

    layout["profile"]["old_own_signing_certificate"].SetString(getOldOwnSigningCertificate().c_str(), layout.GetAllocator());
    layout["profile"]["old_signing_private_key"].SetString(getOldSigningPrivateKey().c_str(), layout.GetAllocator());
    layout["profile"]["old_signing_csr"].SetString(getOldSigningCsr().c_str(), layout.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    layout.Accept(writer);

    // Output {"project":"rapidjson","stars":11}
    std::string content =  buffer.GetString();

    return WriteFileContent(configFile, content, true);
}

bool ConfigProfile::storeNewSigningCertAndKey() {

    
    // Store the new signing certificate and key
    old_own_signing_certificate = own_signing_certificate;
    old_signing_private_key = signing_private_key;
    old_signing_csr = signing_csr;

    //update the internal values
    own_signing_certificate = new_own_signing_certificate;
    signing_private_key = new_signing_private_key;
    signing_csr = new_signing_csr;

    //reset the new values
    new_own_signing_certificate.clear();
    new_signing_private_key.clear();
    new_signing_csr.clear();

    //also pin code needs to be cleared 
    pincode.clear();

    //and now we save / overwrite the config file
    return save();
}