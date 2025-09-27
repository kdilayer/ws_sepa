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

static std::string PAIN_PMT_WITH_REF_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<Document xsi:schemaLocation="urn:iso:std:iso:20022:tech:xsd:pain.001.001.03 pain.001.001.03.xsd" xmlns="urn:iso:std:iso:20022:tech:xsd:pain.001.001.03" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
    <CstmrCdtTrfInitn>
        <GrpHdr>
            <MsgId>20200310-MSGID3</MsgId>
            <CreDtTm>2020-03-10T09:30:47Z</CreDtTm>
            <NbOfTxs>1</NbOfTxs>
            <InitgPty>
                <Nm>Maksaja Oy</Nm>
            </InitgPty>
        </GrpHdr>
        <PmtInf>
            <PmtInfId>20200310-PMTINF3</PmtInfId>
            <PmtMtd>TRF</PmtMtd>
            <BtchBookg>true</BtchBookg>
            <PmtTpInf>
                <SvcLvl>
                    <Cd>SEPA</Cd>
                </SvcLvl>
            </PmtTpInf>
            <ReqdExctnDt>2020-10-03</ReqdExctnDt>
            <Dbtr>
                <Nm>Maksaja Oy</Nm>
                <Id>
                    <OrgId>
                        <Othr>
                            <Id>XXXXXX-2</Id>
                            <Issr>YTJ</Issr>
                        </Othr>
                    </OrgId>
                </Id>
            </Dbtr>
            <DbtrAcct>
                <Id>
                    <IBAN>FIXXXXXXXXXX</IBAN>
                </Id>
                <Ccy>EUR</Ccy>
            </DbtrAcct>
            <DbtrAgt>
                <FinInstnId>
                    <BIC>BICBICBIC</BIC>
                </FinInstnId>
            </DbtrAgt>
            <ChrgBr>SLEV</ChrgBr>
            <CdtTrfTxInf>
                <PmtId>
                    <InstrId>Tapahtuman yksilöintitieto1</InstrId>
                    <EndToEndId>20200310-EndToEnd4</EndToEndId>
                </PmtId>
                <Amt>
                    <InstdAmt Ccy="EUR">0.75</InstdAmt>
                </Amt>
                <CdtrAgt>
                    <FinInstnId>
                        <BIC>ITELFIHH</BIC>
                    </FinInstnId>
                </CdtrAgt>
                <Cdtr>
                    <Nm>Saaja Oy</Nm>
                    <PstlAdr>
                        <Ctry>FI</Ctry>
                        <AdrLine></AdrLine>
                    </PstlAdr>
                </Cdtr>
                <CdtrAcct>
                    <Id>
                        <IBAN>FIXXXXXXXX</IBAN>
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
                            <Ref>123453</Ref>
                        </CdtrRefInf>
                    </Strd>
                </RmtInf>
            </CdtTrfTxInf>
        </PmtInf>
    </CstmrCdtTrfInitn>
</Document>)";

static std::string Constants_APPREQ_ENCRYPTED_REQUEST_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<xenc:EncryptedData Type="http://www.w3.org/2001/04/xmlenc#Element" xmlns:dsig="http://www.w3.org/2000/09/xmldsig#" xmlns:xenc="http://www.w3.org/2001/04/xmlenc#">
  <xenc:EncryptionMethod Algorithm="http://www.w3.org/2001/04/xmlenc#tripledes-cbc"/>
  <dsig:KeyInfo>
    <xenc:EncryptedKey Recipient="name:DanskeBankCryptCERT">
      <xenc:EncryptionMethod Algorithm="http://www.w3.org/2001/04/xmlenc#rsa-1_5"/>
      <dsig:KeyInfo>
        <dsig:X509Data>
          <dsig:X509Certificate></dsig:X509Certificate>
        </dsig:X509Data>
      </dsig:KeyInfo>
      <xenc:CipherData>
        <xenc:CipherValue></xenc:CipherValue>
      </xenc:CipherData>
    </xenc:EncryptedKey>
  </dsig:KeyInfo>
  <xenc:CipherData>
    <xenc:CipherValue></xenc:CipherValue>
  </xenc:CipherData>
