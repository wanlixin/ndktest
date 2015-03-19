#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

namespace android {

typedef uint16_t char16_t;
typedef int32_t status_t;

enum {
	OK = 0, // Everything's swell.
	NO_ERROR = 0, // No errors.

	UNKNOWN_ERROR = 0x80000000,

	NO_MEMORY = -ENOMEM,
	INVALID_OPERATION = -ENOSYS,
	BAD_VALUE = -EINVAL,
	BAD_TYPE = 0x80000001,
	NAME_NOT_FOUND = -ENOENT,
	PERMISSION_DENIED = -EPERM,
	NO_INIT = -ENODEV,
	ALREADY_EXISTS = -EEXIST,
	DEAD_OBJECT = -EPIPE,
	FAILED_TRANSACTION = 0x80000002,
	JPARKS_BROKE_IT = -EPIPE,
#if !defined(HAVE_MS_C_RUNTIME)
	BAD_INDEX = -EOVERFLOW,
	NOT_ENOUGH_DATA = -ENODATA,
	WOULD_BLOCK = -EWOULDBLOCK,
	TIMED_OUT = -ETIMEDOUT,
	UNKNOWN_TRANSACTION = -EBADMSG,
#else
BAD_INDEX = -E2BIG,
NOT_ENOUGH_DATA = 0x80000003,
WOULD_BLOCK = 0x80000004,
TIMED_OUT = 0x80000005,
UNKNOWN_TRANSACTION = 0x80000006,
#endif
};

class String8;
class TextOutput;

class SharedBuffer {
public:
	static inline size_t sizeFromData(const void* data) {
		return (((const SharedBuffer*) data) - 1)->mSize;
	}
	mutable int32_t mRefs;
	size_t mSize;
	uint32_t mReserved[2];

};

class String16 {
public:
	String16();
	String16(const String16& o);
	String16(const String16& o, size_t len, size_t begin = 0);
	explicit String16(const char16_t* o);
	explicit String16(const char16_t* o, size_t len);
	explicit String16(const String8& o);
	explicit String16(const char* o);
	explicit String16(const char* o, size_t len);

	~String16();

	inline const char16_t* string() const;
	inline size_t size() const {
		return SharedBuffer::sizeFromData(mString) / sizeof(char16_t) - 1;
	}

	inline const SharedBuffer* sharedBuffer() const;

	void setTo(const String16& other);
	status_t setTo(const char16_t* other);
	status_t setTo(const char16_t* other, size_t len);
	status_t setTo(const String16& other, size_t len, size_t begin = 0);

	status_t append(const String16& other);
	status_t append(const char16_t* other, size_t len);

	inline String16& operator=(const String16& other);

	inline String16& operator+=(const String16& other);
	inline String16 operator+(const String16& other) const;

	status_t insert(size_t pos, const char16_t* chrs);
	status_t insert(size_t pos, const char16_t* chrs, size_t len);

	ssize_t findFirst(char16_t c) const;
	ssize_t findLast(char16_t c) const;

	bool startsWith(const String16& prefix) const;
	bool startsWith(const char16_t* prefix) const;

	status_t makeLower();

	status_t replaceAll(char16_t replaceThis, char16_t withThis);

	status_t remove(size_t len, size_t begin = 0);

	inline int compare(const String16& other) const;

	inline bool operator<(const String16& other) const;
	inline bool operator<=(const String16& other) const;
	inline bool operator==(const String16& other) const;
	inline bool operator!=(const String16& other) const;
	inline bool operator>=(const String16& other) const;
	inline bool operator>(const String16& other) const;

	inline bool operator<(const char16_t* other) const;
	inline bool operator<=(const char16_t* other) const;
	inline bool operator==(const char16_t* other) const;
	inline bool operator!=(const char16_t* other) const;
	inline bool operator>=(const char16_t* other) const;
	inline bool operator>(const char16_t* other) const;

