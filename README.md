**ws_sepa**

A tool that integrates into banking systems via web services API. 
This tool will send and receive files from a bank using the web services API. 
For generating and processing SEPA XML files see https://github.com/kdilayer/sepa2odoo

This tool currently supports Samlink banks, small changes to be done to support OP, Nordea and DanskeBank.

Many ideas for this implementation was inspired from  https://github.com/devlab-oy/sepa

Essentially webserivces is much like ftp - you can upload / download files.

Features of this tool:
- certificate management - create cerfitifacte and renew certificate if needed
- send files to bank <pre></pre>
- receive files from bank <pre></pre>

Run this tool using:
-d download files
-u upload files
-c config file
<pre>ws_sepa -d -u -c config.conf</pre>

Working folder of the tool will contain the following subfolders
<pre>
FETCHING FROM BANK
./incoming              <- new files fetched from bank
./processed             <- after the incoming file has been processed
                           it will be stored in this folder
                           (processing is done by some other tool)
./not_processed         <- the tool that does processing did not recognize the file
                           so it has copied it to this folder

SENDING TO BANK
./outgoing              <- files stored here will be uploaded to the bank
                           naming convention: TYPE_filename.ext (where TYPE e.g. XL)
                           (if configured to send them)
./not_uploaded          <- files that have not been uploaded 
                           e.g. because type not cofigured, or bank rejected it

./uploaded              <- after successful upload file is copied here


</pre>
Configuration:
<pre>
{
    "profile": {
        "name": "XX",                               <- used for logging
        "bank_iban": "XXX",                         <- iban of your bank 
        "conn_name": "XX",                          <- name of your company
        "csr_cn": "/C=FI/O=XXX/CN=XXX/SN=XXX",      <- Cert request CN
        "pincode": "first time",                    <- pin code to generate cert (first time only)
        "customer_id": "cust id",                   <- customer_id from bank
        "bank": "aktia",                            <- 
        "environment": "PRODUCTION",                <- 
        "download_file_types": [                    <- which types of files to download (store to "incoming" folder)
            { "type": "XT",  "status": "ALL" },
            { "type": "XP",  "status": "ALL" },
            { "type": "RA",  "status": "ALL" }
        ],
        "upload_file_types": [                      <- which types of files to upload (from "outgoing" folder)
            {
                "type": "XL"
            }
        ],
        "output_folder": "/var/lib/ws_sepa",  <- the root folder of working files
        "signing_private_key": "private key", <- base 64 encoded private key
        "own_signing_certificate": "cert",      <- base 64 encoded certificate
        "bank_encryption_certificate": "bank cert", <- base 64 encoded bank certificate
        "signing_csr": "csr ",                  <- base 64 encoded csr that was used to request the cert
        "old_own_signing_certificate": "",      <- old cert if has been renewed
        "old_signing_private_key": ""          <- old private key if has been renewed
    }
}
</pre>

Internally this tool has a very simple high level API:
<pre>
WebServicesAPI::PKI_certificateValid()          <- check to see if certificate is still valid
WebServicesAPI::PKI_certificateValidTimstamp()  <- get certificate validity timestamp
WebServicesAPI::PKI_generateCSR()               <- generate cert request for new certificate
WebServicesAPI::PKI_getNewCertificate()         <- request for new certificate from bank 
WebServicesAPI::PKI_renewCertificate()          <- request for certificate renewal from bank 

WebServicesAPI::FS_uploadFiles()                <- upload files to bank
WebServicesAPI::FS_downloadFiles()              <- download files from bank
</pre>
