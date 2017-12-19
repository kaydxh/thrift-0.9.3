/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef _THRIFT_TRANSPORT_TBUFFERTRANSPORTS_H_
#define _THRIFT_TRANSPORT_TBUFFERTRANSPORTS_H_ 1

#include <cstdlib>
#include <cstring>
#include <limits>
#include <boost/scoped_array.hpp>

#include <thrift/transport/TTransport.h>
#include <thrift/transport/TVirtualTransport.h>

#ifdef __GNUC__
#define TDB_LIKELY(val) (__builtin_expect((val), 1))
#define TDB_UNLIKELY(val) (__builtin_expect((val), 0))
#else
#define TDB_LIKELY(val) (val)
#define TDB_UNLIKELY(val) (val)
#endif

namespace apache {
namespace thrift {
namespace transport {

/**
 * Base class for all transports that use read/write buffers for performance.
 *
 * TBufferBase is designed to implement the fast-path "memcpy" style
 * operations that work in the common case.  It does so with small and
 * (eventually) nonvirtual, inlinable methods.  TBufferBase is an abstract
 * class.  Subclasses are expected to define the "slow path" operations
 * that have to be done when the buffers are full or empty.
 *
 */

//缓存基类TBufferBase就是让传输类所有的读写函数都提供缓存来提高性能
//它在通常情况下采用memcpy来设计和实现快路径的读写访问操作，这些操作函数常都是小、非虚拟和内联函数。
//TBufferBase是一个抽象的基类，子类必须实现慢路径的读写函数等操作，慢路径的读写等操作主要是为了在缓存已经满或空的情况下执行
class TBufferBase : public TVirtualTransport<TBufferBase> {

public:
  /**
   * Fast-path read.
   *
   * When we have enough data buffered to fulfill the read, we can satisfy it
   * with a single memcpy, then adjust our internal pointers.  If the buffer
   * is empty, we call out to our slow path, implemented by a subclass.
   * This method is meant to eventually be nonvirtual and inlinable.
   */

  //读函数
  uint32_t read(uint8_t* buf, uint32_t len) {
    uint8_t* new_rBase = rBase_ + len;  //得到需要读到的缓存边界，也是下次读的开始位置
    if (TDB_LIKELY(new_rBase <= rBound_)) { //判断缓存是否有足够的数据可读，采用了分支预测技术
      std::memcpy(buf, rBase_, len);   //直接内存拷贝
      rBase_ = new_rBase;  //更新新的缓存读基地址
      return len;  //返回读取的长度
    }
    return readSlow(buf, len); //如果缓存已经不能够满足读取长度需要就执行慢读
  }

  /**
   * Shortcutted version of readAll.
   */
  uint32_t readAll(uint8_t* buf, uint32_t len) {
    uint8_t* new_rBase = rBase_ + len;
    if (TDB_LIKELY(new_rBase <= rBound_)) {
      std::memcpy(buf, rBase_, len);
      rBase_ = new_rBase;
      return len;
    }
    return apache::thrift::transport::readAll(*this, buf, len); // 辅助传输层的全局模板函数readAll
  }

  /*
  // 辅助传输层的全局模板函数readAll
  template <class Transport_>
  uint32_t readAll(Transport_& trans, uint8_t* buf, uint32_t len) {
    uint32_t have = 0;
    uint32_t get = 0;

    while (have < len) {
      get = trans.read(buf + have, len - have); //通过具体的传输类读取剩余的需要读取的数据
      if (get <= 0) { //处理数据以读完异常
        throw TTransportException(TTransportException::END_OF_FILE, "No more data to read.");
      }
      have += get; //已经读取的字节数
    }

    return have; //返回读到的字节数
  }
  */

  /**
   * Fast-path write.
   *
   * When we have enough empty space in our buffer to accommodate the write, we
   * can satisfy it with a single memcpy, then adjust our internal pointers.
   * If the buffer is full, we call out to our slow path, implemented by a
   * subclass.  This method is meant to eventually be nonvirtual and
   * inlinable.
   */

  //快速写函数
  void write(const uint8_t* buf, uint32_t len) {
    uint8_t* new_wBase = wBase_ + len; //写入后的新缓存基地址
    if (TDB_LIKELY(new_wBase <= wBound_)) { //判断缓存是否有足够的空间可以写入
      std::memcpy(wBase_, buf, len); //内存拷贝
      wBase_ = new_wBase; //更新基地址
      return;
    }
    writeSlow(buf, len); //缓存空间不足就调用慢写函数
  }

