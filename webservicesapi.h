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
#ifndef WEBSERVICESAPI_H
#define WEBSERVICESAPI_H

#include "config_profile.h"
#include <string>
#include <vector>
#include "wsfiledescriptor.h"

class WebServicesAPI {
public:
    explicit WebServicesAPI(const ConfigProfile& config);

    bool configIsValid(bool needcertsandkeys=true, bool needcertcn=false, bool neednewcertsandkeys=false) const;

    bool PKI_generateCSR(const std::string cn);

    bool PKI_getNewCertificate();
    bool PKI_renewCertificate();
    
    bool PKI_certificateValid(std::string input) const;
    time_t PKI_certificateValidTimstamp(std::string input) const;

    std::vector<std::string> getFilesToUpload();

    std::vector<std::string> FS_uploadFiles(int &uploadCount);
    bool FS_downloadFiles(std::string fileType, std::string fetchPolicy, int &fetchCount);

private:
    ConfigProfile configProfile;

    std::string getUploadableType(std::string fileToUpload);
    bool FS_uploadFile(const std::string& fileType, const std::string& fileName, const std::string& fileContent, std::string &errorMessage);
    bool FS_downloadFile(WSFileDescriptor &fileDescriptor);
    const std::vector<WSFileDescriptor> FS_downloadFileList(const std::string& fileType, std::string fetchPolicy="");

};

#endif // WEBSERVICESAPI_H