</xenc:EncryptedData>
)";
static std::string Constants_SOAP_DOWNLOAD_FILE_LIST_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<env:Envelope xmlns:env="http://schemas.xmlsoap.org/soap/envelope/" xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd" xmlns:cor="http://bxd.fi/CorporateFileService" xmlns:bxd="http://model.bxd.fi">
  <env:Body xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd" wsu:Id="">
    <cor:downloadFileListin xmlns:cor="http://bxd.fi/CorporateFileService">
      <bxd:RequestHeader xmlns:bxd="http://model.bxd.fi">
        <bxd:SenderId></bxd:SenderId>
        <bxd:RequestId></bxd:RequestId>
        <bxd:Timestamp></bxd:Timestamp>
        <bxd:Language></bxd:Language>
        <bxd:UserAgent></bxd:UserAgent>
        <bxd:ReceiverId></bxd:ReceiverId>
      </bxd:RequestHeader>
      <bxd:ApplicationRequest xmlns:bxd="http://model.bxd.fi"></bxd:ApplicationRequest>
    </cor:downloadFileListin>
  </env:Body>
</env:Envelope>
)";
static std::string Constants_SOAP_GET_USER_INFO_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<env:Envelope xmlns:env="http://schemas.xmlsoap.org/soap/envelope/"
              xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd"
              xmlns:cor="http://bxd.fi/CorporateFileService" xmlns:bxd="http://model.bxd.fi">
  <env:Body
          xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd"
          wsu:Id="sdf6sa7d86f87s6df786sd87f6s8fsda">
    <cor:getUserInfoin>
      <bxd:RequestHeader>
        <bxd:SenderId></bxd:SenderId>
        <bxd:RequestId></bxd:RequestId>
        <bxd:Timestamp></bxd:Timestamp>
        <bxd:Language></bxd:Language>
        <bxd:UserAgent></bxd:UserAgent>
        <bxd:ReceiverId></bxd:ReceiverId>
      </bxd:RequestHeader>
      <bxd:ApplicationRequest></bxd:ApplicationRequest>
    </cor:getUserInfoin>
  </env:Body>
</env:Envelope>
)";
static std::string Constants_SOAP_UPLOAD_FILE_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<env:Envelope xmlns:env="http://schemas.xmlsoap.org/soap/envelope/" xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd" xmlns:cor="http://bxd.fi/CorporateFileService" xmlns:bxd="http://model.bxd.fi">
  <env:Body xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd" wsu:Id="sdf6sa7d86f87s6df786sd87f6s8fsda">
    <cor:uploadFilein xmlns:cor="http://bxd.fi/CorporateFileService">
      <bxd:RequestHeader xmlns:bxd="http://model.bxd.fi">
        <bxd:SenderId></bxd:SenderId>
        <bxd:RequestId></bxd:RequestId>
        <bxd:Timestamp></bxd:Timestamp>
        <bxd:Language></bxd:Language>
        <bxd:UserAgent></bxd:UserAgent>
        <bxd:ReceiverId></bxd:ReceiverId>
      </bxd:RequestHeader>
      <bxd:ApplicationRequest xmlns:bxd="http://model.bxd.fi"></bxd:ApplicationRequest>
    </cor:uploadFilein>
  </env:Body>
</env:Envelope>
)";
static std::string Constants_SOAP_DOWNLOAD_FILE_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<env:Envelope xmlns:env="http://schemas.xmlsoap.org/soap/envelope/"
              xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd"
              xmlns:cor="http://bxd.fi/CorporateFileService" xmlns:bxd="http://model.bxd.fi">
  <env:Body
          xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd"
          wsu:Id="sdf6sa7d86f87s6df786sd87f6s8fsda">
    <cor:downloadFilein xmlns:cor="http://bxd.fi/CorporateFileService">
      <bxd:RequestHeader xmlns:bxd="http://model.bxd.fi">
        <bxd:SenderId></bxd:SenderId>
        <bxd:RequestId></bxd:RequestId>
        <bxd:Timestamp></bxd:Timestamp>
        <bxd:Language></bxd:Language>
        <bxd:UserAgent></bxd:UserAgent>
        <bxd:ReceiverId></bxd:ReceiverId>
      </bxd:RequestHeader>
      <bxd:ApplicationRequest xmlns:bxd="http://model.bxd.fi"></bxd:ApplicationRequest>
    </cor:downloadFilein>
  </env:Body>
