#ifndef SRC_NGRAM_HPP_
#define SRC_NGRAM_HPP_

#include <vector>

namespace porkfactor
{
    namespace conan
    {
        template<typename T>
        class ngram
        {
        public:
            ngram() :
                data_(2)
            {
            }

            ngram(T const &a, T const &b) :
                data_(2)
            {
                data_[0] = a;
                data_[1] = b;
            }

            template<size_t i>
            T const &get() const
            {
                return data_[i];
            }

            bool operator == (ngram<T> const &cmp) const
            {
                return data_ == cmp.data_;
            }

            bool operator != (ngram<T> const &cmp) const
            {
                return data_ != cmp.data_;
            }

            bool operator < (ngram<T> const &cmp) const
            {
                return data_ < cmp.data_;
            }

        private:
            std::vector<T> data_;
        };
    }
}

#endif