	inline operator const char16_t*() const {
		return mString;
	}

private:
	const char16_t* mString;
};

template<typename T> class wp;
template<class TYPE> class Vector;

#define COMPARE_WEAK(_op_)                                      \
inline bool operator _op_ (const sp<T>& o) const {              \
    return m_ptr _op_ o.m_ptr;                                  \
}                                                               \
inline bool operator _op_ (const T* o) const {                  \
    return m_ptr _op_ o;                                        \
}                                                               \
template<typename U>                                            \
inline bool operator _op_ (const sp<U>& o) const {              \
    return m_ptr _op_ o.m_ptr;                                  \
}                                                               \
template<typename U>                                            \
inline bool operator _op_ (const U* o) const {                  \
    return m_ptr _op_ o;                                        \
}

#define COMPARE(_op_)                                           \
COMPARE_WEAK(_op_)                                              \
inline bool operator _op_ (const wp<T>& o) const {              \
    return m_ptr _op_ o.m_ptr;                                  \
}                                                               \
template<typename U>                                            \
inline bool operator _op_ (const wp<U>& o) const {              \
    return m_ptr _op_ o.m_ptr;                                  \
}

class RefBase {
public:
	void incStrong(const void* id) const;
	void decStrong(const void* id) const;

	void forceIncStrong(const void* id) const;

	//! DEBUGGING ONLY: Get current strong ref count.
	int32_t getStrongCount() const;

	class weakref_type {
	public:
		RefBase* refBase() const;

		void incWeak(const void* id);
		void decWeak(const void* id);

		bool attemptIncStrong(const void* id);

		//! This is only safe if you have set OBJECT_LIFETIME_FOREVER.
		bool attemptIncWeak(const void* id);

		//! DEBUGGING ONLY: Get current weak ref count.
		int32_t getWeakCount() const;

		//! DEBUGGING ONLY: Print references held on object.
		void printRefs() const;

		//! DEBUGGING ONLY: Enable tracking for this object.
		// enable -- enable/disable tracking
		// retain -- when tracking is enable, if true, then we save a stack trace
		//           for each reference and dereference; when retain == false, we
		//           match up references and dereferences and keep only the
		//           outstanding ones.

		void trackMe(bool enable, bool retain);
	};

	weakref_type* createWeak(const void* id) const;

	weakref_type* getWeakRefs() const;

	//! DEBUGGING ONLY: Print references held on object.
	inline void printRefs() const {
		getWeakRefs()->printRefs();
	}

	//! DEBUGGING ONLY: Enable tracking of object.
	inline void trackMe(bool enable, bool retain) {
		getWeakRefs()->trackMe(enable, retain);
	}

protected:
	RefBase();
	virtual ~RefBase();

	//! Flags for extendObjectLifetime()
	enum {
		OBJECT_LIFETIME_WEAK = 0x0001, OBJECT_LIFETIME_FOREVER = 0x0003
	};

	void extendObjectLifetime(int32_t mode);

	//! Flags for onIncStrongAttempted()
	enum {
		FIRST_INC_STRONG = 0x0001
	};

	virtual void onFirstRef();
	virtual void onLastStrongRef(const void* id);
	virtual bool onIncStrongAttempted(uint32_t flags, const void* id);
	virtual void onLastWeakRef(const void* id);

private:
	friend class weakref_type;
	class weakref_impl;

	RefBase(const RefBase& o);
	RefBase& operator=(const RefBase& o);

	weakref_impl* const mRefs;
};

template<typename T>
class sp {
public:
	typedef typename RefBase::weakref_type weakref_type;

	inline sp() :
			m_ptr(0) {
	}

	sp(T* other);
	sp(const sp<T>& other);
	template<typename U> sp(U* other);
	template<typename U> sp(const sp<U>& other);

	~sp();

	// Assignment

	sp& operator =(T* other);
	sp& operator =(const sp<T>& other);

	template<typename U> sp& operator =(const sp<U>& other);
	template<typename U> sp& operator =(U* other);

	//! Special optimization for use by ProcessState (and nobody else).
	void force_set(T* other);

	// Reset

	void clear();

	// Accessors

	inline T& operator*() const {
		return *m_ptr;
	}
	inline T* operator->() const {
		return m_ptr;
	}
	inline T* get() const {
		return m_ptr;
	}

	// Operators

	COMPARE(==)COMPARE(!=)COMPARE(>)COMPARE(<)COMPARE(<=)COMPARE(>=)

private:
	template<typename Y> friend class sp;
	template<typename Y> friend class wp;

	// Optimization for wp::promote().
	sp(T* p, weakref_type* refs);

	T* m_ptr;
};

#define B_PACK_CHARS(c1, c2, c3, c4) \
    ((((c1)<<24)) | (((c2)<<16)) | (((c3)<<8)) | (c4))

class String16;
class BBinder;
class BpBinder;
class IInterface;
class Parcel;

class IBinder: public virtual RefBase {
public:
	enum {
		FIRST_CALL_TRANSACTION = 0x00000001,
		LAST_CALL_TRANSACTION = 0x00ffffff,