  /**
   * Fast-path borrow.  A lot like the fast-path read.
   */

  //快速路径借
  const uint8_t* borrow(uint8_t* buf, uint32_t* len) {
    if (TDB_LIKELY(static_cast<ptrdiff_t>(*len) <= rBound_ - rBase_)) { //判断是否有足够的长度借
      // With strict aliasing, writing to len shouldn't force us to
      // refetch rBase_ from memory.  TODO(dreiss): Verify this.
      *len = static_cast<uint32_t>(rBound_ - rBase_); //能够借出去的长度
      return rBase_; //返回借的基地址
    }
    return borrowSlow(buf, len); //不足就采用慢路径借
  }

  /**
   * Consume doesn't require a slow path.
   */

  //消费函数
  void consume(uint32_t len) {
    if (TDB_LIKELY(static_cast<ptrdiff_t>(len) <= rBound_ - rBase_)) { //判断缓存是否够消费
      rBase_ += len; //更新已经消耗的长度
    } else {
      throw TTransportException(TTransportException::BAD_ARGS, "consume did not follow a borrow.");
    }
  }

protected:
  /// Slow path read.
  virtual uint32_t readSlow(uint8_t* buf, uint32_t len) = 0;

  /// Slow path write.
  virtual void writeSlow(const uint8_t* buf, uint32_t len) = 0;

  /**
   * Slow path borrow.
   *
   * POSTCONDITION: return == NULL || rBound_ - rBase_ >= *len
   */
  virtual const uint8_t* borrowSlow(uint8_t* buf, uint32_t* len) = 0;

  /**
   * Trivial constructor.
   *
   * Initialize pointers safely.  Constructing is not a very
   * performance-sensitive operation, so it is okay to just leave it to
   * the concrete class to set up pointers correctly.
   */
  TBufferBase() : rBase_(NULL), rBound_(NULL), wBase_(NULL), wBound_(NULL) {}

  /// Convenience mutator for setting the read buffer.
  //设置读缓存空间地址
  void setReadBuffer(uint8_t* buf, uint32_t len) {
    rBase_ = buf; //读缓存开始地址
    rBound_ = buf + len; //读缓存地址界限
  }

  /// Convenience mutator for setting the write buffer.
  //设置写缓存地址空间
  void setWriteBuffer(uint8_t* buf, uint32_t len) {
    wBase_ = buf; //写缓存开始地址
    wBound_ = buf + len;    //写缓存地址界限
  }

  virtual ~TBufferBase() {}

  /// Reads begin here.
  uint8_t* rBase_; //开始读的位置
  /// Reads may extend to just before here.
  uint8_t* rBound_; //读界限

  /// Writes begin here.
  uint8_t* wBase_; //开始写的位置
  /// Writes may extend to just before here.
  uint8_t* wBound_; //写界限
};

/**
 * Buffered transport. For reads it will read more data than is requested
 * and will serve future data out of a local buffer. For writes, data is
 * stored to an in memory buffer before being written out.
 *
 */
class TBufferedTransport : public TVirtualTransport<TBufferedTransport, TBufferBase> {
public:
  static const int DEFAULT_BUFFER_SIZE = 512;   //缓存的大小默认是512字节

  /// Use default buffer sizes.
  TBufferedTransport(boost::shared_ptr<TTransport> transport)
    : transport_(transport),
      rBufSize_(DEFAULT_BUFFER_SIZE),
      wBufSize_(DEFAULT_BUFFER_SIZE),
      rBuf_(new uint8_t[rBufSize_]), //分配缓存
      wBuf_(new uint8_t[wBufSize_]) {
    initPointers();
  }

  /// Use specified buffer sizes.
  TBufferedTransport(boost::shared_ptr<TTransport> transport, uint32_t sz)
    : transport_(transport),
      rBufSize_(sz),
      wBufSize_(sz),
      rBuf_(new uint8_t[rBufSize_]),
      wBuf_(new uint8_t[wBufSize_]) {
    initPointers();
  }

  /// Use specified read and write buffer sizes.
  TBufferedTransport(boost::shared_ptr<TTransport> transport, uint32_t rsz, uint32_t wsz)
    : transport_(transport),
      rBufSize_(rsz),
      wBufSize_(wsz),
      rBuf_(new uint8_t[rBufSize_]),
      wBuf_(new uint8_t[wBufSize_]) {
    initPointers();
  }

