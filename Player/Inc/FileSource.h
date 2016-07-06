#ifndef FILE_SOURCE_H_

#define FILE_SOURCE_H_

#include <stdio.h>
#include <Threads.h>
#include "DataSource.h"
#include "SIGM_Types.h"

class FileSource : public DataSource {
public:
    FileSource(const char *filename);

    virtual Error_Type_e initCheck() const;

    virtual ssize_t readAt(off64_t offset, void *data, size_t size);

    virtual Error_Type_e getSize(off64_t *size);

	virtual String8 getUri(); 

protected:
    virtual ~FileSource();

private:
    FILE*  mFd;
    int64_t mOffset;
    int64_t mLength;
    Mutex mLock;

    FileSource(const FileSource &);
    FileSource &operator=(const FileSource &);
	String8 mUri;
};

#endif  // FILE_SOURCE_H_