</env:Envelope>
)";
static std::string Constants_SOAP_GET_CERTIFICATE_XML=R"(<?xml version="1.0"?>
<soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/" xmlns:cer="http://bxd.fi/CertificateService">
<soapenv:Header/>
<soapenv:Body>
<cer:getCertificatein>
<cer:RequestHeader>
<cer:SenderId></cer:SenderId>
<cer:RequestId></cer:RequestId>
<cer:Timestamp></cer:Timestamp>
</cer:RequestHeader>
<cer:ApplicationRequest></cer:ApplicationRequest>
</cer:getCertificatein>
</soapenv:Body>
</soapenv:Envelope>
)";

static std::string Constants_AKTIA_SOAP_CREATE_CERTIFICATE_XML=R"(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<env:Envelope xmlns:env="http://schemas.xmlsoap.org/soap/envelope/" xmlns:opc="http://mlp.op.fi/OPCertificateService">
	<env:Header></env:Header>
	<env:Body>
		<opc:getCertificatein>
			<opc:RequestHeader>
				<opc:SenderId>98343805</opc:SenderId>
				<opc:RequestId>0000002</opc:RequestId>
				<opc:Timestamp>2014-03-19T07:44:16.914+02:00</opc:Timestamp>
			</opc:RequestHeader>
			<opc:ApplicationRequest></opc:ApplicationRequest>
		</opc:getCertificatein>
	</env:Body>
</env:Envelope>
)";
static std::string Constants_SOAP_RENEW_CERTIFICATE_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<env:Envelope xmlns:pkif="http://danskebank.dk/PKI/PKIFactoryService">
  <env:Header/>
  <env:Body>
    <pkif:RenewCertificateIn>
      <pkif:RequestHeader>
        <pkif:SenderId></pkif:SenderId>
        <pkif:CustomerId></pkif:CustomerId>
        <pkif:RequestId></pkif:RequestId>
        <pkif:Timestamp></pkif:Timestamp>
        <pkif:InterfaceVersion>1</pkif:InterfaceVersion>
        <pkif:Environment></pkif:Environment>
      </pkif:RequestHeader>
    </pkif:RenewCertificateIn>
  </env:Body>
</env:Envelope>
)";
static std::string Constants_SOAP_DANSKE_GET_BANK_CERTIFICATE_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/"
                  xmlns:pkif="http://danskebank.dk/PKI/PKIFactoryService">
  <soapenv:Header/>
  <soapenv:Body>
    <pkif:GetBankCertificateIn xmlns:pkif="http://danskebank.dk/PKI/PKIFactoryService">
      <pkif:RequestHeader xmlns:pkif="http://danskebank.dk/PKI/PKIFactoryService">
        <pkif:SenderId></pkif:SenderId>
        <pkif:CustomerId></pkif:CustomerId>
        <pkif:RequestId></pkif:RequestId>
        <pkif:Timestamp></pkif:Timestamp>
        <pkif:InterfaceVersion></pkif:InterfaceVersion>
      </pkif:RequestHeader>
    </pkif:GetBankCertificateIn>
  </soapenv:Body>
</soapenv:Envelope>
)";

static std::string Constants_SOAP_CREATE_CERTIFICATE_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/"
                  xmlns:pkif="http://danskebank.dk/PKI/PKIFactoryService">
  <soapenv:Header/>
  <soapenv:Body>
    <pkif:CreateCertificateIn>
      <pkif:RequestHeader>
        <pkif:SenderId></pkif:SenderId>
        <pkif:CustomerId></pkif:CustomerId>
        <pkif:RequestId></pkif:RequestId>
        <pkif:Timestamp></pkif:Timestamp>
        <pkif:InterfaceVersion></pkif:InterfaceVersion>
        <pkif:Environment></pkif:Environment>
      </pkif:RequestHeader>
    </pkif:CreateCertificateIn>
  </soapenv:Body>
