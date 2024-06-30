#include <type_traits>
#include <utility>

template <typename T, typename F> class ScopeGuard {
public:
  template <typename TT, typename FF>
  ScopeGuard(TT &&value, FF &&deleter)
      : m_val(std::forward<TT>(value)), m_f(std::forward<FF>(deleter)) {}
  ScopeGuard(ScopeGuard &&that)
      : m_val(std::move(that.m_val)), m_f(std::move(that.m_f)),
        m_isSet(that.m_isSet) {
    that.m_isSet = false;
  }

  ~ScopeGuard() { destroy(); }

  ScopeGuard(const ScopeGuard &) = delete;
  ScopeGuard &operator=(const ScopeGuard &) = delete;
  ScopeGuard &operator=(ScopeGuard &&) = delete;

  T &get() { return m_val; }

  T release() {
    m_isSet = false;
    return m_val;
  }

  template <typename TT> void reset(TT &&value) {
    destroy();
    m_val = std::forward<TT>(value);
    m_isSet = true;
  }

private:
  void destroy() {
    if (m_isSet)
      try {
        m_f(m_val);
      } catch (...) {
      };
  }

  bool m_isSet{true};
  T m_val;
  F m_f;
};

template <typename T, typename F>
ScopeGuard<std::remove_reference_t<T>, F> makeScopeGuard(T value, F &&deleter) {
  return ScopeGuard<T, F>(std::forward<T>(value), std::forward<F>(deleter));
}