		PING_TRANSACTION = B_PACK_CHARS('_', 'P', 'N', 'G'),
		DUMP_TRANSACTION = B_PACK_CHARS('_', 'D', 'M', 'P'),
		INTERFACE_TRANSACTION = B_PACK_CHARS('_', 'N', 'T', 'F'),

		// Corresponds to TF_ONE_WAY -- an asynchronous call.
		FLAG_ONEWAY = 0x00000001
	};

	IBinder();

	/**
	 * Check if this IBinder implements the interface named by
	 * @a descriptor.  If it does, the base pointer to it is returned,
	 * which you can safely static_cast<> to the concrete C++ interface.
	 */
	virtual sp<IInterface> queryLocalInterface(const String16& descriptor);

	/**
	 * Return the canonical name of the interface provided by this IBinder
	 * object.
	 */
	virtual const String16& getInterfaceDescriptor() const = 0;

	virtual bool isBinderAlive() const = 0;
	virtual status_t pingBinder() = 0;
	virtual status_t dump(int fd, const Vector<String16>& args) = 0;

	virtual status_t transact(uint32_t code, const Parcel& data, Parcel* reply,
			uint32_t flags = 0) = 0;

	/**
	 * This method allows you to add data that is transported through
	 * IPC along with your IBinder pointer.  When implementing a Binder
	 * object, override it to write your desired data in to @a outData.
	 * You can then call getConstantData() on your IBinder to retrieve
	 * that data, from any process.  You MUST return the number of bytes
	 * written in to the parcel (including padding).
	 */
	class DeathRecipient: public virtual RefBase {
	public:
		virtual void binderDied(const wp<IBinder>& who) = 0;
	};

	/**
	 * Register the @a recipient for a notification if this binder
	 * goes away.  If this binder object unexpectedly goes away
	 * (typically because its hosting process has been killed),
	 * then DeathRecipient::binderDied() will be called with a referene
	 * to this.
	 *
	 * The @a cookie is optional -- if non-NULL, it should be a
	 * memory address that you own (that is, you know it is unique).
	 *
	 * @note You will only receive death notifications for remote binders,
	 * as local binders by definition can't die without you dying as well.
	 * Trying to use this function on a local binder will result in an
	 * INVALID_OPERATION code being returned and nothing happening.
	 *
	 * @note This link always holds a weak reference to its recipient.
	 *
	 * @note You will only receive a weak reference to the dead
	 * binder.  You should not try to promote this to a strong reference.
	 * (Nor should you need to, as there is nothing useful you can
	 * directly do with it now that it has passed on.)
	 */
	virtual status_t linkToDeath(const sp<DeathRecipient>& recipient,
			void* cookie = NULL, uint32_t flags = 0) = 0;

	/**
	 * Remove a previously registered death notification.
	 * The @a recipient will no longer be called if this object
	 * dies.  The @a cookie is optional.  If non-NULL, you can
	 * supply a NULL @a recipient, and the recipient previously
	 * added with that cookie will be unlinked.
	 */
	virtual status_t unlinkToDeath(const wp<DeathRecipient>& recipient,
			void* cookie = NULL, uint32_t flags = 0,
			wp<DeathRecipient>* outRecipient = NULL) = 0;

	virtual bool checkSubclass(const void* subclassID) const;

	typedef void (*object_cleanup_func)(const void* id, void* obj,
			void* cleanupCookie);

	virtual void attachObject(const void* objectID, void* object,
			void* cleanupCookie, object_cleanup_func func) = 0;
	virtual void* findObject(const void* objectID) const = 0;
	virtual void detachObject(const void* objectID) = 0;

	virtual BBinder* localBinder();
	virtual BpBinder* remoteBinder();

protected:
	virtual ~IBinder();

private:
};

class IInterface: public virtual RefBase {
public:
	IInterface();
	sp<IBinder> asBinder();
	sp<const IBinder> asBinder() const;

protected:
	virtual ~IInterface();
	virtual IBinder* onAsBinder() = 0;
};

template<typename INTERFACE>
inline sp<INTERFACE> interface_cast(const sp<IBinder>& obj) {
	return INTERFACE::asInterface(obj);
}

#define DECLARE_META_INTERFACE(INTERFACE)                               \
    static const android::String16 descriptor;                          \
    static android::sp<I##INTERFACE> asInterface(                       \
            const android::sp<android::IBinder>& obj);                  \
    virtual const android::String16& getInterfaceDescriptor() const;    \
    I##INTERFACE();                                                     \
    virtual ~I##INTERFACE();                                            \