  void open() { transport_->open(); }

  bool isOpen() { return transport_->isOpen(); }

  bool peek() {
    if (rBase_ == rBound_) { //判断读的基地址与读边界是否重合了，也就是已经读取完毕
      setReadBuffer(rBuf_.get(), transport_->read(rBuf_.get(), rBufSize_));   //重新读取底层来的数据
    }
    return (rBound_ > rBase_); //边界大于基地址就是有未读状态数据
  }

  void close() {
    flush();
    transport_->close();
  }

  virtual uint32_t readSlow(uint8_t* buf, uint32_t len);

  virtual void writeSlow(const uint8_t* buf, uint32_t len);

  void flush();

  /**
   * Returns the origin of the underlying transport
   */
  virtual const std::string getOrigin() { return transport_->getOrigin(); }

  /**
   * The following behavior is currently implemented by TBufferedTransport,
   * but that may change in a future version:
   * 1/ If len is at most rBufSize_, borrow will never return NULL.
   *    Depending on the underlying transport, it could throw an exception
   *    or hang forever.
   * 2/ Some borrow requests may copy bytes internally.  However,
   *    if len is at most rBufSize_/2, none of the copied bytes
   *    will ever have to be copied again.  For optimial performance,
   *    stay under this limit.
   */
  virtual const uint8_t* borrowSlow(uint8_t* buf, uint32_t* len);

  boost::shared_ptr<TTransport> getUnderlyingTransport() { return transport_; }

  /*
   * TVirtualTransport provides a default implementation of readAll().
   * We want to use the TBufferBase version instead.
   */
  uint32_t readAll(uint8_t* buf, uint32_t len) { return TBufferBase::readAll(buf, len); }

protected:
  void initPointers() {
    setReadBuffer(rBuf_.get(), 0);
    setWriteBuffer(wBuf_.get(), wBufSize_);
    // Write size never changes.
  }

  boost::shared_ptr<TTransport> transport_;

  uint32_t rBufSize_;
  uint32_t wBufSize_;
  boost::scoped_array<uint8_t> rBuf_;
  boost::scoped_array<uint8_t> wBuf_;
};

/**
 * Wraps a transport into a buffered one.
 *
 */
class TBufferedTransportFactory : public TTransportFactory {
public:
  TBufferedTransportFactory() {}

  virtual ~TBufferedTransportFactory() {}

  /**
   * Wraps the transport into a buffered one.
   */
  virtual boost::shared_ptr<TTransport> getTransport(boost::shared_ptr<TTransport> trans) {
    return boost::shared_ptr<TTransport>(new TBufferedTransport(trans));
  }
};

/**
 * Framed transport. All writes go into an in-memory buffer until flush is
 * called, at which point the transport writes the length of the entire
 * binary chunk followed by the data payload. This allows the receiver on the
 * other end to always do fixed-length reads.
 *
 */

//帧传输类就是按照一帧的固定大小来传输数据，所有的写操作首先都是在内存中完成的直到调用了flush操作，
//然后传输节点在flush操作之后将所有数据根据数据的有效载荷写入数据的长度的二进制块发送出去，允许在接收的另一端按照固定的长度来读取。
class TFramedTransport : public TVirtualTransport<TFramedTransport, TBufferBase> {
public:
  static const int DEFAULT_BUFFER_SIZE = 512; 
  static const int DEFAULT_MAX_FRAME_SIZE = 256 * 1024 * 1024;

  /// Use default buffer sizes.
  TFramedTransport(boost::shared_ptr<TTransport> transport)
    : transport_(transport),
      rBufSize_(0),
      wBufSize_(DEFAULT_BUFFER_SIZE),
      rBuf_(),
      wBuf_(new uint8_t[wBufSize_]),
      bufReclaimThresh_((std::numeric_limits<uint32_t>::max)()),
      maxFrameSize_(DEFAULT_MAX_FRAME_SIZE) {
    initPointers();
  }

  TFramedTransport(boost::shared_ptr<TTransport> transport,
                   uint32_t sz,
                   uint32_t bufReclaimThresh = (std::numeric_limits<uint32_t>::max)())
    : transport_(transport),
      rBufSize_(0),
      wBufSize_(sz),
      rBuf_(),
      wBuf_(new uint8_t[wBufSize_]),
      bufReclaimThresh_(bufReclaimThresh),
      maxFrameSize_(DEFAULT_MAX_FRAME_SIZE) {
    initPointers();
  }

