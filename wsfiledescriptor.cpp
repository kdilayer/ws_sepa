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
#include "wsfiledescriptor.h"

WSFileDescriptor::WSFileDescriptor(const std::string& fileReference,
                     const std::string& fileType,
                     const std::string& fileTimestamp,
                     const std::string& status, 
                     const std::string& serviceId, 
                     const std::string& fileName,
                     const std::string& lastDownloadTimestamp)
    : fileReference(fileReference),
      fileType(fileType),
      fileTimestamp(fileTimestamp),
      status(status),
      serviceId(serviceId),
      fileName(fileName),
      lastDownloadTimestamp(lastDownloadTimestamp) {}

const std::string& WSFileDescriptor::getFileReference() const {
    return fileReference;
}
const std::string& WSFileDescriptor::getFileName() const {
    return fileName;
}

const std::string& WSFileDescriptor::getServiceId() const {
    return serviceId;
}

const std::string& WSFileDescriptor::getFileType() const {
    return fileType;
}

const std::string& WSFileDescriptor::getFileTimestamp() const {
    return fileTimestamp;
}

const std::string& WSFileDescriptor::getStatus() const {
    return status;
}
