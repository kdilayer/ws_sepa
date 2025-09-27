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
/*
  sudo apt install libxmlrpc-c++8-dev
  sudo apt install libcurl4-openssl-dev
  g++ odoo_api2.cpp -o odoo_api -lxmlrpc_client++ -lxmlrpc++ -lxmlrpc_client -lxmlrpc

  "account.bank.statement.line"

    sudo mkdir /var/lib/ws_sepa
    sudo mkdir /var/log/ws_sepa
    sudo nano /etc/toolsconf/ws_sepa.conf

    sudo chown -R toolsusr:toolsusr /var/log/ws_sepa
    sudo chown -R toolsusr:toolsusr /etc/toolsconf/ws_sepa.conf
    sudo chown -R toolsusr:toolsusr /var/lib/ws_sepa

    sudo nano /etc/crontab
    30 6    * * *   toolsusr /usr/sbin/ws_sepa -c /etc/toolsconf/ws_sepa.conf > /var/log/ws_sepa/ws_sepa.log
    00 15    * * *   toolsusr /usr/sbin/ws_sepa -c /etc/toolsconf/ws_sepa.conf > /var/log/ws_sepa/ws_sepa.log
    test:
    sudo -u toolsusr /usr/sbin/ws_sepa -c /etc/toolsconf/ws_sepa.conf > /var/log/ws_sepa/ws_sepa.log
*/
/* EXMAPLE PAYMENT INSTRUCTION


<?xml version="1.0" encoding="UTF-8"?>
<Document xsi:schemaLocation="urn:iso:std:iso:20022:tech:xsd:pain.001.001.03 pain.001.001.03.xsd" xmlns="urn:iso:std:iso:20022:tech:xsd:pain.001.001.03" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
	<CstmrCdtTrfInitn>
		<GrpHdr>
			<MsgId>WSSEPA-MSGID-27556</MsgId>
			<CreDtTm>2025-09-02T16:10:35Z</CreDtTm>
			<NbOfTxs>1</NbOfTxs>
			<InitgPty>
				<Nm>Comtec SMD Oy</Nm>
			</InitgPty>
		</GrpHdr>
		<PmtInf>
			<PmtInfId>WSSEPA-UPID-27556</PmtInfId>
			<PmtMtd>TRF</PmtMtd>
			<BtchBookg>false</BtchBookg>
			<PmtTpInf>
				<SvcLvl>
					<Cd>SEPA</Cd>
				</SvcLvl>
			</PmtTpInf>
			<ReqdExctnDt>2025-09-03</ReqdExctnDt>
			<Dbtr>
				<Nm>Comtec SMD Oy</Nm>
				<Id>
					<OrgId>
						<Othr>
							<Id>03261372-5</Id>
                            <SchmeNm>
                                <Cd>BANK</Cd>
                            </SchmeNm>
							<Issr>YTJ</Issr>
						</Othr>
					</OrgId>
				</Id>
			</Dbtr>
			<DbtrAcct>
				<Id>
					<IBAN>FI1649630040018035</IBAN>
				</Id>
				<Ccy>EUR</Ccy>
			</DbtrAcct>
			<DbtrAgt>
				<FinInstnId>
					<BIC>ITELFIHH</BIC>
				</FinInstnId>
			</DbtrAgt>
			<ChrgBr>SLEV</ChrgBr>
			<CdtTrfTxInf>
				<PmtId>
					<InstrId>WSSEPA-IID-27556</InstrId>
					<EndToEndId>WSSEPA-E2EID-27556</EndToEndId>
				</PmtId>
				<Amt>
					<InstdAmt Ccy="EUR">1</InstdAmt>
				</Amt>
				<CdtrAgt>
					<FinInstnId>
						<BIC>ITELFIHH</BIC>
					</FinInstnId>
				</CdtrAgt>
				<Cdtr>
					<Nm>WWB Drink System Oy</Nm>
					<PstlAdr>
						<Ctry>FI</Ctry>
						<AdrLine>Sepänkyläntie 212, Mustasaari</AdrLine>
					</PstlAdr>
				</Cdtr>
				<CdtrAcct>
					<Id>
						<IBAN>FI2949630040017995</IBAN>
					</Id>
				</CdtrAcct>
				<RmtInf>
					<Strd>
						<CdtrRefInf>
							<Tp>
								<CdOrPrtry>
									<Cd>SCOR</Cd>
								</CdOrPrtry>
							</Tp>
							<Ref>00000000000000260808</Ref>
						</CdtrRefInf>
					</Strd>
				</RmtInf>
			</CdtTrfTxInf>
		</PmtInf>
	</CstmrCdtTrfInitn>
</Document>


*/
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client_simple.hpp>
#include <iostream>
#include <map>


#include "version_num.h"
#include "build_defs.h"
#include <unistd.h> 

#include "util.h"
#include <rapidxml.hpp>
#include <rapidxpath.hpp>
#include <xml2json.hpp>
#include <rapidjson/error/en.h>
#include <filesystem>

#include "logger.h"

#include "config_profile.h"
#include "webservicesapi.h"

INITIALIZE_EASYLOGGINGPP