  void open() { transport_->open(); }

  bool isOpen() { return transport_->isOpen(); }

  bool peek() { return (rBase_ < rBound_) || transport_->peek(); }

  void close() {
    flush();
    transport_->close();
  }

  virtual uint32_t readSlow(uint8_t* buf, uint32_t len);

  virtual void writeSlow(const uint8_t* buf, uint32_t len);

  virtual void flush();

  uint32_t readEnd();

  uint32_t writeEnd();

  const uint8_t* borrowSlow(uint8_t* buf, uint32_t* len);

  boost::shared_ptr<TTransport> getUnderlyingTransport() { return transport_; }

  /*
   * TVirtualTransport provides a default implementation of readAll().
   * We want to use the TBufferBase version instead.
   */
  uint32_t readAll(uint8_t* buf, uint32_t len) { return TBufferBase::readAll(buf, len); }

  /**
   * Returns the origin of the underlying transport
   */
  virtual const std::string getOrigin() { return transport_->getOrigin(); }

  /**
   * Set the maximum size of the frame at read
   */
  void setMaxFrameSize(uint32_t maxFrameSize) { maxFrameSize_ = maxFrameSize; }

  /**
   * Get the maximum size of the frame at read
   */
  uint32_t getMaxFrameSize() { return maxFrameSize_; }

protected:
  /**
   * Reads a frame of input from the underlying stream.
   *
   * Returns true if a frame was read successfully, or false on EOF.
   * (Raises a TTransportException if EOF occurs after a partial frame.)
   */
  bool readFrame();

  void initPointers() {
    setReadBuffer(NULL, 0);
    setWriteBuffer(wBuf_.get(), wBufSize_);

    // Pad the buffer so we can insert the size later.
    int32_t pad = 0;
    this->write((uint8_t*)&pad, sizeof(pad));
  }

  boost::shared_ptr<TTransport> transport_;

  uint32_t rBufSize_;
  uint32_t wBufSize_;
  boost::scoped_array<uint8_t> rBuf_;
  boost::scoped_array<uint8_t> wBuf_;
  uint32_t bufReclaimThresh_;
  uint32_t maxFrameSize_;
};

/**
 * Wraps a transport into a framed one.
 *
 */
class TFramedTransportFactory : public TTransportFactory {
public:
  TFramedTransportFactory() {}

  virtual ~TFramedTransportFactory() {}

  /**
   * Wraps the transport into a framed one.
   */
  virtual boost::shared_ptr<TTransport> getTransport(boost::shared_ptr<TTransport> trans) {
    return boost::shared_ptr<TTransport>(new TFramedTransport(trans));
  }
};

/**
 * A memory buffer is a tranpsort that simply reads from and writes to an
 * in memory buffer. Anytime you call write on it, the data is simply placed
 * into a buffer, and anytime you call read, data is read from that buffer.
 *
 * The buffers are allocated using C constructs malloc,realloc, and the size
 * doubles as necessary.  We've considered using scoped
 *
 */
class TMemoryBuffer : public TVirtualTransport<TMemoryBuffer, TBufferBase> {
private:
  // Common initialization done by all constructors.
  void initCommon(uint8_t* buf, uint32_t size, bool owner, uint32_t wPos) {
    if (buf == NULL && size != 0) {
      assert(owner);
      buf = (uint8_t*)std::malloc(size);
      if (buf == NULL) {
        throw std::bad_alloc();
      }
    }

    buffer_ = buf; //buf起始地址
    bufferSize_ = size; //buf大小

    rBase_ = buffer_; //读起始地址
    rBound_ = buffer_ + wPos; //读界限
    // TODO(dreiss): Investigate NULL-ing this if !owner.
    wBase_ = buffer_ + wPos; //写起始地址
    wBound_ = buffer_ + bufferSize_; //写界限

    owner_ = owner; //TMemoryBuffer是否自己拥有该buffer_,拥有的话，就TMemoryBuffer自己负责释放缓存

    // rBound_ is really an artifact.  In principle, it should always be
    // equal to wBase_.  We update it in a few places (computeRead, etc.).
  }

public:
  static const uint32_t defaultSize = 1024;

