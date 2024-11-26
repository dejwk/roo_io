#pragma once

#include <cstdint>

#include "roo_io/status.h"

// Iterable defines a template contract that allows clients to read their
// content as a stream of bytes, by creating input iterators. They are intended
// for performance-critical applications where it is desirable that byte reads
// are inlined. An example application is to read byte-by-byte from an in-memory
// resource.

// The baseline 'iterable' contract is as follows:
//
// class MyIterable {
//  public:
//   // Iterable must be movable.
//   //
//   // MyIterable(MyIterable&& other);
//
//   // Returns a new input iterator over the iterable's content.
//   //
//   MyInputIterator iterator() const;
// };

// The more advanced 'multipass iterable' contract is as follows:
//
// class MyMultipassIterable {
//  public:
//   // Iterable must be movable.
//   //
//   MyMultipassIterable(MyMultipassIterable&& other);
//
//   // Returns a new multipass input iterator over the iterable's content.
//   //
//   MyMultipassInputIterator iterator() const;
// };