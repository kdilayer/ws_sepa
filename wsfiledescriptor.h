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
#ifndef WSFILEDESCRIPTOR_H
#define WSFILEDESCRIPTOR_H

#include <string>

class WSFileDescriptor {
public:
    WSFileDescriptor(const std::string& fileReference,
                     const std::string& fileType,
                     const std::string& fileTimestamp,
                     const std::string& status, 
                     const std::string& serviceId, 
                     const std::string& fileName,
                     const std::string& lastDownloadTimestamp);

    const std::string& getFileName() const;
    const std::string& getServiceId() const;
    const std::string& getFileType() const;
    const std::string& getFileTimestamp() const;
    const std::string& getStatus() const;
    const std::string& getFileReference() const;
private:
    std::string fileReference;
    std::string fileType;
    std::string fileTimestamp;
    std::string status;  //Status can be e.g. NEW (only new files, that I have not seen), ALL (all available), or DLD (Downloaded??).
    std::string serviceId;
    std::string fileName;
    std::string lastDownloadTimestamp;
};

#endif // WSFILEDESCRIPTOR_H
