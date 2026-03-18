#pragma once

#include <semaphore>

struct semaphore_pair_lock
{
  std::binary_semaphore sem_cleanup{0};
  std::binary_semaphore sem_needpost{0};
  void prepare_release_client()
  {
    using namespace std::literals;

    // FIXME if jack is not running we can skip this
    this->sem_needpost.release();
    this->sem_cleanup.try_acquire_for(1s);
  }

  void check_client_released()
  {
    if (!this->sem_needpost.try_acquire())
      this->sem_cleanup.release();
  }
};
