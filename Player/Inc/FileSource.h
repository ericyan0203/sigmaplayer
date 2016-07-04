#ifndef FILE_SOURCE_H_

#define FILE_SOURCE_H_

#include <stdio.h>

#include "DataSource.h"
#include <Threads.h>

class FileSource : public DataSource {
public:
    FileSource(const char *filename);

    virtual status_t initCheck() const;

    virtual ssize_t readAt(off64_t offset, void *data, size_t size);

    virtual status_t getSize(off64_t *size);

	virtual String8 getUri(); 

protected:
    virtual ~FileSource();

private:
    int mFd;
    int64_t mOffset;
    int64_t mLength;
    Mutex mLock;

    FileSource(const FileSource &);
    FileSource &operator=(const FileSource &);
	String8 mUri;
};

#endif  // FILE_SOURCE_H_



