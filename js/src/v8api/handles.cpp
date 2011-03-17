#include "v8-internal.h"

namespace v8 {
using namespace internal;

namespace internal {
  template <typename SlotOps>
  class ReferenceContainer {
    static const size_t kBlockSize = 16;
    struct SlotBlock {
      typename SlotOps::Slot elements[kBlockSize];
      SlotBlock *next;
    };
    SlotBlock *mBlock;
    size_t mUsedThisBlock;
    void deallocateBlock() {
      if (!mBlock)
        return;

      SlotBlock *next = mBlock->next;

      while (mUsedThisBlock--)
        SlotOps::onRemoveSlot(&mBlock->elements[mUsedThisBlock]);

      delete mBlock;
      mUsedThisBlock = kBlockSize;
      mBlock = next;
    }
  public:
    ReferenceContainer() :
      mBlock(NULL),
      mUsedThisBlock(0)
    {}
    typename SlotOps::Slot *allocate() {
      if (mUsedThisBlock == kBlockSize || !mBlock) {
        SlotBlock *block = new SlotBlock;
        block->next = mBlock;
        mBlock = block;
        mUsedThisBlock = 0;
      }
      typename SlotOps::Slot *slot = &mBlock->elements[mUsedThisBlock];
      mUsedThisBlock++;
      SlotOps::onNewSlot(slot);
      return slot;
    }
    ~ReferenceContainer() {
      while (mBlock) {
        deallocateBlock();
      }
    }
  };

  struct GCOps {
    typedef internal::GCReference Slot;
    static void onNewSlot(Slot *s) {
      s->root(cx());
    }
    static void onRemoveSlot(Slot *s) {
      s->unroot(cx());
    }
  };

  struct RCOps {
    typedef internal::RCReference Slot;
    static void onNewSlot(Slot *s) {
      s->Globalize();
    }
    static void onRemoveSlot(Slot *s) {
      s->Dispose();
    }
  };

  class GCReferenceContainer : public ReferenceContainer<GCOps> {};
  class RCReferenceContainer : public ReferenceContainer<RCOps> {};

  GCReference* GCReference::Globalize() {
    GCReference *r = new GCReference(*this);
    r->root(cx());
    return r;
  }

  void GCReference::Dispose() {
      unroot(cx());
      delete this;
  }

  GCReference *GCReference::Localize() {
    return HandleScope::CreateHandle(*this);
  }
}

HandleScope *HandleScope::sCurrent = 0;

HandleScope::HandleScope() :
  mGCReferences(new internal::GCReferenceContainer),
  mRCReferences(new internal::RCReferenceContainer),
  mPrevious(sCurrent)
{
  sCurrent = this;
}

HandleScope::~HandleScope() {
  sCurrent = mPrevious;
  delete mRCReferences;
  delete mGCReferences;
}

internal::GCReference *HandleScope::CreateHandle(internal::GCReference r) {
  internal::GCReference *ref = sCurrent->mGCReferences->allocate();
  *ref = r;
  return ref;
}

}
