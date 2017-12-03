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

#include <cassert>
#include <algorithm>

#include <thrift/transport/TBufferTransports.h>

using std::string;

namespace apache {
namespace thrift {
namespace transport {

//快读是直接从缓存中拿数据，慢读是要么从缓存中拿部分数据，要么是从传输层调用系统调用读取
uint32_t TBufferedTransport::readSlow(uint8_t* buf, uint32_t len) {
  uint32_t have = static_cast<uint32_t>(rBound_ - rBase_); //计算还有多少数据在缓存中

  // We should only take the slow path if we can't satisfy the read
  // with the data already in the buffer.

  // 如果读取缓存中已经存在的数据不能满足我们，
　// 我们（也仅仅在这种情况下）应该才从慢路径读数据
  assert(have < len);

  // If we have some data in the buffer, copy it out and return it.
  // We have to return it without attempting to read more, since we aren't
  // guaranteed that the underlying transport actually has more data, so
  // attempting to read from it could block.

  // 如果我们有一些数据在缓存，拷贝出来并返回它
　// 我们不得不返回它而去尝试读更多的数据，因为我们不能保证
　// 下层传输实际有更多的数据， 因此尝试阻塞式读取它。

  if (have > 0) {
    memcpy(buf, rBase_, have); //拷贝数据
    setReadBuffer(rBuf_.get(), 0); //设置读缓存，基类实现该函数
    return have;  //返回缓存中已经存在的不完整数据
  }

  // No data is available in our buffer.
  // Get more from underlying transport up to buffer size.
  // Note that this makes a lot of sense if len < rBufSize_
  // and almost no sense otherwise.  TODO(dreiss): Fix that
  // case (possibly including some readv hotness).

  //缓存里没有数据
  //从下层传输得到更多以达到buffer的大小
  //尝试read BufSize_数据
  //读取的长度可能大于、小于或等于我们需要读取的长度
  setReadBuffer(rBuf_.get(), transport_->read(rBuf_.get(), rBufSize_)); //读取数据并设置读缓存

  // Hand over whatever we have.
  // 处理我们已有的数据
  uint32_t give = (std::min)(len, static_cast<uint32_t>(rBound_ - rBase_));
  memcpy(buf, rBase_, give);
  rBase_ += give;

  return give;
}

void TBufferedTransport::writeSlow(const uint8_t* buf, uint32_t len) {
  //计算写缓存区中已有的字节数
  uint32_t have_bytes = static_cast<uint32_t>(wBase_ - wBuf_.get()); 

  //计算剩余写缓存空间
  uint32_t space = static_cast<uint32_t>(wBound_ - wBase_);
  // We should only take the slow path if we can't accommodate the write
  // with the free space already in the buffer.

  // 如果在缓存区的空闲空间不能容纳我们的数据，我们采用慢路径写（仅仅）
  assert(wBound_ - wBase_ < static_cast<ptrdiff_t>(len));

  // Now here's the tricky question: should we copy data from buf into our
  // internal buffer and write it from there, or should we just write out
  // the current internal buffer in one syscall and write out buf in another.
  // If our currently buffered data plus buf is at least double our buffer
  // size, we will have to do two syscalls no matter what (except in the
  // degenerate case when our buffer is empty), so there is no use copying.
  // Otherwise, there is sort of a sliding scale.  If we have N-1 bytes
  // buffered and need to write 2, it would be crazy to do two syscalls.
  // On the other hand, if we have 2 bytes buffered and are writing 2N-3,
  // we can save a syscall in the short term by loading up our buffer, writing
  // it out, and copying the rest of the bytes into our buffer.  Of course,
  // if we get another 2-byte write, we haven't saved any syscalls at all,
  // and have just copied nearly 2N bytes for nothing.  Finding a perfect
  // policy would require predicting the size of future writes, so we're just
  // going to always eschew syscalls if we have less than 2N bytes to write.

  // The case where we have to do two syscalls.
  // This case also covers the case where the buffer is empty,
  // but it is clearer (I think) to think of it as two separate cases.

  //已有数据加上需要写入的数据是否大于2倍写缓存区或者缓存区为空
  if ((have_bytes + len >= 2 * wBufSize_) || (have_bytes == 0)) {
    // TODO(dreiss): writev

    //缓存大于0且加上需要再写入数据的长度大于2倍缓存区
    if (have_bytes > 0) { 

      //先将已有数据写入下层传输
      transport_->write(wBuf_.get(), have_bytes);
    }

    //写入这次的len长度的数据
    transport_->write(buf, len);
    wBase_ = wBuf_.get(); //更新写缓存的基地址
    return;
  }

  //have_bytes > 0 && have_bytes + len < 2 * wBufSize_
  // Fill up our internal buffer for a write.
  //填充剩余写缓存区
  memcpy(wBase_, buf, space);
  buf += space;
  len -= space;

  //写入wBufSize_数据
  transport_->write(wBuf_.get(), wBufSize_);

  // Copy the rest into our buffer.
  //因为上面已经发送了wBufSize_个数据，所以剩下的len肯定小于wBufSize_
  assert(len < wBufSize_);

  //拷贝剩余的数据到缓存
  memcpy(wBuf_.get(), buf, len);

  //更新写缓存的基地址
  wBase_ = wBuf_.get() + len; 
  return;
}

const uint8_t* TBufferedTransport::borrowSlow(uint8_t* buf, uint32_t* len) {
  (void)buf;
  (void)len;
  // Simply return NULL.  We don't know if there is actually data available on
  // the underlying transport, so calling read() might block.
  return NULL;
}

void TBufferedTransport::flush() {
  // Write out any data waiting in the write buffer.
  //将缓存区的数据发送出去
  uint32_t have_bytes = static_cast<uint32_t>(wBase_ - wBuf_.get());
  if (have_bytes > 0) {
    // Note that we reset wBase_ prior to the underlying write
    // to ensure we're in a sane state (i.e. internal buffer cleaned)
    // if the underlying write throws up an exception
    wBase_ = wBuf_.get();
    transport_->write(wBuf_.get(), have_bytes);
  }

  // Flush the underlying transport.
  transport_->flush();
}

uint32_t TFramedTransport::readSlow(uint8_t* buf, uint32_t len) {
  uint32_t want = len; //想要读取的长度
  uint32_t have = static_cast<uint32_t>(rBound_ - rBase_); //内存缓存中已经有的数据的长度

  // We should only take the slow path if we can't satisfy the read
  // with the data already in the buffer.

  //只有数据长度不满足需要读的长度时，才采用慢读
  assert(have < want);

  // If we have some data in the buffer, copy it out and return it.
  // We have to return it without attempting to read more, since we aren't
  // guaranteed that the underlying transport actually has more data, so
  // attempting to read from it could block.

  // 如果我们有一些数据在缓存，拷贝出来并且返回它。
　// 我们没有试图读取更多的数据而是不得不返回它，因为我们不能保证在下面的  
  // 传输层实际上有更多的数据，因此应该尝试阻塞式读它。

  if (have > 0) {
    memcpy(buf, rBase_, have);
    setReadBuffer(rBuf_.get(), 0);
    return have;
  }

  //缓存中没有数据，就实际在传输层读取一桢

  // Read another frame.
  if (!readFrame()) {
    // EOF.  No frame available.
    return 0;
  }

  // TODO(dreiss): Should we warn when reads cross frames?

  // Hand over whatever we have.
  // 处理我们已有的数据， 已有数据为want和缓存中数据的最小值
  uint32_t give = (std::min)(want, static_cast<uint32_t>(rBound_ - rBase_));
  memcpy(buf, rBase_, give);
  rBase_ += give; //调整缓存基地址
  want -= give; //计算还有多少想要的数据没有得到

  return (len - want); //返回实际读取长度
}


//缓存中没有数据的时候就会调用读取帧的函数readFrame
bool TFramedTransport::readFrame() {
  // TODO(dreiss): Think about using readv here, even though it would
  // result in (gasp) read-ahead.

  // Read the size of the next frame.
  // We can't use readAll(&sz, sizeof(sz)), since that always throws an
  // exception on EOF.  We want to throw an exception only if EOF occurs after
  // partial size data.

  //存放长度的变量
  int32_t sz = -1;

  //已经读取长度数据的字节数
  uint32_t size_bytes_read = 0;

  //表示长度的数据小于存放长度数据的字节数
  while (size_bytes_read < sizeof(sz)) {
    //长度变量转换为指针
    uint8_t* szp = reinterpret_cast<uint8_t*>(&sz) + size_bytes_read;

    uint32_t bytes_read
        = transport_->read(szp, static_cast<uint32_t>(sizeof(sz)) - size_bytes_read);

    //如果返回为0表示没有数据了
    if (bytes_read == 0) {
      if (size_bytes_read == 0) { //没有任何数据读到，返回false
        // EOF before any data was read.
        return false;
      } else {
        // EOF after a partial frame header.  Raise an exception.
        throw TTransportException(TTransportException::END_OF_FILE,
                                  "No more data to read after "
                                  "partial frame header.");
      }
    }

    //已读取的长度
    size_bytes_read += bytes_read;
  }

  //长整数的网络字节序转换为主机字节序
  sz = ntohl(sz);

  if (sz < 0) { //帧的长度不能是负数涩，抛出异常
    throw TTransportException("Frame size has negative value");
  }

  // Check for oversized frame
  // 读取有效数据负大于最大桢大小（默认256M）
  if (sz > static_cast<int32_t>(maxFrameSize_))
    throw TTransportException(TTransportException::CORRUPTED_DATA, "Received an oversized frame");

  // Read the frame payload, and reset markers.

  // 数据长度大于缓存区长度，就重新分配缓存区
  if (sz > static_cast<int32_t>(rBufSize_)) {
    rBuf_.reset(new uint8_t[sz]);
    rBufSize_ = sz;
  }

  //调用readAll读取sz长度的数据
  transport_->readAll(rBuf_.get(), sz);

  //设置读缓存基地址
  setReadBuffer(rBuf_.get(), sz);
  return true;
}

void TFramedTransport::writeSlow(const uint8_t* buf, uint32_t len) {
  // Double buffer size until sufficient.
  // 直到有足够的双缓冲大小
  uint32_t have = static_cast<uint32_t>(wBase_ - wBuf_.get()); //缓存空间已经有多少数据
  uint32_t new_size = wBufSize_;

  //如果长度溢出或大于2GB了
  if (len + have < have /* overflow */ || len + have > 0x7fffffff) {
    throw TTransportException(TTransportException::BAD_ARGS,
                              "Attempted to write over 2 GB to TFramedTransport.");
  }

  //缓存空间的长度小于已有数据的长度和需要写入数据长度的和
  //这个while循环保证有足够的缓存来存放写入的数据到缓存中，每次增长的长度是上次的一倍
  while (new_size < len + have) {
    //如果缓存空间长度是大于0的话就扩容一倍的空间
    new_size = new_size > 0 ? new_size * 2 : 1;
  }

  // TODO(dreiss): Consider modifying this class to use malloc/free
  // so we can use realloc here.

  // Allocate new buffer.
  // 分配新空间
  uint8_t* new_buf = new uint8_t[new_size];

  // Copy the old buffer to the new one.
  // 拷贝已有的数据到新空间.
  memcpy(new_buf, wBuf_.get(), have);

  // Now point buf to the new one.
  // 缓存地址重新设置
  wBuf_.reset(new_buf);

  // 缓存新长度
  wBufSize_ = new_size;

  //新的开始写入地址
  wBase_ = wBuf_.get() + have;

  //写入界限
  wBound_ = wBuf_.get() + wBufSize_;

  // Copy the data into the new buffer.

  //拷贝数据到新缓存地址
  memcpy(wBase_, buf, len);
  wBase_ += len; //更新缓存基地址
}

void TFramedTransport::flush() {
  int32_t sz_hbo, sz_nbo;

  //断言缓存长度应该大于个字节sizeof(int32_t)
  assert(wBufSize_ > sizeof(sz_nbo));

  // Slip the frame size into the start of the buffer.

  // 获取缓存中的数据长度（不包括4字节的头）
  sz_hbo = static_cast<uint32_t>(wBase_ - (wBuf_.get() + sizeof(sz_nbo)));

  //主机字节序转换为网络字节序
  sz_nbo = (int32_t)htonl((uint32_t)(sz_hbo));

  //头部长度拷贝写缓存
  memcpy(wBuf_.get(), (uint8_t*)&sz_nbo, sizeof(sz_nbo));

  if (sz_hbo > 0) {
    // Note that we reset wBase_ (with a pad for the frame size)
    // prior to the underlying write to ensure we're in a sane state
    // (i.e. internal buffer cleaned) if the underlying write throws
    // up an exception
    wBase_ = wBuf_.get() + sizeof(sz_nbo); //更新wBase_

    // Write size and frame body.
     // 写入长度和帧
    transport_->write(wBuf_.get(), static_cast<uint32_t>(sizeof(sz_nbo)) + sz_hbo);
  }

  // Flush the underlying transport.
  transport_->flush();

  // reclaim write buffer
  if (wBufSize_ > bufReclaimThresh_) {
    wBufSize_ = DEFAULT_BUFFER_SIZE;
    wBuf_.reset(new uint8_t[wBufSize_]);
    setWriteBuffer(wBuf_.get(), wBufSize_);

    // reset wBase_ with a pad for the frame size
    int32_t pad = 0;
    wBase_ = wBuf_.get() + sizeof(pad);
  }
}

uint32_t TFramedTransport::writeEnd() {
  return static_cast<uint32_t>(wBase_ - wBuf_.get());
}

const uint8_t* TFramedTransport::borrowSlow(uint8_t* buf, uint32_t* len) {
  (void)buf;
  (void)len;
  // Don't try to be clever with shifting buffers.
  // If the fast path failed let the protocol use its slow path.
  // Besides, who is going to try to borrow across messages?
  return NULL;
}

uint32_t TFramedTransport::readEnd() {
  // include framing bytes
  uint32_t bytes_read = static_cast<uint32_t>(rBound_ - rBuf_.get() + sizeof(uint32_t));

  if (rBufSize_ > bufReclaimThresh_) {
    rBufSize_ = 0;
    rBuf_.reset();
    setReadBuffer(rBuf_.get(), rBufSize_);
  }

  return bytes_read;
}

void TMemoryBuffer::computeRead(uint32_t len, uint8_t** out_start, uint32_t* out_give) {
  // Correct rBound_ so we can use the fast path in the future.
  rBound_ = wBase_;

  // Decide how much to give.
  uint32_t give = (std::min)(len, available_read());

  *out_start = rBase_;
  *out_give = give;

  // Preincrement rBase_ so the caller doesn't have to.
  rBase_ += give;
}

uint32_t TMemoryBuffer::readSlow(uint8_t* buf, uint32_t len) {
  uint8_t* start;
  uint32_t give;
  computeRead(len, &start, &give);

  // Copy into the provided buffer.
  memcpy(buf, start, give);

  return give;
}

uint32_t TMemoryBuffer::readAppendToString(std::string& str, uint32_t len) {
  // Don't get some stupid assertion failure.
  if (buffer_ == NULL) {
    return 0;
  }

  uint8_t* start;
  uint32_t give;
  computeRead(len, &start, &give);

  // Append to the provided string.
  str.append((char*)start, give);

  return give;
}

void TMemoryBuffer::ensureCanWrite(uint32_t len) {
  // Check available space
  uint32_t avail = available_write();
  if (len <= avail) {
    return;
  }

  if (!owner_) {
    throw TTransportException("Insufficient space in external MemoryBuffer");
  }

  // Grow the buffer as necessary.
  uint32_t new_size = bufferSize_;
  while (len > avail) {
    new_size = new_size > 0 ? new_size * 2 : 1;
    avail = available_write() + (new_size - bufferSize_);
  }

  // Allocate into a new pointer so we don't bork ours if it fails.
  uint8_t* new_buffer = static_cast<uint8_t*>(std::realloc(buffer_, new_size));
  if (new_buffer == NULL) {
    throw std::bad_alloc();
  }

  rBase_ = new_buffer + (rBase_ - buffer_);
  rBound_ = new_buffer + (rBound_ - buffer_);
  wBase_ = new_buffer + (wBase_ - buffer_);
  wBound_ = new_buffer + new_size;
  buffer_ = new_buffer;
  bufferSize_ = new_size;
}

void TMemoryBuffer::writeSlow(const uint8_t* buf, uint32_t len) {
  ensureCanWrite(len);

  // Copy into the buffer and increment wBase_.
  memcpy(wBase_, buf, len);
  wBase_ += len;
}

void TMemoryBuffer::wroteBytes(uint32_t len) {
  uint32_t avail = available_write();
  if (len > avail) {
    throw TTransportException("Client wrote more bytes than size of buffer.");
  }
  wBase_ += len;
}

const uint8_t* TMemoryBuffer::borrowSlow(uint8_t* buf, uint32_t* len) {
  (void)buf;
  rBound_ = wBase_;
  if (available_read() >= *len) {
    *len = available_read();
    return rBase_;
  }
  return NULL;
}
}
}
} // apache::thrift::transport