</soapenv:Envelope>
)";

static std::string Constants_AKTIA_GET_CERT_SOAP_HEADER_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<soapenv:Envelope xmlns:opc="http://mlp.op.fi/OPCertificateService" xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/">
  <soapenv:Header>
  </soapenv:Header>
</soapenv:Envelope>)";
static std::string Constants_SOAP_HEADER_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<env:Envelope xmlns:env="http://schemas.xmlsoap.org/soap/envelope/" xmlns:wsse="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd" xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd" xmlns:dsig="http://www.w3.org/2000/09/xmldsig#">
  <env:Header>
    <wsse:Security xmlns:env="http://schemas.xmlsoap.org/soap/envelope/" env:mustUnderstand="1" xmlns:wsse="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd">
      <wsse:BinarySecurityToken xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd" EncodingType="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary" ValueType="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-x509-token-profile-1.0#X509v3" wsu:Id=""></wsse:BinarySecurityToken>
      <dsig:Signature xmlns:dsig="http://www.w3.org/2000/09/xmldsig#">
        <dsig:SignedInfo>
          <dsig:CanonicalizationMethod Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#"/>
          <dsig:SignatureMethod Algorithm="http://www.w3.org/2000/09/xmldsig#rsa-sha1"/>
          <dsig:Reference URI="">
            <dsig:Transforms>
              <dsig:Transform Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#"/>
            </dsig:Transforms>
            <dsig:DigestMethod Algorithm="http://www.w3.org/2000/09/xmldsig#sha1"/>
            <dsig:DigestValue></dsig:DigestValue>
          </dsig:Reference>
          <dsig:Reference URI="">
            <dsig:Transforms>
              <dsig:Transform Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#"/>
            </dsig:Transforms>
            <dsig:DigestMethod Algorithm="http://www.w3.org/2000/09/xmldsig#sha1"/>
            <dsig:DigestValue></dsig:DigestValue>
          </dsig:Reference>
        </dsig:SignedInfo>
        <dsig:SignatureValue></dsig:SignatureValue>
        <dsig:KeyInfo>
          <wsse:SecurityTokenReference>
            <wsse:Reference URI="" ValueType="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-x509-token-profile-1.0#X509v3"/>
          </wsse:SecurityTokenReference>
        </dsig:KeyInfo>
      </dsig:Signature>
      <wsu:Timestamp xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd" wsu:Id="">
        <wsu:Created></wsu:Created>
        <wsu:Expires></wsu:Expires>
      </wsu:Timestamp>
    </wsse:Security>
  </env:Header>
</env:Envelope>
)";
static std::string Constants_APPREQ_DOWNLOAD_FILE_LIST_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<ApplicationRequest xmlns="http://bxd.fi/xmldata/">
  <CustomerId></CustomerId>
  <Command></Command>
  <Timestamp></Timestamp>
  <Status></Status>
  <Environment></Environment>
  <SoftwareId></SoftwareId>
  <FileType></FileType>
  <dsig:Signature xmlns:dsig="http://www.w3.org/2000/09/xmldsig#">
    <dsig:SignedInfo>
      <dsig:CanonicalizationMethod Algorithm="http://www.w3.org/TR/2001/REC-xml-c14n-20010315#WithComments"/>
      <dsig:SignatureMethod Algorithm="http://www.w3.org/2000/09/xmldsig#rsa-sha1"/>
      <dsig:Reference URI="">
        <dsig:Transforms>
          <dsig:Transform Algorithm="http://www.w3.org/2000/09/xmldsig#enveloped-signature"/>
        </dsig:Transforms>
        <dsig:DigestMethod Algorithm="http://www.w3.org/2000/09/xmldsig#sha1"/>
        <dsig:DigestValue>
        </dsig:DigestValue>
      </dsig:Reference>
    </dsig:SignedInfo>
    <dsig:SignatureValue>
    </dsig:SignatureValue>
    <dsig:KeyInfo>
      <dsig:X509Data>
        <dsig:X509Certificate></dsig:X509Certificate>
      </dsig:X509Data>
    </dsig:KeyInfo>
  </dsig:Signature>
