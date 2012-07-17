/* -*- C++ -*- */

/**
 * @file   libdiehard.cpp
 * @brief  Replaces malloc and friends with DieHard versions.
 * @author Emery Berger <http://www.cs.umass.edu/~emery>
 * @note   Copyright (C) 2005-2011 by Emery Berger, University of Massachusetts Amherst.
 */

// The undef below ensures that any pthread_* calls get strong
// linkage.  Otherwise, our versions here won't replace them.  It is
// IMPERATIVE that this line appear before any files get included.

#undef __GXX_WEAK__ 

#include <stdlib.h>
#include <new>

// The heap multiplier.

enum { Numerator = 8, Denominator = 7 };

// enum { Numerator = 2, Denominator = 1 };
// enum { Numerator = 3, Denominator = 1 };
// enum { Numerator = 4, Denominator = 1 };
// enum { Numerator = 8, Denominator = 1 };
// enum { Numerator = 16, Denominator = 1 };

#include "heaplayers.h"
// #include "ansiwrapper.h"
// #include "mmapwrapper.h"
// #include "lockedheap.h"
// #include "posixlock.h"
// #include "oneheap.h"

#include "combineheap.h"
#include "diehard.h"
#include "randomnumbergenerator.h"
#include "realrandomvalue.h"
#include "largeheap.h"
#include "diehardheap.h"
// #include "reentrantheap.h"
#include "version.h"


/*************************  define the DieHard heap ************************/

class TheLargeHeap : public OneHeap<LargeHeap<MmapWrapper> > {};


#include "../test/brokenheap.h"


// Temporarily testing spin locks.

class PosixSpinLockType {
public:

  PosixSpinLockType()
  {
    pthread_spin_init (&_lock, 0);
  }

  ~PosixSpinLockType() {
    pthread_spin_destroy (&_lock);
  }

  void lock() {
    pthread_spin_lock (&_lock);
  }

  void unlock() {
    pthread_spin_unlock (&_lock);
  }


private:
  pthread_spinlock_t _lock;
};



typedef
 ANSIWrapper<
  LockedHeap<PosixSpinLockType, // PosixLockType,
	     CombineHeap<DieHardHeap<Numerator, Denominator, 4096,
				     (DIEHARD_DIEFAST == 1),
				     (DIEHARD_DIEHARDER == 1)>,
			 TheLargeHeap> > >
TheDieHardHeap;

class TheCustomHeapType : public TheDieHardHeap {};

inline static TheCustomHeapType * getCustomHeap (void) {
  static char buf[sizeof(TheCustomHeapType)];
  static TheCustomHeapType * _theCustomHeap = 
    new (buf) TheCustomHeapType;
  return _theCustomHeap;
}

#if defined(_WIN32)
#pragma warning(disable:4273)
#endif

extern "C" {

  void * xxmalloc (size_t sz) {
    return getCustomHeap()->malloc (sz);
  }

  void xxfree (void * ptr) {
    getCustomHeap()->free (ptr);
  }

  size_t xxmalloc_usable_size (void * ptr) {
    return getCustomHeap()->getSize (ptr);
  }

  void xxmalloc_lock() {
    getCustomHeap()->lock();
  }

  void xxmalloc_unlock() {
    getCustomHeap()->unlock();
  }

}