  /**
   * This enum specifies how a TMemoryBuffer should treat
   * memory passed to it via constructors or resetBuffer.
   *
   * OBSERVE:
   *   TMemoryBuffer will simply store a pointer to the memory.
   *   It is the callers responsibility to ensure that the pointer
   *   remains valid for the lifetime of the TMemoryBuffer,
   *   and that it is properly cleaned up.
   *   Note that no data can be written to observed buffers.
   *
   * COPY:
   *   TMemoryBuffer will make an internal copy of the buffer.
   *   The caller has no responsibilities.
   *
   * TAKE_OWNERSHIP:
   *   TMemoryBuffer will become the "owner" of the buffer,
   *   and will be responsible for freeing it.
   *   The membory must have been allocated with malloc.
   */
  enum MemoryPolicy { OBSERVE = 1, COPY = 2, TAKE_OWNERSHIP = 3 };

  /**
   * Construct a TMemoryBuffer with a default-sized buffer,
   * owned by the TMemoryBuffer object.
   */
  TMemoryBuffer() { initCommon(NULL, defaultSize, true, 0); }

  /**
   * Construct a TMemoryBuffer with a buffer of a specified size,
   * owned by the TMemoryBuffer object.
   *
   * @param sz  The initial size of the buffer.
   */
  TMemoryBuffer(uint32_t sz) { initCommon(NULL, sz, true, 0); }

  /**
   * Construct a TMemoryBuffer with buf as its initial contents.
   *
   * @param buf    The initial contents of the buffer.
   *               Note that, while buf is a non-const pointer,
   *               TMemoryBuffer will not write to it if policy == OBSERVE,
   *               so it is safe to const_cast<uint8_t*>(whatever).
   * @param sz     The size of @c buf.
   * @param policy See @link MemoryPolicy @endlink .
   */
  TMemoryBuffer(uint8_t* buf, uint32_t sz, MemoryPolicy policy = OBSERVE) {
    if (buf == NULL && sz != 0) {
      throw TTransportException(TTransportException::BAD_ARGS,
                                "TMemoryBuffer given null buffer with non-zero size.");
    }

    switch (policy) {
    case OBSERVE:
    case TAKE_OWNERSHIP:
      initCommon(buf, sz, policy == TAKE_OWNERSHIP, sz);
      break;
    case COPY:
      initCommon(NULL, sz, true, 0);
      this->write(buf, sz);
      break;
    default:
      throw TTransportException(TTransportException::BAD_ARGS,
                                "Invalid MemoryPolicy for TMemoryBuffer");
    }
  }

  ~TMemoryBuffer() {
    if (owner_) {
      std::free(buffer_);
    }
  }

  bool isOpen() { return true; }

  bool peek() { return (rBase_ < wBase_); }

  void open() {}

  void close() {}

  // TODO(dreiss): Make bufPtr const.
  //获取缓存中的可读数据
  void getBuffer(uint8_t** bufPtr, uint32_t* sz) {
    *bufPtr = rBase_;
    *sz = static_cast<uint32_t>(wBase_ - rBase_);
  }

  //将缓存中的可读数据以string的对象返回
  std::string getBufferAsString() {
    if (buffer_ == NULL) {
      return "";
    }
    uint8_t* buf;
    uint32_t sz;
    getBuffer(&buf, &sz);
    return std::string((char*)buf, (std::string::size_type)sz);
  }

  void appendBufferToString(std::string& str) {
    if (buffer_ == NULL) {
      return;
    }
    uint8_t* buf;
    uint32_t sz;
    getBuffer(&buf, &sz);
    str.append((char*)buf, sz);
  }

  void resetBuffer() {
    rBase_ = buffer_;
    rBound_ = buffer_;
    wBase_ = buffer_;
    // It isn't safe to write into a buffer we don't own.
    if (!owner_) {
      wBound_ = wBase_;
      bufferSize_ = 0;
    }
  }

  /// See constructor documentation.
  void resetBuffer(uint8_t* buf, uint32_t sz, MemoryPolicy policy = OBSERVE) {
    // Use a variant of the copy-and-swap trick for assignment operators.
    // This is sub-optimal in terms of performance for two reasons:
    //   1/ The constructing and swapping of the (small) values
    //      in the temporary object takes some time, and is not necessary.
    //   2/ If policy == COPY, we allocate the new buffer before
    //      freeing the old one, precluding the possibility of
    //      reusing that memory.
    // I doubt that either of these problems could be optimized away,
    // but the second is probably no a common case, and the first is minor.
    // I don't expect resetBuffer to be a common operation, so I'm willing to
    // bite the performance bullet to make the method this simple.

    // Construct the new buffer.
    TMemoryBuffer new_buffer(buf, sz, policy);
    // Move it into ourself.
    this->swap(new_buffer);
    // Our old self gets destroyed.
  }