</ApplicationRequest>
)";
static std::string Constants_APPREQ_GET_USER_INFO_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<ApplicationRequest xmlns="http://bxd.fi/xmldata/">
  <CustomerId></CustomerId>
  <Command></Command>
  <Timestamp></Timestamp>
  <Environment></Environment>
  <SoftwareId></SoftwareId>
  <dsig:Signature xmlns:dsig="http://www.w3.org/2000/09/xmldsig#">
    <dsig:SignedInfo>
      <dsig:CanonicalizationMethod
              Algorithm="http://www.w3.org/TR/2001/REC-xml-c14n-20010315#WithComments"/>
      <dsig:SignatureMethod Algorithm="http://www.w3.org/2000/09/xmldsig#rsa-sha1"/>
      <dsig:Reference URI="">
        <dsig:Transforms>
          <dsig:Transform Algorithm="http://www.w3.org/2000/09/xmldsig#enveloped-signature"/>
        </dsig:Transforms>
        <dsig:DigestMethod Algorithm="http://www.w3.org/2000/09/xmldsig#sha1"/>
        <dsig:DigestValue></dsig:DigestValue>
      </dsig:Reference>
    </dsig:SignedInfo>
    <dsig:SignatureValue></dsig:SignatureValue>
    <dsig:KeyInfo>
      <dsig:X509Data>
        <dsig:X509Certificate></dsig:X509Certificate>
      </dsig:X509Data>
    </dsig:KeyInfo>
  </dsig:Signature>
</ApplicationRequest>
)";
static std::string Constants_AKTIA_APPREQ_UPLOAD_FILE_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<ApplicationRequest xmlns="http://bxd.fi/xmldata/">
  <CustomerId></CustomerId>
  <Command></Command>
  <Timestamp></Timestamp>
  <Environment></Environment>
  <TargetId>NONE</TargetId>
  <Compresssion>false</Compresssion>
  <SoftwareId></SoftwareId>
  <FileType></FileType>
  <Content></Content>
  <dsig:Signature xmlns:dsig="http://www.w3.org/2000/09/xmldsig#">
    <dsig:SignedInfo>
      <dsig:CanonicalizationMethod Algorithm="http://www.w3.org/TR/2001/REC-xml-c14n-20010315#WithComments"/>
      <dsig:SignatureMethod Algorithm="http://www.w3.org/2000/09/xmldsig#rsa-sha1"/>
      <dsig:Reference URI="">
        <dsig:Transforms>
          <dsig:Transform Algorithm="http://www.w3.org/2000/09/xmldsig#enveloped-signature"/>
        </dsig:Transforms>
        <dsig:DigestMethod Algorithm="http://www.w3.org/2000/09/xmldsig#sha1"/>
        <dsig:DigestValue></dsig:DigestValue>
      </dsig:Reference>
    </dsig:SignedInfo>
    <dsig:SignatureValue></dsig:SignatureValue>
    <dsig:KeyInfo>
      <dsig:X509Data>
        <dsig:X509Certificate></dsig:X509Certificate>
      </dsig:X509Data>
    </dsig:KeyInfo>
  </dsig:Signature>