class IServiceManager: public IInterface {
public:
	DECLARE_META_INTERFACE (ServiceManager)
	;

	/**
	 * Retrieve an existing service, blocking for a few seconds
	 * if it doesn't yet exist.
	 */
	virtual sp<IBinder> getService(const String16& name) const = 0;

	/**
	 * Retrieve an existing service, non-blocking.
	 */
	virtual sp<IBinder> checkService(const String16& name) const = 0;

	/**
	 * Register a service.
	 */
	virtual status_t addService(const String16& name,
			const sp<IBinder>& service) = 0;

	/**
	 * Return list of all existing services.
	 */
	virtual Vector<String16> listServices() = 0;

	enum {
		GET_SERVICE_TRANSACTION = IBinder::FIRST_CALL_TRANSACTION,
		CHECK_SERVICE_TRANSACTION,
		ADD_SERVICE_TRANSACTION,
		LIST_SERVICES_TRANSACTION,
	};
};

sp<IServiceManager> defaultServiceManager();

template<typename INTERFACE>
status_t getService(const String16& name, sp<INTERFACE>* outService) {
	const sp<IServiceManager> sm = defaultServiceManager();
	if (sm != NULL) {
		*outService = interface_cast<INTERFACE>(sm->getService(name));
		if ((*outService) != NULL)
			return NO_ERROR;
	}
	return NAME_NOT_FOUND;
}

class Flattenable;
class IBinder;
class IPCThreadState;
class ProcessState;
class String8;
class TextOutput;

struct native_handle;
struct flat_binder_object;

class Parcel {
public:
	Parcel();
	~Parcel();

	const uint8_t* data() const;
	size_t dataSize() const;
	size_t dataAvail() const;
	size_t dataPosition() const;
	size_t dataCapacity() const;

	status_t setDataSize(size_t size);
	void setDataPosition(size_t pos) const;
	status_t setDataCapacity(size_t size);

	status_t setData(const uint8_t* buffer, size_t len);

	status_t appendFrom(Parcel *parcel, size_t start, size_t len);

	bool hasFileDescriptors() const;

	// Writes the RPC header.
	status_t writeInterfaceToken(const String16& interface);

	// Parses the RPC header, returning true if the interface name
	// in the header matches the expected interface from the caller.
	//
	// Additionally, enforceInterface does part of the work of
	// propagating the StrictMode policy mask, populating the current
	// IPCThreadState, which as an optimization may optionally be
	// passed in.
	bool enforceInterface(const String16& interface,
			IPCThreadState* threadState = NULL) const;
	bool checkInterface(IBinder*) const;

	void freeData();

	const size_t* objects() const;
	size_t objectsCount() const;

	status_t errorCheck() const;
	void setError(status_t err);

	status_t write(const void* data, size_t len);
	void* writeInplace(size_t len);
	status_t writeUnpadded(const void* data, size_t len);
	status_t writeInt32(int32_t val);
	status_t writeInt64(int64_t val);
	status_t writeFloat(float val);
	status_t writeDouble(double val);
	status_t writeIntPtr(intptr_t val);
	status_t writeCString(const char* str);
	status_t writeString8(const String8& str);
	status_t writeString16(const String16& str);
	status_t writeString16(const char16_t* str, size_t len);
	status_t writeStrongBinder(const sp<IBinder>& val);
	status_t writeWeakBinder(const wp<IBinder>& val);
	status_t write(const Flattenable& val);

	// Place a native_handle into the parcel (the native_handle's file-
	// descriptors are dup'ed, so it is safe to delete the native_handle
	// when this function returns).
	// Doesn't take ownership of the native_handle.
	status_t writeNativeHandle(const native_handle* handle);

	// Place a file descriptor into the parcel.  The given fd must remain
	// valid for the lifetime of the parcel.
	status_t writeFileDescriptor(int fd);

	// Place a file descriptor into the parcel.  A dup of the fd is made, which
	// will be closed once the parcel is destroyed.
	status_t writeDupFileDescriptor(int fd);

	status_t writeObject(const flat_binder_object& val, bool nullMetaData);

	// Like Parcel.java's writeNoException().  Just writes a zero int32.
	// Currently the native implementation doesn't do any of the StrictMode
	// stack gathering and serialization that the Java implementation does.
	status_t writeNoException();

	void remove(size_t start, size_t amt);