using namespace std;
void exampleConfig() {
    std::string ex_conf = R"({
    "profile": {
        "name": "name of profile",
        "bank_iban": "FI9664633310000606",
        "conn_name": "name of connection",
        "csr_cn": "subject name for csr generation, ex /C=FI/O=Aineistopalvelut-Saastopankki/CN=YRITYS OY/SN=522876918",
        "pincode": "pin code for generating new cert",
        "customer_id": "customer id from webservices contract (same as SN) ex 522876918",
        "bank": "aktia",
        "environment": "PRODUCTION",
        "download_file_types": [
            {"type": "RA", "status": "ALL"},
            {"type": "OP", "status": "ALL"},
            {"type": "XP", "status": "ALL"}
        ],
        "upload_file_types": [
            {"type": "XL"}
        ],
        "output_folder": "/tmp/ws_sepa/output",
        "signing_private_key": "pem formatted private key (will be generated) ",
        "own_signing_certificate": "pem formatted own signing certificate (bank will generate) ",
        "bank_encryption_certificate": "pem formatted bank ca cert -----BEGIN ",
    }
}        
)";
    std::cout << "EXAMPLE CONFIG:\n" << ex_conf << std::endl;
    return;
}

void doHelp(const char *app, const char* ver, bool longver = true){
   
    if(longver == false) {
        fprintf(stderr, "\nuse -h for help\n");
        return;

    }
    fprintf(stderr,
            "Usage: %s [OPTION]...\n\n"
            "This tool will connect to a bank through the webservices channel.\n"
            "If your own certificate does not exists, a request is automatically sent to the bank to generate one.\n"
            "If your own certificate is about to expire, a request is automatically sent to renew it.\n\n"
            "This tool will:\n"
            "1) Upload all files from the \"outgoing\" directory (upload_file_types setting).\n"
            "After uploading the file it will be deleted from the \"outgoing\" directory.\n\n"
            "2) Download all files specified in the download_file_types setting\nand save them into the \"incoming\" directory\n"
            "\nNOTE: banks usually charge for each query\n\n"
            "Example: %s -h\n"
            "\n"
             " -c <filename> Config file\n"
             " -u            Upload files (optional)\n"
             " -d            Download files (optional)\n"
             " -e            Show example config file (optional)\n"
             " -h            Print out this help (optional)\n"
             "\n\n"
            "%s version %s\n",
            app,
            app,
            app, ver);
}
std::string buildTime = formattedString(
    "%c%c.%c%c.%c%c%c%c-%c%c:%c%c:%c%c", 
    BUILD_DAY_CH0,  BUILD_DAY_CH1,
    BUILD_MONTH_CH0, BUILD_MONTH_CH1,
    BUILD_YEAR_CH0, BUILD_YEAR_CH1, BUILD_YEAR_CH2, BUILD_YEAR_CH3,
    BUILD_HOUR_CH0, BUILD_HOUR_CH1,
    BUILD_MIN_CH0,  BUILD_MIN_CH1,
        BUILD_SEC_CH0, BUILD_SEC_CH1
    );
std::string versionNumber= formattedString("%c.%c-%s%s", 
    VERSION_MAJOR_INIT, 
    VERSION_MINOR_INIT, 
    buildTime.c_str(),
    #ifdef _DEBUG
        "d"
    #else
        "r"
    #endif
);
std::string serverName = "ws_sepa";
#define DOWNLOAD_FILES  1