  /// See constructor documentation.
  void resetBuffer(uint32_t sz) {
    // Construct the new buffer.
    TMemoryBuffer new_buffer(sz);
    // Move it into ourself.
    this->swap(new_buffer);
    // Our old self gets destroyed.
  }

  std::string readAsString(uint32_t len) {
    std::string str;
    (void)readAppendToString(str, len);
    return str;
  }

  uint32_t readAppendToString(std::string& str, uint32_t len);

  // return number of bytes read
  //返回已读的字节数
  uint32_t readEnd() {
    // This cast should be safe, because buffer_'s size is a uint32_t
    uint32_t bytes = static_cast<uint32_t>(rBase_ - buffer_);
    if (rBase_ == wBase_) {
      resetBuffer(); //读地址如果和写地址相等，那么就要复位，因为没有可读的数据了
    }
    return bytes;
  }

  // Return number of bytes written
  //返回已写的字节数
  uint32_t writeEnd() {
    // This cast should be safe, because buffer_'s size is a uint32_t
    return static_cast<uint32_t>(wBase_ - buffer_);
  }

  //返回可读的缓存大小
  uint32_t available_read() const {
    // Remember, wBase_ is the real rBound_.
    return static_cast<uint32_t>(wBase_ - rBase_);
  }

  //返回可写的缓存大小
  uint32_t available_write() const { return static_cast<uint32_t>(wBound_ - wBase_); }

  // Returns a pointer to where the client can write data to append to
  // the TMemoryBuffer, and ensures the buffer is big enough to accommodate a
  // write of the provided length.  The returned pointer is very convenient for
  // passing to read(), recv(), or similar. You must call wroteBytes() as soon
  // as data is written or the buffer will not be aware that data has changed.

  //返回能写len个字节的写地址
  uint8_t* getWritePtr(uint32_t len) {
    ensureCanWrite(len);
    return wBase_;
  }

  // Informs the buffer that the client has written 'len' bytes into storage
  // that had been provided by getWritePtr().
  ////写地址先偏移len字节，空余出来 
  void wroteBytes(uint32_t len);

  /*
   * TVirtualTransport provides a default implementation of readAll().
   * We want to use the TBufferBase version instead.
   */
  uint32_t readAll(uint8_t* buf, uint32_t len) { return TBufferBase::readAll(buf, len); }

protected:
  void swap(TMemoryBuffer& that) {
    using std::swap;
    swap(buffer_, that.buffer_);
    swap(bufferSize_, that.bufferSize_);

    swap(rBase_, that.rBase_);
    swap(rBound_, that.rBound_);
    swap(wBase_, that.wBase_);
    swap(wBound_, that.wBound_);

    swap(owner_, that.owner_);
  }

  // Make sure there's at least 'len' bytes available for writing.
  void ensureCanWrite(uint32_t len);

  // Compute the position and available data for reading.
  //计算可读的数据，返回的可读数据长度为要求读的len和缓存中的能读取的数据长度的最小值
  void computeRead(uint32_t len, uint8_t** out_start, uint32_t* out_give);

  //将可读数据（<=len）copy到buf中,因为要读取的数据长度<=len，所以叫readslow
  uint32_t readSlow(uint8_t* buf, uint32_t len);

  //首先分配内存到足够写len的空间，再往wBase_里写数据，因为开始缓存的大小<=len，所以要先分配，再写，就叫writeshow
  void writeSlow(const uint8_t* buf, uint32_t len);

  const uint8_t* borrowSlow(uint8_t* buf, uint32_t* len);

  // Data buffer
  uint8_t* buffer_;

  // Allocated buffer size
  uint32_t bufferSize_;

  // Is this object the owner of the buffer?
  bool owner_;

  // Don't forget to update constrctors, initCommon, and swap if
  // you add new members.
};
}
}
} // apache::thrift::transport

#endif // #ifndef _THRIFT_TRANSPORT_TBUFFERTRANSPORTS_H_