</ApplicationRequest>
)";
static std::string Constants_APPREQ_UPLOAD_FILE_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<ApplicationRequest xmlns="http://bxd.fi/xmldata/">
  <CustomerId></CustomerId>
  <Command></Command>
  <Timestamp></Timestamp>
  <Environment></Environment>
  <SoftwareId></SoftwareId>
  <FileType></FileType>
  <Content></Content>
  <dsig:Signature xmlns:dsig="http://www.w3.org/2000/09/xmldsig#">
    <dsig:SignedInfo>
      <dsig:CanonicalizationMethod Algorithm="http://www.w3.org/TR/2001/REC-xml-c14n-20010315#WithComments"/>
      <dsig:SignatureMethod Algorithm="http://www.w3.org/2000/09/xmldsig#rsa-sha1"/>
      <dsig:Reference URI="">
        <dsig:Transforms>
          <dsig:Transform Algorithm="http://www.w3.org/2000/09/xmldsig#enveloped-signature"/>
        </dsig:Transforms>
        <dsig:DigestMethod Algorithm="http://www.w3.org/2000/09/xmldsig#sha1"/>
        <dsig:DigestValue></dsig:DigestValue>
      </dsig:Reference>
    </dsig:SignedInfo>
    <dsig:SignatureValue></dsig:SignatureValue>
    <dsig:KeyInfo>
      <dsig:X509Data>
        <dsig:X509Certificate></dsig:X509Certificate>
      </dsig:X509Data>
    </dsig:KeyInfo>
  </dsig:Signature>
</ApplicationRequest>
)";
static std::string Constants_APPREQ_DOWNLOAD_FILE_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<ApplicationRequest xmlns="http://bxd.fi/xmldata/">
  <CustomerId></CustomerId>
  <Command></Command>
  <Timestamp></Timestamp>
  <Status></Status>
  <Environment></Environment>
  <FileReferences>
    <FileReference></FileReference>
  </FileReferences>
  <SoftwareId></SoftwareId>
  <FileType></FileType>
  <dsig:Signature xmlns:dsig="http://www.w3.org/2000/09/xmldsig#">
    <dsig:SignedInfo>
      <dsig:CanonicalizationMethod Algorithm="http://www.w3.org/TR/2001/REC-xml-c14n-20010315#WithComments"/>
      <dsig:SignatureMethod Algorithm="http://www.w3.org/2000/09/xmldsig#rsa-sha1"/>
      <dsig:Reference URI="">
        <dsig:Transforms>
          <dsig:Transform Algorithm="http://www.w3.org/2000/09/xmldsig#enveloped-signature"/>
        </dsig:Transforms>
        <dsig:DigestMethod Algorithm="http://www.w3.org/2000/09/xmldsig#sha1"/>
        <dsig:DigestValue></dsig:DigestValue>
      </dsig:Reference>
    </dsig:SignedInfo>
    <dsig:SignatureValue></dsig:SignatureValue>
    <dsig:KeyInfo>
      <dsig:X509Data>
        <dsig:X509Certificate></dsig:X509Certificate>
      </dsig:X509Data>
    </dsig:KeyInfo>
  </dsig:Signature>
</ApplicationRequest>
)";
static std::string Constants_APPREQ_GET_CERTIFICATE_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<CertApplicationRequest xmlns="http://filetransfer.nordea.com/xmldata/">
  <CustomerId></CustomerId>
  <Timestamp></Timestamp>
  <Environment></Environment>
  <SoftwareId></SoftwareId>
  <Command></Command>
  <Service></Service>
  <Content></Content>
  <HMAC></HMAC>
</CertApplicationRequest>
)";
static std::string Constants_APPREQ_DANSKE_GET_BANK_CERTIFICATE_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/"
                  xmlns:pkif="http://danskebank.dk/PKI/PKIFactoryService"
                  xmlns:elem="http://danskebank.dk/PKI/PKIFactoryService/elements">
  <soapenv:Header/>
  <soapenv:Body>
    <elem:GetBankCertificateRequest
            xmlns:elem="http://danskebank.dk/PKI/PKIFactoryService/elements">
      <elem:BankRootCertificateSerialNo></elem:BankRootCertificateSerialNo>
      <elem:Timestamp></elem:Timestamp>
      <elem:RequestId></elem:RequestId>
    </elem:GetBankCertificateRequest>
  </soapenv:Body>