	status_t read(void* outData, size_t len) const;
	const void* readInplace(size_t len) const;
	int32_t readInt32() const;
	status_t readInt32(int32_t *pArg) const;
	int64_t readInt64() const;
	status_t readInt64(int64_t *pArg) const;
	float readFloat() const;
	status_t readFloat(float *pArg) const;
	double readDouble() const;
	status_t readDouble(double *pArg) const;
	intptr_t readIntPtr() const;
	status_t readIntPtr(intptr_t *pArg) const;

	const char* readCString() const;
	String8 readString8() const;
	String16 readString16() const;
	const char16_t* readString16Inplace(size_t* outLen) const;
	sp<IBinder> readStrongBinder() const;
	wp<IBinder> readWeakBinder() const;
	status_t read(Flattenable& val) const;

	// Like Parcel.java's readExceptionCode().  Reads the first int32
	// off of a Parcel's header, returning 0 or the negative error
	// code on exceptions, but also deals with skipping over rich
	// response headers.  Callers should use this to read & parse the
	// response headers rather than doing it by hand.
	int32_t readExceptionCode() const;

	// Retrieve native_handle from the parcel. This returns a copy of the
	// parcel's native_handle (the caller takes ownership). The caller
	// must free the native_handle with native_handle_close() and
	// native_handle_delete().
	native_handle* readNativeHandle() const;

	// Retrieve a file descriptor from the parcel.  This returns the raw fd
	// in the parcel, which you do not own -- use dup() to get your own copy.
	int readFileDescriptor() const;

	const flat_binder_object* readObject(bool nullMetaData) const;

	// Explicitly close all file descriptors in the parcel.
	void closeFileDescriptors();

	typedef void (*release_func)(Parcel* parcel, const uint8_t* data,
			size_t dataSize, const size_t* objects, size_t objectsSize,
			void* cookie);

	const uint8_t* ipcData() const;
	size_t ipcDataSize() const;
	const size_t* ipcObjects() const;
	size_t ipcObjectsCount() const;
	void ipcSetDataReference(const uint8_t* data, size_t dataSize,
			const size_t* objects, size_t objectsCount, release_func relFunc,
			void* relCookie);

	void print(TextOutput& to, uint32_t flags = 0) const;

private:
	Parcel(const Parcel& o);
	Parcel& operator=(const Parcel& o);

	status_t finishWrite(size_t len);
	void releaseObjects();
	void acquireObjects();
	status_t growData(size_t len);
	status_t restartWrite(size_t desired);
	status_t continueWrite(size_t desired);
	void freeDataNoInit();
	void initState();
	void scanForFds() const;

	template<class T>
	status_t readAligned(T *pArg) const;

	template<class T> T readAligned() const;

	template<class T>
	status_t writeAligned(T val);

	status_t mError;
	uint8_t* mData;
	size_t mDataSize;
	size_t mDataCapacity;
	mutable size_t mDataPos;
	size_t* mObjects;
	size_t mObjectsSize;
	size_t mObjectsCapacity;
	mutable size_t mNextObjectHint;

	mutable bool mFdsKnown;
	mutable bool mHasFds;

	release_func mOwner;
	void* mOwnerCookie;
};

void getPackagesForUid(IBinder* packageManager, int uid, int transaction_id) {
	Parcel data, reply;
	data.writeInterfaceToken(String16("android.content.pm.IPackageManager"));

	data.writeInt32(uid);
	if (packageManager->transact(transaction_id, data, &reply) != NO_ERROR) {
		printf("getPackagesForUid could not contact remote\n");
	}
	int32_t err = reply.readExceptionCode();
	if (err < 0) {
		printf("getPackagesForUid caught exception %d\n", err);
		return;
	}
	int count = reply.readInt32();
	printf("getPackagesForUid: uid:%d count:%d transaction_id:%d\n", uid, count,
			transaction_id);
	for (int i = 0; i < count; i++) {
		const String16& pkg = reply.readString16();
		for (int j = 0; j < pkg.size(); j++) {
			printf("%c", pkg.operator const char16_t *()[j]);
		}
		printf("\n\n");
	}
}
}
;
// namespace android

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("usages: testbinder uid\n");
		return 0;
	}
	android::sp<android::IBinder> binder =
			android::defaultServiceManager()->getService(
					android::String16("package"));

	if (binder.get() == 0) {
		printf("getService failed\n");
		return 0;
	}

	for (int i = 19; i < 35; i++) {
		getPackagesForUid(binder.get(), atoi(argv[1]), i);
	}

	return 0;
}
