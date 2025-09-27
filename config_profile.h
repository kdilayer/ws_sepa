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
#include "util.h"
#include "logger.h"
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <string>

class ConfigProfile {
public:
    
    ConfigProfile(std::string configFile_);
    bool load();
    bool save();

    std::string getName() const { return name; }
    std::string getConnName() const { return conn_name; }
    std::string getBankIban() const { return bank_iban; }   
    std::string getCustomerId() const { return customer_id; }
    std::string getBank() const { return bank; }
    std::string getEnvironment() const { return environment; }
    std::string getSigningPrivateKey() const { return signing_private_key; }
    std::string getEncryptionPrivateKey() const { return encryption_private_key; }
    std::string getOwnSigningCertificate() const { return own_signing_certificate; }
    std::string getBankEncryptionCertificate() const { return bank_encryption_certificate; }
    std::string getCsrCn() const { return csr_cn; }
    std::string getPincode() const { return pincode; }
    std::string getOutputFolder() const { return output_folder; }
    std::string getSigningCsr() const { return signing_csr; }
    std::string getNewSigningCsr() const { return new_signing_csr; }
    std::string getNewSigningPrivateKey() const { return new_signing_private_key; }

    std::string getOldOwnSigningCertificate() const { return old_own_signing_certificate; }
    std::string getOldSigningPrivateKey() const { return old_signing_private_key; }
    std::string getOldSigningCsr() const { return old_signing_csr; }

    void setNewSigningCsr(const std::string& csr) { new_signing_csr = csr; }
    void setNewSigningPrivateKey(const std::string& privateKey) { new_signing_private_key = privateKey; }
    void setNewOwnSigningCertificate(const std::string &certPem) { new_own_signing_certificate = certPem; }

    bool storeNewSigningCertAndKey();

    void toParams(std::map<std::string, std::string> &params);

    std::vector<std::pair<std::string, std::string>> getDownloadFileTypes() const { return download_file_types; }
    std::vector<std::pair<std::string, std::string>> getUploadFileTypes() const { return upload_file_types; }
    
private:
    bool parseConfig(std::string content); 
    bool parseJson(const rapidjson::Value& profile);

    std::string configFile;

    std::string name;
	std::string conn_name;
    std::string bank_iban;
	std::string customer_id;

	std::string bank;
	std::string environment="production";
	std::string signing_private_key; //SIGNING_KEY,
	std::string encryption_private_key; //ENCRYPTION_KEY, -> does not exist for aktia
	std::string own_signing_certificate;//OWN_SIGNING_CERT,
    std::string signing_csr;

    std::string own_encryption_certificate; //does not exist  for aktia

	std::string bank_encryption_certificate;//bank_encryption_certificate

    std::vector<std::pair<std::string, std::string>> download_file_types;
    std::vector<std::pair<std::string, std::string>> upload_file_types;

    std::string csr_cn;
    std::string pincode; //PINCODE
    std::string output_folder = "output"; //default output folder - working folder for this program

    std::string new_signing_csr;
    std::string new_signing_private_key;
    std::string new_own_signing_certificate;

    std::string old_own_signing_certificate;
    std::string old_signing_private_key;
    std::string old_signing_csr;

};