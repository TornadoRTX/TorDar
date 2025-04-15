#include <scwx/qt/util/queue_counter.hpp>

#include <atomic>

namespace scwx::qt::util
{

class QueueCounter::Impl
{
public:
   explicit Impl(size_t maxCount) : maxCount_ {maxCount} {}

   const size_t        maxCount_;
   std::atomic<size_t> count_ {0};
};

QueueCounter::QueueCounter(size_t maxCount) :
    p {std::make_unique<Impl>(maxCount)}
{
}

QueueCounter::~QueueCounter() = default;

bool QueueCounter::add()
{
   const size_t count = p->count_.fetch_add(1);
   // Must be >= (not ==) to avoid race conditions
   if (count >= p->maxCount_)
   {
      p->count_.fetch_sub(1);
      return false;
   }
   else
   {
      return true;
   }
}

void QueueCounter::remove()
{
   p->count_.fetch_sub(1);
}

bool QueueCounter::is_lock_free()
{
   return p->count_.is_lock_free();
}

} // namespace scwx::qt::util