</soapenv:Envelope>
)";
static std::string Constants_APPREQ_DANSKE_RENEW_CERTIFICATE_XML=R"(<?xml version="1.0"?>
<tns:RenewCertificateRequest xmlns:tns="http://danskebank.dk/PKI/PKIFactoryService/elements">
  <tns:CustomerId></tns:CustomerId>
  <tns:KeyGeneratorType>software</tns:KeyGeneratorType>
  <tns:EncryptionCertPKCS10></tns:EncryptionCertPKCS10>
  <tns:SigningCertPKCS10></tns:SigningCertPKCS10>
  <tns:Timestamp></tns:Timestamp>
  <tns:RequestId/>
  <tns:Environment></tns:Environment>
  <Signature xmlns="http://www.w3.org/2000/09/xmldsig#">
    <SignedInfo>
      <CanonicalizationMethod Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#"/>
      <SignatureMethod Algorithm="http://www.w3.org/2000/09/xmldsig#rsa-sha1"/>
      <Reference URI="">
        <Transforms>
          <Transform Algorithm="http://www.w3.org/2000/09/xmldsig#enveloped-signature"/>
          <Transform Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#"/>
        </Transforms>
        <DigestMethod Algorithm="http://www.w3.org/2000/09/xmldsig#sha1"/>
        <DigestValue/>
      </Reference>
    </SignedInfo>
    <SignatureValue/>
    <KeyInfo>
      <X509Data>
        <X509Certificate/>
      </X509Data>
    </KeyInfo>
  </Signature>
</tns:RenewCertificateRequest>
)";
static std::string Constants_AKTIA_APPREQ_CREATE_CERTIFICATE_XML=R"(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<CertApplicationRequest xmlns="http://op.fi/mlp/xmldata/">
  <CustomerId></CustomerId>
  <Timestamp></Timestamp>
  <Environment></Environment>
  <SoftwareId></SoftwareId>
  <Compression>false</Compression>
  <RequestId></RequestId>
  <Command></Command>
  <Service></Service>
  <Content></Content>
  <TransferKey></TransferKey>
  <dsig:Signature xmlns="http://op.fi/mlp/xmldata/" xmlns:dsig="http://www.w3.org/2000/09/xmldsig#">
    <dsig:SignedInfo>
      <dsig:CanonicalizationMethod Algorithm="http://www.w3.org/TR/2001/REC-xml-c14n-20010315#WithComments"/>
      <dsig:SignatureMethod Algorithm="http://www.w3.org/2000/09/xmldsig#rsa-sha1"/>
      <dsig:Reference URI="">
        <dsig:Transforms>
          <dsig:Transform Algorithm="http://www.w3.org/2000/09/xmldsig#enveloped-signature"/>
        </dsig:Transforms>
        <dsig:DigestMethod Algorithm="http://www.w3.org/2000/09/xmldsig#sha1"/>
        <dsig:DigestValue></dsig:DigestValue>
      </dsig:Reference>
    </dsig:SignedInfo>
    <dsig:SignatureValue></dsig:SignatureValue>
    <dsig:KeyInfo>
      <dsig:X509Data>
        <dsig:X509Certificate></dsig:X509Certificate>
      </dsig:X509Data>
    </dsig:KeyInfo>
  </dsig:Signature></CertApplicationRequest>)";
static std::string Constants_APPREQ_CREATE_CERTIFICATE_XML=R"(<?xml version="1.0" encoding="UTF-8"?>
<tns:CreateCertificateRequest xmlns:xe="http://www.w3.org/2001/04/xmlenc#"
                              xmlns:xd="http://www.w3.org/2000/09/xmldsig#"
                              xmlns:tns="http://danskebank.dk/PKI/PKIFactoryService/elements"
                              xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/"
                              xmlns:pkif="http://danskebank.dk/PKI/PKIFactoryService">
  <tns:CustomerId></tns:CustomerId>
  <tns:KeyGeneratorType></tns:KeyGeneratorType>
  <tns:EncryptionCertPKCS10></tns:EncryptionCertPKCS10>
  <tns:SigningCertPKCS10></tns:SigningCertPKCS10>
  <tns:Timestamp></tns:Timestamp>
  <tns:RequestId></tns:RequestId>
  <tns:Environment></tns:Environment>
  <tns:PIN></tns:PIN>
</tns:CreateCertificateRequest>
)";