#if 0 //2025-09-02T16:02:35Z
#include <iomanip>
std::string currentIsoDateTime(int tofuture = 0) {
    time_t now = time(nullptr);
    if (tofuture > 0) {
        now += tofuture * 60; // tofuture is in minutes
    }
    std::tm tm_utc;
#if defined(_WIN32) || defined(_WIN64)
    gmtime_s(&tm_utc, &now);
#else
    gmtime_r(&now, &tm_utc);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_utc, "%Y-%m-%dT%H:%M:%S") << "Z";
    return oss.str();
}
#endif
int main(int argc, char *argv[]) {

    //LOG(DEBUG) << "Log started " << currentIsoDateTime();
    std::string configFile = "ws_sepa.conf";
    int c = 0;
    bool uploadFiles = false;
    bool downloadFiles = false;
    while ((c = getopt (argc, argv, "c:hude")) != -1) {
        switch (c){
            case 'e':
                exampleConfig();
                return -1;
                break;
            case 'u':
                uploadFiles = true;
                break;
            case 'd':
                downloadFiles = true;
                break;
            case 'c':
                configFile = optarg;
                break;
            case 'h':
                doHelp(serverName.c_str(), versionNumber.c_str());
                return -1;
            continue;
        
        }
    }   
    if (optind != argc)    {
        LOG(ERROR) << "A non option was supplied";
        doHelp(serverName.c_str(), versionNumber.c_str());
        return -1;
    }  
    if(configFile == "") {
        LOG(ERROR) << "Config file not specified";
        doHelp(serverName.c_str(), versionNumber.c_str(), false);
        return -1;  
    }
    ConfigProfile configProfile(configFile);
    if(!configProfile.load()) {
        LOG(ERROR) << "Config file not specified or is empty";
        doHelp(serverName.c_str(), versionNumber.c_str(), false);
        return -1;
    }
    if(configProfile.getOutputFolder()=="") {
        LOG(ERROR) << "Output folder not specified";
        doHelp(serverName.c_str(), versionNumber.c_str(), false);
        return -1;
    }
    LOG(INFO) << "Starting " << serverName << " version " << versionNumber; 
    
    
    //mkdir(configProfile.getOutputFolder().c_str(), 0755);
    mkdir(std::string(configProfile.getOutputFolder()+"/incoming").c_str(), 0755);
    mkdir(std::string(configProfile.getOutputFolder()+"/outgoing").c_str(), 0755);

    WebServicesAPI wsApi(configProfile);

    //bank encryption cert (or CA) is required 
    if(configProfile.getBankEncryptionCertificate().empty()) {
        LOG(ERROR) << "Bank encryption certificate does not exist, download a new one from https://samlink.fi/software-services/";
        return -1;
    } 
    //and the bank encryption has to be valid
    if(!wsApi.PKI_certificateValid(configProfile.getBankEncryptionCertificate())) {
        LOG(ERROR) << "Bank encryption certificate is not valid, download a new one from https://samlink.fi/software-services/";
        return -1;
    }
    //issue error if the bank encryption certificate is about to expire in less than 60 days
    if(wsApi.PKI_certificateValidTimstamp(configProfile.getBankEncryptionCertificate()) < time(nullptr) + 60*24*60*60) {
        LOG(ERROR) << "BankEncryptionCertificate is about to expire";
    }
    if(configProfile.getOwnSigningCertificate().empty()) {
        //if we donät have a signing certificate we need to 
        //ask the bank to create one for us
        if(configProfile.getPincode().empty() ){
            //to create one we need a pin code
            LOG(ERROR) << "OwnSigningCertificate is missing and Pincode not found in profile";
            return -1;
        }
        //first we need a CSR
        if(!wsApi.PKI_generateCSR(configProfile.getCsrCn())){
            LOG(ERROR) << "Failed to generate CSR";
            return -1;
        }
        //then we can ask the bank to create the certificate
        //if this is successfull a new config file will be saved (old is overwritten)
        //the new certificate is stored in the config file
        if(!wsApi.PKI_getNewCertificate()){
            LOG(ERROR) << "Failed to get new certificate";
            return -1;
        }
    }   
    else if(wsApi.PKI_certificateValidTimstamp(configProfile.getOwnSigningCertificate()) < time(nullptr) + 30*24*60*60){
        //if the certificate expires in less than 30 days
        //we can ask the bank to renew the certificate
        //if this is successfull a new config file will be saved (old is overwritten)
        //the new certificate is stored in the config file
        if(!wsApi.PKI_renewCertificate()){
            LOG(ERROR) << "Failed to renew certificate";
            return -1;
        }
    }
    //double check that my own signing certificate is valid
    if(!wsApi.PKI_certificateValid(configProfile.getOwnSigningCertificate())) {
        LOG(ERROR) << "Own signing certificate is not valid";
        return -1;
    }
    if(downloadFiles == false && uploadFiles == false) {
        LOG(ERROR) << "No operation performed, you should either specify -u (upload) or -d (download) or both";
        return -1;
    }
    //download all files
    //RA = Tilitapahtumakysely (nouto) is available contains last 100 ? normally this
    //     will not be downloaded I guess. Will always download all.
    //XT = XML-tiliote (nouto)  seems to have all, but contains only latest info, once downloaded you loose it
    if(downloadFiles) {
        LOG(INFO) << "Downloading files";
        int fetchCount = 0;
        for (const auto& fileType : configProfile.getDownloadFileTypes()) {
            if (!wsApi.FS_downloadFiles(fileType.first, fileType.second, fetchCount)) {
                LOG(ERROR) << "Failed to download files of type " << fileType.first;
            }
        }
        LOG(INFO) << "Downloaded " << fetchCount << " files...";
    }
    //upload all files
    if(uploadFiles) {
        LOG(INFO) << "Uploading files";
        int uploadCount = 0;
        std::vector<std::string> filesLeft = wsApi.FS_uploadFiles(uploadCount);
        if (filesLeft.size() > 0) {
            for(int i=0; i<filesLeft.size(); i++) {
                std::string filename = filesLeft[i];
                LOG(ERROR) << "Failed to upload file (type not recognized or failed): " << filename;
                //move this file to the failed directory
                std::string failed_dir = configProfile.getOutputFolder() + "/failed";
                std::filesystem::create_directories(failed_dir);
                std::time_t t = std::time(nullptr);
                char timebuf[32];
                std::strftime(timebuf, sizeof(timebuf), "%Y%m%d%H%M%S", std::localtime(&t));
                std::string newfilename = filename + "_" + timebuf;

                std::filesystem::rename(configProfile.getOutputFolder() + "/outgoing/" + filename, failed_dir + "/" + newfilename);
            }
        }
        LOG(INFO) << "Uploaded " << uploadCount << " files...";
    }


    return 0;
}
